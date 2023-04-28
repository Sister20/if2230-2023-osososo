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

void printPath(void *restrict buff, uint8_t count) {
    syscall(5, (uint32_t) buff, count, 0xA);
    syscall(5, (uint32_t) ":/$ ", 4, 0xD);
}

// int memcmp(const void *s1, const void *s2, size_t n) {
//     const uint8_t *buf1 = (const uint8_t*) s1;
//     const uint8_t *buf2 = (const uint8_t*) s2;
//     for (size_t i = 0; i < n; i++) {
//         if (buf1[i] < buf2[i])
//             return -1;
//         else if (buf1[i] > buf2[i])
//             return 1;
//     }

//     return 0;
// }

int main(void) {
    struct FAT32DirectoryTable currentDir = {0};
    struct FAT32DriverRequest request = {
        .buf                   = &currentDir,
        .name                  = "root",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = CLUSTER_SIZE,
    };

    // baca isi file/folder dari root 
    int32_t retcode;
    syscall(1, (uint32_t) &request, (uint32_t) &retcode, 0);

    // output path root directory
    char path[150] = "root";
    printPath(path, stringLength(path));

    char buf[16];    
    while (TRUE) {         
        syscall(4, (uint32_t) buf, 16, 0);
        printPath(path, stringLength(path));

        // if (memcmp(buf, "cd", 2) == 0) {
        //     syscall(5, (uint32_t) "xxx", 3, 0xF); 
        // }   
        memset(buf, 0, sizeof(buf));

        // syscall(5, (uint32_t) buf, 16, 0xF);
    }

    return 0;
}

