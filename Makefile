all: kernel boot image

.PHONY: image
image: kernel boot
	dd if=/dev/zero of=floppy.img bs=512 count=2880
	mkfs.msdos floppy.img
	sudo mount floppy.img /mnt -o loop,umask=0
	dd if=boot/boot.bin of=floppy.img bs=512 conv=notrunc
	-cp kernel/kernel.bin /mnt
	-cp kernel/symbols.bin /mnt
	sudo umount /mnt

.PHONY: floppy
image.win: kernel boot
	-rm floppy.img
	./fat_imgen -c -f floppy.img -s boot/boot.bin
	./fat_imgen -m -f floppy.img -i kernel/kernel.bin -n kernel.bin
	./fat_imgen -m -f floppy.img -i kernel/symbols.bin -n symbols.bin

.PHONY: kernel
kernel: init
	make -C kernel/

.PHONY: boot
boot:
	make -C boot/

.PHONY: init
init:
	make -C init/

.PHONY: clean
clean:
	make -C kernel/ clean
	-rm `find -name "*.o"`
	-rm `find -name "*.bin"`


