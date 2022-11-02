#ifndef _KERNEL_ITR_H
#define _KERNEL_ITR_H

#include <stddef.h>
#include <stdint.h>

#define IDT_MAX_DESCRIPTORS 			256
#define IDT_CPU_EXCEPTION_COUNT			16
#define IDT_HW_INT_COUNT			    16

/*
* Interrupt Descriptor Table entry
*/
struct IDT_ENTRY {
    uint16_t isr_low;
    uint16_t kernel_cs;
    uint8_t reserved;
    uint8_t attributes;
    uint16_t isr_high;
} __attribute__((packed));

/*
* Interrupt Descriptor Table
*/
__attribute__((aligned(0x10)))
static struct IDT_ENTRY idt[IDT_MAX_DESCRIPTORS];

/*
* IDTR
*/
struct IDTR{
	uint16_t	limit;
	uint32_t	base;
} __attribute__((packed));

typedef struct {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;  // pushed by pusha
    uint32_t int_no;                                  // interrupt number
    uint32_t eip, cs, eflags;                          // pushed by the processor automatically
} REGISTERS;

void idt_set_descriptor(uint8_t vector, uint32_t isr, uint8_t flags);
void idt_init();

#endif