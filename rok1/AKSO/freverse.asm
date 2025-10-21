section .data
    SYS_EXIT equ 60         ;wychodzi, kod zakończenia w rdi
    SYS_OPEN equ 2          ;otwiera plik, nazwa w rdi
    SYS_FSAT equ 5          ;sprawdza rozmiar plika, deskryptor w rdi
    SYS_CLOSE equ 3         ;zamyka plik, deskryptor w rdi
    SYS_MMAP equ 9          ;w rdi adres mapowania, w rsi dłg mapowania, w rdx flagi dostepu, w r10 flaga zmiany, r9 deskryptor i r8 poczatek
    SYS_MSYNC equ 26        ;synchonizuje zmiany, w rdi adres mmapu, w rsi rozmiar, w rdx flaga
    SYS_MUNMAP equ 11       ;kończy mapowanie, w rdi adres mmapu, w rsi rozmiar
    O_RDWR equ 2            ;flaga, uprawnienia do odczytywania i zapisywania
    PROT_RW equ 1 | 2       ;flaga do mmap, pozwala odczytywac(1) i zapisywac(2) 
    MAP_SHARED equ 1        ;flaga do mmap, widoczność zmian na zmapowanej pamięci
    MS_SYNC equ 4           ;flaga natychmiastowej synchronizacji

section .text
global _start

_start:

.args_check:
    mov rax, [rsp]          ;w rax jest liczba argumentow
    cmp rax, 2              ;sprawdzenie liczby argumentow
    jne .exit1              ;jeśli niepoprawna exit1

.open_file:
    mov rax, SYS_OPEN       
    mov rdi, [rsp+16]       ;w rdi ląduje nazwa danego pliku, ze stosu
    mov rsi, O_RDWR         
    mov rdx, 0
    syscall                 ;otwiera dany plik

    cmp rax, 0              ;sprawdzenie wyniku syscall
    jl .exit1               ;jeśli rax ujemne to otwarcie pliku zakończyło sie błędem, wtedy exit1

    mov r12, rax            ;deskryptor pliku od tej pory w r12

.check_file_size:
    mov rax, SYS_FSAT
    mov rdi, r12            ;deskryptor pliku (w r12) do rdi
    mov rsi, rsp            ;wskaźnik na stos w rsi, tu bedzie zapisane stat 
    syscall

    cmp rax, 0              ;sprawdza czy fstat zakończył się sukcesem
    jl .clean_then_exit1    ;jeśli nie, zamyka otwarty plik i kończy działanie z kodem 1

    mov r13, [rsp+48]       ;w r13 rozmiar pliku w bajtach 
    cmp r13, 2              ;sprawdzam czy rozmiar pliku jest mniejszy niż 2 bajty
    jb .the_end             ;nie ma potrzeby odwracania pliku, wiec koniec programu bez błędu

.map_memory:
    mov rax, SYS_MMAP       ;mapuje do pamieci wirtualnej
    mov rdi, 0              ;dowolne miejsce mapowania
    mov rsi, r13            ;do rsi długość mapowanego pliku, czyli r13
    mov rdx, PROT_RW        ;do rdx flaga ochrony pamięci
    mov r10, MAP_SHARED     ;do r10 flaga widoczności
    mov r8, r12             ;do r8 deskryptor
    mov r9, 0               ;mapowanie od początku pliku
    syscall

    cmp rax, 0              ;sprawdza czy sys_mmap się zakończyło poprawnie
    jl .clean_then_exit1    ;jeśli mmap się nie udało zamyka plik i kończy działanie z kodem 1

    mov r14, rax            ;adres zmapowanej pamięci jest od teraz w r14

.reverse_file:
    mov rsi, r14            ;rsi to wskaźnik na początek pliku, potem to będzie lewy wskaźnik
    mov rdi, r14            
    add rdi, r13            
    dec rdi                 ;rdi = początek pliku + długość - 1, czyli wskazuje na ostatni element, potem prawy wskaźnik

.swap_loop:
    mov al, [rsi]           ;w al bajt z lewej
    mov bl, [rdi]           ;w bl bajt z prawej
    mov [rsi], bl           
    mov [rdi], al           ;zamiana bajtów miejscami

    inc rsi                 ;rsi zwiększony o 1, czyli pokazuje na kolejny bajt
    dec rdi                 ;rdi zmniejszony o 1, czyli pokazuje na poprzedni bajt

    cmp rsi, rdi            ;porównuje, odpowiednik while (rsi < rdi)
    jb .swap_loop

.sync_end_mmap:
    mov rax, SYS_MSYNC      ;synchonizuje zmiany
    mov rdi, r14            ;ustawia rdi na adres mmapu (był w r14)
    mov rsi, r13            ;ustawia rsi na długość mapowania
    mov rdx, MS_SYNC        ;ustawia flagę
    syscall

    cmp rax, 0              ;sprawdza czy synchronizacja się udała
    jl .close_then_exit1    ;jeśli zakończone błędem, kończy mapowanie, zamyka plik i kończy działanie z błędem

    mov rax, SYS_MUNMAP     ;kończy mapowanie pamięci
    mov rdi, r14            
    mov rsi, r13
    syscall
    
    cmp rax, 0              ;sprawdza czy zakończenie mapowania się udało
    jl .clean_then_exit1    ;jeśli zakończone błędem, zamyka plik i kończy działanie błędem

.the_end:
    mov rax, SYS_CLOSE      
    mov rdi, r12            ;deskryptor z r12 do rdi
    syscall   

    cmp rax, 0              
    jl .exit1               ;jeśli zamknięcie pliku się nie udało, kończy działanie z kodem błędu

    mov rax, SYS_EXIT       
    mov rdi, 0              ;program zakończony poprawnie
    syscall                 

.clean_then_exit1:
    mov rax, SYS_CLOSE      ;zamyka plik
    mov rdi, r12            ;deskryptor z r12 do rdi
    syscall                 ;nawet jeśli zakończy się błędem, program przejdzie do exit1, więc nie wymaga sprawdzenia 

.exit1:
    mov rax, SYS_EXIT       
    mov rdi, 1              ;kod zakończenia programu = 1
    syscall 

.close_then_exit1:
    mov rax, SYS_MUNMAP     ;kończy mapowanie pamięci
    mov rdi, r14            
    mov rsi, r13            
    syscall                 ;nawet jeśli błąd, i tak kończymy z kodem 1
    
    jmp .clean_then_exit1   ;zamyka otwarty plik i kończy z kodem 1


