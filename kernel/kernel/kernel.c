#include <stdio.h>
#include <stdint.h>
#include <kernel/tty.h>
#include <kernel/itr.h>
#include <driver/keyboard.h>

void kernel_main(void) {
	terminal_initialize();
	printf("Hello, kernel World!\n");
	idt_init();
	kbd_init();
	for(;;){
		__asm__ volatile("hlt");
	}
}
