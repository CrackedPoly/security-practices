section .text
  global _start
    _start:
      ; Store the argument string on stack
      xor  eax, eax 
      push eax          ; Use 0 to terminate the string
      push "//sh"
      push "/bin"
      mov  ebx, esp     ; Get the string address

      ; Construct the argument array argv[]
      push eax          ; argv[1] = 0
      push ebx          ; argv[0] points "/bin//sh"
      mov  ecx, esp     ; Get the address of argv[]
   
      ; For environment variable 
      ;xor  edx, edx     ; No env variables 
      push eax
      mov edx, "4###"
      shl edx, 24
      shr edx, 24
      push edx
      push "=123"
      push "cccc"
      mov esi, esp
      push eax
      push "5678"
      push "bbb="
      mov edi, esp
      push eax
      push "1234"
      push "aaa="
      mov edx, esp
      
      push eax
      push esi
      push edi
      push edx
      mov edx, esp

      ; Invoke execve()
      xor  eax, eax     ; eax = 0x00000000
      mov   al, 0x0b    ; eax = 0x0000000b
      int 0x80
