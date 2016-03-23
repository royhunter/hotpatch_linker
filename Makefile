mips32:
	make -f makefile.mips32
	
	
x86_64:
	make -f makefile.x86_64
	
clean:
	make -f makefile.x86_64 clean
	make -f makefile.mips32 clean
