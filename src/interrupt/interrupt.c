#include "interrupt.h"

void main_interrupt_handler(
    __attribute__((unused)) struct CPURegister cpu,
    uint32_t int_number,
    __attribute__((unused)) struct InterruptStack info
) {
    switch (int_number)
    {
        case IRQ_TIMER:
            // Call ISR for Timer interrupt
            // Example: timer_interrupt_handler();
            break;
        case IRQ_KEYBOARD:
            // Call ISR for Keyboard interrupt
            // Example: keyboard_interrupt_handler();
            break;
        case IRQ_CASCADE:
            // Call ISR for Mouse interrupt
            // Example: mouse_interrupt_handler();
            break;
        case IRQ_COM2:
            // Call ISR for Mouse interrupt
            // Example: mouse_interrupt_handler();
            break;
        case IRQ_COM1:

            break;
        case IRQ_LPT2:
            
            break;
        case IRQ_FLOPPY_DISK:
        
            break;
        
        case IRQ_LPT1_SPUR:

            break;
        
        case IRQ_CMOS:

            break;

        case IRQ_PERIPHERAL_1:
            
            break;

        case IRQ_PERIPHERAL_2:

            break;
        
        case IRQ_PERIPHERAL_3:

            break;
        
        case IRQ_MOUSE:
            // Call ISR for Mouse interrupt
            // Example: mouse_interrupt_handler();
            break;

        case IRQ_FPU:

            break;
        
        case IRQ_PRIMARY_ATA:

            break;

        case IRQ_SECOND_ATA:

            break;

        
        // Add more cases for other interrupt numbers
        default:
            // Unknown interrupt, do nothing
            break;
    }

    // Send ACK to PIC to indicate the interrupt has been handled
    pic_ack(int_number - PIC1_OFFSET);
}
