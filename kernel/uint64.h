#ifndef UINT64_H
#define UINT64_H

typedef struct
{
	unsigned int i[4];
}Uint64;

void init64_32(Uint64* p, unsigned long h, unsigned long l);
void init64(Uint64* p, unsigned int hh, unsigned int hl, unsigned int lh, unsigned int ll);
int cmp64(Uint64* a, Uint64* b);
void inc64(Uint64* a);
void set64(Uint64* a, Uint64* b);
void add64(Uint64* a, Uint64* b);
void sub64(Uint64* a, Uint64* b);
void shl64(Uint64* a, unsigned int n);

#endif

