all: boot.bin

boot.bin: boot.s
	as86 -b boot.bin -s symbols.txt -l list.txt boot.s
