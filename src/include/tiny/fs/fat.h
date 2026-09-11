
#ifndef FAT_H
#define FAT_H

#include <tiny/fs/vfs.h>
#include <tiny/types.h>
#include <tiny/compiler.h>

/* On-disk FAT layout */
#define FAT_BPB_JUMP_OFFSET                     0U
#define FAT_BPB_OEM_NAME_OFFSET                 3U
#define FAT_BPB_BYTES_PER_SECTOR_OFFSET         11U
#define FAT_BPB_SECTORS_PER_CLUSTER_OFFSET      13U
#define FAT_BPB_RESERVED_SECTORS_OFFSET         14U
#define FAT_BPB_NUMBER_OF_FATS_OFFSET           16U
#define FAT_BPB_ROOT_ENTRY_COUNT_OFFSET         17U
#define FAT_BPB_TOTAL_SECTORS_16_OFFSET         19U
#define FAT_BPB_MEDIA_OFFSET                    21U
#define FAT_BPB_FAT_SIZE_16_OFFSET              22U
#define FAT_BPB_SECTORS_PER_TRACK_OFFSET        24U
#define FAT_BPB_NUMBER_OF_HEADS_OFFSET          26U
#define FAT_BPB_HIDDEN_SECTORS_OFFSET           28U
#define FAT_BPB_TOTAL_SECTORS_32_OFFSET         32U
#define FAT_BPB_SIZE                            36U

#define FAT_BPB_FAT32_SIZE_32_OFFSET            36U
#define FAT_BPB_FAT32_EXT_FLAGS_OFFSET          40U
#define FAT_BPB_FAT32_FS_VERSION_OFFSET         42U
#define FAT_BPB_FAT32_ROOT_CLUSTER_OFFSET       44U
#define FAT_BPB_FAT32_FS_INFO_OFFSET            48U
#define FAT_BPB_FAT32_BACKUP_BOOT_OFFSET        50U
#define FAT_BPB_FAT32_RESERVED_OFFSET           52U
#define FAT_BPB_FAT32_DRIVE_NUMBER_OFFSET       64U
#define FAT_BPB_FAT32_RESERVED_1_OFFSET         65U
#define FAT_BPB_FAT32_BOOT_SIGNATURE_OFFSET     66U
#define FAT_BPB_FAT32_VOLUME_ID_OFFSET          67U
#define FAT_BPB_FAT32_VOLUME_LABEL_OFFSET       71U
#define FAT_BPB_FAT32_FILESYSTEM_TYPE_OFFSET    82U

#define FAT_BPB_FAT12_16_DRIVE_NUMBER_OFFSET    36U
#define FAT_BPB_FAT12_16_RESERVED_OFFSET        37U
#define FAT_BPB_FAT12_16_BOOT_SIGNATURE_OFFSET  38U
#define FAT_BPB_FAT12_16_VOLUME_ID_OFFSET       39U
#define FAT_BPB_FAT12_16_VOLUME_LABEL_OFFSET    43U
#define FAT_BPB_FAT12_16_FILESYSTEM_TYPE_OFFSET 54U

#define FAT_BPB_FAT12_16_EBPB_SIZE              26U
#define FAT_BPB_FAT32_EBPB_SIZE                 54U

#define FAT_BPB_BOOT_SIGNATURE_OFFSET           510U
#define FAT_BPB_BOOT_SIGNATURE                  0xAA55U

#define FAT_BPB_EXTENDED_SIGNATURE_OLD          0x28U
#define FAT_BPB_EXTENDED_SIGNATURE              0x29U

/* Cluster-count thresholds used to identify the FAT variant */
#define FAT12_CLUSTER_COUNT_MAX                 4084U
#define FAT16_CLUSTER_COUNT_MIN                 4085U
#define FAT16_CLUSTER_COUNT_MAX                 65524U
#define FAT32_CLUSTER_COUNT_MIN                 65525U

/* BPB values and fixed record sizes */
#define FAT_BYTES_PER_SECTOR_512                512U
#define FAT_BYTES_PER_SECTOR_1024               1024U
#define FAT_BYTES_PER_SECTOR_2048               2048U
#define FAT_BYTES_PER_SECTOR_4096               4096U

#define FAT_SECTORS_PER_CLUSTER_MIN             1U
#define FAT_DIRECTORY_ENTRY_SIZE                32U
#define FAT_SHORT_NAME_SIZE                     11U
#define FAT_SHORT_NAME_BASE_SIZE                8U
#define FAT_SHORT_NAME_EXTENSION_SIZE           3U
#define FAT_LONG_NAME_CHARS_PER_ENTRY           13U
#define FAT_VOLUME_LABEL_SIZE                   11U
#define FAT_FILESYSTEM_TYPE_LABEL_SIZE          8U

/* Cluster numbering and FAT entry values common to all FAT variants */
#define FAT_CLUSTER_FREE                        0x00000000U
#define FAT_CLUSTER_RESERVED_0                  0x00000000U
#define FAT_CLUSTER_RESERVED_1                  0x00000001U
#define FAT_FIRST_DATA_CLUSTER                  0x00000002U

/* FAT12 entry values */
#define FAT12_ENTRY_BITS                        12U
#define FAT12_ENTRY_READ_SIZE                   2U
#define FAT12_ENTRY_MASK                        0x0FFFU
#define FAT12_RESERVED_MIN                      0x0FF0U
#define FAT12_RESERVED_MAX                      0x0FF6U
#define FAT12_BAD_CLUSTER                       0x0FF7U
#define FAT12_END_OF_CHAIN_MIN                  0x0FF8U
#define FAT12_END_OF_CHAIN_MAX                  0x0FFFU

/* FAT16 entry values */
#define FAT16_ENTRY_SIZE                        2U
#define FAT16_ENTRY_MASK                        0xFFFFU
#define FAT16_RESERVED_MIN                      0xFFF0U
#define FAT16_RESERVED_MAX                      0xFFF6U
#define FAT16_BAD_CLUSTER                       0xFFF7U
#define FAT16_END_OF_CHAIN_MIN                  0xFFF8U
#define FAT16_END_OF_CHAIN_MAX                  0xFFFFU

/* FAT32 entry values. Only the low 28 bits are defined. */
#define FAT32_ENTRY_SIZE                        4U
#define FAT32_ENTRY_MASK                        0x0FFFFFFFU
#define FAT32_RESERVED_BITS_MASK                0xF0000000U
#define FAT32_RESERVED_MIN                      0x0FFFFFF0U
#define FAT32_RESERVED_MAX                      0x0FFFFFF6U
#define FAT32_BAD_CLUSTER                       0x0FFFFFF7U
#define FAT32_END_OF_CHAIN_MIN                  0x0FFFFFF8U
#define FAT32_END_OF_CHAIN_MAX                  0x0FFFFFFFU

/* Directory-entry markers */
#define FAT_DIRECTORY_ENTRY_END                 0x00U
#define FAT_DIRECTORY_ENTRY_DELETED             0xE5U
#define FAT_DIRECTORY_ENTRY_REPLACED_E5         0x05U

/* Directory-entry attributes */
#define FAT_ATTR_READ_ONLY                      0x01U
#define FAT_ATTR_HIDDEN                         0x02U
#define FAT_ATTR_SYSTEM                         0x04U
#define FAT_ATTR_VOLUME_ID                      0x08U
#define FAT_ATTR_DIRECTORY                      0x10U
#define FAT_ATTR_ARCHIVE                        0x20U
#define FAT_ATTR_LONG_NAME                      0x0FU

/* Long filename sequence-number bits */
#define FAT_LFN_LAST_ENTRY                      0x40U
#define FAT_LFN_ORDER_MASK                      0x1FU

/* Directory-entry field offsets */
#define FAT_DIRENT_NAME_OFFSET                  0U
#define FAT_DIRENT_ATTRIBUTES_OFFSET            11U
#define FAT_DIRENT_NT_FLAGS_OFFSET              12U
#define FAT_DIRENT_CREATE_TIME_TENTHS_OFFSET    13U
#define FAT_DIRENT_CREATE_TIME_OFFSET           14U
#define FAT_DIRENT_CREATE_DATE_OFFSET           16U
#define FAT_DIRENT_ACCESS_DATE_OFFSET           18U
#define FAT_DIRENT_FIRST_CLUSTER_HIGH_OFFSET    20U
#define FAT_DIRENT_WRITE_TIME_OFFSET            22U
#define FAT_DIRENT_WRITE_DATE_OFFSET            24U
#define FAT_DIRENT_FIRST_CLUSTER_LOW_OFFSET     26U
#define FAT_DIRENT_FILE_SIZE_OFFSET             28U

/* FAT date/time encoding */
#define FAT_DATE_YEAR_BASE                      1980U
#define FAT_DATE_DAY_MASK                       0x001FU
#define FAT_DATE_MONTH_MASK                     0x01E0U
#define FAT_DATE_MONTH_SHIFT                    5U
#define FAT_DATE_YEAR_MASK                      0xFE00U
#define FAT_DATE_YEAR_SHIFT                     9U
#define FAT_TIME_SECOND_MASK                    0x001FU
#define FAT_TIME_MINUTE_MASK                    0x07E0U
#define FAT_TIME_MINUTE_SHIFT                   5U
#define FAT_TIME_HOUR_MASK                      0xF800U
#define FAT_TIME_HOUR_SHIFT                     11U
#define FAT_TIME_SECONDS_PER_UNIT               2U

struct fat_bpb {
    u8  reserved[3];
    u8  oem_name[8];
    u16 bytes_per_sector;
    u8  sectors_per_cluster;
    u16 reserved_sector_count;
    u8  fat_count;
    u16 root_entry_count;
    u16 total_sectors_16;
    u8  media;
    u16 fat_size_16;
    u16 sectors_per_track;
    u16 head_count;
    u32 hidden_sectors_count;
    u32 total_sectors_32;

    u8 extended_data[FAT_BPB_FAT32_EBPB_SIZE];
} __packed;

struct fat16_ebpb {
    u8  drive_number;
    u8  reserved;
    u8  extended_boot_signature;
    u32 volume_id;
    u8  volume_label[FAT_VOLUME_LABEL_SIZE];
    u8  filesystem_type[FAT_FILESYSTEM_TYPE_LABEL_SIZE];
} __packed;

struct fat32_ebpb {
    u32 fat_size_32;
    u16 extended_flags;
    u16 fs_version;
    u32 root_cluster;
    u16 fs_info_sector;
    u16 backup_boot_sector;
    u8  reserved[12];
    u8  drive_number;
    u8  reserved_1;
    u8  extended_boot_signature;
    u32 volume_id;
    u8  volume_label[FAT_VOLUME_LABEL_SIZE];
    u8  filesystem_type[FAT_FILESYSTEM_TYPE_LABEL_SIZE];
} __packed;

struct fat_dirent {
    u8  short_filename[11];
    u8  attributes;
    u8  nt_flags;
    u8  creation_time_tenths;
    u16 creation_time;
    u16 creation_date;
    u16 last_access_date;
    u16 first_cluster_high;
    u16 write_time;
    u16 write_date;
    u16 first_cluster_low;
    u32 file_size;
} __packed;

struct fat_inode {
    u32          first_cluster;
    u32          attributes;
    u32          dirent_sector;
    u16          dirent_offset; 
    struct inode inode;
} __packed;

static __force_inline u16 fat12_entry(u8 *fat, u32 cluster)
{
    off_t offset = cluster + (cluster / 2);
    u16 entry = cluster % 2 == 0 ? 
        (fat[offset])      | ((fat[offset + 1] & 0xF) << 8) : 
        (fat[offset] >> 4) | (fat[offset + 1] << 4);
    return entry & FAT12_ENTRY_MASK;
}

static __force_inline u16 fat16_entry(u8 *fat, u32 cluster)
{
    size_t index = cluster - 2;
    return ((u16*)fat)[index] & FAT16_ENTRY_MASK;
}

static __force_inline u32 fat32_entry(u8* fat, u32 cluster)
{
    size_t index = cluster - 2;
    return ((u32*)fat)[index] & FAT32_ENTRY_MASK;
}

void init_fat();

#endif // FAT_H
