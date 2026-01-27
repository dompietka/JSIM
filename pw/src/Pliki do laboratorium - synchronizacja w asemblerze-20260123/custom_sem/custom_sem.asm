global sem_init
global sem_acquire
global sem_release

section .text

;void sem_init(custom_sem_t* sem, int initial_value);
sem_init:
    mov     dword [rdi], esi ;wartosc poczatkowa do semafora
    ret

sem_acquire:
.try_acquire:
    mov     eax, esi            ;w rsi to co chcemy odjac
    neg     eax                 ; eax = -amount (wartość do odjęcia)

    lock xadd dword [rdi], eax

    cmp     eax, esi
    jge     .done               ; Jeśli udało się pobrać, kończymy

    lock add dword [rdi], esi
    jmp     .try_acquire

.done:
    ret

; void sem_release(custom_sem_t* sem, int amount);
sem_release:
    mov     eax, esi
    lock add dword [rdi], eax
    ret