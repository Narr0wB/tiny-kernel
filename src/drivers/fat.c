
#include <tiny/drivers/fat.h>

struct filesystem fat_fs = {0};

void init_fat_fs()
{
    fat_fs.name = "fat32";
}