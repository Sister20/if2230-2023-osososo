#include "lib-header/portio.h"
#include "lib-header/stdtype.h"
#include "lib-header/stdmem.h"
#include "lib-header/gdt.h"
#include "lib-header/framebuffer.h"
#include "lib-header/kernel_loader.h"
#include "lib-header/interrupt.h"
#include "lib-header/idt.h"
#include "lib-header/keyboard.h"
#include "lib-header/fat32.h"
#include "lib-header/paging.h"


void kernel_setup(void) {
    enter_protected_mode(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    activate_keyboard_interrupt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);
    initialize_filesystem_fat32();
    gdt_install_tss();
    set_tss_register();

    // Allocate first 4 MiB virtual memory
    allocate_single_user_page_frame((uint8_t*) 0);

    // Write shell into memory
    struct FAT32DriverRequest request = {
        .buf                   = (uint8_t*) 0,
        .name                  = "shell",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = 0x100000,
    };
    read(request);

    // Set TSS $esp pointer and jump into shell 
    set_tss_kernel_current_stack();
    kernel_execute_user_program((uint8_t*) 0);
    while (TRUE);
}


// void kernel_setup(void) {
//     enter_protected_mode(&_gdt_gdtr);
//     pic_remap();
//     initialize_idt();
//     activate_keyboard_interrupt();
//     framebuffer_clear();
//     framebuffer_set_cursor(0, 0);
//     initialize_filesystem_fat32();
//     keyboard_state_activate();

//     void *virtual_addr = (void *) 0x200000;
//     if (allocate_single_user_page_frame(virtual_addr) == 0) {
//         *((uint8_t*) 0x200000) = 5;
//     }
// }
    // struct ClusterBuffer cbuf[5];
    // for (uint32_t i = 0; i < 5; i++)
    //     for (uint32_t j = 0; j < CLUSTER_SIZE; j++)
    //         cbuf[i].buf[j] = i + 'a';

    // struct FAT32DriverRequest request = {
    //     .buf                   = cbuf,
    //     .name                  = "ikanaide",
    //     .ext                   = "uwu",
    //     .parent_cluster_number = ROOT_CLUSTER_NUMBER,
    //     .buffer_size           = 0,
    // } ;

    // write(request);  // Create folder "ikanaide"
    // memcpy(request.name, "kano1\0\0\0", 8);
    // write(request);  // Create folder "kano1"
    // memcpy(request.name, "ikanaide", 8);
    // memcpy(request.ext, "\0\0\0", 3);
    // // delete(request); // Delete first folder, thus creating hole in FS

    // memcpy(request.name, "daijoubu", 8);
    // request.buffer_size = 5*CLUSTER_SIZE;
    // write(request);  // Create fragmented file "daijoubu"

    // memcpy(request.name, "kano1\0\0\0", 8);
    // request.buffer_size = 0;
    // delete(request);
    // write(request);
    
    // read_directory(request);

    // struct ClusterBuffer dbuf[5] = {0};
    // struct FAT32DriverRequest newRequest = {
    //     .buf                   = dbuf,
    //     .name                  = "daijoubu",
    //     .ext                   = "\0\0\0",
    //     .parent_cluster_number = ROOT_CLUSTER_NUMBER,
    //     .buffer_size           = 5*CLUSTER_SIZE ,
    // };
    
    // read(newRequest);
    // struct FAT32DirectoryTable dir_table = {0};
    // read_clusters(&dir_table, newRequest.parent_cluster_number, 1);
    // if (dir_table.table[0].attribute == ATTR_SUBDIRECTORY) {
    //     delete(request);
    // }

    // memcpy(newRequest.ext, "uwu", 3);
    // write(newRequest);
//     while (TRUE);
// }
