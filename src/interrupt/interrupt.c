#include "../lib-header/interrupt.h"
#include "../lib-header/portio.h"

void io_wait(void) {
    out(0x80, 0);
}

void pic_ack(uint8_t irq) {
    if (irq >= 8)
        out(PIC2_COMMAND, PIC_ACK);
    out(PIC1_COMMAND, PIC_ACK);
}

void pic_remap(void) {
    uint8_t a1, a2;

    // Save masks
    a1 = in(PIC1_DATA); 
    a2 = in(PIC2_DATA);

    // Starts the initialization sequence in cascade mode
    out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); 
    io_wait();
    out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    out(PIC1_DATA, PIC1_OFFSET); // ICW2: Master PIC vector offset
    io_wait();
    out(PIC2_DATA, PIC2_OFFSET); // ICW2: Slave PIC vector offset
    io_wait();
    out(PIC1_DATA, 0b0100);      // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    io_wait();
    out(PIC2_DATA, 0b0010);      // ICW3: tell Slave PIC its cascade identity (0000 0010)
    io_wait();

    out(PIC1_DATA, ICW4_8086);
    io_wait();
    out(PIC2_DATA, ICW4_8086);
    io_wait();

    // Restore masks
    out(PIC1_DATA, a1);
    out(PIC2_DATA, a2);
}
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

void activate_keyboard_interrupt(void) {
    out(PIC1_DATA, PIC_DISABLE_ALL_MASK ^ (1 << IRQ_KEYBOARD));
    out(PIC2_DATA, PIC_DISABLE_ALL_MASK);
}
