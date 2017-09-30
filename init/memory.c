#include "memory.h"

struct BlockHeader
{
	struct BlockHeader* prev;
	struct BlockHeader* next;
	unsigned int free:1;
};

struct BlockHeader* first=0;

/* this is the address of the byte after the pool */
struct BlockHeader* overblock = 0;

static void mergeFreeBlocks(struct BlockHeader* block)
{
	struct BlockHeader* first_free=block;
	struct BlockHeader* last_free=block;

	while(first_free->prev && first_free->prev->free)
		first_free=first_free->prev;

	while(last_free != overblock && last_free->free)
		last_free=last_free->next;

	first_free->next=last_free;
	last_free->prev=first_free;
}

void malloc_init(void* start,void* end)
{
	unsigned int size;

	first=start;
	overblock=(void*)(((char*)end)+1);

	size=(unsigned int)overblock-(unsigned int)first-1;

	clearMemory(first, size);

	first->prev = 0;
	first->next = overblock;
	first->free = 1;
}

void* allocateMemory(unsigned int size)
{
	struct BlockHeader* curr;
	unsigned int curr_size;

	curr = first;

	do
	{
		if(curr==overblock)
			break;

		if(!curr->free)
			continue;

		curr_size=(unsigned int)curr->next-(unsigned int)curr;
		curr_size-=sizeof(struct BlockHeader);

		/* OK, free block found */

		if(curr_size<size)
			continue;

		/* OK, the block is large enough */
		/* Now we find out if the block can be split */

		if(curr_size>sizeof(struct BlockHeader)+size)
		{
			/* do splitting */

			struct BlockHeader* new_head=(char*)curr+sizeof(struct BlockHeader)
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

		return (void*)((unsigned int)curr+sizeof(struct BlockHeader));
	}
	while(curr=curr->next);

	return 0;
}

void freeMemory(void* block)
{
	struct BlockHeader* curr=first;

	do
	{
		if(curr==overblock)
			break;

		if(curr==(char*)block-sizeof(struct BlockHeader))
		{
			curr->free = 1;

			mergeFreeBlocks(curr);

			break;
		}
	}
	while(curr=curr->next);
}

void clearMemory(void* ptr, unsigned int size)
{
	char* p = ptr;

	while(size-- > 0)
	{
		*p = 0;

		p++;
	}
}
