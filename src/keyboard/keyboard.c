#include "../lib-header/keyboard.h"
#include "../lib-header/portio.h"
#include "../lib-header/framebuffer.h"
#include "../lib-header/stdmem.h"

const char keyboard_scancode_1_to_ascii_map[256] = {
      0, 0x1B, '1', '2', '3', '4', '5', '6',  '7', '8', '9',  '0',  '-', '=', '\b', '\t',
    'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',  'o', 'p', '[',  ']', '\n',   0,  'a',  's',
    'd',  'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, '\\',  'z', 'x',  'c',  'v',
    'b',  'n', 'm', ',', '.', '/',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0, '-',    0,    0,   0,  '+',    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,

      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
};

/* -- Driver Interfaces -- */

static struct KeyboardDriverState keyboard_state = {FALSE, FALSE, 0, {0}};

// Activate keyboard ISR / start listen keyboard & save to buffer
void keyboard_state_activate(void){
  keyboard_state.keyboard_input_on = TRUE;
}

// Deactivate keyboard ISR / stop listening keyboard interrupt
void keyboard_state_deactivate(void){
  keyboard_state.keyboard_input_on = FALSE;
}

// Get keyboard buffer values - @param buf Pointer to char buffer, recommended size at least KEYBOARD_BUFFER_SIZE
void get_keyboard_buffer(char *buf){
  for (int i=0; i<KEYBOARD_BUFFER_SIZE; i++) {
    buf[i] = keyboard_state.keyboard_buffer[i];
  }
}

// Check whether keyboard ISR is active or not - @return Equal with keyboard_input_on value
bool is_keyboard_blocking(void){
  return keyboard_state.keyboard_input_on;
}

/* -- Keyboard Interrupt Service Routine -- */

/**
 * Handling keyboard interrupt & process scancodes into ASCII character.
 * Will start listen and process keyboard scancode if keyboard_input_on.
 * 
 * Will only print printable character into framebuffer.
 * Stop processing when enter key (line feed) is pressed.
 * 
 * Note that, with keyboard interrupt & ISR, keyboard reading is non-blocking.
 * This can be made into blocking input with `while (is_keyboard_blocking());` 
 * after calling `keyboard_state_activate();`
 */
void keyboard_isr(void){
  if (!keyboard_state.keyboard_input_on)
        keyboard_state.buffer_index = 0;
    else {
        uint8_t scancode = in(KEYBOARD_DATA_PORT);

        if (scancode == EXTENDED_SCANCODE_BYTE) {
            keyboard_state.read_extended_mode = true;
        } else {
            char mapped_char = keyboard_scancode_1_to_ascii_map[scancode];

            if (keyboard_state.read_extended_mode) {
                keyboard_state.read_extended_mode = false;
                switch (scancode) {
                    case EXT_SCANCODE_UP:
                        mapped_char = 'A';
                        break;
                    case EXT_SCANCODE_DOWN:
                        mapped_char = 'B';
                        break;
                    case EXT_SCANCODE_LEFT:
                        mapped_char = 'D';
                        break;
                    case EXT_SCANCODE_RIGHT:
                        mapped_char = 'C';
                        break;
                    default:
                        // Ignore any other extended scancode
                        return;
                }
            }

            if (mapped_char != 0) {
                if (mapped_char == '\n') {
                    // Line feed is pressed, stop processing
                    keyboard_state.keyboard_input_on = false;
                } else if (keyboard_state.buffer_index < KEYBOARD_BUFFER_SIZE) {
                    // Save to buffer
                    keyboard_state.keyboard_buffer[keyboard_state.buffer_index++] = mapped_char;
                }
            }
        }
    }

    pic_ack(IRQ_KEYBOARD);

}