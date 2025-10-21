global nsqrt 

section .text
nsqrt:

	push rbx
	push r12 
    push r13

    mov r13, rdx
	mov rcx, rdx 
	shr rcx, 6

.set_to_zero_loop:
	mov qword [rdi+8*rcx-8], 0
	loop .set_to_zero_loop
	
	mov rax, 1



.check_bit_loop:
	mov r8, rdx
	sub r8, rax
	sub r8, 1
	
	bts [rdi], r8 ;zapala n-j-1 bit w q  (rdx-rax-1),
	mov r9, 1 ;"flaga" czy dziala
	
	mov rbx, rdx
	shr rbx, 5
	sub rbx, 1 ;rbx to i
	
.compare_with_x_loop:
    mov rdx, r13
	shr rdx, 6 
	mov r11, 0 ;A
	
	add r8, 2
	shr r8, 6
	mov r10, rbx 
	sub r10, r8 ;a = i-((n-j+1)/64
	
	jb .skip_value_r11
	
	cmp r10, rdx
	jae .skip_value_r11
	
	mov r11, [rdi+8*r10];
	
.skip_value_r11:
	mov r12, 0 ; B
	
	mov r10, rbx 
	sub r10, r8
	
	jbe .skip_value_r12
	
	cmp r10, rdx
	ja .skip_value_r12
	
	mov r12, [rdi+8*r10-8]
	
.skip_value_r12:

	mov cl, r8b
	shld r11, r12, cl
	
	cmp r11, [rsi+8*rbx]
	jbe .skip_set_flag
	
	mov r9, 0
	
.skip_set_flag:
	
	cmp r11, [rsi+8*rbx]
	je .continue_loop
	
	mov rbx, 0 ;ustawia iterator na 0, wiec w sumie konczy petle
	
.continue_loop:
	;koniec fora wewnetrznego
	sub rbx, 1
	jae .compare_with_x_loop
	
	cmp r9, 1
	jne .end_of_substraction ;z rdx=n/64
	
	mov rbx, 0
	mov rdx, r13
    shr rdx, 1;
	mov r9, 0 ;flaga na borrow, 0
	
.substraction_loop: ;odejmuje
	mov rdx, r13 ;przywrcam wartosc
	mov r8, rdx
	sub r8, rax
	add r8, 1 ;r8 to n-j+1
	
	mov r11, 0 ;A
	shr r8, 6
	mov r10, rbx 
	sub r10, r8 ;a = i-((n-j+1)/64
	
	jb .skip_value_r11_2
	
	cmp r10, rdx
	jae .skip_value_r11_2
	
	mov r11, [rdi+8*r10];
	
.skip_value_r11_2:
	mov r12, 0 ; B
	
	mov r10, rbx 
	sub r10, r8
	
	jbe .skip_value_r12_2
	
	cmp r10, rdx
	ja .skip_value_r12_2
	
	mov r12, [rdi+8*r10-8]
	
.skip_value_r12_2:

	mov cl, r8b
	shld r11, r12, cl ;
	
	btr r9, 0 
	sbb [rsi+8*rbx], r11
	setc r9b
	
	shr rdx, 5
	add rbx, 1
	cmp rbx, rdx
	jb .substraction_loop
	
	mov rdx, r13 ;przywracam oryginalna wartosc rdx
	mov r8, rdx
	sub r8, rax ;r8 to n-j
	bts [rdi], r8 ;zapalam n-jty bit bo o to chodzilo caly czas
	shr rdx, 6 ;wracamy do rdx=n/64, zeby w . 
	

.end_of_substraction:
	shl rdx, 6; rdx = n
	mov r8, rdx
	sub r8, rax
	sub r8, 1 ;r8 to n-j-1
	btr [rdi], r8 ;wylacza n-j-1 bit, bo byl pomocniczo wlaczany

	
	;koniec fora
	add rax, 1
	cmp rdx, rax ;n-j
	jae .check_bit_loop
	
	
	mov r9, 1 ;flaga 
	shr rdx, 5
	sub rdx, 1
	mov rbx, rdx ;i=n/32-1
	add rdx, 1
	shl rdx, 5
	
.smallest_bit_check_loop:
	mov r10, 0 ;A
	shr rdx, 6 ;n/64
	cmp rbx, rdx ;rbx to male a
	jae .skip_r10_set
	mov r10, [rdi+8*r10]
	
.skip_r10_set:
	mov r11, 0;B
	
	cmp rbx, 0
	jnz .a_is_not_0
	bts r11, 63
.a_is_not_0:
	cmp rbx, rdx
	ja .skip_r11_set
	mov r11, [rdi+8*r10-8]
.skip_r11_set:
	shld r10, r11, 1
	
	cmp r10, [rsi+8*rbx]
	je .continue_this_loop
	
	jb .end_this_loop
	mov r9, 0
.end_this_loop:
	mov rbx, 0
	
.continue_this_loop:
	sub rbx, 1
	jae .smallest_bit_check_loop
	
	cmp r9, 0
	je .the_end
	add qword [rdi], 1
.the_end:
    pop r13
	pop r12
	pop rbx
	ret