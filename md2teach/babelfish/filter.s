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
	
_d		equ 1
ptr	    equ 5
_rtlb	equ 7
DirPtr	equ 11
retval	equ 15

		phb		;even up the stack
		phd
		pha
		pha
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
		
		lda [<ptr]
		cmp #4
		blt exit
		tay
		
		short m
		
		dey
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
		cmp #'m'
		bne noMatch
		
		long m
		lda #4
		sta <retval	;; I handle it
		bra exit
		
noMatch anop
		long m

exit	anop
		pla
		pla
		pld
		pla
		sta 3,s
		pla
		sta 3,s

		plb
		rtl
        end
