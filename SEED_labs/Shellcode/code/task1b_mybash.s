section .text
  global _start
    _start:
      ; Store the argument string on stack
      xor  edx, edx
      mov edx, "h###"
      shl edx, 24
      shr edx, 24 
      push edx
      
      push "/bas"
      push "/bin"
      mov  ebx, esp     ; Get the string address

      ; Construct the argument array argv[]
      push eax          ; argv[1] = 0
      push ebx          ; argv[0] points "/bin/sh "
      mov  ecx, esp     ; Get the address of argv[]
   
      ; For environment variable 
      xor  edx, edx     ; No env variables 

      ; Invoke execve()
      xor  eax, eax     ; eax = 0x00000000
      mov   al, 0x0b    ; eax = 0x0000000b
      int 0x80