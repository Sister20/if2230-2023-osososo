#include "../lib-header/stdtype.h"
#include "../lib-header/fat32.h"
#include "../lib-header/stdmem.h"

const uint8_t fs_signature[BLOCK_SIZE] = {
    'C', 'o', 'u', 'r', 's', 'e', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ' ',
    'D', 'e', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'b', 'y', ' ', ' ', ' ', ' ',  ' ',
    'L', 'a', 'b', ' ', 'S', 'i', 's', 't', 'e', 'r', ' ', 'I', 'T', 'B', ' ',  ' ',
    'M', 'a', 'd', 'e', ' ', 'w', 'i', 't', 'h', ' ', '<', '3', ' ', ' ', ' ',  ' ',
    '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '2', '0', '2', '3', '\n',
    [BLOCK_SIZE-2] = 'O',
    [BLOCK_SIZE-1] = 'k',
};

struct FAT32DriverState driver_state = {0};

uint32_t cluster_to_lba(uint32_t cluster) {
    return 4*cluster;
}

void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count) {
    uint32_t lba = cluster_to_lba(cluster_number);
    uint32_t block_count = cluster_to_lba(cluster_count);
    write_blocks(ptr, lba, block_count);
}

void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count) {
    uint32_t lba = cluster_to_lba(cluster_number);
    uint32_t block_count = cluster_to_lba(cluster_count);
    read_blocks(ptr, lba, block_count);
}

void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster) {
// Keknya tinggal ini yang belum bener untuk 3.3.3
    // dir_table->table[0].name = name;
    name = name;
    dir_table->table[0].attribute = ATTR_SUBDIRECTORY;
    dir_table->table[0].user_attribute = UATTR_NOT_EMPTY;
    
    dir_table->table[0].cluster_high = (parent_dir_cluster >> 16) & 0xFFFF;
    dir_table->table[0].cluster_low = parent_dir_cluster & 0xFFFF;
    
    dir_table->table[0].filesize = 0;
}

bool is_empty_storage() {
    uint8_t boot_sector[BLOCK_SIZE];
    read_blocks(&boot_sector, BOOT_SECTOR, 1);
    if (memcmp(boot_sector, fs_signature, BLOCK_SIZE) != 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void create_fat32(void) {
    write_blocks(fs_signature, BOOT_SECTOR, 1);

    struct FAT32FileAllocationTable fat_table = {0};
    fat_table.cluster_map[0] = CLUSTER_0_VALUE;
    fat_table.cluster_map[1] = CLUSTER_1_VALUE;
    fat_table.cluster_map[2] = FAT32_FAT_END_OF_FILE;

    driver_state.fat_table = fat_table;

    write_clusters(&fat_table, FAT_CLUSTER_NUMBER, 1);

    struct FAT32DirectoryTable dir_table = {0};
    init_directory_table(&dir_table, "root", ROOT_CLUSTER_NUMBER);
    write_clusters(&dir_table, ROOT_CLUSTER_NUMBER, 1);
}

void initialize_filesystem_fat32(void) {
    if (is_empty_storage()) {
        create_fat32();
    } else {
        read_blocks(&driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);
    }
}

