#include "lib-header/stdtype.h"
#include "lib-header/fat32.h"
#include "lib-header/stdmem.h"

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

void concatStrings(char *dest, const char *src) {
    int i = 0;
    while (dest[i] != '\0') {
        i++;
    }
    int j = 0;
    while (src[j] != '\0') {
        dest[i] = src[j];
        i++;
        j++;
    }
    dest[i] = '\0';
}

int stringLength(const char *str) {
    int i = 0;
    while (str[i] != '\0') {
        i++;
    }
    return i;
}

void printPath(void *restrict path, uint16_t count, struct FAT32DirectoryTable *current_dir) {
    syscall(6, (uint32_t) current_dir, sizeof(struct FAT32DirectoryTable), 0);
    for (uint16_t i = 0; i < count; i++) {
        syscall(8, (uint32_t) current_dir, (uint32_t) *((uint16_t*)path + i), 0);
        syscall(5, (uint32_t) current_dir->table[0].name, stringLength(current_dir->table[0].name), 0xA);
    }
    syscall(5, (uint32_t) ":/$ ", 4, 0xD);    
}

void ls_cmd(struct FAT32DirectoryTable *current_dir) {
    uint16_t retcode = 1;
    
    uint16_t i = 0;
    syscall(9, (uint32_t) *((uint16_t*)current_dir + i), (uint32_t) &retcode, 0);
    while (retcode != 0) {
        syscall(5, (uint32_t) current_dir->table[i].name, stringLength(current_dir->table[i].name), 0xF);
        syscall(5, (uint32_t) "\n", 1, 0xA);
        
        i++;
        syscall(9, (uint32_t) *((uint16_t*)current_dir + i), (uint32_t) &retcode, 0);
    }
    
    // while 
}

int main(void) {
    int32_t retcode;
    struct FAT32DirectoryTable current_dir = {0};
    // struct FAT32DriverRequest request = {
    //     .buf                   = &current_dir,
    //     .name                  = "root",
    //     .ext                   = "\0\0\0",
    //     .parent_cluster_number = ROOT_CLUSTER_NUMBER,
    //     .buffer_size           = CLUSTER_SIZE,
    // };

    // baca isi file/folder dari root 
    
    // syscall(1, (uint32_t) &request, (uint32_t) &retcode, 0);

    // output path root directory
    uint16_t path_cluster[20] = {2}; 
    uint16_t n_path = 1;
    printPath(path_cluster, n_path, &current_dir);

    char buf[16];    
    while (TRUE) {         
        // Getting the user input 
        syscall(4, (uint32_t) buf, 16, 0);
        
        // Parsing command from user input
        syscall(7, (uint32_t) buf, (uint32_t) &retcode, 0);       

        // checking return code and calling the right syscall
        if (retcode == 0) {
           syscall(5, (uint32_t) "xxx\n", 4, 0xF);
        } 
        else if (retcode == 1) {
            ls_cmd(&current_dir);
        }

        else if (retcode == 8) {
           syscall(5, (uint32_t) buf, stringLength(buf), 0xF); 
           syscall(5, (uint32_t) ": command not found\n", stringLength(": command not found\n"), 0xF);
        }
        
        // Clearing buffer and printing the cwd path
        syscall(6, (uint32_t) buf, sizeof(buf), 0);
        printPath(path_cluster, n_path, &current_dir);
    }

    return 0;
}
