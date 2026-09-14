# Simple block-device interface

## Goal

Expose each usable SATA disk as a whole-disk block device, with an ID, logical-sector geometry, and synchronous read/write operations. Keep AHCI commands and DMA details inside the SATA driver.

First milestone: find a registered disk by ID and read known sectors through the generic interface. Then add writes and explicit flushing.

## Current starting point

- `src/include/tiny/block/block.h` already defines `bdev_t`, `block_device`, and read/write callbacks. Extend this header rather than replace it.
- `src/platform/ahci.c` maintains `sata_list` and allocates command-list and received-FIS buffers. It does not yet issue IDENTIFY or sector-transfer commands: `total_sectors` remains zero and `sector_size` defaults to 512.
- `init_ahci()` is already called during boot.
- The VFS already stores a `bdev_t` in `superblock.device`; device ID zero is used for device-less filesystems. Preserve that convention.

## 1. Keep the existing device structure

Retain its ID, name, sector size/count, operations pointer, private pointer, and intrusive list node. Make the operations pointer `const` so all SATA disks can share one operations table.

Add only these public functions:

```c
int block_register(struct block_device *dev);
struct block_device *block_find(bdev_t id);

int block_read(struct block_device *dev, u64 lba, u32 count, void *buf);
int block_write(struct block_device *dev, u64 lba, u32 count, const void *buf);
int block_flush(struct block_device *dev);
```

Keep the existing `read_sector` and `write_sector` callbacks; add `int (*flush)(struct block_device *)` when enabling writes. No open/close, request objects, or separate transport interface.

Define the contract before implementing it:

- LBA and count are in the device's **logical sectors**, not bytes or fixed 512-byte units.
- I/O is synchronous: return zero only when the whole request completes; otherwise return a negative kernel error. Errors may leave a partially modified read buffer or partially written disk range; there is no atomic-write guarantee.
- The caller supplies a kernel buffer of at least `count * sector_size` bytes. It need not be physically contiguous or DMA-aligned.
- For a valid device, count zero is a successful no-op. Otherwise reject null buffers, invalid geometry, byte-count overflow, and out-of-range requests before invoking the driver.
- Check bounds using `lba < sector_count` and `count <= sector_count - lba`, avoiding an overflowing `lba + count`.
- A missing write callback means read-only. Successful writes mean command completion; durability requires successful `block_flush()`. Do not silently report flush success without a driver guarantee.
- Add only the missing error codes actually used, such as `EIO`, `ETIMEDOUT`, and `EROFS`; reuse existing `EINVAL`, `ENOMEM`, `ENOENT`, and `EEXIST`.

## 2. Implement one registry

Use a private `LIST_HEAD(block_devices)` in `src/block/block.c` and a monotonically increasing ID starting at 1. `block_find()` performs a linear lookup and returns NULL when absent.

Registration validates the geometry and read callback, rejects registering the same object twice, assigns its ID, and links it once. It does not allocate or copy the device. Reject ID exhaustion rather than wrapping to zero.

The driver owns the device and its name for the kernel's lifetime. Registration happens during boot; no unregister, hotplug, reference counting, or registry locks in this version. IDs are boot-local, not persistent disk identifiers.

## 3. Complete the SATA backend

Before publishing a disk, successfully IDENTIFY it and fill in its logical-sector size and total sector count. Never register the current zero-capacity placeholder or assume every disk uses 512-byte logical sectors.

Implement driver-local synchronous read, write, and flush functions. Initially use one command slot and a reusable, physically contiguous bounce buffer per disk. Copy between that buffer and the caller's buffer, splitting large requests to the command and bounce-buffer limits. This keeps DMA layout out of the block API.

Use DMA memory addressable by the controller. Check allocation and command errors, and give every hardware wait a timeout. On timeout, stop or otherwise establish that DMA has ended before freeing or reusing its buffers; if recovery fails, mark that disk unusable and retain any buffers the controller could still access.

Keep requests serialized. The initial contract is one caller at a time per disk; add a per-disk lock when concurrent I/O is introduced. No interrupt-handler calls into this blocking interface.

This backend is a prerequisite, not functionality supplied by the registry. Read-only registration is sufficient for the first milestone; leave `write_sector` unset until writes and flushing work.

## 4. Register disks where discovery succeeds

Embed a `struct block_device` and persistent name storage in `struct sata_device`. Keep this software bookkeeping structure naturally aligned; `__packed` is appropriate for hardware layouts, not this structure.

For each successfully initialized and identified SATA disk:

1. Fill its block-device geometry and name, for example `sata0`.
2. Set its shared SATA operations table and `private = sata_device`.
3. Call `block_register()` and log the assigned ID and geometry.
4. On failure, clean up the unpublished device safely and continue discovering other ports.

The adapter callbacks recover `sata_device` from `dev->private` and invoke the driver-local operations. Keep `sata_list` for driver bookkeeping; use the block registry for generic consumers.

Register every usable disk found, not just the first port. The current controller search stops at the first AHCI controller: iterate all matching PCI devices if the goal is all attached SATA disks. Keep each controller's mapping and capabilities associated with its disks, and give each ABAR a distinct MMIO mapping. Skip failed ports without aborting the remaining scan. SATAPI and port multipliers stay unsupported for now.

## 5. Wire the build and verify

Add `src/block/block.c` and its small `meson.build`, then add the subdirectory and library to `src/meson.build`. Other implementation changes belong in the existing block header, AHCI source/header, and error header as needed.

Leave VFS and FAT integration for the next milestone. A future disk-backed filesystem can resolve `superblock.device` through `block_find()` and call `block_read()` directly.

Add one runnable host-side test with a memory-backed fake device. Cover registration/lookup, duplicate registration, 512- and 4096-byte sectors, last-sector access, invalid ranges and overflow, zero-length requests, callback error propagation, read-only behavior, and flush dispatch.

Build the kernel. When runtime testing is authorized, use a disposable disk image with known contents: verify registration and reads first, then write/flush/read back a designated scratch range, including a transfer larger than the bounce buffer. Never use the kernel's boot image for the write test.

## Done when

- Every usable discovered SATA disk has one nonzero block ID and identified geometry.
- Generic callers can read and, once enabled, write/flush without accessing AHCI structures.
- Invalid requests fail before hardware access, and hardware failures return errors without unsafe DMA-buffer reuse.
- Registry and wrapper tests pass, and the kernel builds.

Defer partitions, `/dev` nodes, caching, I/O scheduling, asynchronous requests, NCQ, hotplug, and a general device framework until an actual caller needs them.
