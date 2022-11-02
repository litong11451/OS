#ifndef _KERNEL_PIC_H
#define _KERNEL_PIC_H

#include <stdint.h>

void pic_init(uint8_t offset1, uint8_t offset2);
void IRQ_set_mask(uint8_t IRQline);
void IRQ_clear_mask(uint8_t IRQline); 
void PIC_sendEOI(uint8_t irq);

#endif