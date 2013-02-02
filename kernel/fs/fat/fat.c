#include "fat.h"
#include "fs/file.h"
#include "fs/directory.h"

struct Parameters
{
	unsigned int bytes_per_sector;
	unsigned char sectors_per_cluster;
	unsigned int reserved_sectors;
	unsigned char fats;
	unsigned int root_entries;
	unsigned int sectors_per_fat;
	unsigned long hidden_sectors;
	unsigned long total_sectors;
	unsigned char extended_section[54];
};

struct DirectoryEntry
{
	unsigned char name[11];
	unsigned char attributes;
	unsigned char reserved;
	unsigned char creation_tenths;
	unsigned int creation_time;
	unsigned int creation_date;
	unsigned int access_date;
	unsigned int cluster_hi;
	unsigned int modify_time;
	unsigned int modify_date;
	unsigned int cluster_lo;
	unsigned long size;
};

struct FatRootDirectory
{
	struct Directory super;

	Uint64 root_begin;
	unsigned int root_entries;
	struct FileHandle* handle;
};

struct Parameters* fat_read_parameters(struct FileHandle* handle)
{
	char* mbr = malloc(512);
	struct Parameters* params = malloc(sizeof(struct Parameters));

	if(mbr)
	{

	}
}

void parse_params(struct Parameters* params, struct FatRootDirectory* directory)
{
	unsigned long root_start = params->reserved_sectors + (params->fats * params->sectors_per_fat) * params->bytes_per_sector;
	int i;
	char* a = params;

	printf("handle: %x%x\n", (root_start&0xffff0000) >> 16, root_start&0xffff);
	//printf("oem: %s\n", params->oem);
	printf("root_entries: %d\n", params->root_entries);
	printf("reserved_sectors: %d\n", params->reserved_sectors);
	printf("fats: %d\n", (unsigned int)params->fats);
	printf("sectors_per_fat: %d\n", params->sectors_per_fat);
	printf("bytes_per_sector: %d\n", params->bytes_per_sector);

	for(i = 0; i < sizeof(struct Parameters); i++)
	{
		if(i % 16 == 0 && i != 0)
			printf("\n");

		printf("%a ", a[i]);
	}

	directory->root_entries = params->root_entries;
	init64(&directory->root_begin, 0, root_start);
}

struct Directory* create_fat_fs(struct File* data)
{
	struct FatRootDirectory* directory = malloc(sizeof(struct FatRootDirectory));
	struct FileHandle* handle = file_open(data, HANDLE_READ);
	struct Parameters* params = malloc(sizeof(struct Parameters));
	struct DirectoryEntry entry;
	unsigned int fat_type;
	int i;

	if(directory && params && handle)
	{
		fat_read_parameters(params, handle);
		parse_params(params, directory);

		file_seek(handle, &directory->root_begin);

		for(i = 0; i < directory->root_entries; ++i)
		{
			file_read(handle, &entry, sizeof(entry));

			printf("%s %b\n", entry.name, entry.attributes);
		}
	}

	file_close(handle);
	free(params);

	return directory;
}
