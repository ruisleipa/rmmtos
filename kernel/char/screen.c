#include "screen.h"
#include "printf.h"
#include "string.h"
#include "fs/devfs/devfs.h"
#include "fs/file.h"
#include "mutex.h"

#define SCREEN_WIDTH	80
#define SCREEN_HEIGHT	25

static unsigned int col=0;
static unsigned int row=0;
static unsigned char attr=0xF0;
static unsigned int size=SCREEN_WIDTH*SCREEN_HEIGHT*2;

static unsigned int cur_col=0;
static unsigned int cur_row=0;

static MUTEX(screen_lock);

static struct FileOps screen_ops =
{
	0,

	0,
	screen_write
};

static void set_cursor(unsigned int col,unsigned int row)
{
	unsigned int pos=(row*SCREEN_WIDTH+col);

	outb(0x3D4,14);
	outb(0x3D5,pos>>8);
	outb(0x3D4,15);
	outb(0x3D5,pos);
}

static void scroll()
{
	unsigned int r=SCREEN_WIDTH*2;
	unsigned int empty=' '|attr<<8;

	farmemcpy(0,r,r*(SCREEN_HEIGHT-1),0xb800,0xb800);
	farmemsetw(r*(SCREEN_HEIGHT-1),empty,r,0xb800);
}

static void newline()
{
	if(row==SCREEN_HEIGHT-1)
		scroll();
	else
		row++;

	col=0;
}

static void tab()
{
	col=(col+8)&~(8-1);

	if(col==SCREEN_WIDTH)
		newline();
}

static void backspace()
{
	unsigned int index;

	if(col>0)
		col--;

	index=(row*SCREEN_WIDTH+col)*2;

	poke(0xb800,index,' ');
}

void screen_putch(char c)
{
	unsigned int index=(row*SCREEN_WIDTH+col)*2;

	switch(c)
	{
		case '\n':
			newline();
			break;
		case '\t':
			tab();
			break;
		case '\b':
			backspace();
			break;
		default:
			poke(0xb800,index,c);

			col++;

			if(col==SCREEN_WIDTH)
				newline();
			break;
	}

	set_cursor(col,row);
}

static void clear_screen()
{
	unsigned int i;

	for(i=0;i<size;i+=2)
	{
		poke(0xb800,i,' ');
		poke(0xb800,i+1,attr);
	}
}

void screen_init()
{
	clear_screen();
	set_cursor(col,row);
	putch_set(screen_putch);
}

void screen_init_device()
{
	struct Directory* devfs = devfs_get_root();
	struct DirectoryHandle* handle = directory_open(devfs, HANDLE_WRITE);
	struct File* screen = file_create_node("screen", STATIC, &screen_ops);

	if(handle)
	{
		if(screen)
			directory_add_node(handle, screen);

		directory_close(handle);
	}
}

static MUTEX(screen_mutex);

unsigned int screen_write(struct FileHandle* handle, char* buffer, unsigned int size)
{
	unsigned int i;

	/*lock(screen_mutex);
	*/
	for(i = 0; i < size; i++)
		screen_putch(buffer[i]);
	/*
	unlock(screen_mutex);
	*/
	return i;
}

