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
  // Read the scan code from the keyboard data port
  uint8_t scan_code = inb(KEYBOARD_DATA_PORT);

  // Check if this is an extended scan code
  if (scan_code == EXTENDED_SCANCODE_BYTE) {
      // Set the read extended mode flag to TRUE
      keyboard_state.read_extended_mode = TRUE;
  } else {
      // Determine the ASCII character corresponding to the scan code
      char ascii_char = 0;
      if (keyboard_state.read_extended_mode) {
          // Handle extended scan codes for arrow keys
          switch (scan_code) {
              case EXT_SCANCODE_UP:
                  ascii_char = '^';
                  break;
              case EXT_SCANCODE_DOWN:
                  ascii_char = 'v';
                  break;
              case EXT_SCANCODE_LEFT:
                  ascii_char = '<';
                  break;
              case EXT_SCANCODE_RIGHT:
                  ascii_char = '>';
                  break;
          }
          // Reset the read extended mode flag
          keyboard_state.read_extended_mode = FALSE;
      } else {
            // Convert scan code to ASCII character
          ascii_char = keyboard_scancode_1_to_ascii_map[scan_code];
      }

      // Check if the character is printable
      if (ascii_char >= 32 && ascii_char <= 126) {
          // Add the character to the keyboard buffer
          keyboard_state.keyboard_buffer[keyboard_state.buffer_index] = ascii_char;
          keyboard_state.buffer_index++;
          // Print the character to the console or framebuffer
          // TODO: Implement console or framebuffer output
      }

      // Check if the enter key was pressed to signal end of input
      if (ascii_char == '\n') {
          // Deactivate the keyboard ISR and signal end of input
          keyboard_state.keyboard_input_on = FALSE;
          keyboard_state.buffer_index = 0;
          // TODO: Implement end of input signal
      }
  }
}