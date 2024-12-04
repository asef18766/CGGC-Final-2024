all:
	gcc main.c chacha20.c -o ../dist/main.exe -lIPHLPAPI -lws2_32
	strip main.exe

	gcc readfile_shellcode.c loadfunc.c -o readfile_shellcode.exe
	objdump.exe -D -F readfile_shellcode.exe -D > shellcode.asm
	python.exe shellcraft.py