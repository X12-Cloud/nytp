section .text
extern printf
extern strcmp
global _start
global test
global main
_start:
  call main
  mov rdi, rax
  mov rax, 60
  syscall
test:
    push rbp
    mov rbp, rsp
    and rsp, -16
    sub rsp, 16
    lea rax, [rel _str_0]
    mov rsi, rax
    lea rdi, [rel _print_str_format]
    xor rax, rax
    call printf
test_epilogue:
    leave
    ret
main:
    push rbp
    mov rbp, rsp
    and rsp, -16
    sub rsp, 16
    call test
    mov rax, 10
    mov dword [rbp + -4], eax
    mov rax, 20
    mov dword [rbp + -8], eax
    movsx rax, dword [rbp + -4]
    push rax
    movsx rax, dword [rbp + -8]
    pop rbx
    mov rcx, rbx
    cmp rcx, rax
    setl al
    movzx rax, al
    cmp rax, 0
    je _if_false_0
_if_true_0:
    lea rax, [rel _str_1]
    mov rsi, rax
    lea rdi, [rel _print_str_format]
    xor rax, rax
    call printf
    jmp _if_end_0
_if_false_0:
    lea rax, [rel _str_2]
    mov rsi, rax
    lea rdi, [rel _print_str_format]
    xor rax, rax
    call printf
_if_end_0:
    mov rax, 0
    jmp main_epilogue
main_epilogue:
    leave
    ret

section .data
    _print_int_format db "%d", 10, 0
    _print_str_format db "%s", 10, 0
    _print_char_format db "%c", 10, 0
    _print_float_format db "%f", 10, 0
    _str_0 db "hello", 0
    _str_1 db "y bigger than x", 0
    _str_2 db "x > y", 0
