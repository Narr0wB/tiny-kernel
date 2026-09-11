#ifndef BLOCK_H
#define BLOCK_H

#include <tiny/types.h>
#include <tiny/list.h>

typedef u64 bdev_t;

struct block_device;

struct block_device_ops {
    int (*read_sector)(struct block_device *dev, u64 sector, u32 count, void *buf);
    int (*write_sector)(struct block_device *dev, u64 sector, u32 count, const void *buf);
};

struct block_device {
    bdev_t                    bdev;
    const char               *name;
    u32                       sector_size;
    u64                       sector_count;
    struct block_device_ops  *ops;
    void                     *private;
    struct list_head          list;
};

#endif // BLOCK_H
