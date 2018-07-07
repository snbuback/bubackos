#include <core/configuration.h>

.globl _boot

.text
_boot:
.code32
  /* save grub parameters in the data section */
  mov     $cpu0_kernel_stack + SYSTEM_STACKSIZE, %ebp
  mov     %ebp, %esp
  pushl   $0
  pushl   %eax
  pushl   $0
  pushl   %ebx

  /* The cli instruction disables maskable external interrupts. */
  cli
  call enable_paging

  /* Load Global Descriptor Table (outdated access control, but needs to be set) */
  lgdt  gdt_hdr

  /* Adjust all register segment to a value compatible with 64 bits GDT table.
  ; This is very important since in 64 mode you can't change the CS register anymore (there is no segment in 64bits) */

  /* adjust data segment */
  movw  $0x20, %ax
  mov   %ax, %ds
  mov   %ax, %es
  mov   %ax, %ss
  mov   %ax, %fs
  mov   %ax, %gs

  // Jump into 64-bit zone. Adjust cs segment
  jmp   $0x10, $k_64bits

enable_paging:
  /* Now we're going to set up the page tables for 64-bit mode.
  ; Since this is a minimal example, we're just going to set up a single page.
  ; The 64-bit page table uses four levels of paging,
  ;    PML4E table => PDPTE table => PDE table => PTE table => physical addr
  ; You don't have to use all of them, but you have to use at least the first
  ; three. So we're going to set up PML4E, PDPTE, and PDE tables here, each
  ; with a single entry.
  */
#define PML4E_ADDR  0x8000
#define PDPTE_ADDR  0x9000
#define PDE_ADDR    0xa000

  /* clean memory around all page tables: 4*4096 */
  mov   $12288, %eax
  mov   $PML4E_ADDR, %ebx
  call  clear_memory

  /* Set up PML4 entry, which will point to PDPT entry. */
  movl  $PDPTE_ADDR, %eax
  movl  $PML4E_ADDR, %ebx
  movl  $PDE_ADDR, %ecx
  movl  $PDPTE_ADDR, %edx

  /* The low 12 bits of the PML4E entry are zeroed out when it's dereferenced,
  ; and used to encode metadata instead. Here we're setting the Present and
  ; Read/Write bits. You might also want to set the User bit, if you want a
  ; page to remain accessible in user-mode code.*/
  orl   $0b111, %eax  /* Would be 0b111 to set User bit also */

  movl  %eax, (%ebx)
  /* Although we're in 32-bit mode, the table entry is 64 bits. We can just zero
  ; out the upper bits in this case.*/
  movl   $0, 4(%ebx)
  /* Set up PDPT entry, which will point to PD entry. */
  movl   %ecx, %eax
  orl    $0b111, %eax
  movl   %eax, (%edx)
  movl   $0, 4(%edx)

  /* Page 2806
  ; Set up PD entry, which will point to the first 2MB page (0).  But we
  ; need to set three bits this time, Present, Read/Write and Page Size (to
  ; indicate that this is the last level of paging in use). */
  movl  $0b10000111, (%ecx)

  movl  $0x200000, 8(%ecx)
  orl   $0b10000111, 8(%ecx)

  movl  $0x400000, 16(%ecx)
  orl   $0b10000111, 16(%ecx)

  /* Enable PGE and PAE bits of CR4 to get 64-bit paging available. */
  movl  $0b10100000, %eax
  movl  %eax, %cr4

  /* Page 2749
  ; Set master (PML4) page table in CR3. */
  movl    $PML4E_ADDR, %eax
  movl    %eax, %cr3

  /* Page 4382
  ; Set IA-32e Mode Enable (read: 64-bit mode enable) in the "model-specific
  ; register" (MSR) called Extended Features Enable (EFER). */
  mov   $0xc0000080, %ecx
  rdmsr /* takes ecx as argument, deposits contents of MSR into eax */
  or    $0b100000000, %eax
  wrmsr /* exactly the reverse of rdmsr */

  /* Enable PG flag of CR0 to actually turn on paging. */
  mov   %cr0, %eax
  or    $0x80000000, %eax
  mov   %eax, %cr0
  ret

clear_memory:
  /* clean memory around page tables:
  eax total size
  ebx initial address
  */

  /* loop */
1:
  or    %eax, %eax /* Fast compare to 0 */
  je    2f
  movb  $0, (%ebx)
  inc   %ebx
  dec   %eax
  jmp   1b
2:
  ret


.code64
enable_sse:
  /*now enable SSE and the like */
  movq  %cr0, %rax
  andw  $0xFFFB, %ax     /* clear coprocessor emulation CR0.EM */
  orw   $0x2, %ax        /* set coprocessor monitoring  CR0.MP */
  movq  %rax, %cr0
  movq  %cr4, %rax
  orw   $(3 << 9), %ax 		/*set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time */
  movq  %rax, %cr4
  ret

k_64bits:
  popq   %rsi
  popq   %rdi

  call  enable_sse
  call  native_boot
  
  // do_task_switch should never returns... but just to ensure the return address is 1234
  jmp   do_task_switch


.data
/* Global descriptor table entry format
; See Intel 64 Software Developers' Manual, Vol. 3A, Figure 3-8
; or http://en.wikipedia.org/wiki/Global_Descriptor_Table */
.macro GDT_ENTRY base, segment, flags, type
  // %1 is base address, %2 is segment limit, %3 is flags, %4 is type.
  .word \segment & 0xffff
  .word \base & 0xffff
  .byte (\base >> 16) & 0xff
  .byte \type | ((\flags << 4) & 0xf0)
  .byte (\flags & 0xf0) | ((\flags >> 16) & 0x0f)
  .byte \base >> 24
.endm
#define EXECUTE_READ 0b1010
#define READ_WRITE 0b0010
#define RING0 0b10101001    // Flags set: Granularity, 64-bit, Present, S; Ring=00
                   // Note: Ring is determined by bits 1 and 2 (the only "00")

// Global descriptor table (loaded by lgdt instruction) - should be compatible with 64 bits table
gdt_hdr:
  .word     gdt_end - gdt - 1
  .quad     gdt
gdt:
  GDT_ENTRY 0, 0, 0, 0
  GDT_ENTRY 0, 0, 0, 0 // need to compatibility with 64 bits table
  GDT_ENTRY 0, 0xffffff, RING0, EXECUTE_READ
  GDT_ENTRY 0, 0, 0, 0 // need to compatibility with 64 bits table
  GDT_ENTRY 0, 0xffffff, RING0, READ_WRITE
  // You'd want to have entries for other rings here, if you were using them.
gdt_end:

.bss
.globl cpu0_kernel_stack
.lcomm cpu0_kernel_stack, SYSTEM_STACKSIZE

