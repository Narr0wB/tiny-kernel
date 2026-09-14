
#include <tiny/mm/kmalloc.h>
#include <tiny/mm/palloc.h>

#include <tiny/drivers/fat.h>

struct filesystem fat_fs = {0};

static int fat_read_sector(struct block_device *dev, u64 lba, void *buf)
{

}


static int fat_fill_super(struct superblock *sb)
{
    struct fat_super *fat = (struct fat_super *)kmalloc(sizeof(struct fat_super), PAL_KERNEL);
    struct block_device *dev = block_find(sb->device);

    fat->dev = dev;

    void *buf = vpalloc(0, PALLOC_ZERO);



    vpfree(buf);
    sb->private = fat;
    return 0;
}


static void fat_kill_super(struct superblock *sb)
{
    
}

void init_fat()
{
    fat_fs.name  = "fat";
    fat_fs.fill_super = fat_fill_super;
    fat_fs.kill_super = fat_kill_super;

    register_filesystem(&fat_fs);
}