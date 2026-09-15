
#include <tiny/kernel.h>
#include <tiny/string.h>
#include <tiny/mm/kmalloc.h>
#include <tiny/mm/palloc.h>
#include <tiny/compiler.h>

#include <tiny/drivers/fat.h>

struct filesystem fat_fs = {0};

static u32 fat_next_cluster(struct block_device *dev, struct fat_super *super, u32 cluster)
{
    u32 next = 0;

    switch (super->type) {
        case FAT_TYPE_FAT12:
        case FAT_TYPE_FAT16:
            break;

        case FAT_TYPE_FAT32: {
            u32 entries_per_sector = dev->sector_size / sizeof(u32);
            u32 sector = super->fat_start + (cluster / entries_per_sector);

            if (block_read(dev, sector, 1, super->scratch))
                return 0;

            u32 entry = ((u32*)super->scratch)[cluster % entries_per_sector];
            next = entry & FAT32_ENTRY_MASK;
        }
    }

    return next;
}


static __force_inline u64 fat_cluster_lba(struct fat_super *super, u32 cluster)
{
    return super->data_start + (cluster - 2) * super->bpb.sectors_per_cluster;
}


static int fat_read_cluster(struct block_device *dev, struct fat_super *super, u32 cluster, void *buf)
{
    return block_read(dev, fat_cluster_lba(super, cluster), super->bpb.sectors_per_cluster, buf);
}

static struct fat_dirent *fat_get_dirent(struct fat_inode *dir)
{
    struct fat_super *super = (struct fat_super *)dir->inode.sb->private;

    if (block_read(super->dev, dir->dirent_sector, 1, super->scratch)) 
        return NULL;

    struct fat_dirent *entry = (struct fat_dirent *)((u8 *)super->scratch + dir->dirent_offset);
    return entry;
}


static int fat_lookup(struct inode *dir, struct dentry *negative, u32 flags)
{
    // (void)dir;
    // (void)negative;
    // (void)flags;

    struct fat_inode *fi = container_of(dir, struct fat_inode, inode);
    struct fat_dirent *entry = fat_get_dirent(fi);
    /* TODO: Look up the directory entry and instantiate its inode. */
    return -EIO;
}

static int fat_create(struct inode *dir, struct dentry *negative, mode_t mode)
{
    (void)dir;
    (void)negative;
    (void)mode;
    /* TODO: Create a regular file. */
    return -EROFS;
}

static int fat_mkdir(struct inode *dir, struct dentry *negative, mode_t mode)
{
    (void)dir;
    (void)negative;
    (void)mode;
    /* TODO: Create a directory. */
    return -EROFS;
}

static int fat_rmdir(struct inode *dir, struct dentry *entry)
{
    (void)dir;
    (void)entry;
    /* TODO: Remove an empty directory. */
    return -EROFS;
}

static int fat_rename(struct inode *old_dir, struct dentry *old_entry,
                      struct inode *new_dir, struct dentry *new_entry, u32 flags)
{
    (void)old_dir;
    (void)old_entry;
    (void)new_dir;
    (void)new_entry;
    (void)flags;
    /* TODO: Move or rename a directory entry. */
    return -EROFS;
}

static int fat_truncate(struct inode *inode)
{
    (void)inode;
    /* TODO: Resize the file's cluster chain to match inode->size. */
    return -EROFS;
}

static struct inode_ops fat_iops = {
    .lookup   = fat_lookup,
    .create   = fat_create,
    .mkdir    = fat_mkdir,
    .rmdir    = fat_rmdir,
    .rename   = fat_rename,
    .truncate = fat_truncate,
};

static struct file_ops fat_fops = {

};


static struct inode *fat_alloc_inode(struct superblock *sb)
{
    struct fat_inode *fi = (struct fat_inode *)kmalloc(sizeof(struct fat_inode), PAL_KERNEL);
    if (!fi)
        return NULL;

    memset(fi, 0, sizeof(*fi));
    return &fi->inode;
}

static void fat_free_inode(struct inode *inode)
{
    if (!inode)
        return;

    struct fat_inode *fi = container_of(inode, struct fat_inode, inode);
    kfree(fi);
}

static int fat_write_inode(struct inode *inode)
{
    struct fat_inode *fi = container_of(inode, struct fat_inode, inode);
    struct fat_super *super = (struct fat_super *)inode->sb->private;

    /* The root directory has no directory entry of its own. */
    if (inode->sb->root && inode->sb->root->inode == inode)
        return 0;

    if (fi->dirent_offset % sizeof(struct fat_dirent) ||
        (size_t)fi->dirent_offset + sizeof(struct fat_dirent) > super->dev->sector_size ||
        (!S_ISDIR(inode->mode) && inode->size > UINT32_MAX))
        return -EINVAL;

    int err = block_read(super->dev, fi->dirent_sector, 1, super->scratch);
    if (err) return err;

    struct fat_dirent *entry = (struct fat_dirent *)((u8 *)super->scratch + fi->dirent_offset);
    entry->attributes = fi->attributes;
    entry->first_cluster_low = (u16)fi->first_cluster;
    entry->first_cluster_high = super->type == FAT_TYPE_FAT32
        ? (u16)((fi->first_cluster & FAT32_ENTRY_MASK) >> 16) : 0;
    entry->file_size = S_ISDIR(inode->mode) ? 0 : (u32)inode->size;
    /* TODO: Encode inode timestamps as FAT dates/times */

    err = block_write(super->dev, fi->dirent_sector, 1, super->scratch);
    if (err) return err;
    return 0;
}

static void fat_evict_inode(struct inode *inode)
{
    (void)inode;
}

static int fat_statfs(struct superblock *sb, struct statfs *stat)
{
    (void)sb;
    (void)stat;
    return 0;
}

static struct superblock_ops fat_sbops = {
    .alloc_inode = fat_alloc_inode,
    .evict_inode = fat_evict_inode,
    .free_inode  = fat_free_inode,
    .write_inode = fat_write_inode,
    .statfs      = fat_statfs
};

static int fat_parse_bpb(struct fat_bpb *bpb, struct fat_super *super)
{
    if (!bpb->bytes_per_sector || !bpb->sectors_per_cluster) {
        super->type = FAT_TYPE_UNKNOWN;
        return -1;
    }

    super->total_sectors = bpb->total_sectors_16 ? bpb->total_sectors_16 : bpb->total_sectors_32;
    super->fat_start = bpb->reserved_sector_count;

    u32 fat_size = bpb->fat_size_16;
    if (fat_size == 0) {
        struct fat32_ebpb *ebpb = (struct fat32_ebpb *)bpb->extended_data;
        fat_size = ebpb->fat_size_32;
    }

    u32 root_dir_sectors = ALIGN_UP(bpb->root_entry_count * FAT_DIRECTORY_ENTRY_SIZE, bpb->bytes_per_sector) / bpb->bytes_per_sector;

    super->data_start = super->fat_start + (bpb->fat_count * fat_size) + root_dir_sectors;

    if (super->total_sectors <= super->data_start) {
        super->type = FAT_TYPE_UNKNOWN;
        return -1;
    }

    super->cluster_count = (super->total_sectors - super->data_start) / bpb->sectors_per_cluster;

    if (super->cluster_count <= FAT12_CLUSTER_COUNT_MAX) {
        super->type = FAT_TYPE_FAT12;
        super->root_start = super->fat_start + (bpb->fat_count * fat_size);
    } else if (super->cluster_count <= FAT16_CLUSTER_COUNT_MAX) {
        super->type = FAT_TYPE_FAT16;
        super->root_start = super->fat_start + (bpb->fat_count * fat_size);
    } else {
        super->type = FAT_TYPE_FAT32;
        struct fat32_ebpb *ebpb = (struct fat32_ebpb *)bpb->extended_data;
        if (ebpb->root_cluster >= FAT_FIRST_DATA_CLUSTER)
            super->root_start = super->data_start + (ebpb->root_cluster - FAT_FIRST_DATA_CLUSTER) * bpb->sectors_per_cluster;
        else
            super->root_start = super->data_start;
    }

    return 0;
}


static int fat_fill_super(struct superblock *sb)
{
    struct block_device *dev = block_find(sb->device);
    if (!dev)
        return -ENOENT;
    if (dev->sector_size < FAT_BYTES_PER_SECTOR_512 ||
        dev->sector_size > FAT_BYTES_PER_SECTOR_4096)
        return -EINVAL;

    struct fat_super *fat = kmalloc(sizeof(*fat), PAL_KERNEL);
    if (!fat)
        return -ENOMEM;

    void *buf = vpalloc(1, PAL_KERNEL);
    if (!buf) {
        kfree(fat);
        return -ENOMEM;
    }

    int err = block_read(dev, 0, 1, buf);
    if (err)
        goto fail;

    memcpy(&fat->bpb, buf, sizeof(fat->bpb));
    err = -EINVAL;
    if (fat->bpb.bytes_per_sector != dev->sector_size || fat_parse_bpb(&fat->bpb, fat))
        goto fail;

    u32 first_cluster = 0;
    size_t root_size = 0;

    switch (fat->type) {
        case FAT_TYPE_FAT12:
        case FAT_TYPE_FAT16:
            root_size = fat->bpb.root_entry_count * FAT_DIRECTORY_ENTRY_SIZE;
            break;

        case FAT_TYPE_FAT32: {
            struct fat32_ebpb *ebpb = (struct fat32_ebpb *)fat->bpb.extended_data;
            first_cluster = ebpb->root_cluster;
            if (first_cluster < FAT_FIRST_DATA_CLUSTER ||
                first_cluster >= FAT32_RESERVED_MIN ||
                (u64)first_cluster >= (u64)fat->cluster_count + FAT_FIRST_DATA_CLUSTER)
                goto fail;
            break;
        }

        default:
            goto fail;
    }

    fat->dev = dev;
    fat->scratch = buf;
    sb->private = fat;
    sb->ops = &fat_sbops;

    struct inode *inode = new_inode(sb);
    if (!inode) {
        err = -ENOMEM;
        goto fail;
    }

    struct dentry *root = dalloc(NULL, NULL);
    if (!root) {
        list_del(&inode->sb_list);
        fat_free_inode(inode);
        err = -ENOMEM;
        goto fail;
    }

    struct fat_inode *fi = container_of(inode, struct fat_inode, inode);
    fi->first_cluster = first_cluster;
    fi->attributes = FAT_ATTR_DIRECTORY;

    inode->ino = 1;
    inode->mode = S_IFDIR;
    inode->size = root_size;
    inode->links = 2;
    inode->ops = &fat_iops;
    atomic_set(&inode->ref, 1);

    root->inode = inode;
    sb->root = root;
    return 0;

fail:
    vpfree(buf);
    kfree(fat);
    sb->private = NULL;
    sb->ops = NULL;
    return err;
}


static void fat_kill_super(struct superblock *sb)
{
    kfree(sb->private);
    vpfree(((struct fat_super *)sb->private)->scratch);
}

void init_fat()
{
    fat_fs.name  = "fat";
    fat_fs.fill_super = fat_fill_super;
    fat_fs.kill_super = fat_kill_super;

    register_filesystem(&fat_fs);
}
