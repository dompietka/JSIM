global inc_thread

section .text

align 8
inc_thread:
  mov     rsi, [rdi]      ; value
  mov     ecx, [rdi + 8]  ; count
  jmp     count_test
count_loop:
  inc     dword [rsi]     ; ++*value
count_test:
  sub     ecx, 1          ; --count
  jge     count_loop      ; skok, gdy count >= 0
  xor     eax, eax        ; return NULL
  ret
