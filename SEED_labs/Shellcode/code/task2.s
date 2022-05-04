section .text
  global _start
    _start:
		BITS 32
		jmp short two		
    one:
		pop ebx	
		xor eax, eax
		mov [ebx+12], al	; 第一个'*'替换为'\x00'
		mov [ebx+23], ebx 
		mov [ebx+27], eax
		lea ecx, [ebx+23] 
		xor edx, edx
		mov edi,ebx
		mov [ebx+17], al	
		mov [ebx+22], al	
		add edi, 13
		mov [ebx+31], edi 
		add edi, 5
		mov [ebx+35], edi 
		mov [ebx+39], eax
		lea edx, [ebx+31]
		mov al,  0x0b
		int 0x80	
    two:
		call one	
		db '/usr/bin/env*a=11*b=22*AAAABBBBCCCCDDDDEEEE' 
