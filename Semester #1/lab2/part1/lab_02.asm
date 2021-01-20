.586P 
descr struc 
	lim dw 0 
	base_l dw 0
	base_m db 0
	attr_1 db 0 
	attr_2 db 0
	base_h db 0 
descr ends

data SEGMENT USE16
	gdt_null descr <0, 0, 0, 0, 0, 0>
	gdt_data descr <data_size - 1, 0, 0, 92h, 0, 0>
	gdt_code descr <code_size - 1, 0, 0, 98h, 0, 0>
	gdt_stack descr <255, 0, 0, 92h, 0, 0>
	gdt_screen descr <3999, 8000h, 0Bh, 92h, 0, 0>
	gdt_size=$-gdt_null
	pdescr df 0
	sym    db 1
	attr   db 1Eh
	msg db 27, '[31;42m Back to Real Mode! ', 27, '[0m$' 
	msg1 db 27, '[31;42m Start in Real Mode! ', 27, '[0m', 0Dh, 0Ah, 0Dh, 0Ah, '$'
	data_size=$-gdt_null
data ends

text SEGMENT USE16
    assume CS:text, DS:data
main proc
    xor eax, eax
    mov ax, data
    mov ds, ax
    shl eax, 4
    mov ebp, eax
    mov bx, offset gdt_data
    mov [bx].base_l, ax
    shr eax, 16
    mov [bx].base_m, al
    xor eax, eax
    mov ax, CS
    shl eax, 4
    mov bx, offset gdt_code
    mov [bx].base_l, ax
    shr eax, 16
    mov [bx].base_m, al
    xor eax, eax
    mov ax, SS
    shl eax, 4
    mov bx, offset gdt_stack
    mov [bx].base_l, ax
    shr eax, 16
    mov [bx].base_m, al
;
    mov dword ptr pdescr+2, ebp
    mov word ptr pdescr, gdt_size-1
    lgdt pdescr
;
    mov ax, 40h
    mov es, ax
    mov word ptr es:[67h], offset return
    mov es:[69h], cs
    mov al, 0Fh
    out 70h, al
    mov al, 0Ah
    out 71h, al 
    cli
	
	mov ah, 09h
    mov dx, offset msg1
    int 21h
	
; TO PROTECTED MODE HERE
    mov eax, cr0
    or eax, 1
    mov cr0, eax
; PROCESSOR'S RUNNING IN PROTECTED MODE
    db 0EAh 
    dw offset continue
    dw 16
continue:
    mov ax, 8
    mov ds, ax
;
    mov ax, 24
    mov ss, ax
;
    mov ax, 32
    mov es, ax
;
    mov di, 1920
    mov cx, 80
    mov ax, word ptr sym
	
	mov DI, 3680
	mov ah, 10000011b
	
	mov al, 'I'
	stosw
	mov al, 'N'
	stosw
	mov al, ' '
	stosw
	mov al, 'P'
	stosw
	mov al, 'R'
	stosw
	mov al, 'O'
	stosw
	mov al, 'T'
	stosw
	mov al, 'E'
	stosw
	mov al, 'C'
	stosw
	mov al, 'T'
	stosw
	mov al, 'E'
	stosw
	mov al, 'D'
	stosw
	
;scrn: 
;    stosw
;    inc al 
;    loop scrn
;
; BACK TO REAL MODE
;    mov al, 0FEh
;    out 64h, al
;    hlt
;    
; MODIFICATION HERE
    mov gdt_data.lim, 0FFFFh
    mov gdt_code.lim, 0FFFFh
    mov gdt_stack.lim, 0FFFFh
    mov gdt_screen.lim, 0FFFFh
    push DS
    pop DS
    push SS
    pop SS
    push ES
    pop ES
;
    db 0EAh
    dw offset go 
    dw 16
go: 
    mov eax, cr0
    and eax,  0FFFFFFFEh
    mov cr0, eax
    db 0EAh
    dw offset return
    dw text
; PROCESSOR'S RUNNING IN REAL MODE
return:
    mov ax, data
    mov ds, ax
    mov ax, stk
    mov SS, ax 
    sti
; 
    mov ah, 09h
    mov dx, offset msg
    int 21h
    mov ax, 4C00h
    int 21h
main endp
code_size=$-main 
text ends 
;
stk segment stack use16
    db 256 dup ('^')
stk ends
    end main