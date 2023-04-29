#include "../lib-header/interrupt.h"
#include "../lib-header/portio.h"
#include "../lib-header/keyboard.h"
#include "../lib-header/stdmem.h"
#include "../lib-header/idt.h"
#include "../lib-header/fat32.h"
#include "../lib-header/framebuffer.h"

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

struct TSSEntry _interrupt_tss_entry = {
    .prev_tss = 0,
    .esp0 = 0, 
    .unused_register = {0},
    .ss0  = GDT_KERNEL_DATA_SEGMENT_SELECTOR,
};

void main_interrupt_handler(
    __attribute__((unused)) struct CPURegister cpu,
    uint32_t int_number,
    __attribute__((unused)) struct InterruptStack info
) {
    switch (int_number)
    {
        case PAGE_FAULT:
            __asm__("hlt"); 
            break;
        case PIC1_OFFSET + IRQ_TIMER:

            break;
        case  PIC1_OFFSET + IRQ_KEYBOARD:
            keyboard_isr();
            break;
        case 0x30:
            syscall(cpu, info);
            break;            
        case  PIC1_OFFSET + IRQ_CASCADE:

            break;
        case  PIC1_OFFSET + IRQ_COM2:

            break;
        case  PIC1_OFFSET + IRQ_COM1:

            break;
        case  PIC1_OFFSET + IRQ_LPT2:
            
            break;
        case  PIC1_OFFSET + IRQ_FLOPPY_DISK:
        
            break;
        
        case  PIC1_OFFSET + IRQ_LPT1_SPUR:

            break;
        
        case  PIC1_OFFSET + IRQ_CMOS:

            break;

        case  PIC1_OFFSET + IRQ_PERIPHERAL_1:
            
            break;

        case  PIC1_OFFSET + IRQ_PERIPHERAL_2:

            break;
        
        case  PIC1_OFFSET + IRQ_PERIPHERAL_3:

            break;
        
        case  PIC1_OFFSET + IRQ_MOUSE:

            break;

        case  PIC1_OFFSET + IRQ_FPU:

            break;
        
        case  PIC1_OFFSET + IRQ_PRIMARY_ATA:

            break;

        case  PIC1_OFFSET + IRQ_SECOND_ATA:

            break;

        default:

            break;
    }
    // pic_ack(int_number - PIC1_OFFSET);
}

void activate_keyboard_interrupt(void) {
    out(PIC1_DATA, PIC_DISABLE_ALL_MASK ^ (1 << IRQ_KEYBOARD));
    out(PIC2_DATA, PIC_DISABLE_ALL_MASK);
}

void set_tss_kernel_current_stack(void) {
    uint32_t stack_ptr;
    // Reading base stack frame instead esp
    __asm__ volatile ("mov %%ebp, %0": "=r"(stack_ptr) : /* <Empty> */);
    // Add 8 because 4 for ret address and other 4 is for stack_ptr variable
    _interrupt_tss_entry.esp0 = stack_ptr + 8; 
}

void syscall(struct CPURegister cpu, __attribute__((unused)) struct InterruptStack info) {
    info = info;
    if (cpu.eax == 0) {
        struct FAT32DriverRequest request = *(struct FAT32DriverRequest*) cpu.ebx;
        *((int8_t*) cpu.ecx) = read(request);
    } 
    else if (cpu.eax == 1) {
        struct FAT32DriverRequest request = *(struct FAT32DriverRequest*) cpu.ebx;
        *((int8_t*) cpu.ecx) = read_directory(request);
    } 
    else if (cpu.eax == 2) {
        struct FAT32DriverRequest request = *(struct FAT32DriverRequest*) cpu.ebx;
        *((int8_t*) cpu.ecx) = write(request);
    }         
    else if (cpu.eax == 3) {
        struct FAT32DriverRequest request = *(struct FAT32DriverRequest*) cpu.ebx;
        *((int8_t*) cpu.ecx) = delete(request);
    }             
    else if (cpu.eax == 4) {
        clear_keyboard_buffer();
        keyboard_state_activate();
        __asm__("sti"); // Due IRQ is disabled when main_interrupt_handler() called
        while (is_keyboard_blocking());
        char buf[KEYBOARD_BUFFER_SIZE];
        get_keyboard_buffer(buf);
        memcpy((char *) cpu.ebx, buf, cpu.ecx);
    } 
    else if (cpu.eax == 5) {
        puts((char *) cpu.ebx, cpu.ecx, cpu.edx); // Modified puts() on kernel side 
    } 
    else if (cpu.eax == 6) { 
        memset((char *) cpu.ebx, 0, cpu.ecx);
    }     
    else if (cpu.eax == 7) {
        const char* cmd = (char *) cpu.ebx;        
        while(isspace((unsigned char)*cmd)) {
            cmd++;
        }

        if (memcmp((char *) cmd, "cd", stringLength(cmd)) == 0) {
            *((int8_t*) cpu.ecx) = 0;
        }
        else if (memcmp((char *) cmd, "ls", stringLength(cmd)) == 0) {
            *((int8_t*) cpu.ecx) = 1;
        }    
        else if (memcmp((char *) cmd, "mkdir", stringLength(cmd)) == 0) {
            *((int8_t*) cpu.ecx) = 2;
        }    
        else if (memcmp((char *) cmd, "cat", stringLength(cmd)) == 0) {
            *((int8_t*) cpu.ecx) = 3;
        }    
        else if (memcmp((char *) cmd, "cp", stringLength(cmd)) == 0) {
            *((int8_t*) cpu.ecx) = 4;
        }  
        else if (memcmp((char *) cmd, "rm", stringLength(cmd)) == 0) {
            *((int8_t*) cpu.ecx) = 5;
        }                                    
        else if (memcmp((char *) cmd, "mv", stringLength(cmd)) == 0) {
            *((int8_t*) cpu.ecx) = 6;
        }              
        else if (memcmp((char *) cmd, "whereis", stringLength(cmd)) == 0) {
            *((int8_t*) cpu.ecx) = 7;
        } else {
            *((int8_t*) cpu.ecx) = 8;
        }
    }

    else if (cpu.eax == 8) {
        read_clusters(&*(struct FAT32DirectoryTable*) cpu.ebx, cpu.ecx, 1);
    }
    
    else if (cpu.eax == 9) {
        *((int8_t*) cpu.ecx) = memcmp((char *) cpu.ebx, (char *) cpu.edx, sizeof(cpu.ebx));
    }
    else if (cpu.eax == 10) {
        memcpy((uint32_t*) cpu.ebx, (uint32_t*)cpu.ecx, cpu.edx);
    }
}
