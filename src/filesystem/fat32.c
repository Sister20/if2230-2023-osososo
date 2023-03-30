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

/* -- CRUD Operation -- */

int8_t read_directory(struct FAT32DriverRequest request){
    // inisialisasi directory table parent
    struct FAT32DirectoryTable dir_table = {0};

    // mendapatkan directory table dari parent
    read_clusters(&dir_table, request.parent_cluster_number, 1);

    // traversal parent directory untuk mencari file/folder yang diinginkan
    uint16_t idx = 0;
    int8_t found=-1;

    // found=0 jika ketemu dan folder
    // found=1 jika ketemu tapi bukan folder
    // found=2 ga ketemu

    //code: 0 success - 1 not a folder - 2 not found - -1 unknown
    for (unsigned int i = 0; i < (CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)); i++) {
        if(memcmp(dir_table.table[i].name, request.name, sizeof(request.name)) == 0) {
            if(memcmp(dir_table.table[i].ext, request.ext, sizeof(request.ext)) == 0) {
                if(dir_table.table[i].ext[0]=='\0'){
                    uint32_t temp=(dir_table.table[i].cluster_high<<16) + dir_table.table[i].cluster_low;
                    read_clusters(&request.buf + CLUSTER_SIZE*0,temp,1);
                    return 0;
                }
                else{
                    return 1;
                }
            }
            else{
                found=1;
            }
            idx = i;
        }
    }
    if(idx==0){
        return 2;
    }
    return found;
}

int8_t read(struct FAT32DriverRequest request) {
    // Error code: 0 success - 1 not a file - 2 not enough buffer - 3 not found - -1 unknown

    // inisialisasi directory table parent
    struct FAT32DirectoryTable dir_table = {0};

    // mendapatkan directory table dari parent
    read_clusters(&dir_table, request.parent_cluster_number, 1);

    // Check if file exists and is not a directory

    //kalau request bukan file(tapi folder)
    if(request.ext[0]=='\0'){
        return 1;
    }

    // Check if buffer is large enough to hold file
    // if (request.buffer_size < dir_table.table[0].file_size) {
    //     return 2; // Not enough buffer
    // }

    // iterasi sebanyak cluster yang dibutuhkan untuk mencari file yang diinginkan
    int found=3;
    int idx=0;
    // int limit;
    for (unsigned int i = 0; i < (CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)); i++) {
        if (memcmp(dir_table.table[i].name, request.name, sizeof(request.name)) == 0 && (memcmp(dir_table.table[i].ext, request.ext, sizeof(request.ext)) == 0)) {
            found=0;
            idx=i;
            break;
            // read_clusters(&request.buf + CLUSTER_SIZE*idx, dir_table.table[i].cluster_low,1);
            // idx++;
            // if(idx==0){
            //     // limit=request.buffer_size/CLUSTER_SIZE;
            // }
        }
    }

    struct FAT32DirectoryEntry file = dir_table.table[idx];
    uint32_t cluster_number = (file.cluster_high<<16) + file.cluster_low;

    // driver_state.fat_table.cluster_map[cluster_number]
    int t=0;
    while(driver_state.fat_table.cluster_map[cluster_number]!=FAT32_FAT_END_OF_FILE){
        read_clusters(&request.buf + CLUSTER_SIZE*t,cluster_number,1);
        t++;
        cluster_number=driver_state.fat_table.cluster_map[cluster_number];
    }

    //kalau ga ketemu
    if(found==3) return 3;
    //kalau ketemu
    return found;
    
}


// Error code: 0 success - 1 file/folder already exist - 2 invalid parent cluster - -1 unknown
int8_t write(struct FAT32DriverRequest request) {

    struct FAT32DirectoryTable dir_table = {0};

    // initialize the parent directory table
    read_clusters(&dir_table, request.parent_cluster_number, 1);
    
    // Check if parent cluster exists
    if (dir_table.table[0].attribute != ATTR_SUBDIRECTORY) {
        return 2;
    }

    // Check if file/folder already exists
    for (unsigned int i = 0; i < (CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)); i++) {
        if (memcmp(dir_table.table[i].name, request.name, sizeof(request.name)) == 0 && (memcmp(dir_table.table[i].ext, request.ext, sizeof(request.ext)) == 0)) {
            return 1;
        }
    }

    // Find the first empty directory entry to update the parent directory table
    struct FAT32DirectoryEntry *empty_entry = 0;
    for (unsigned int i = 0; i < (CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)); i++) {
        if (dir_table.table[i].user_attribute != UATTR_NOT_EMPTY) {
            empty_entry = &dir_table.table[i];
            break;
        }
    }

    // If buffer_size == 0, create a new directory
    if (request.buffer_size == 0) {
        // Find the empty clusters in FATable
        uint16_t empty_cluster_number = 0;
        for (uint16_t i = 0; i < CLUSTER_MAP_SIZE; i++) {
            if (driver_state.fat_table.cluster_map[i] == FAT32_FAT_EMPTY_ENTRY) {
                empty_cluster_number = i;
                break;
            }
        }

        // Initialize new directory table
        struct FAT32DirectoryTable new_table = {0};
        init_directory_table(&new_table, request.name, request.parent_cluster_number);

        // Write directory table
        write_clusters(&new_table, empty_cluster_number, 1);
        
        // Update parent directory entry
        *empty_entry = new_table.table[0];
    } else {
        // number of clusters needed
        uint16_t n_clusters = ceil_divide(request.buffer_size, CLUSTER_SIZE);   
        uint16_t cluster_number_list[n_clusters];
        uint8_t found = 0;
        
        // Find the empty clusters in FATable and put them in cluster_number_list
        for (uint16_t i = 0; i < CLUSTER_MAP_SIZE; i++) {
            if (driver_state.fat_table.cluster_map[i] == FAT32_FAT_EMPTY_ENTRY) {
                cluster_number_list[found] = i;
                found++;
                if (found == n_clusters) {
                    break;
                }
            }
        }   

        // Make a linked list in the fattable and write the cluster buff data into the storage.bin
        for (uint8_t i = 0; i < (n_clusters - 1); i++) {
            driver_state.fat_table.cluster_map[cluster_number_list[i]] = cluster_number_list[i+1];
            write_clusters(request.buf + CLUSTER_SIZE*i, cluster_number_list[i], 1);
        }

        struct FAT32DirectoryEntry new_entry = {0};
        memcpy(new_entry.name, request.name, sizeof(new_entry.name));
        memcpy(new_entry.ext, request.ext, sizeof(new_entry.ext));
        new_entry.user_attribute = UATTR_NOT_EMPTY;
        new_entry.cluster_high = (cluster_number_list[0] >> 16) & 0xffff;
        new_entry.cluster_low = cluster_number_list[0] & 0xffff;
        new_entry.filesize = request.buffer_size;
        *empty_entry = new_entry;
    }

    // Update the storage.bin
    write_clusters(&dir_table, request.parent_cluster_number, 1);

    return 0; // Success
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
    if (request.ext[0] == 0 && request.ext[1] == 0 && request.ext[2] == 0) {
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

uint16_t ceil_divide(uint32_t numerator, uint32_t denominator) {
    uint16_t result = numerator / denominator;
    if (numerator % denominator != 0) {
        result++;
    }
    return result;
}