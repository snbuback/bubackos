.code64

/* void gdt_flush(uintptr_t base, uint16_t limit); */
.globl gdt_flush
gdt_flush:
  enter $10, $0
  movq  %rsi, -10(%rbp)
  movq  %rdi, -8(%rbp)
  lgdt (%rsp)

  // the data segment was adjust before jump into 64 bits mode
  leave
  ret

/* void tss_flush(uint16_t gdt_entry_number); */
.globl tss_flush
tss_flush:
  enter $0, $0
  ltr %di
  leave
  ret
