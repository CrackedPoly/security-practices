section .text
  global _start
    _start:
	xor eax, eax
	push eax
	push "//sh"
	push "/bin"
	mov ebx, esp
	mov edx, "-c##"
	shl edx, 16
	shr edx, 16
	push edx
	mov ecx, esp
	push eax
	push " -la"
	push "ls  "
	mov edx, esp
	push eax	; "ls -la"
	push edx	; "-c"
	push ecx	; 0
	push ebx	; "/bin//sh"
	mov ecx, esp
	
	xor edx, edx
	xor eax, eax
	mov al, 0x0b
	int 0x80
