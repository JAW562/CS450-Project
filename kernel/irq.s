bits 32
global rtc_isr, sys_call_isr, serial_isr
extern sys_call
extern call_to_assm

; RTC interrupt handler
; Tells the slave PIC to ignore interrupts from the RTC
rtc_isr:
	cli
	push ax
	
	; Tell the PIC this is EOI
	; This really should be done
	; at the RTC level -- but this is
	; okay for now...
	mov al, 0x20
	out 0xA0, al
	
	pop ax
	sti
	iret

;;; System call interrupt handler. To be implemented in Module R3.
extern sys_call			; The C function that sys_call_isr will call
sys_call_isr:
	pusha
	push ss
	push ds
	push es
	push fs
	push gs
	push esp	
	call sys_call
	mov esp, eax
	pop gs
	pop fs
	pop es
	pop ds
	pop ss
	popa
	iret

;;; Serial port ISR. To be implemented in Module R6,
extern serial_interrupt;
serial_isr:
	cli
	call serial_interrupt
	sti
	iret