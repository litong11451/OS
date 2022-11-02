
.code32
# Declare constants for the multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare a multiboot header that marks the program as a kernel.
.section .multiboot.data, "aw"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .data
.align 8
# GDT initial
gdt:
	.skip 8
gdt_code:
	.word 0xFFFF
	.word 0
	.byte 0	
	.byte 0b10011010
	.byte 0b11001111
	.byte 0
gdt_data:
	.word 0xFFFF
	.word 0
	.byte 0
	.byte 0b10010010
	.byte 0b11001111
	.byte 0
gdt_end:	

gdt_desc:
	.word gdt_end - gdt
	.long gdt

# Allocate the initial stack.
.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# Preallocate pages used for paging. Don't hard-code addresses and assume they
# are available, as the bootloader might have loaded its multiboot structures or
# modules there. This lets the bootloader know it must avoid the addresses.
.section .bss, "aw", @nobits
	.align 4096
boot_page_directory:
	.skip 4096
boot_page_table1:
	.skip 4096
# Further page tables may be required if the kernel grows beyond 3 MiB.

# The kernel entry point.
.section .multiboot.text, "a"
.global _start
.type _start, @function

_start:
# Physical address of boot_page_table1.
	# TODO: I recall seeing some assembly that used a macro to do the
	#       conversions to and from physical. Maybe this should be done in this
	#       code as well?
	movl $(boot_page_table1 - 0xC0000000), %edi
	# First address to map is address 0.
	# TODO: Start at the first kernel page instead. Alternatively map the first
	#       1 MiB as it can be generally useful, and there's no need to
	#       specially map the VGA buffer.
	movl $0, %esi
	# Map 1023 pages. The 1024th will be the VGA text buffer.
	movl $1023, %ecx
1:
	# Only map the kernel.
	cmpl $_kernel_start, %esi
	jl 2f
	cmpl $(_kernel_end - 0xC0000000), %esi
	jge 3f

	# Map physical address as "present, writable". Note that this maps
	# .text and .rodata as writable. Mind security and map them as non-writable.
	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)

2:
	# Size of page is 4096 bytes.
	addl $4096, %esi
	# Size of entries in boot_page_table1 is 4 bytes.
	addl $4, %edi
	# Loop to the next entry if we haven't finished.
	loop 1b

3:
	# Map VGA video memory to 0xC03FF000 as "present, writable".
	movl $(0x000B8000 | 0x003), boot_page_table1 - 0xC0000000 + 1023 * 4

	# The page table is used at both page directory entry 0 (virtually from 0x0
	# to 0x3FFFFF) (thus identity mapping the kernel) and page directory entry
	# 768 (virtually from 0xC0000000 to 0xC03FFFFF) (thus mapping it in the
	# higher half). The kernel is identity mapped because enabling paging does
	# not change the next instruction, which continues to be physical. The CPU
	# would instead page fault if there was no identity mapping.

	# Map the page table to both virtual addresses 0x00000000 and 0xC0000000.
	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0
	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 768 * 4

	# Set cr3 to the address of the boot_page_directory.
	movl $(boot_page_directory - 0xC0000000), %ecx
	movl %ecx, %cr3

	# Enable paging and the write-protect bit.
	movl %cr0, %ecx
	orl $0x80010000, %ecx
	movl %ecx, %cr0

	# Jump to higher half with an absolute jump. 
	lea 4f, %ecx
	jmp *%ecx

.section .text


4:
	# At this point, paging is fully set up and enabled.

	# Unmap the identity mapping as it is now unnecessary. 
	movl $0, boot_page_directory + 0

	# Reload crc3 to force a TLB flush so the changes to take effect.
	movl %cr3, %ecx
	movl %ecx, %cr3

	# Set up the stack.
	mov $stack_top, %esp

	# load GDT
	lgdt gdt_desc
	# Reload CS register containing code selector
	jmp $0x08,$reload_CS

reload_CS:
   # Reload data segment registers:
   # 0x10 is a stand-in for your data segment
	movl $0x10, %eax
	movl %eax, %ds
	movl %eax, %es
	movl %eax, %fs
	movl %eax, %gs
	movl %eax, %ss		

enter_kernel:
	call _init
	# Enter the high-level kernel.
	call kernel_main
	# Infinite loop if the system has nothing more to do.
	cli
	1:	hlt
	jmp 1b


execption:
	pusha
    cld /* C code following the sysV ABI requires DF to be clear on function entry */
    call interrupt_handler
    popa
	addl $0x4,%esp
    iret

execption_error:
	pusha
    cld /* C code following the sysV ABI requires DF to be clear on function entry */
    call interrupt_handler
    popa
	addl $0x4,%esp
    iret

devide_error:
	pushl $0
	jmp execption
debug:
	pushl $1
	jmp execption
nmi:
	pushl $2
	jmp execption
breakpoint:
	pushl $3
	jmp execption
overflow:
	pushl $4
	jmp execption
bound:
	pushl $5
	jmp execption
invalid_opcode:
	pushl $6
	jmp execption
device_not_available:
	pushl $7
	jmp execption
double_fault:
	pushl $8
	jmp execption_error
copr_seg_overrun:
	pushl $9
	jmp execption
invalid_tss:
	pushl $10
	jmp execption_error
seg_not_present:
	pushl $11
	jmp execption_error
stack_seg_fault:
	pushl $12
	jmp execption_error
general_protection_fault:
	pushl $13
	jmp execption_error
page_fault:
	pushl $14
	jmp execption_error
float_p_exception:
	pushl $16
	jmp execption

.globl isr_stub_t
isr_stub_t:
	.long devide_error
	.long debug
	.long nmi
	.long breakpoint
	.long overflow
	.long bound
	.long invalid_opcode
	.long device_not_available
	.long double_fault
	.long copr_seg_overrun
	.long invalid_tss
	.long seg_not_present
	.long stack_seg_fault
	.long general_protection_fault
	.long page_fault
	.long float_p_exception

hw0:
	pushl $0x20
	jmp execption
hw1:
	pushl $0x21
	jmp execption
hw2:
	pushl $0x22
	jmp execption
hw3:
	pushl $0x23
	jmp execption
hw4:
	pushl $0x24
	jmp execption
hw5:
	pushl $0x25
	jmp execption
hw6:
	pushl $0x26
	jmp execption
hw7:
	pushl $0x27
	jmp execption
hw8:
	pushl $0x28
	jmp execption
hw9:
	pushl $0x29
	jmp execption
hw10:
	pushl $0x30
	jmp execption
hw11:
	pushl $0x31
	jmp execption
hw12:
	pushl $0x32
	jmp execption
hw13:
	pushl $0x33
	jmp execption
hw14:
	pushl $0x34
	jmp execption
hw15:
	pushl $0x35
	jmp execption

.globl isr_hw_t
isr_hw_t:
	.long hw0
	.long hw1
	.long hw2						
	.long hw3
	.long hw4
	.long hw5
	.long hw6
	.long hw7
	.long hw8						
	.long hw9
	.long hw10
	.long hw11
	.long hw12
	.long hw13
	.long hw14						
	.long hw15


do_test:
	mov $0,%bl
	div %bl


