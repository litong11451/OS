
.text

int_handler:
	//movl $0x123abc, 0x0 // this places magic value "0x123abc" at the beginning of memory
	call interrupt_handler
    hlt
 
.p2align 3
idt:
.skip 50*8
 
idtr:
    .short (50*8)-1
    .long idt
    
.globl do_test
do_test:
    lidt idtr
	sti
    movl $int_handler, %eax
    movw %ax, (idt+49*8)
    movw $0x8, (idt+49*8+2)
    movw $0x8e00, (idt+49*8+4)
    shr $16, %eax
    movw %ax, (idt+49*8+6)
    int $49