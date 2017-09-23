USE16 86

jmp start
nop

BYTES_PER_SECTOR: EQU 512
SECTORS_PER_CLUSTER: EQU 1
RESERVED_SECTOR_COUNT: EQU 1
SECTOR_COUNT: EQU 2880
SECTORS_PER_FAT: EQU 9
SECTORS_PER_TRACK: EQU 18
HEAD_COUNT: EQU 2
FAT_COUNT: EQU 2
ROOT_ENTRY_COUNT: EQU 224
ROOT_DIR_SECTOR_COUNT: EQU ((ROOT_ENTRY_COUNT * 32) + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR
FAT_SECTOR_COUNT: EQU (FAT_COUNT * SECTORS_PER_FAT)

oem_name:
	.ascii 'RMMTOS  '
	dw BYTES_PER_SECTOR
	db SECTORS_PER_CLUSTER
	dw RESERVED_SECTOR_COUNT
	db FAT_COUNT
	dw ROOT_ENTRY_COUNT
	dw SECTOR_COUNT
media:
	db 0xF0
	dw SECTORS_PER_FAT
sectors_per_track:
	dw SECTORS_PER_TRACK
head_count:
	dw HEAD_COUNT
hidden_sectors:
	dd 0
SECTOR_COUNT_32:
	dd 0
;FAT16 and FAT12 structure
drive_num:
	db 0
	db 0 ;Reserved
boot_signature:
	db 0x29
volume_id:
	.ascii  'RMMT'
volume_label:
	.ascii  'RMMTOSv0.01'
filesystem_type:
	.ascii  'FAT12   '

start:
	mov ax, #0x07c0
	mov ds, ax
	mov es, ax
	;:mov [drive], dx
fat_read:
	; FAT begins after the reserved sectors
        mov ax, #RESERVED_SECTOR_COUNT
	mov bx, #fat
	mov cx, #SECTORS_PER_FAT
        call read_blocks
load_root_dir:
	; root directory begins after the FATs
        mov ax, #(RESERVED_SECTOR_COUNT + FAT_SECTOR_COUNT)
	mov bx, #dir
        mov cx, #ROOT_DIR_SECTOR_COUNT
        call read_blocks

load_files:
	mov cx, #3
	mov bx, #files
.load:
	push cx
	push bx

	call load_file

	pop bx
	add bx, #14
	pop cx

	loop .load

jump_to_kernel:
	cli

	mov ax, #0x0050
	mov ds,ax
	mov es,ax

	jmp 0x0050:0

load_file:
	mov si, bx
	call print
	mov si, #tdot
	call print

search_dir:
	mov cx, #ROOT_ENTRY_COUNT
	mov ax, #dir

check_entry:
	; save the count of remaining entries
	push cx
	push ax

	mov di, ax
	mov si, bx
	mov cx, #11
	repe
	cmpsb

	jne .next_entry

	pop ax
	push es

	; save the directory entry address
	mov cx, ax

	; set the file buffer segment
	mov ax, 12[bx]
	mov es, ax

	; move directory entry address to correct register
	mov bx, cx

	call read_file

	pop es

	; print newline to separate filenames
	mov si, #newline
	call print

	pop cx
	ret

.next_entry:
	pop ax
	add ax, #32
	; restore entry count
	pop cx
	loop check_entry

	mov si, #filenotfound_msg
	call panic

; Reads file from FAT filesystem.
;
; Arguments:
; ds:bx = directory entry of the file
; es:0 = file buffer

; first cluster is after the root directory
; two first cluster indexes are not used for data
CLUSTERS_START: EQU RESERVED_SECTOR_COUNT + FAT_SECTOR_COUNT + ROOT_DIR_SECTOR_COUNT - 2

read_file:
	mov ax, 26[bx]

.read:
	; print dot per cluster
	mov si, #dot
	call print

	; save the cluster number
	push ax

	; convert cluster number to lba number
	add ax, #CLUSTERS_START

	xor bx, bx
	mov cx, #SECTORS_PER_CLUSTER
	call read_blocks

	; update destination segment register
	mov ax, es
	add ax, #((BYTES_PER_SECTOR * SECTORS_PER_CLUSTER) / 16)
	mov es, ax

	pop ax
.next_cluster:
	push dx
	mov bx, #3
	mul bx
	xor dx, dx
	mov bx, #2
	div bx

	mov bx, ax
	add bx, #fat

	mov ax, [bx]

	test dx, dx
	jz .even_c
.odd_c:
	mov cl, #4
	shr ax, cl
	jmp .check_end
.even_c:
	and ax, #0x0FFF

.check_end:
	pop dx
	cmp ax, #0x0FF8
	jb .read
	ret

; Reads blocks from the disk.
;
; Arguments:
; ax = number of first LBA block
; es:bx = buffer address
; cx = block count
; dl = drive number

read_blocks:
	push cx
	; save drive number
	push dx

	; set dx to zero as the division operation uses it as a part of an operand
	xor dx, dx

	div word sectors_per_track

	; move sector number to the correct register
	; sector = (lba % SECTORS_PER_TRACK) + 1
	mov cl, dl
	inc cl

	; set dx to zero as the division operation uses it as a part of an operand
	xor dx, dx

	div word head_count

	; move head number to the correct register
	; head = lba / SECTORS_PER_TRACK % HEAD_COUNT
	mov dh, dl

	; move cylinder number to the correct register
	; cylinder = lba / SECTORS_PER_TRACK / HEAD_COUNT
	mov ch, al

	; restore drive number
	pop ax
	mov dl, al

	; restore sector count
	pop ax

	; set the operation number
	mov ah, #0x02

.retry:
	; save ax for retries
	push ax

	; read the sectors
	int #0x13

	jnc .ok

	; reset the drive if the read failed
	dec byte [resetsleft]
	jz .panic

	xor ah, ah
	int #0x13

	; restore ax for the retry
	pop ax

	jmp .retry
.panic:
	mov si, #error_msg
	call panic
.ok:
	pop ax
	ret

; Print string and halt
;
; Arguments:
; si = address of the string
; ds = segment of the string
;
; Returns:
;
panic:
	call print
	cli
	hlt

; Print string
;
; Arguments:
; si = address of the string
; ds = segment of the string
;
; Returns:
;
print:
	push ax
	mov ah, #0x0E
.loop:
	lodsb
	cmp al, #0
	je .end
	int  #0x10
	jmp .loop
.end:
	pop ax
	ret

files:
	.asciz 'KERNEL  BIN'
	dw 0x0050
	.asciz 'INIT    BIN'
	dw 0x1050
        .asciz 'SYMBOLS BIN'
	dw 0x2050

resetsleft:
	db 25
tdot:
	.ascii '..'
dot:
	.asciz '.'
newline:
	db 10
	db 13
	db 0
filenotfound_msg:
	.asciz 'missing'
error_msg:
	.asciz 'error'

BLOCK 440
	.space 70
	dw 0xAA55
ENDB

drive: EQU 0x200 - 2
fat: EQU drive + 2
dir: EQU fat + SECTORS_PER_FAT * 0x200
