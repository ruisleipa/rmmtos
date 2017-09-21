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
	Uint64 root_end;
	struct FileHandle* handle;
};

struct FatFile
{
	struct File super;

	char name[8+3+1];
	Uint64 entry_position;
};

struct Parameters* fat_read_parameters(struct Parameters* params, struct FileHandle* handle)
{
	char* mbr = malloc(512);
	Uint64 zero;

	init64_32(&zero, 0, 0);

	if(mbr)
	{
		file_seek(handle, &zero);
		file_read(handle, mbr, 512);

		memcpy(&params->bytes_per_sector, &mbr[11], sizeof(params->bytes_per_sector));
		memcpy(&params->sectors_per_cluster, &mbr[13], sizeof(params->sectors_per_cluster));
		memcpy(&params->reserved_sectors, &mbr[14], sizeof(params->reserved_sectors));
		memcpy(&params->fats, &mbr[16], sizeof(params->fats));
		memcpy(&params->root_entries, &mbr[17], sizeof(params->root_entries));
		memcpy(&params->sectors_per_fat, &mbr[22], sizeof(params->sectors_per_fat));
		memcpy(&params->hidden_sectors, &mbr[28], sizeof(params->hidden_sectors));
		memcpy(&params->total_sectors, &mbr[32], sizeof(params->total_sectors));

		free(mbr);
	}
}

void parse_params(struct Parameters* params, struct FatRootDirectory* directory)
{
	init64(&directory->root_begin, 0, 0, 0, (params->reserved_sectors + (params->fats * params->sectors_per_fat)));

	init64(&directory->root_end, 0, 0, 0, params->root_entries * 32);
	add64(&directory->root_end, &directory->root_begin);

	shl64(&directory->root_begin, 9);
	shl64(&directory->root_end, 9);
}

#define FLAG_DIRECTORY 0x10
#define FLAG_VOLUME 0x04

struct Node* fat_root_get_next_node(struct DirectoryHandle* handle, struct FatFile* current_node)
{
	struct FatRootDirectory* directory = handle->super.node;
	Uint64 position;
	struct DirectoryEntry entry;

	if(!current_node) {
		set64(&position, &directory->root_begin);
	} else {
		init64(&position, 0, 0, 0, sizeof(struct DirectoryEntry));
		add64(&position, &current_node->entry_position);
	}

	file_seek(directory->handle, &position);

	while(cmp64(&position, &directory->root_end) < 0) {
		set64(&position, &directory->handle->position);
		file_read(directory->handle, &entry, sizeof(entry));

		if(entry.attributes & FLAG_VOLUME) {
			continue;
		}

		if(entry.name[0] == 0xe5) {
			continue;
		}

		if(entry.name[0] == 0x00) {
			break;
		}

		{
			struct File* file = file_create_node(0, 0, 0);
			struct FatFile* fat_file = realloc(file, sizeof(*fat_file));

			memcpy(fat_file->name, &entry.name, 8 + 3);
			fat_file->name[8 + 3] = 0;

			// XXX: self referential pointer, invalid on realloc
			fat_file->super.super.name = fat_file->name;

			set64(&fat_file->entry_position, &position);

			return fat_file;
		}
	}

	return 0;
}

static struct DirectoryOps fat_root_dir_ops = {
	0,
	fat_root_get_next_node,
	0,
	0
};

struct Directory* create_fat_fs(struct File* data)
{
	struct Directory* directory = directory_create_node("", 0, &fat_root_dir_ops);
	struct FileHandle* handle = file_open(data, HANDLE_READ);
	struct Parameters* params = malloc(sizeof(struct Parameters));
	struct FatRootDirectory* root_directory = realloc(directory, sizeof(*root_directory));

	printf("create_fat_fs: %x %x %x\n", root_directory, params, handle);

	if(directory && params && handle)
	{
		fat_read_parameters(params, handle);
		parse_params(params, root_directory);
	}

	root_directory->handle = handle;

	free(params);

	return root_directory;
}
