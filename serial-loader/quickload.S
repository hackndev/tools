@ quickload.S - primitive serial bootloader for pxa devices
@
@ This bootloader enables you to load and execute a kernel image via the serial
@ port using a very simple transfer protocol.  The bootloader expects the system
@ to have the MMU disabled.
@
@ Protocol:
@
@ Upon starting the bootloader will output the prompt 'Q?'.  It will then read
@ echo characters until it encounters a 'G' character.  This echo mode is to
@ allow reliability testing of the link and to soak up any stray characters that
@ may have been received.  Upon receiving 'G' the bootloader will read a DWORD
@ which it treats as the length of the kernel image.  It will then read that
@ many bytes loading them into RAM starting at 0xA0008000.  It prints a '.'
@ character every 256 or so bytes.  After the image loading is complete it
@ prints 'D' and jumps to the start of the image.
@
@ Author: Alex Osborne <alex at-sign hackndev dot com>
@ Created: July 2007
@

.text
.globl _start
_start:
	b	 _copier

@ magic number for tools to recognize us (currently unused)
	.word	0xb007104d

@ here we store how many bytes will be uploaded
data_length:
	.word 	0x0

_copier:
@ figure out where we are loaded
        mov     r7, pc
	sub     r7, #(_copier-_start+8)

@ r5 := FFUART base
	mov	r5, #0x40000000
	orr	r5, #0x00100000

@ show our ready prompt "Q?"
	mov	r0, #'Q'
	bl	ffuart_putc
	mov	r0, #'?'
	bl	ffuart_putc

@ echo until we get a 'G' indicating start of transmission
wait_ready:
	bl	ffuart_getc
	bl	ffuart_putc
	cmp	r0, #'G'
	bne	wait_ready

@ read in the length of the image
	mov	r0, #0
	str	r0, [r7, #8]

	bl	ffuart_getc
	strb	r0, [r7, #8]
	bl	ffuart_getc
	strb	r0, [r7, #8 + 1]
	bl	ffuart_getc
	strb	r0, [r7, #8 + 2]
	bl	ffuart_getc
	strb	r0, [r7, #8 + 3]


@ start loading in the image
	mov	r1, #0xa0000000
	orr	r1, #0x00008000
	ldr	r2, [r7, #8] @ remaining bytes 

_loop:
	@ if no remaining bytes, goto done
	cmp	r2, #0
	beq	done

	@ decrement remaining bytes
	sub	r2, #1
	
	@ print '.' every now and then
	and	r0, r2, #0xff
	cmp	r0, #0
	mov	r0, #'.'
	bleq	ffuart_putc

	@ get a character
	bl	ffuart_getc

	@ store it in RAM
	strb	r0, [r1]

	@ increment our storage location
	add	r1, #1
	

	b	_loop

done:
	mov	r0, #'D'
	bl	ffuart_putc

	@ setup the kernel parameters
	mov	r0, #0
	mov	r1, #0x340	@ mach id
	orr	r1, #0x003	@ 0x343 = palmld
	mov	r2, #0		@ atag base

	@ jump to the kernel image
	mov	r3, #0xa0000000
	orr	r3, #0x00008000
	mov	pc, r3

ffuart_putc:
@ wait for LSR to show we're ready to transmit
	ldr	r6, [r5, #0x14] @ get LSR
	and	r6, #0x32       @ is TDRQ set?
	cmp	r6, #0
	beq	ffuart_putc

	str	r0, [r5] @ put character into THR

	mov	pc, lr

ffuart_getc:
@ wait for LSR to show we're ready to receive
	ldr	r6, [r5, #0x14] @ get LSR
	and	r6, #1          @ is DR set?
	cmp	r6, #0
	beq	ffuart_getc

	ldr	r0, [r5] @ get character from RBR

	mov	pc, lr


