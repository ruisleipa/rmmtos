#include "malloc.h"
#include "string.h"
#include "panic.h"

struct head
{
	struct head* prev;
	struct head* next;
	unsigned int free:1;
};

struct head* first;

/* this is the address of the byte after the pool */
struct head* overblock;

static unsigned int used_bytes;
static unsigned int total_bytes;

void malloc_stats()
{
	unsigned int total_blocks = 0;
	struct head* curr=first;

	do
	{
		total_blocks++;

		if(curr==overblock)
			break;
	}
	while(curr=curr->next);

	printf("Heap usage: %d/%d B in %d blocks\n", used_bytes, total_bytes, total_blocks);
}

void merge_free_blocks(struct head* block)
{
	struct head* first_free=block;
	struct head* last_free=block;

	while(first_free->prev && first_free->prev->free)
		first_free=first_free->prev;

	while(last_free != overblock && last_free->free)
		last_free=last_free->next;

	first_free->next=last_free;
	last_free->prev=first_free;
}

void* malloc(unsigned int size)
{
	struct head* curr=first;
	unsigned int curr_size;

	do
	{
		if(curr==overblock)
			break;

		if(!curr->free)
			continue;

		curr_size=(unsigned int)curr->next-(unsigned int)curr;
		curr_size-=sizeof(struct head);

		/* OK, free block found */

		if(curr_size<size)
			continue;

		/* OK, the block is large enough */
		/* Now we find out if the block can be split */

		if(curr_size>sizeof(struct head)+size)
		{
			/* do splitting */

			struct head* new_head=(char*)curr+sizeof(struct head)
						+size;

			/* setup the new block header */
			new_head->next=curr->next;
			new_head->prev=curr;
			new_head->free=1;

			/* setup the next block */
			if(curr->next!=overblock)
				curr->next->prev=new_head;

			/* set the splitted block */
			curr->next=new_head;
		}

		curr->free=0;

		used_bytes += (unsigned int)curr->next-(unsigned int)curr;

		/*malloc_stats();*/

		return (void*)((unsigned int)curr+sizeof(struct head));
	}
	while(curr=curr->next);
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
	struct head* curr=first;

	do
	{
		if(curr==overblock)
			break;

		if(curr==(char*)block-sizeof(struct head))
		{
			curr->free = 1;

			used_bytes -= (unsigned int)curr->next-(unsigned int)curr;

			merge_free_blocks(curr);

			/*malloc_stats();*/

			break;
		}
	}
	while(curr=curr->next);
}

void malloc_init(void* start,void* end)
{
	unsigned int size;

	first=start;
	overblock=(void*)(((char*)end)+1);

	size=(unsigned int)overblock-(unsigned int)first-1;

	used_bytes = 0;
	total_bytes = size;

	/*printf("heap: %x-%x, %dB\n",start,end,size);
*/
	memset(first,0,size);

	first->prev=0;
	first->next=overblock;
	first->free=1;
}

void malloc_info_terse()
{
	struct head* curr=first;

	do
	{
		if(curr==overblock)
			break;

		printf("%d",curr->free);
	}
	while(curr=curr->next);

	printf("\n");
}

void malloc_info()
{
	struct head* curr=first;
	unsigned int curr_size;

	printf("Malloc info:\n");

	do
	{
		if(curr==overblock)
			break;

		curr_size=(unsigned int)curr->next-(unsigned int)curr;
		curr_size-=sizeof(struct head);

		printf("%x free: %d size: %d bytes next: %x prev: %x\n",curr,curr->free,curr_size,curr->next,curr->prev);
	}
	while(curr=curr->next);
}

