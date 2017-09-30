all: kernel boot image versions

floppyclean.img:
	dd if=/dev/zero of=floppyclean.img bs=512 count=2880
	mkfs.msdos floppyclean.img

.PHONY: versions
versions:
	md5sum boot/boot.bin kernel/kernel.bin kernel/symbols.bin init/init.bin

.PHONY: image
image: kernel boot floppyclean.img
	cp floppyclean.img floppy.img
	sudo mount floppy.img /mnt -o loop,umask=0
	-cp kernel/kernel.bin /mnt
	-cp init/init.bin /mnt
	-cp kernel/symbols.bin /mnt
	sudo umount /mnt
	dd if=boot/boot.bin of=floppy.img bs=512 conv=notrunc

.PHONY: floppy
image.win: kernel boot
	-rm floppy.img
	./fat_imgen -c -f floppy.img -s boot/boot.bin
	./fat_imgen -m -f floppy.img -i kernel/kernel.bin -n kernel.bin
	./fat_imgen -m -f floppy.img -i init/init.bin -n init.bin
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


