#include "lib-header/portio.h"
#include "lib-header/stdtype.h"
#include "lib-header/stdmem.h"
#include "lib-header/gdt.h"
#include "lib-header/framebuffer.h"
#include "lib-header/kernel_loader.h"


void kernel_setup(void) {
    enter_protected_mode(&_gdt_gdtr);
    framebuffer_clear();
    framebuffer_write(3, 8,  'H', 0, 200);
    framebuffer_write(3, 9,  'a', 0, 200);
    framebuffer_write(3, 10, 'i', 0, 200);
    framebuffer_write(3, 11, '!', 0, 200);
    framebuffer_write(3, 12, ' ', 0, 0);
    framebuffer_write(3, 13, 'S', 0, 100);
    framebuffer_write(3, 14, 'y', 0, 100);
    framebuffer_write(3, 15, 'a', 0, 100);
    framebuffer_write(3, 16, '\'', 0, 100);
    framebuffer_write(3, 17, 'b', 0, 100);
    framebuffer_write(3, 18, 'a', 0, 100);
    framebuffer_write(3, 19, 'n', 0, 100);
    framebuffer_set_cursor(3, 12);
    while (TRUE);
}