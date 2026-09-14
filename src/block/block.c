
#include <tiny/errno.h>
#include <tiny/string.h>
#include <tiny/io.h>

#include <tiny/block/block.h>

LIST_HEAD(block_devices);
static bdev_t block_counter = 1;

int block_register(struct block_device *dev)
{
    if (!dev || !dev->ops || !dev->name || !dev->ops->read_sector || !dev->sector_count || !dev->sector_size)
        return -EINVAL;

    struct block_device *tmp = NULL;
    list_foreach_entry(&block_devices, tmp, list) {
        if (strcmp(tmp->name, dev->name) == 0) 
            return -EEXIST; 
    }

    if (block_counter < UINT64_MAX)
        dev->bdev = block_counter++;
    else
        return -EINVAL;

    list_add(&dev->list, &block_devices);
    kprintf(KERN_INFO, "Successfully registered device %s with id %d"EOL, dev->name, dev->bdev);
    return 0;
}


struct block_device *block_find(bdev_t id)
{
    struct block_device *dev = NULL;
    list_foreach_entry(&block_devices, dev, list) {
        if (dev->bdev == id) return dev; 
    }

    return NULL;
}


int block_read(struct block_device *dev, u64 lba, u32 count, void *buf)
{
    if (count == 0)
        return 0;

    if (lba + count > dev->sector_count || !buf)
        return -EINVAL;

    return dev->ops->read_sector(dev, lba, count, buf);
}

int block_write(struct block_device *dev, u64 lba, u32 count, const void *buf)
{
    if (count == 0)
        return 0;

    if (lba + count > dev->sector_count || !dev->ops->write_sector || !buf)
        return -EINVAL;

    return dev->ops->write_sector(dev, lba, count, buf);
}

int block_flush(struct block_device *dev)
{
    if (!dev->ops->flush)    
        return -EINVAL;

    return dev->ops->flush(dev);
}
