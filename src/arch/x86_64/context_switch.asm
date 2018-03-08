[BITS 32]
ALIGN 4

global _gdt_flush	; Allows the C code to link to this
extern _gp		; Says that '_gp' is in another file
_gdt_flush:
	lgdt [_gp]	; Load the GDT with our '_gp' which is a special pointer
	mov ax, 0x10	; 0x10 is the offset in the GDT to our data segment
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	jmp 0x08:flush2	; 0x08 is the offset to our code segment: Far jump!
flush2:
	ret		; Returns back to the C code!

; This will refresh the TSS for the processor.
global _tss_flush	; Allows the C code to link to this
_tss_flush:
	mov ax, 0x2B	; Load the index of our TSS structure
			; The index is 0x28 as it is the 5th
			; selector and each is 8 bytes long,
			; but we set the bottom two bits (making
			; 0x2B) so that is has an RPL of 3,
			; not zero.
	ltr ax		; Load 0x2B into the task state register
	ret

; Loads the IDT defined in '_idtp' into the processor.
; This is declared in C as 'extern void _idt_load();'
; global _idt_load
; extern _idtp
; _idt_load:
	; lidt [_idtp]
	; ret
