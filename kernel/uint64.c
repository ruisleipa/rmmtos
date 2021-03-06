#include "uint64.h"
#include "panic.h"

void init64_32(Uint64* p, unsigned long h, unsigned long l)
{
	init64(p, (h & 0xffff0000) >> 16, h & 0xffff, (l & 0xffff0000) >> 16, l & 0xffff);
}

void init64(Uint64* p, unsigned int hh, unsigned int hl, unsigned int lh, unsigned int ll)
{
	p->i[0] = ll;
	p->i[1] = lh;
	p->i[2] = hl;
	p->i[3] = hh;
}

int cmp64_16(Uint64* a, unsigned int b)
{
	int i;

	for(i = 3; i > 0; --i)
	{
		if(a->i[i] > 0)
			return 1;
	}

	if(a->i[i] > b)
		return 1;
	if(a->i[i] < b)
		return -1;

	return 0;
}

int cmp64(Uint64* a, Uint64* b)
{
	int i;

	for(i = 3; i >= 0; --i)
	{
		if(a->i[i] > b->i[i])
			return 1;
		if(a->i[i] < b->i[i])
			return -1;
	}

	return 0;
}

void inc64(Uint64* a)
{
	unsigned int carry = 1;
	unsigned int i;

	for(i = 0; i < 4; ++i)
	{
		a->i[i] += carry;
		carry = (a->i[i] < carry);
	}
}

void set64(Uint64* a, Uint64* b)
{
	a->i[0] = b->i[0];
	a->i[1] = b->i[1];
	a->i[2] = b->i[2];
	a->i[3] = b->i[3];
}

void add64(Uint64* a, Uint64* b)
{
	unsigned int carry = 0;
	unsigned int t = 0;
	unsigned int i;

	for(i = 0; i < 4; ++i)
	{
		t = b->i[i] + carry;
		a->i[i] += t;
		carry = (a->i[i] < t);
	}
}

void add64_16(Uint64* a, unsigned int b)
{
	unsigned int carry = 0;
	unsigned int t = 0;
	unsigned int i;

	for(i = 0; i < 4; ++i)
	{
		t = b + carry;
		b = 0;
		a->i[i] += t;
		carry = (a->i[i] < t);
	}
}

void dec64(Uint64* a)
{
	unsigned int carry = 0;
	unsigned int t = 0;
	unsigned int i;

	t = ~(1) + 1 - carry;
	a->i[0] += t;
	carry = (a->i[0] > t);

	for(i = 1; i < 4; ++i)
	{
		t = ~(0) + 1 - carry;
		a->i[i] += t;
		carry = (a->i[i] > t);
		t = 0;
	}
}

void sub64(Uint64* a, Uint64* b)
{
	unsigned int carry = 0;
	unsigned int t = 0;
	unsigned int i;

	for(i = 0; i < 4; ++i)
	{
		t = ~(b->i[i]) + 1 - carry;
		a->i[i] += t;
		carry = (a->i[i] > t);
	}
}

unsigned int shr64(Uint64* p, unsigned int n)
{
	unsigned int shifted = 0;

	if(n > 16)
		panic("too large shr64, fix this");

	while(n > 16)
	{
		shifted = p->i[0];
		p->i[0] = p->i[1];
		p->i[1] = p->i[2];
		p->i[2] = p->i[3];
		p->i[3] = 0;

		n -= 16;
	}

	shifted = (p->i[0] << (16 - n)) >> (16 - n);

	p->i[0] >>= n;
	p->i[0] |= p->i[1] << (16 - n);

	p->i[1] >>= n;
	p->i[1] |= p->i[2] << (16 - n);

	p->i[2] >>= n;
	p->i[2] |= p->i[3] << (16 - n);

	p->i[3] >>= n;

	return shifted;
}

void shl64(Uint64* p, unsigned int n)
{
	while(n > 16)
	{
		p->i[3] = p->i[2];
		p->i[2] = p->i[1];
		p->i[1] = p->i[0];
		p->i[0] = 0;

		n -= 16;
	}

	p->i[3] <<= n;
	p->i[3] |= p->i[2] >> (16 - n);

	p->i[2] <<= n;
	p->i[2] |= p->i[1] >> (16 - n);

	p->i[1] <<= n;
	p->i[1] |= p->i[0] >> (16 - n);

	p->i[0] <<= n;
}
