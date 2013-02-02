#ifndef __STRING_H
#define __STRING_H

char* strcpy(char* dest,char* src);
int strcmp(char* str1,char* str2);

char* farmemcpy(char* dest,char* src,int count,unsigned int destseg,unsigned int srcseg);
char* memcpy(char* dest, char* src, int count);

char* farmemset(char* dest,unsigned int val,int count,unsigned int seg);
char* memset(char* dest,unsigned int val,int count);

char* farmemsetw(char* dest,unsigned int val,int count,unsigned int seg);
#define memsetw(dest,val,count) farmemsetw(dest,val,count,0x0050)

unsigned int strlen(char* str);

int atoi(char* str);

#define isdigit(x) ((x)>='0' && (x)<='9')
#define isalpha(x) ((x)>='A' && (x)<='z')
#define isalnum(x) (isdigit(x) || isalpha(x))
#define isspace(x) ((x)==0x20 || (x)==0x09 || ((x)>=0x0a && (x)<=0x0d) )

#endif
