# Block-device to FAT integration

## First milestone

Mount a FAT volume at `/disk` and read `/disk/HELLO.TXT` through the existing VFS. Support read-only FAT16 and FAT32, short names, subdirectories, and fragmented files. Keep AHCI completely out of the filesystem driver.

Start with a FAT volume at sector zero of a dedicated test disk. Require BPB bytes-per-sector to equal the block device's logical sector size. Reject unsupported layouts explicitly. Partitions, FAT12, long names, and writes are later milestones.

```text
path_walk("/disk/HELLO.TXT") → FAT directory lookup
vfs_open() → FAT open
vfs_read() → FAT cluster/sector translation → block_read() → SATA/AHCI
```

## What already exists

- `src/drivers/fat.c` is compiled, but `fat_fill_super()` is empty and `init_fat()` does not register its filesystem.
- `src/include/tiny/fs/fat.h` provides BPB/directory layouts, constants, and an embedded `fat_inode`.
- `vfs_mount()` passes the block ID through `superblock.device`; use `block_find(sb->device)` during mount.
- RAMFS already creates `/disk`. Keep RAMFS as the root and mount FAT there.
- `scripts/create-disk.sh` currently creates four sectors containing a text message. That is a block-I/O fixture, not a FAT filesystem.

## 1. Establish the sector-reading boundary

Add a small `fat_super` stored in `sb->private`. It holds the block-device pointer, FAT variant, validated geometry, selected FAT location, data-region location, and root-directory information.

Keep one private helper as the filesystem's only route to storage:

```c
static int fat_read_sector(struct superblock *sb, u64 sector, void *buf);
```

Here `sector` is volume-relative. For this milestone the volume starts at device LBA zero. The helper bounds-checks against the validated volume size and calls `block_read(dev, sector, 1, buf)`. Do not add `BPB_HiddSec` to reads for this whole-device volume.

Use page-backed scratch memory for sectors: the current `kmalloc()` supports at most 2048 bytes, so it cannot supply a 4096-byte sector buffer. Pass scratch storage through a lookup/read operation and release it on every exit. Avoid loading an entire FAT, directory, cluster, or file into memory.

Before using the block wrappers, fix their unchecked device/operation pointers and overflowing `lba + count` bounds checks. Propagate negative I/O errors; do not translate corruption or failed reads into successful empty results.

## 2. Implement mount and validate the volume

`init_fat()` should register `fat_fs`. `fat_fill_super()` resolves the device, allocates private state/scratch memory, reads the boot sector, validates it, computes the layout, and creates the root inode and dentry.

Validate the boot signature, supported sector size, nonzero power-of-two sectors-per-cluster, reserved/FAT sizes, root layout, volume extent, FAT capacity, and FAT32 root cluster/version. Use checked wide arithmetic. Reject inconsistent or unsupported volumes before publishing `sb->root`.

The layout calculations are:

```text
root_sectors  = ceil(root_entry_count * 32 / bytes_per_sector)
fat_start     = reserved_sectors
root_start    = fat_start + fat_count * sectors_per_fat
data_start    = root_start + root_sectors
cluster_count = (total_sectors - data_start) / sectors_per_cluster
cluster_lba(n)= data_start + (n - 2) * sectors_per_cluster
```

Classify the FAT variant by cluster count, not the text label. FAT16 has a fixed root-directory region; FAT32's root follows a cluster chain. Honor FAT32's active-FAT selection when mirroring is disabled. [Microsoft FAT specification, sections 3–4 and 6.6](https://www.scs.stanford.edu/~zyedidia/docs/_other/fat.pdf)

On mount failure, free everything allocated by `fat_fill_super()`: `sget()` currently only frees the superblock itself. Log a useful reason and return a negative error.

## 3. Read allocation chains and directories

Implement a sector-based `fat_next_cluster()` shared by directory lookup and file reads. Fix or replace the existing FAT16/FAT32 helpers: their `cluster - 2` index is wrong for a buffer beginning at the FAT. Entry offsets are `cluster * 2` and `cluster * 4`; mask FAT32 values to 28 bits. The subtraction belongs in data-cluster addressing. [Microsoft FAT specification, section 4](https://www.scs.stanford.edu/~zyedidia/docs/_other/fat.pdf)

Distinguish end-of-chain from invalid/free/reserved/bad-cluster values. Validate every next cluster and bound traversal by the volume's cluster count so corrupt cycles cannot hang the kernel. Treat a chain ending before the advertised file size as corruption.

Implement `fat_lookup(dir, negative, flags)` using a streaming directory scan. Handle fixed FAT16 roots separately from chained directories. Scan 32-byte records, stop at the end marker, skip deleted/long-name/volume-label records, and compare normalized 8.3 names. Instantiate a matching inode and dentry. [Microsoft FAT specification, section 6](https://www.scs.stanford.edu/~zyedidia/docs/_other/fat.pdf)

Keep name comparison ASCII-only initially; resolve files with long-name records through their short aliases. Do not expose label entries as files. Return `-ENOENT` only when no matching entry exists.

Give each instantiated inode a per-mount monotonically assigned inode number; do not use first-cluster alone, because empty files can share zero. Reuse VFS dentries for lookup caching. Record the first cluster, attributes, and directory-entry position in `fat_inode`; widen the stored sector position to `u64`.

## 4. Connect the existing VFS callbacks

Use the existing operations structures:

| Callback | FAT responsibility |
|---|---|
| `fill_super` / `kill_super` | Mount validation, root creation, private-state cleanup |
| `alloc_inode` / `free_inode` | Allocate/free the containing `fat_inode` |
| Directory `lookup` | Find a short-name entry and instantiate its inode |
| File `open` / `release` | Allocate a file, assign `*out`, hold/release its inode reference |
| File `read` | Translate `file->pos` into a cluster, sector, and byte offset |

Read at most `min(requested_bytes, inode->size - file->pos)` after checking EOF. Copy partial sectors through scratch memory and follow the FAT at cluster boundaries. Return the actual byte count; return zero at EOF. If an error follows some copied bytes, return those bytes; otherwise return the negative error. `vfs_read()` already advances `file->pos`, so FAT must not advance it again.

A simple read can walk from the first cluster each time. Add a per-open cluster cursor only when repeated-read performance matters. Expose mutation callbacks that return `-EROFS`; no create, truncate, remove, rename, or write operation may reach `block_write()` in this milestone.

## 5. Fix VFS blockers along this path

These are existing problems, not a reason to introduce a new VFS:

- `vfs_open()` discards the filesystem callback's error. Return it and leave the output NULL on failure. FAT open must set `*out`; the RAMFS implementation currently omits that assignment too.
- `iput()` calls `kfree(inode)`, but the existing `fat_inode.inode` is embedded after other fields. Dispatch final freeing through `sb->ops->free_inode`, with the existing generic fallback. FAT can use `container_of()` to free its allocation. Remove `__packed` from software-only FAT inode state and initialize its full allocation.
- `deactivate_super()` currently reverses the final-reference check and can free roots twice. Define one owner for root/dentry/inode release; `kill_super()` and the VFS must not both release the same objects. Keep superblock private state alive until its inodes are gone. Reject unmount while files are open.
- Initialize anonymous root dentries' name/list/hash state; deletion must tolerate roots that were never linked or hashed and must not release a self-parent. Repair `dlookup()` failure cleanup so allocation failure is safe and dropping an unpublished negative dentry releases its parent once.
- Fix the mounted-flag expression and missing not-found return in `vfs_lookup_mount()`. Check allocation/results in `vfs_mount()`, `iget()`, and path walking before dereferencing them. Ensure lookup of `/disk` itself crosses the mount and `..` crosses back to the mountpoint's parent correctly.

Keep these changes limited to the mount/open/read/close/unmount lifecycle and verify RAMFS still works.

## 6. Boot integration and a real fixture

After block registration, `init_vfs()`, and `init_ramfs()`:

1. Call `init_fat()` and select the intended test block device; do not blindly assume the first disk contains FAT.
2. Resolve `/disk` using `path_walk()`.
3. Call `vfs_mount(&fat_fs, device_id, &mountpoint)` and check success. Declare `fat_fs` in its header if using this existing object directly.
4. Resolve `/disk/HELLO.TXT`, open it, read its bytes, and close it. Print by returned length, not as an assumed NUL-terminated string.

Plan a new disposable fixture alongside the current raw-sector fixture: format a sufficiently sized image explicitly as FAT16, place short-name files and a subdirectory into it, then make a second FAT32 fixture. Use the host formatter and mtools rather than constructing filesystem metadata by hand. Do not overwrite the boot image. Image generation and live boot validation are future implementation checks, not actions in this planning task.

## Implementation order and acceptance checks

1. **Mount:** sector helper, validated BPB/layout, root inode, and the required VFS lifecycle fixes. Malformed or unsupported volumes fail without leaks/crashes.
2. **Lookup:** find root files and nested short-name files through `/disk`; missing paths fail cleanly.
3. **Read:** open/read/close empty, single-sector, multi-sector, and fragmented files; confirm partial reads, EOF, repeated reads, and injected I/O errors.
4. **Regression:** one runnable host test suite with a fake block backend covers invalid BPBs, FAT indices, out-of-range/cyclic chains, and mount/open failure cleanup. Include a malformed FAT32 active-FAT index and sector-size mismatch. Verify FAT never invokes write callbacks.
5. **Build and later boot check:** kernel builds; when runtime testing is authorized, read known files on both disposable images and verify file contents, RAMFS behavior, and safe unmount/remount.

Keep implementation primarily in the existing FAT source/header, with targeted fixes in VFS/namei/block code, a boot call site, and tests. FAT is already included in the driver build.

After this works, add partition block-device views so FAT still sees a volume starting at sector zero. Add write support separately, with allocation, metadata update ordering, FAT-copy handling, and explicit flush/error semantics. Long names, FAT12, directory enumeration, and caching can follow actual consumers.
