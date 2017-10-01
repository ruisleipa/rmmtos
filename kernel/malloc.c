#include "malloc.h"
#include "string.h"
#include "panic.h"
#include "task.h"

static unsigned int used_bytes;
static unsigned int total_bytes;
unsigned int total_blocks;
void* pool_start;

#define SIZE_COUNT 6


struct size_block {
	unsigned int size;
	unsigned int count;
	unsigned int expand;
};


struct size_block zones[SIZE_COUNT] = {
	{4, 0, 1},
	{8, 0, 1},
	{18, 0, 3},
	{36, 0, 1},
	{512, 8, 0},
	{KERNEL_STACK_WORDS * sizeof(unsigned int), 2, 0}
};

#define BITMAP_SIZE 1000

//TODO: check if really zero allocated
unsigned char bitmap[BITMAP_SIZE] = {0};

void malloc_stats()
{
	printf("Heap usage: %d/%d B in %d blocks\n", used_bytes, total_bytes, total_blocks);
}

void* malloc(unsigned int size)
{
	int current_block = 0;
	unsigned int address = (unsigned int)pool_start;
	int i = 0;
	int j = 0;

	while(zones[i].size < size && i < SIZE_COUNT) {
		debug_printf("j%x\t", zones[i].count);
		current_block += zones[i].count;

		i++;
	}



	while(bitmap[current_block / 8] == 0xff && current_block < total_blocks) {
		// move to first block of next byte
		current_block = (current_block + 8) & (~0x7);
	}

	debug_printf("b%a\t", bitmap[current_block / 8]);
	debug_printf("m%a\t", (0x80 >> (current_block % 8)));
	debug_printf("r%x\t", bitmap[current_block / 8] & (0x80 >> (current_block % 8)));
	debug_printf("s%x\t", current_block);

	while(current_block < total_blocks && (bitmap[current_block / 8] & (0x80 >> (current_block % 8))) != 0) {
		debug_printf("s%x\t", current_block);
		current_block++;
	}

	debug_printf("u%x\t", current_block);

	// block found, get an address for it
	if(current_block < total_blocks) {
		int j = 0;

		for(i = 0; i < SIZE_COUNT; ++i) {
			if(j + zones[i].count <= current_block) {
				j += zones[i].count;
				address += zones[i].count * zones[i].size;
			} else {
				address += zones[i].size * (current_block - j);
				break;
			}
		}

		bitmap[current_block / 8] |= (0x80 >> (current_block % 8));
		used_bytes += zones[i].size;

		debug_printf("\nalloc %x %x\n", address, current_block);

		return (void*)address;
	}

	malloc_info();
	printf("alloc: %d\n", size);
	panic("out of mem");

	return 0;
}

void* realloc(void* block, unsigned int new_size)
{
	void* new_block = malloc(new_size);

	if(new_block)
	{
		memcpy(new_block, block, new_size);

		free(block);
	}

	return new_block;
}

void free(void* block)
{
	unsigned int address = (unsigned int)block - (unsigned int)pool_start;
	unsigned int current_block = 0;
	unsigned int zone = 0;

	while(address >= zones[zone].count * zones[zone].size && zone < SIZE_COUNT) {
		address -= zones[zone].count * zones[zone].size;
		current_block += zones[zone].count;
		zone++;
	}

	if(zone == SIZE_COUNT) {
		panic("invalid free");
	}

	current_block += address / zones[zone].size;

	// mark as free
	bitmap[current_block / 8] ^= (0x80 >> (current_block % 8));
	used_bytes -= zones[zone].size;

	debug_printf("freed %x %x\n", block, current_block);
}


void malloc_init(void* start,void* end)
{
	unsigned int allocated_size = 0;
	unsigned int pool_size;
	unsigned int first_phase_increment = 0;
	int i;

	pool_start = start;

	// TODO: check this
	pool_size=(unsigned int)end-(unsigned int)start	;

	for (i = 0; i < SIZE_COUNT; ++i) {
		allocated_size += zones[i].size * zones[i].count;
		first_phase_increment += zones[i].size * zones[i].expand;
	}

	while (allocated_size + first_phase_increment <= pool_size) {
		for (i = 0; i < SIZE_COUNT; ++i) {
			zones[i].count += zones[i].expand;
		}

		allocated_size += first_phase_increment;
	}

	while (allocated_size + zones[0].size <= pool_size) {
		zones[0].count++;
		allocated_size += zones[0].size;
	}

	total_blocks = 0;

	for (i = 0; i < SIZE_COUNT; ++i) {
		total_blocks += zones[i].count;
	}

	if(total_blocks > BITMAP_SIZE * sizeof(unsigned char) * 8) {
		panic("malloc bitmap");
	}

	memset(bitmap, 0, BITMAP_SIZE);

	used_bytes = 0;
	total_bytes = allocated_size;

	malloc_info();
}

void malloc_info_terse()
{
	int i;

	debug_printf("bitmap: ");

	for(i = 0; i < total_blocks / 8 + 1; ++i) {
		debug_printf("%a  ", bitmap[i]);
	}

	debug_printf("\n");
}

void malloc_info()
{
	int i;

	debug_printf("malloc zones:\n");

	for(i = 0; i < SIZE_COUNT; ++i) {
		debug_printf("%d blocks of %d bytes = %d bytes in total\n", zones[i].count, zones[i].size, zones[i].size * zones[i].count);
	}

	malloc_stats();
}

