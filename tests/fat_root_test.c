/* Run: cc -ffreestanding -ffunction-sections -fdata-sections -Isrc/include -Isrc/arch/include tests/fat_root_test.c -Wl,--gc-sections -o /tmp/fat_root_test && /tmp/fat_root_test */
#include <assert.h>
#include "../src/drivers/fat.c"
#include "../src/fs/vfs.c"

extern void *malloc(size_t);
extern void free(void *);
static int allocations, live, fail_at, read_error;
static struct block_device device = { .sector_size = 512 };
static struct fat_bpb bpb;

void *kmalloc(size_t size, unsigned int flags)
{
    (void)flags;
    if (++allocations == fail_at)
        return NULL;
    void *p = malloc(size);
    assert(p);
    memset(p, 0xa5, size);
    ++live;
    return p;
}

void kfree(void *p)
{
    if (p) {
        --live;
        free(p);
    }
}

void *vpalloc(int order, unsigned int flags)
{
    return kmalloc((size_t)4096 << order, flags);
}

void vpfree(void *p) { kfree(p); }

struct block_device *block_find(bdev_t id)
{
    return id == 1 ? &device : NULL;
}

int block_read(struct block_device *dev, u64 lba, u32 count, void *buf)
{
    assert(dev == &device && lba == 0 && count == 1);
    memcpy(buf, &bpb, sizeof(bpb));
    return read_error;
}

int block_write(struct block_device *dev, u64 lba, u32 count, const void *buf)
{
    (void)dev; (void)lba; (void)count; (void)buf;
    assert(0);
    return -EIO;
}

static void check_mount(int type, int expected_error)
{
    struct superblock sb = { .device = 1 };
    list_head_init(&sb.inodes);
    allocations = 0;
    assert(fat_fill_super(&sb) == expected_error);
    if (expected_error) {
        assert(!sb.root && !sb.private && !sb.ops);
        assert(list_empty(&sb.inodes) && live == 0);
        return;
    }

    struct fat_super *fat = sb.private;
    struct inode *inode = sb.root->inode;
    struct fat_inode *fi = container_of(inode, struct fat_inode, inode);
    assert(fat->type == type && inode->sb == &sb);
    assert(sb.root->parent == sb.root && !sb.root->name.str);
    assert(!sb.root->hnode.pprev && list_empty(&sb.root->child));
    assert(list_empty(&sb.root->subdirs));
    assert(inode->ino == 1 && inode->mode == S_IFDIR && inode->links == 2);
    assert(inode->ops == &fat_iops && fi->attributes == FAT_ATTR_DIRECTORY);
    assert(fi->first_cluster == (type == FAT_TYPE_FAT32 ? 5 : 0));
    assert(inode->size == (type == FAT_TYPE_FAT32 ? 0 : 512));
    assert(fat->root_start == (type == FAT_TYPE_FAT32 ? 1204 : 3));
    assert(fat_write_inode(inode) == 0);

    list_del(&inode->sb_list);
    fat_free_inode(inode);
    kfree(sb.root);
    vpfree(fat->scratch);
    kfree(fat);
    assert(live == 0);
}

int main(void)
{
    bpb.bytes_per_sector = 512;
    bpb.sectors_per_cluster = 1;
    bpb.reserved_sector_count = 1;
    bpb.fat_count = 1;
    bpb.fat_size_16 = 2;
    bpb.root_entry_count = 16;
    bpb.total_sectors_16 = 100;
    check_mount(FAT_TYPE_FAT12, 0);
    bpb.total_sectors_16 = 5000;
    check_mount(FAT_TYPE_FAT16, 0);
    bpb.total_sectors_16 = 0;
    bpb.total_sectors_32 = 100000;
    bpb.root_entry_count = 0;
    bpb.fat_size_16 = 0;
    struct fat32_ebpb *ebpb = (struct fat32_ebpb *)bpb.extended_data;
    ebpb->fat_size_32 = 1200;
    ebpb->root_cluster = 5;
    check_mount(FAT_TYPE_FAT32, 0);
    for (fail_at = 1; fail_at <= 4; ++fail_at)
        check_mount(FAT_TYPE_FAT32, -ENOMEM);
    fail_at = 0;
    read_error = -EIO;
    check_mount(FAT_TYPE_FAT32, -EIO);
    read_error = 0;
    ebpb->root_cluster = 1;
    check_mount(FAT_TYPE_FAT32, -EINVAL);
    ebpb->root_cluster = 100000;
    check_mount(FAT_TYPE_FAT32, -EINVAL);
    return 0;
}
