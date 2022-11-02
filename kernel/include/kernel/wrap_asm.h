#ifndef _KERNEL_WRAP_ASM_H
#define _KERNEL_WRAP_ASM_H

#include <stdint.h>


inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}
inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}
inline void io_wait(void) {
    outb(0x80, 0);
}

#endif