#include "lib-header/framebuffer.h"
#include "lib-header/stdtype.h"
#include "lib-header/stdmem.h"
#include "lib-header/portio.h"
#include "portio.c"


void framebuffer_set_cursor(uint8_t r, uint8_t c) {
	uint16_t pos = r * 80 + c;
	out(CURSOR_PORT_CMD, 0x0F);
	out(CURSOR_PORT_DATA, (uint8_t) (pos & 0xFF));
	out(CURSOR_PORT_CMD, 0x0E);
	out(CURSOR_PORT_DATA, (uint8_t) ((pos >> 8) & 0xFF));
}

void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg) {
    uint16_t attrib = (bg << 4) | (fg & 0x0F);
    volatile uint16_t * where;
    where = (uint16_t *) MEMORY_FRAMEBUFFER + (row * 80 + col) ;
    *where = c | (attrib << 8);
}

void framebuffer_clear(void) {
    volatile uint16_t * w = (uint16_t *) MEMORY_FRAMEBUFFER;
    uint16_t framesize = 80 * 25; 
    for (uint16_t i = 0; i < framesize; i++){
        w[i] = 0x20 | (0x07 << 8);
    }
}