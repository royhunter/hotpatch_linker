# Hotpatch_linker
Hotpatch_linker is a project which provide a tool for link and pack a patch package.


## Building
Compile x86_64 Arch:

	#make x86_64

Compile mips32 Arch:
	
	#make mips32

Compile all:

	#make all

## Executable File

	# ls bin/
	mips32/ x86_64/ 
	#ls bin/mips32/
	mips32_linker
	#ls bin/x86_64/
	x86_64_linker


## Usage:
```bash
	Usage:
		Arch_linker [options] -o obj_filename  -e executable_filename
        Arch: x86_64,mips32

    Options:
        -d  --debug             Output debug info
        -v, --version           Show version
        -h, --help              Show this help
        -o obj,  --obj=NAME     Set obj file name
        -e exec, --exec=NAME    Set exec file name
```