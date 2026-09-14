# Block–AHCI integration plan

## Target and current state

Connect the existing `block_device_ops` to the existing AHCI driver. Use synchronous, polled I/O, one command slot, and one reusable transfer buffer per SATA disk. No new public abstraction is needed.

This updates the implementation assumptions in `block-devices-plan.md`: `block.c` and IDENTIFY now exist. `ahci_fetch_device_geometry()` fills sector size and capacity, but no sector-transfer callbacks or block registrations exist yet.

The intended path is:

```text
block_find(id)
    → block_read / block_write / block_flush
    → shared SATA block_device_ops
    → sata_device from block_device.private
    → AHCI command submission and completion
```

## 1. Fix the small integration prerequisites

In `src/block/block.c`:

- Start IDs at 1; VFS already reserves zero for device-less filesystems. Reject exhaustion instead of wrapping.
- Return NULL when `block_find()` finds nothing.
- Validate device, operations, name, and nonzero sector geometry before registration. Reject duplicate objects and names.
- Validate device/operations before dereferencing in I/O wrappers. For nonzero requests check the buffer and use `lba < sector_count` and `count <= sector_count - lba`; also guard `count * sector_size` against `SIZE_MAX` overflow.
- Keep zero-count I/O a no-op for a valid device. Return `-EROFS` for missing write support; missing flush support must return an error rather than imply durability.

`src/block/meson.build` already exists. Add `subdir('block')` and the `block` library to `src/meson.build`.

## 2. Give each SATA disk its block identity and working memory

Extend `struct sata_device` in the existing AHCI header with:

```c
struct block_device block;
char name[24];
struct page *command_page;
struct page *bounce_page;
bool lba48;
bool usable;
```

Include or forward-declare the required types. Remove `__packed` from this software bookkeeping structure; retain the hardware layouts.

Reuse the command-table and data pages currently allocated temporarily by `ahci_fetch_device_geometry()`. Keep them for subsequent commands instead of allocating/freeing for every request. Retain ownership of the existing command-list and FIS pages too, so failed initialization can clean up safely.

Use one page for the bounce buffer initially. Support identified sector sizes that fit it; explicitly skip unsupported geometry rather than silently treating it as 512 bytes. Keep the LBA28/LBA48 capability from IDENTIFY, not just the capacity derived from it.

For controllers without 64-bit DMA support, require all DMA allocations below 4 GiB. The current allocator cannot request a low-memory zone: validate the addresses and fail initialization cleanly if necessary. Do not truncate them.

## 3. Reuse IDENTIFY's command path

Extract one driver-private synchronous submission helper from `ahci_fetch_device_geometry()`. IDENTIFY, reads, writes, and flushes should share command setup, issuance, completion checks, and error handling.

Keep command slot 0 exclusively owned by this driver and leave command interrupts disabled during the polled implementation. Verify it is idle before reuse, clear the header/table and stale status, and publish descriptor/buffer writes before issuing the command. Program MMIO address registers through their 32-bit halves; the current `clb64`/`fb64` unions should not imply a portable 64-bit MMIO store. [AHCI specification, chapters 3–5](https://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/serial-ata-ahci-spec-rev1-3-1.pdf)

Correct the existing error paths as part of this extraction:

- Propagate `ahci_wait_port()` failures from `ahci_port_start()` and preserve specific errors through initialization.
- Bound engine-stop and ownership waits as well as command completion. Use elapsed-time deadlines when a timer is available; the current iteration constant is not a duration.
- After a command timeout/error, do not free or reuse its command/data pages until DMA is known to have stopped. Stop the engine with a bounded wait. If quiescence cannot be established, mark the disk unusable and retain its DMA memory. Future requests fail with `-EIO`; automatic recovery can wait.
- Check errors at completion even when the command-issue bit has already cleared.

## 4. Add three thin block callbacks

Keep these functions and the shared operations table private to `ahci.c`:

```c
static int sata_block_read(struct block_device *, u64, u32, void *);
static int sata_block_write(struct block_device *, u64, u32, const void *);
static int sata_block_flush(struct block_device *);

static const struct block_device_ops sata_block_ops = {
    .read_sector = sata_block_read,
    .write_sector = sata_block_write,
    .flush = sata_block_flush,
};
```

Each callback gets `struct sata_device *sata = dev->private` and rejects an unusable disk. Read/write callbacks split requests into bounce-buffer-sized chunks. For writes, copy into the bounce buffer before submission; for reads, copy out only after successful completion. Advance LBA by sectors and the caller's buffer by bytes.

Encode READ/WRITE DMA EXT for LBA48 disks and READ/WRITE DMA for supported LBA28 disks, respecting each command's addressing/count limits. Set the command-header direction and PRDT byte count for each chunk. Keep all of this out of the generic block layer. SeaBIOS demonstrates the same separation between command preparation, submission, and block read/write. [SeaBIOS AHCI implementation](https://raw.githubusercontent.com/coreboot/seabios/master/src/hw/ahci.c)

Flush uses the device-supported ATA cache-flush command through the same helper, without a data PRDT. Return success only after completion. If durable writes cannot be supported, initially publish the disk as read-only. Writes are not automatically flushed after each chunk; callers explicitly flush when they need persistence. [AHCI command structures](https://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/serial-ata-ahci-spec-rev1-3-1.pdf)

All callbacks return zero for complete success and negative kernel errors otherwise. A later chunk's failure does not undo earlier transfers. The initial caller contract is serialized I/O per disk, including flush; add a per-disk lock when concurrent callers appear.

## 5. Register only ready disks

After successful port setup, IDENTIFY, and capability validation, but before publishing the disk to consumers:

```c
dev->block = (struct block_device) {
    .name = dev->name,
    .sector_size = dev->sector_size,
    .sector_count = dev->total_sectors,
    .ops = &sata_block_ops,
    .private = dev,
};
err = block_register(&dev->block);
```

Generate a unique, persistent name such as `sata0` in `dev->name` first. Let the block registry assign the ID. Check every allocation and `vmap()` result before dereferencing it. On failure, safely quiesce and release unpublished resources, then continue with other ports. Once registered, the object and name live for the kernel's lifetime.

Keep `sata_list` as driver bookkeeping and the embedded block list node as registry membership. They must be separate nodes.

Register every usable port on each discovered AHCI controller. The current PCI search stops at the first controller; replace that first-match selection with iteration. Give controllers distinct MMIO mappings and retain the relevant controller pointer/capabilities per disk instead of relying on one global `controller`.

## 6. Verify in small milestones

1. **Read path:** finish the common submit helper, read callback, and registration. Publish read-only devices until write/flush are ready.
2. **Write path:** add write and flush callbacks with explicit completion/error semantics.
3. **Host checks:** use a fake backend to test block lookup/validation, callback routing to two different disks, chunk boundaries, 512/4096-byte sectors, command encoding, error propagation, and no buffer reuse following uncertain DMA completion. Keep hardware-dependent checks separate from simulated assertions.
4. **Build:** verify the block library links into the kernel. No filesystem or VFS API changes are required.
5. **Later runtime check, when authorized:** use disposable SATA images with known contents. Confirm both disks register; read through the block API; then write, flush, and read back a scratch range crossing a chunk boundary. Never use the boot image for write tests.

Completion means generic callers can use every registered supported SATA disk by block ID, without seeing AHCI structures. Defer partitions, `/dev`, caching, queues, NCQ, hotplug, and a general driver framework.
