#include "keyb.h"

#include "int.h"

#include "kmap_us.h"

#include "fs/devfs/devfs.h"
#include "fs/file.h"

#define BUF_SIZE 8

static int buffer_used=0;
static char keyb_buf[BUF_SIZE];
static char is_esc=0;

static char shift_l=0;
static char shift_r=0;
static char altgr=0;
static char caps=0;

unsigned int keyb_read(struct FileHandle* handle, char* buffer, unsigned int size);

static struct FileOps keyb_ops =
{
	0,

	keyb_read,
	0,
};

void keyb_func(struct int_regs* regs)
{
	char c=inb(0x60);
	char make=!(c&0x80);
	char* keymap=keymap_normal;
	char is_mappable=0;

	if(buffer_used >= BUF_SIZE)
		return;

	c=c&0x7f;

	if(is_esc)
	{
		switch(c)
		{
			/* altgr */
			case 0x38:
				altgr=make;
				break;
		}

		return;
	}

	switch(c)
	{
		case 0x2a:
			shift_l=make;
			break;
		case 0x36:
			shift_r=make;
			break;
		case 0x3a:
			if(make)
				caps=!caps;
			break;
		case 0xe0:
			is_esc=make;
			break;
		default:
			is_mappable=1;
	}

	if(!is_mappable)
		return;

	if(caps)
		keymap=keymap_capslock;

	if(shift_l || shift_r)
	{
		if(caps)
			keymap=keymap_shift_capslock;
		else
			keymap=keymap_shift;
	}

	if(altgr)
		keymap=keymap_altgr;

	if(make)
	{
		keyb_buf[buffer_used] = keymap[c];
		buffer_used++;
	}
}

void keyb_init()
{
	struct Directory* devfs = devfs_get_root();
	struct DirectoryHandle* handle = directory_open(devfs, HANDLE_WRITE);
	struct File* keyb = file_create_node("keyboard", STATIC, &keyb_ops);

	irq_capture(1,keyb_func);

	if(handle)
	{
		if(keyb)
			directory_add_node(handle, keyb);

		directory_close(handle);
	}
}

unsigned int keyb_read(struct FileHandle* handle, char* buffer, unsigned int size)
{
	if(size > buffer_used)
		size = buffer_used;

	memcpy(buffer, keyb_buf, size);

	memcpy(keyb_buf, keyb_buf + size, BUF_SIZE - buffer_used);

	buffer_used -= size;

	return size;
}

