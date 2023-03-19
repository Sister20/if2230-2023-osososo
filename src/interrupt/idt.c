#include "../lib-header/idt.h"
#include "../lib-header/stdmem.h"

extern void *isr_stub_table[ISR_STUB_TABLE_LIMIT];

static struct IDT idt;

void set_interrupt_gate(uint8_t int_vector, void *handler_address, uint16_t gdt_seg_selector, uint8_t privilege) {
    struct IDTGate *gate = &idt.entries[int_vector];
    uint32_t offset = (uint32_t) handler_address;
    gate->offset_low = offset & 0xFFFF;
    gate->offset_high = (offset >> 16) & 0xFFFF;
    gate->segment = gdt_seg_selector;
    gate->_reserved = 0;
    gate->_r_bit_1 = INTERRUPT_GATE_R_BIT_1;
    gate->_r_bit_2 = INTERRUPT_GATE_R_BIT_2;
    gate->gate_32 = 1; // Always set to 32 bit
    gate->_r_bit_3 = INTERRUPT_GATE_R_BIT_3;
    gate->privilege = privilege;
    gate->present = 1;
}

void initialize_idt() {
    _idt_idtr.limit = sizeof(idt) - 1;
    _idt_idtr.base = &idt;

    memset(&idt, 0, sizeof(idt));

    // Set up interrupt handler / ISR stub
    for (int i = 0; i < ISR_STUB_TABLE_LIMIT; i++) {
        set_interrupt_gate(i, isr_stub_table[i], GDT_KERNEL_CODE_SEGMENT_SELECTOR, 0);
    }
    // Load interrupt descriptor table
    __asm__ volatile ("lidt %0" : : "m" (_idt_idtr));
}