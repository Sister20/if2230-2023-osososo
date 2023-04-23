#include "lib-header/framebuffer.h"
#include "lib-header/stdtype.h"
#include "lib-header/stdmem.h"
#include "lib-header/portio.h"

void framebuffer_set_cursor(uint8_t r, uint8_t c) {
    char mapped_char = (char)*(MEMORY_FRAMEBUFFER + (r*80 + c)*2);
	uint16_t pos = r * 80 + c;
	out(CURSOR_PORT_CMD, 0x0F);
	out(CURSOR_PORT_DATA, (uint8_t) (pos & 0xFF));
	out(CURSOR_PORT_CMD, 0x0E);
	out(CURSOR_PORT_DATA, (uint8_t) ((pos >> 8) & 0xFF));
    framebuffer_write(r, c, mapped_char, 0xF, 0);
}

void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg) {
    uint16_t attrib = (bg << 4) | (fg & 0x0F);
    volatile uint16_t * where;
    where = (uint16_t *) MEMORY_FRAMEBUFFER + (row * 80 + col) ;
    *where = c | (attrib << 8);
}

void framebuffer_clear(void) {
    memset(MEMORY_FRAMEBUFFER, 0, 80*25*2);
}