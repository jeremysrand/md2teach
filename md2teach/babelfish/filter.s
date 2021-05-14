;
;  filter.s
;  md2teach
;
;  Created by Jeremy Rand on 2021-05-12.
;  Based on bfish_asoft by Kelvin Sherlock.
;

        mcopy filter.macros
        keep filter


offset_fileName	gequ 10

; SFFilter
;
; in stack:
;
; (3) |rtl
;     |----
; (4) |DirEntryRecPtr
;     |----
; (2) |returnval
;     |----
;     |.........

; out stack:
;
;   |returnval
;   |----
;   |.........


;
; Returns 0 if it's not an markdown file, or 4 if it is.
;

filter start
	
_b		equ 1
ptr	    equ _b+2
_d		equ ptr+4
_rtlb	equ _d+1
DirPtr	equ _rtlb+3
retval	equ DirPtr+4

		phb		;even up the stack
		pha
		pha
		phd
		tsc
		tcd

		stz <retval	;; assume no

		ldy #offset_fileName
		lda [<DirPtr],y
		sta <ptr
		iny
		iny
		lda [<DirPtr],y
		sta <ptr+2
		
		ldy #2
		lda [<ptr],y
		cmp #4
		blt exit
		tay
		
		short m
		
		iny
		iny
		iny
		lda [<ptr],y
		cmp #'d'
		beq checkM
		cmp #'D'
		bne noMatch
		
checkM 	anop
		dey
		lda [<ptr],y
		cmp #'m'
		beq checkDot
		cmp #'M'
		bne noMatch
		
checkDot anop
		dey
		lda [<ptr],y
		cmp #'.'
		bne noMatch
		
		long m
		lda #4
		sta <retval	;; I handle it
		bra exit
		
noMatch anop
		long m

exit	anop
		pld
		pla
		pla
		pla
		sta 3,s
		pla
		sta 3,s

		plb
		rtl
        end
