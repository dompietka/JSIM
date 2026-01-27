global inc_thread

section .bss                                                        ;zmienne zerowe.
    spinlock_var: resd 1

section .text

align 8
inc_thread:
    mov     rsi, [rdi]      ;rsi to value
    mov     ecx, [rdi + 8]  ;ecx to licznik petli
    jmp     count_test      ;sprawdzamy petle

count_loop:
.acquire_lock:
    mov     eax, 1        
    xchg    [spinlock_var], eax
    cmp eax, 0     
    jnz     .acquire_lock  ;jesli lock wczesniej niezerowy to zajety wiec probujemy znowu

    inc  dword [rsi]    
    mov  dword [spinlock_var], 0  ;to jest niepodzielne

count_test:
    sub     ecx, 1          ; --count
    jge     count_loop      ; Jeśli count >= 0, powtórz pętlę
    
    xor     eax, eax        ; return NULL
    ret