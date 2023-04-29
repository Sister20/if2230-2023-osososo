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
int stringCompare(const char *str1, const char *str2) {
    int i = 0;
    while (str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] != str2[i]) {
            return 1;
        }
        i++;
    }
    if (str1[i] != '\0' || str2[i] != '\0') {
        return 1;
    }
    return 0;
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

#define MAX_ARGS 10
#define MAX_ARG_LEN 50

int parse_input(char* input, char args[MAX_ARGS][MAX_ARG_LEN]) {
    int arg_count = 0;
    int len = stringLength(input);
    int i = 0, j = 0;

    while (i < len) {
        // skip leading whitespace
        while (input[i] == ' ') {
            i++;
        }

        // break at end of string
        if (i == len) {
            break;
        }

        // start of new argument
        j = 0;

        // copy argument to array
        while (input[i] != ' ' && i < len) {
            args[arg_count][j++] = input[i++];
        }

        args[arg_count][j] = '\0';
        arg_count++;

        // check if we've exceeded the maximum number of arguments
        if (arg_count == MAX_ARGS) {
            break;
        }
    }

    return arg_count;
}



void printPath(void *restrict path, uint16_t count, struct FAT32DirectoryTable *current_dir) {
    syscall(6, (uint32_t) current_dir, sizeof(struct FAT32DirectoryTable), 0);
    for (uint16_t i = 0; i < count; i++) {
        if (i != 0) syscall(5, (uint32_t) "/", stringLength("/"), 0xA); 
        syscall(8, (uint32_t) current_dir, (uint32_t) *((uint32_t*)path + i), 0);
        syscall(5, (uint32_t) current_dir->table[0].name, stringLength(current_dir->table[0].name), 0xA);
    }
    syscall(5, (uint32_t) ":/$ ", 4, 0xD);
}

void ls_cmd(struct FAT32DirectoryTable *current_dir) {
    uint16_t retcode = 1;
    
    uint16_t i = 1;
    syscall(9, (uint32_t) &current_dir->table[i].name, (uint32_t) &retcode, (uint32_t) "\0\0\0");
    while (retcode != 0) {
        syscall(5, (uint32_t) current_dir->table[i].name, stringLength(current_dir->table[i].name), 0xF);
        if (current_dir->table[i].ext[0] != '\0') {
            syscall(5, (uint32_t) '.', 1, 0xA);
            syscall(5, (uint32_t) current_dir->table[i].ext, 3, 0xA);
        }
        syscall(5, (uint32_t) "\n", 1, 0xA);
        
        i++;
        syscall(9, (uint32_t) &current_dir->table[i].name, (uint32_t) &retcode, (uint32_t) "\0\0\0");
    }
}



void cat_cmd(struct FAT32DirectoryTable *current_dir, char *filename) {
    struct ClusterBuffer cl           = {0};
    uint16_t retcode = 1;
    uint16_t i = 0;
    uint32_t parent_cluster = (current_dir->table[0].cluster_high << 16) | current_dir->table[0].cluster_low;
    syscall(9, (uint32_t) &current_dir->table[i].name, (uint32_t) &retcode, (uint32_t) "\0\0\0");
    while (retcode != 0) {
        
        if (stringCompare(current_dir->table[i].name, filename) == 0) {
    
            struct FAT32DriverRequest request2 = {
                .buf                   = &cl,
                .name                  = {0},
                .ext                   = {0},
                .parent_cluster_number = parent_cluster,
                .buffer_size           = CLUSTER_SIZE,
            };
            for(int j = 0; j < 11; j++) {
                request2.name[j] = current_dir->table[i].name[j];
            }
            for(int j = 0; j < 3; j++) {
                request2.ext[j] = current_dir->table[i].ext[j];
            }
            syscall(0, (uint32_t) &request2, (uint32_t) &retcode, 0);
            if(retcode == 0) {
                syscall(5, (uint32_t) request2.buf, stringLength(request2.buf), 0xF);
                syscall(5, (uint32_t) "\n",1, 0xF);
                return;
            }
            
        }
        i++;
        syscall(9, (uint32_t) &current_dir->table[i].name, (uint32_t) &retcode, (uint32_t) "\0\0\0");
    }
    syscall(5, (uint32_t) "File not found\n", 15, 0xF);
}


void cp_cmd(struct FAT32DirectoryTable *current_dir, char *asal,char *tujuan) {
    struct ClusterBuffer cl           = {0};
    uint16_t retcode = 1;
    uint16_t i = 0;
    uint32_t parent_cluster = (current_dir->table[0].cluster_high << 16) | current_dir->table[0].cluster_low;
    syscall(9, (uint32_t) &current_dir->table[i].name, (uint32_t) &retcode, (uint32_t) "\0\0\0");
    while (retcode != 0) {
        
        if (stringCompare(current_dir->table[i].name, asal) == 0) {
    
            struct FAT32DriverRequest requestAsal = {
                .buf                   = &cl,
                .name                  = {0},
                .ext                   = {0},
                .parent_cluster_number = parent_cluster,
                .buffer_size           = CLUSTER_SIZE,
            };
            for(int j = 0; j < 11; j++) {
                requestAsal.name[j] = current_dir->table[i].name[j];
            }
            for(int j = 0; j < 3; j++) {
                requestAsal.ext[j] = current_dir->table[i].ext[j];
            }
            syscall(0, (uint32_t) &requestAsal, (uint32_t) &retcode, 0);
            if(retcode == 0) {
                syscall(5, (uint32_t) "masuk\n", 6, 0xF);
                struct FAT32DriverRequest requestTujuan = {
                    .buf                   = &cl,
                    .name                  = {0},
                    .ext                   = {0},
                    .parent_cluster_number = parent_cluster,
                    .buffer_size           = CLUSTER_SIZE,
                };
                for(int j = 0; j < 11; j++) {
                    requestTujuan.name[j] = tujuan[j];
                }
                for(int j = 0; j < 3; j++) {
                    requestTujuan.ext[j] = current_dir->table[i].ext[j];
                }
                syscall(2, (uint32_t) &requestTujuan, (uint32_t) &retcode, 0);
                syscall(5, (uint32_t) "Berhasil ditambahkan\n", 21, 0xF);
                return;
            }
            
        }
        i++;
        syscall(9, (uint32_t) &current_dir->table[i].name, (uint32_t) &retcode, (uint32_t) "\0\0\0");
    }
    syscall(5, (uint32_t) "File not found\n", 15, 0xF);
}

void mkdir_cmd(char *input, struct FAT32DirectoryTable *current_dir) {
    uint8_t retcode = 0;
    syscall(9, (uint32_t) input, (uint32_t) &retcode, (uint32_t) "\0\0\0");
    
    if (retcode == 0) {
        syscall(5, (uint32_t) "mkdir: missing operand\n", stringLength("mkdir: missing operand\n"), 0xF);;
    } else {
        struct ClusterBuffer cl           = {0};
        
        uint32_t parent_cluster = (current_dir->table[0].cluster_high << 16) | current_dir->table[0].cluster_low;
        struct FAT32DriverRequest request = {
            .buf                   = &cl,
            .name                  = {0},
            .ext                   = "\0\0\0",
            .parent_cluster_number = parent_cluster,
            .buffer_size           = 0,
        };
        for (uint8_t i = 0; i < 8; i++) request.name[i] = input[i];
        syscall(2, (uint32_t) &request, (uint32_t) &retcode, 0);
    }
}

void cd_cmd(char *input, void *restrict path, uint16_t *count, struct FAT32DirectoryTable *current_dir) {
    uint8_t retcode = 0;
    // jika args dimulai dengan ".." (naik directory)
    const char* str_path = input;  
    if (*str_path == '.' && *(str_path+1) == '.') {
        if (*count > 1) {
            *count = *count - 1;
            *((uint32_t*)path + *count) = 0;   
        }
    } 
    // jika args kosong (pindah ke root)
    else if (*str_path == 0) {
        for (uint8_t i = 1; i < *count; i++) {
            *((uint32_t*)path + i) = 0;
        }
        *count = 1;
    }  
    // masuk ke directory
    else {
        struct FAT32DirectoryTable new_dir;
        uint32_t parent_cluster = (current_dir->table[0].cluster_high << 16) | current_dir->table[0].cluster_low;
        struct FAT32DriverRequest request = {
            .buf                   = &new_dir,
            .name                  = {0},
            .ext                   = "\0\0\0",
            .parent_cluster_number = parent_cluster,
            .buffer_size           = CLUSTER_SIZE,
        };      
        // jika path dimulai dengan "./"
        if (*str_path == '.' && *(str_path+1) == '/') str_path = str_path + 2;
        
        for (uint8_t i = 0; i < 8; i++) request.name[i] = str_path[i];
        syscall(1, (uint32_t) &request, (uint32_t) &retcode, 0);
        uint32_t new_cluster_path = new_dir.table[0].cluster_high << 16 | new_dir.table[0].cluster_low;
        
        // jika bukan merupakan directory
        if (retcode == 1) {
            syscall(5, (uint32_t) input, stringLength(input), 0xF);
            syscall(5, (uint32_t) ": Not a directory\n", stringLength(": Not a directory\n"), 0xF);
        } 
        // jika directory tidak ada
        else if (retcode == 2) {
            syscall(5, (uint32_t) input, stringLength(input), 0xF);
            syscall(5, (uint32_t) ": No such file or directory\n", stringLength(": No such file or directory\n"), 0xF);
        } 
        // sukses
        else {
            *((uint32_t*)path + *count) = new_cluster_path;
            *count = *count + 1;
        }            
    }
}


int main(void) {
    int32_t retcode;
    char args[MAX_ARGS][MAX_ARG_LEN];
    // int arg_count;    
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
    uint32_t path_cluster[20] = {2}; 
    uint16_t n_path = 1;
    printPath(path_cluster, n_path, &current_dir);

    char buf[16];    
    while (TRUE) {         
        // Getting the user input 
        syscall(4, (uint32_t) buf, 16, 0);
        
        // Parsing command from user input
        parse_input(buf, args);
        syscall(7, (uint32_t) args[0], (uint32_t) &retcode, 0); 

        // checking return code and calling the right syscall
        if (retcode == 0) {
            cd_cmd(args[1], &path_cluster, &n_path, &current_dir);
        } 
        else if (retcode == 1) {
            ls_cmd(&current_dir);
        }
        else if (retcode == 2) {
            mkdir_cmd(args[1], &current_dir);
        }        
        else if(retcode==3){
            cat_cmd(&current_dir, args[1]);
        }
        else if(retcode==4){
            cp_cmd(&current_dir, args[1], args[2]);
        }        
        else if (retcode == 8) {
           syscall(5, (uint32_t) buf, stringLength(buf), 0xF); 
           syscall(5, (uint32_t) ": command not found\n", stringLength(": command not found\n"), 0xF);
        }
        
        // Clearing buffer and printing the cwd path
        syscall(6, (uint32_t) buf, sizeof(buf), 0);
        syscall(6, (uint32_t) args, sizeof(args), 0);
        printPath(path_cluster, n_path, &current_dir);
    }

    return 0;
}

