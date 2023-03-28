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

    // Belum selesai, gatau bener apa engga
    struct FAT32FileAllocationTable fat_table = {0};
    fat_table.cluster_map[0] = CLUSTER_0_VALUE;
    fat_table.cluster_map[1] = CLUSTER_1_VALUE;
    write_blocks(&fat_table, FAT_CLUSTER_NUMBER, 1);
}

struct FAT32DriverState driver_state = {0};

void initialize_filesystem_fat32(void) {
    // Belum selesai, gatau bener apa engga
    // create_fat32();
    if (is_empty_storage()) {
        create_fat32();
    } else {
        read_blocks(&driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);
    }
}