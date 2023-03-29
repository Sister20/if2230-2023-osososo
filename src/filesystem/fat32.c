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
    return CLUSTER_BLOCK_COUNT*cluster;
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
    memcpy(dir_table->table[0].name, name, sizeof(name));

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
        read_clusters(&driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);
    }
}

int8_t delete(struct FAT32DriverRequest request) {
    // inisialisasi directory table parent
    struct FAT32DirectoryTable dir_table = {0};

    // mendapatkan directory table dari parent
    read_clusters(&dir_table, request.parent_cluster_number, 1);

    // traversal parent directory untuk mencari file/folder yang diinginkan
    uint16_t idx = 0;
    for (unsigned int i = 0; i < (CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)); i++) {
        if (memcmp(dir_table.table[i].name, request.name, sizeof(request.name)) == 0 && (memcmp(dir_table.table[i].ext, request.ext, sizeof(request.ext)) == 0)) {
            idx = i;
            break;
        }
    }

    // jika file gaketemu atau ketemunya di idx == 0, return 1
    if (idx == CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry) || idx == 0) {
        return 1;
    }

    // mednapatkan directory entry dari file yang dicari
    struct FAT32DirectoryEntry tobedeleted_entry = dir_table.table[idx];

    // mednapatkan cluster number dari file yang dicari
    uint32_t tobedeleted_cluster_number = (uint32_t)(tobedeleted_entry.cluster_high << 16) + (uint32_t)tobedeleted_entry.cluster_low; 
    
    // inisialisasi cluster kosong 
    uint8_t deleteSpace[CLUSTER_SIZE] = {0}; 

    // mengecek sebuah directory atau bukan
    if (request.ext[0] == 0 && request.ext[1] == 0 && request.ext[1] == 0) {
        // inisialisasi directory table pada dir yang akan didelete
        struct FAT32DirectoryTable tobedeleted_dir_table = {0};
        
        // mendapatkan directory table dari dir yang akan didelete
        read_clusters(&tobedeleted_dir_table, tobedeleted_cluster_number, 1);

        // traversal untuk mengecek merupakan directory root atau bukan
        if (tobedeleted_entry.name[0] == 'r' && tobedeleted_entry.name[0] == 'o' && tobedeleted_entry.name[0] == 'o' && tobedeleted_entry.name[0] == 't') {
            return -1;
        }

        // traversal directory table dari dir yang akan didelete untuk mengecek folder kosong atau tidak
        for (unsigned int i = 1; i < (CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)); i++) {
            if (tobedeleted_dir_table.table[i].name[0] != 0) {
                // directory ga kosong
                return 2;
            }
        }
    }
    // menuliskan deletedSpace ke nomor cluster yang akan didelete
    write_clusters(&deleteSpace, tobedeleted_cluster_number, 1);
    return 0;
}
