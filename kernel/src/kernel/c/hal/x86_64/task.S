/* About arguments here: https://www.nasm.us/doc/nasmdo11.html and http://cons.mit.edu/fa17/x86-64-architecture-guide.html*/
/* Summary:
Register order: RDI, RSI, RDX, RCX, R8, R9
Return in RAX

Register to save: RBX, RBP, and R12–R15

func:
	pushq	%rbp
	movq	%rsp, %rbp
  ...
  leave
  ret

*/

.code64

  .globl _platform_task_switch

  .section .text
_platform_task_switch:
  mov $0xbababa, %rdi
  hlt

  .globl gdt_flush
/* void gdt_flush(uintptr_t base, uint16_t limit); */
gdt_flush:
  enter $10, $0
  movq  %rsi, -10(%rbp)
  movq  %rdi, -8(%rbp)
  lgdt (%rsp)

  /* the data segment was adjust before jump into 64 bits mode */
  leave
  ret

  .globl idt_flush
/* void idt_flush(uintptr_t base, uint16_t limit); */
idt_flush:
  enter $10, $0
  movq  %rsi, -10(%rbp)
  movq  %rdi, -8(%rbp)
  lidt (%rsp)
  leave
  ret


  .globl tss_flush
/* void tss_flush(uint16_t gdt_entry_number); */
tss_flush:
  enter $0, $0
  ltr %di
  leave
  ret

_noop:
  ret


