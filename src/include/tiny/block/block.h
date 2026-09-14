#ifndef BLOCK_H
#define BLOCK_H

#include <tiny/types.h>
#include <tiny/list.h>

typedef u64 bdev_t;

struct block_device;

struct block_device_ops {
    int (*read_sector)(struct block_device *, u64, u32, void *);
    int (*write_sector)(struct block_device *, u64, u32, const void *);
    int (*flush)(struct block_device *);
};

struct block_device {
    bdev_t                          bdev;
    const char                     *name;
    u32                             sector_size;
    u64                             sector_count;
    const struct block_device_ops  *ops;
    void                           *private;
    struct list_head                list;
};

int block_register(struct block_device *dev);
struct block_device *block_find(bdev_t id);

int block_read(struct block_device *dev, u64 lba, u32 count, void *buf);
int block_write(struct block_device *dev, u64 lba, u32 count, const void *buf);
int block_flush(struct block_device *dev);

#endif
