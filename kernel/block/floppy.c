#include "floppy.h"
#include "io.h"
#include "task.h"
#include "fs/block.h"
#include "panic.h"

unsigned int floppy_read(struct File* file, char* buffer, Uint64* block);
unsigned int floppy_write(struct File* file, char* buffer, Uint64* block);

static struct BlockFileOps floppy_ops =
{
	floppy_read,
	0
};

#define DRIVES 2
#define IO_BASE 0x3f0
#define IO_DOR IO_BASE + 2
#define IO_DSR IO_BASE + 4
#define IO_MSR IO_BASE + 4
#define IO_FIFO IO_BASE + 5
#define IO_CCR IO_BASE + 7

#define CMD_SENSE 0x08
#define CMD_SPECIFY 0x03
#define CMD_CONFIGURE 0x13
#define CMD_RECALIBRATE 0x07
#define CMD_SEEK 0x0f
#define CMD_READ 0x46
#define CMD_WRITE 0x45

static char drives[DRIVES];

#define CCR_DATA_RATE 0x00

extern struct Task* current_task;

#define floppydebug debug_printf

static int floppy_reset()
{
	int i;
	floppydebug("reset\t");

	outb(IO_DSR, 0x80);

	return 1;
}

static int floppy_set_data_rate()
{
	floppydebug("drate\t");
	outb(IO_CCR, CCR_DATA_RATE);
}

static int floppy_write_cmd(unsigned char byte)
{
	int i = 0;
	unsigned char msr;

	floppydebug("wrcmd\t");
	while(((msr = inb(IO_MSR)) & 0xc0) != 0x80)
	{
		if(i++ > 30) {
			floppydebug("floppy_write_cmd msr was %a\n", msr);

			return 0;
		}
	}

	outb(IO_FIFO, byte);
	return 1;
}

static int floppy_read_result(unsigned char* byte)
{
	int i = 0;
	unsigned char msr;

	floppydebug("rdres\t");
	while(((msr = inb(IO_MSR)) & 0xc0) != 0xc0)
	{
		if(i++ > 30) {
			floppydebug("floppy_read_result msr was %a\n", msr);

			return 0;
		}
	}

	*byte = inb(IO_FIFO);
	return 1;
}

static int floppy_sense()
{
	unsigned char status0;
	unsigned char cylinder;

	floppydebug("sense\t");
	return floppy_write_cmd(CMD_SENSE)
		&& floppy_read_result(&status0)
		&& floppy_read_result(&cylinder);

}

#define IMPLIED_SEEK 0
#define FIFO_DISABLE 0
#define POLLING_DISABLE 1
#define TRESHOLD 0

static int floppy_configure()
{
	unsigned char b2 = 0;

	b2 |= IMPLIED_SEEK << 6;
	b2 |= FIFO_DISABLE << 5;
	b2 |= POLLING_DISABLE << 4;
	b2 |= TRESHOLD;

	floppydebug("conf\t");

	return floppy_write_cmd(CMD_CONFIGURE)
		&& floppy_write_cmd(0)
		&& floppy_write_cmd(b2)
		&& floppy_write_cmd(0);
}

#define STEP_RATE 8
#define HEAD_LOAD_TIME 5
#define HEAD_UNLOAD_TIME 0
#define GAP_LENGTH 0x1b

static int floppy_specify()
{
	floppydebug("spec\t");

	return floppy_write_cmd(CMD_SPECIFY)
		&& floppy_write_cmd((STEP_RATE << 4) | (HEAD_UNLOAD_TIME & 0xf))
		&& floppy_write_cmd((HEAD_LOAD_TIME << 1));
}

static int selected = 0;

static int floppy_select_drive()
{
	floppydebug("sel\t");

	outb(IO_DOR, 0x1C);

	clock_set_wakeup(current_task, 500);
	task_sleep(current_task);

	selected = 1;

	return 1;
}

static int floppy_recalibrate()
{
	int res = 1;

	floppydebug("recal\t");

	irq_set_wakeup(current_task, 6);

	res = floppy_write_cmd(CMD_RECALIBRATE)
		&& floppy_write_cmd(0);

	task_sleep(current_task);

	return res && floppy_sense();
}

static int floppy_seek(unsigned char cylinder, unsigned char head)
{
	int res = 1;

	floppydebug("seek\t");

	irq_set_wakeup(current_task, 6);

	res = floppy_write_cmd(CMD_SEEK)
		&& floppy_write_cmd(0 | ((head & 0x1) << 2))
		&& floppy_write_cmd(cylinder);

	task_sleep(current_task);

	return res && floppy_sense();
}

#define WRITE 0x2
#define READ 0x1

#define KERNEL_BASE 0x500

static int floppy_set_dma(unsigned int buffer, unsigned int count, int dir)
{
	floppydebug("set_dma\t");

	buffer += KERNEL_BASE;

	/* mask channel */
	outb(0x0a, 0x4 | 0x2);

	/* reset flip-flop */
	outb(0x0c, 0xff);

	/* set address */
	outb(0x04, buffer & 0xff);
	outb(0x04, buffer >> 8);

	outb(0x81, 0);

	/* reset flip-flop */
	outb(0x0c, 0xff);

	/* set count */
	outb(0x05, count & 0xff);
	outb(0x05, count >> 8);

	/* set mode */
	outb(0x0b, (0x01 << 6) | (dir << 2) | 0x02);

	/* unmask channel*/
	outb(0x0a, 0x2);

	return 1;
}

static int floppy_transfer(int dir, unsigned char cyl, unsigned char head, unsigned char sec)
{
	unsigned char command;
	unsigned char st0, st1, st2, rcy, rhe, rse, bps;
	int res;
	floppydebug("xfer\t");

	if(dir == WRITE)
		command = CMD_WRITE;
	else
		command = CMD_READ;

	irq_set_wakeup(current_task, 6);

	res = floppy_write_cmd(command)
		&& floppy_write_cmd((head & 0x1) << 2)
		&& floppy_write_cmd(cyl)
		&& floppy_write_cmd(head)
		&& floppy_write_cmd(sec)
		&& floppy_write_cmd(0x02)
		&& floppy_write_cmd(1)
		&& floppy_write_cmd(GAP_LENGTH)
		&& floppy_write_cmd(0xff);

	task_sleep(current_task);

	return res
		&& floppy_read_result(&st0)
		&& floppy_read_result(&st1)
		&& floppy_read_result(&st2)
		&& floppy_read_result(&rcy)
		&& floppy_read_result(&rhe)
		&& floppy_read_result(&rse)
		&& floppy_read_result(&bps);
}

#define SEC 18
#define HD 2
#define CYL 80

int floppy_init2();

#define SEC_POT 9
#define SEC_SZ (1 << SEC_POT)

unsigned int floppy_read(struct File* node, char* buffer, Uint64* block)
{
	unsigned int lba = block->i[0];
	unsigned int cylinder = lba / (SEC * HD);
	unsigned int head = (lba / SEC) % HD;
	unsigned int sector = (lba % SEC) + 1;
	int res = 0;

	while(res == 0) {
		res = floppy_init2()
			&& floppy_select_drive()
			&& floppy_specify()
			&& floppy_set_data_rate()
			&& floppy_recalibrate()
			&& floppy_seek(cylinder, head)
			&& floppy_set_dma(buffer, SEC_SZ, READ)
			&& floppy_transfer(READ, cylinder, head, sector);

		if(!res) {
			clock_set_wakeup(current_task, 1000);
			task_sleep(current_task);
		}
	}

	return res;
}

int floppy_init2()
{
	unsigned int i;
	int res = 1;

	irq_set_wakeup(current_task, 6);

	res = floppy_reset();

	task_sleep(current_task);

	for(i = 0; i < 4; i++)
	{
		res = res && floppy_sense();
	}

	return res
		&& floppy_configure()
		&& floppy_specify();
}

void floppy_create_files()
{
	struct Directory* devfs = devfs_get_root();
	struct DirectoryHandle* handle = directory_open(devfs, HANDLE_WRITE);
	struct BlockFile* block_file;
	struct File* file;

	if(!handle)
		return;

	if(drives[0] == 0x04)
	{
		file = blockfile_create_node("floppy0", STATIC, &floppy_ops, SEC_POT);

		block_file = file;

		if(file)
			directory_add_node(handle, file);
	}

	if(drives[1] == 0x04)
	{
		file = blockfile_create_node("floppy1", STATIC, &floppy_ops, SEC_POT);

		if(file)
			directory_add_node(handle, file);
	}

	directory_close(handle);
}

void floppy_init()
{
	unsigned char t;

	outb(0x70, 0x10);

	t = inb(0x71);

	if(t >> 4 == 0)
		return;

	drives[0] = t>>4;
	drives[1] = t & 0xf;

	floppy_create_files();
}
