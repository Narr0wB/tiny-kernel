
#include <tiny/fs/fat.h>

struct filesystem fat_fs = {0};

static int fat_fill_super(struct superblock *sb)
{
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
}