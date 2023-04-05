#include "lib-header/stdtype.h"
#include "lib-header/gdt.h"

/**
 * global_descriptor_table, predefined GDT.
 * Initial SegmentDescriptor already set properly according to GDT definition in Intel Manual & OSDev.
 * Table entry : [{Null Descriptor}, {Kernel Code}, {Kernel Data (variable, etc)}, ...].
 */
struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        {
            // Null Descriptor
            .segment_low  = 0x0000,
            .base_low     = 0x0000,
            .base_mid     = 0x00,
            .type_bit     = 0x00,
            .non_system   = 0x00,
            .descriptor_previlage_lavel = 0x00,
            .present_bit  = 0x00,
            .segment_high = 0x00,
            .avl          = 0x00,
            .long_mode_code_flag = 0x00,
            .size_flag    = 0x00,
            .granulity_flag = 0x00,
            .base_high    = 0x00,
            
        },
        {
            // Kernel Code
            .segment_low  = 0xFFFF,
            .base_low     = 0x0000,
            .base_mid     = 0x00,
            .type_bit     = 0x0A,
            .non_system   = 0x01,
            .descriptor_previlage_lavel = 0x00,
            .present_bit  = 0x01,
            .segment_high = 0x0F,
            .avl          = 0x00,
            .long_mode_code_flag = 0x00,
            .size_flag    = 0x01,
            .granulity_flag = 0x01,
            .base_high    = 0x00,
        },
        {
            // Kernel Data
            .segment_low  = 0xFFFF,
            .base_low     = 0x0000,
            .base_mid     = 0x00,
            .type_bit     = 0x02,
            .non_system   = 0x01,
            .descriptor_previlage_lavel = 0x00,
            .present_bit  = 0x01,
            .segment_high = 0x0F,
            .avl          = 0x00,
            .long_mode_code_flag = 0x00,
            .size_flag    = 0x01,
            .granulity_flag = 0x01,
            .base_high    = 0x00,
        }
    }
};

/**
 * _gdt_gdtr, predefined system GDTR. 
 * GDT pointed by this variable is already set to point global_descriptor_table above.
 * From: https://wiki.osdev.org/Global_Descriptor_Table, GDTR.size is GDT size minus 1.
 */
struct GDTR _gdt_gdtr = {
    // TODO : Implement, this GDTR will point to global_descriptor_table. 
    .size    = sizeof(struct GlobalDescriptorTable) - 1,
    .address = &global_descriptor_table,
};

static struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        {/* TODO: Null Descriptor */},
        {/* TODO: Kernel Code Descriptor */},
        {/* TODO: Kernel Data Descriptor */},
        {/* TODO: User   Code Descriptor */},
        {/* TODO: User   Data Descriptor */},
        {
            .segment_high      = (sizeof(struct TSSEntry) & (0xF << 16)) >> 16,
            .segment_low       = sizeof(struct TSSEntry),
            .base_high         = 0,
            .base_mid          = 0,
            .base_low          = 0,
            .non_system        = 0,    // S bit
            .type_bit          = 0x9,
            .privilege         = 0,    // DPL
            .valid_bit         = 1,    // P bit
            .opr_32_bit        = 1,    // D/B bit
            .long_mode         = 0,    // L bit
            .granularity       = 0,    // G bit
        },
        {0}
    }
};

void gdt_install_tss(void) {
    uint32_t base = (uint32_t) &_interrupt_tss_entry;
    global_descriptor_table.table[5].base_high = (base & (0xFF << 24)) >> 24;
    global_descriptor_table.table[5].base_mid  = (base & (0xFF << 16)) >> 16;
    global_descriptor_table.table[5].base_low  = base & 0xFFFF;
}
