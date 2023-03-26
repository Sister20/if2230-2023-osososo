#include "lib-header/portio.h"
#include "lib-header/stdtype.h"
#include "lib-header/stdmem.h"
#include "lib-header/gdt.h"
#include "lib-header/framebuffer.h"
#include "lib-header/kernel_loader.h"
#include "lib-header/interrupt.h"
#include "lib-header/idt.h"

void kernel_setup(void) {
    enter_protected_mode(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    framebuffer_clear();
    framebuffer_write(3, 8,  'H', 0xf, 0);
    framebuffer_write(3, 9,  'a', 0xf, 0);
    framebuffer_write(3, 10, 'i', 0xf, 0);
    framebuffer_write(3, 11, '!', 0xf, 0);
    framebuffer_write(3, 12, ' ', 0xf, 0);
    framebuffer_write(3, 13, 'S', 0xf, 0);
    framebuffer_write(3, 14, 'y', 0xf, 0);
    framebuffer_write(3, 15, 'a', 0xf, 0);
    framebuffer_write(3, 16, '\'', 0, 0);
    framebuffer_write(3, 17, 'b', 100, 0);
    framebuffer_write(3, 18, 'a', 100, 0);
    framebuffer_write(3, 19, 'n', 100, 0);
    framebuffer_set_cursor(3, 12);
    __asm__("int $0x4");
    while (TRUE);
}
