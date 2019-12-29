.include "xc.inc"

.text                       ;BP (put the following data in ROM(program memory))

; This is a library, thus it can *not* contain a _main function: the C file will
; deine main().  However, we
; we will need a .global statement to make available ASM functions to C code.
; All functions utilized outside of this file will need to have a leading 
; underscore (_) and be included in a comment delimited list below.
.global _write_0, _write_1, _one_milli, _hundred_micro
    
    
_hundred_micro:
    repeat #1593	;1 cycle
    nop			;1599 cycles
    ;;1000 nanoseconds = 1 microsecond, 100,000 nanoseconds = 100 microseconds
    return
    
    
    
_one_milli:
    repeat #15993   ;1 cycle
    nop		    ;15999 cycles   
    ;;1,000,000 nanoseconds = 1 millisecond
    return
    
    
    
_write_0:
    ;;function call takes 2
    ;;return takes 3
    inc LATA	;1 cycle
    repeat #3	;1 cycle
    nop		;1 cycle, repeated 4 times
    clr LATA	;1 cycle
    repeat #6	;1 cycle
    nop		;1 cycle, repeated 7 times
    return	;3 cycles
    
    
    
    
_write_1:
    inc LATA
    repeat #8
    nop
    clr LATA
    repeat #1
    nop
    return
    

    


