
#include <tiny/fs/fat.h>

struct filesystem fat_fs = {0};

void init_fat()
{
    fat_fs.name = "fat";
}