/* Run: cc -ffreestanding -ffunction-sections -fdata-sections -Isrc/include -Isrc/arch/include tests/fat_inode_test.c -Wl,--gc-sections -o /tmp/fat_inode_test && /tmp/fat_inode_test */
#include <assert.h>
#include "../src/drivers/fat.c"

static u8 disk[512];
static int read_error, write_error, writes;

int block_read(struct block_device *dev, u64 lba, u32 count, void *buf)
{
    assert(dev->sector_size == sizeof(disk) && lba == 7 && count == 1);
    if (read_error)
        return read_error;
    memcpy(buf, disk, sizeof(disk));
    return 0;
}

int block_write(struct block_device *dev, u64 lba, u32 count, const void *buf)
{
    assert(dev->sector_size == sizeof(disk) && lba == 7 && count == 1);
    ++writes;
    if (write_error)
        return write_error;
    memcpy(disk, buf, sizeof(disk));
    return 0;
}

int main(void)
{
    u8 scratch[512], expected[512];
    struct block_device dev = { .sector_size = 512 };
    struct fat_super super = { .dev = &dev, .scratch = scratch, .type = FAT_TYPE_FAT32 };
    struct superblock sb = { .private = &super };
    struct fat_inode fi = {
        .first_cluster = 0x01234567, .attributes = FAT_ATTR_ARCHIVE,
        .dirent_sector = 7, .dirent_offset = 480,
        .inode = { .sb = &sb, .mode = S_IFREG, .size = 12345 },
    };
    memset(disk, 0xa5, sizeof(disk));
    memcpy(expected, disk, sizeof(disk));
    struct fat_dirent *entry = (struct fat_dirent *)(expected + 480);
    entry->attributes = FAT_ATTR_ARCHIVE;
    entry->first_cluster_low = 0x4567;
    entry->first_cluster_high = 0x0123;
    entry->file_size = 12345;
    assert(fat_write_inode(&fi.inode) == 0);
    assert(memcmp(disk, expected, sizeof(disk)) == 0);

    super.type = FAT_TYPE_FAT16;
    fi.first_cluster = 0x4567;
    fi.inode.mode = S_IFDIR;
    fi.attributes = FAT_ATTR_DIRECTORY;
    entry->attributes = FAT_ATTR_DIRECTORY;
    entry->first_cluster_high = 0;
    entry->file_size = 0;
    assert(fat_write_inode(&fi.inode) == 0);
    assert(memcmp(disk, expected, sizeof(disk)) == 0);

    fi.dirent_offset = 481;
    assert(fat_write_inode(&fi.inode) == -EINVAL);
    fi.dirent_offset = 512;
    assert(fat_write_inode(&fi.inode) == -EINVAL);
    fi.dirent_offset = 480;
    fi.inode.mode = S_IFREG;
    fi.inode.size = (u64)UINT32_MAX + 1;
    assert(fat_write_inode(&fi.inode) == -EINVAL);
    fi.inode.size = 12345;
    read_error = -EIO;
    assert(fat_write_inode(&fi.inode) == -EIO && writes == 2);
    read_error = 0;
    write_error = -EIO;
    assert(fat_write_inode(&fi.inode) == -EIO && writes == 3);
    struct dentry root = { .inode = &fi.inode };
    sb.root = &root;
    assert(fat_write_inode(&fi.inode) == 0 && writes == 3);
    return 0;
}
