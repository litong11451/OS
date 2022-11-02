#include <stdint.h>
#include <stddef.h>
#include <kernel/itr.h>
#include <kernel/tty.h>
#include <stdio.h>
#include <kernel/pic.h>

static struct IDTR idtr_t;

extern uint32_t isr_stub_t[];
extern uint32_t isr_hw_t[];

void idt_set_descriptor(uint8_t vector,uint32_t isr, uint8_t flags) {
    struct IDT_ENTRY* descriptor = &idt[vector];

    descriptor->isr_low = isr & 0xFFFF;
    descriptor->kernel_cs = 0x08;
    descriptor->reserved = 0;
    descriptor->attributes = flags;
    descriptor->isr_high = isr >> 16;
}

void idt_init() {
    idtr_t.base = (uint32_t)&idt;
    idtr_t.limit = 8 * IDT_MAX_DESCRIPTORS - 1;
    
    for (uint8_t vector = 0; vector < IDT_CPU_EXCEPTION_COUNT ; vector++) {
        idt_set_descriptor(vector, isr_stub_t[vector], 0x8E);
    }
    for (uint8_t vector = 0; vector < IDT_HW_INT_COUNT ; vector++) {
        idt_set_descriptor(vector+32, isr_hw_t[vector], 0x8E);
    }
    pic_init(0x20,0x28);
    
    __asm__ volatile ("lidt %0" : : "m"(idtr_t)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag

    IRQ_set_mask(0);
}

__attribute__((noreturn))
void interrupt_handler(REGISTERS reg) {
    printf("Interrupt REGISTERS:\n");
    
    if(reg.int_no >= 32) {
        if(reg.int_no = 33) {
            keyboard_handler(reg);
        }
        PIC_sendEOI(reg.int_no - 32);
    }
    //__asm__ volatile ("cli;hlt"); // Completely hangs the computer
}

 