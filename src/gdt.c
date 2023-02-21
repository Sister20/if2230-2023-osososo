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
            .long_mode_code_flag = 0x01,
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
            .long_mode_code_flag = 0x01,
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
