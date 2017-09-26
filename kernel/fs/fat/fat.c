#include "fat.h"
#include "fs/file.h"
#include "fs/directory.h"

unsigned int fat_read_file(struct FileHandle* handle, char* buffer, unsigned int count);

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
	Uint64 fat_begin;
	Uint64 data_begin;
	unsigned char cluster_pot;
	struct FileHandle* handle;
};

struct FatFile
{
	struct File super;

	char name[8+3+1];
	struct FatRootDirectory* root;
	Uint64 entry_position;
};

struct FileOps fat_file_ops = {
	0, /* open */
	fat_read_file, /* read */
	0, /* write */
};

struct Parameters* fat_read_parameters(struct Parameters* params, struct FileHandle* handle)
{
	char* mbr = malloc(512);
	Uint64 zero;

	init64(&zero, 0, 0, 0, 0);

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

		printf("fat spc: %x bps: %x\n", params->sectors_per_cluster, params->bytes_per_sector);
		printf("fat rootent: %x secs: %x\n", params->root_entries, (params->root_entries *32 + params->bytes_per_sector - 1) / params->bytes_per_sector);

		free(mbr);
	}
}

void parse_params(struct Parameters* params, struct FatRootDirectory* directory)
{
	unsigned int bytes_per_cluster;

	directory->cluster_pot = 0;

	bytes_per_cluster = params->sectors_per_cluster * params->bytes_per_sector;

	while(!(bytes_per_cluster & 1)) {
		directory->cluster_pot++;
		bytes_per_cluster >>= 1;
	}

	debug_printf("directory->cluster_pot: %x\n", directory->cluster_pot);

	init64(&directory->root_begin, 0, 0, 0, (params->reserved_sectors + (params->fats * params->sectors_per_fat)));
	shl64(&directory->root_begin, 9);

	set64(&directory->root_end, &directory->root_begin);
	add64_16(&directory->root_end, ((params->root_entries * 32 + params->bytes_per_sector - 1) / params->bytes_per_sector) * params->bytes_per_sector);
	// TODO: Round this up to the next sector

	init64(&directory->fat_begin, 0, 0, 0, params->reserved_sectors);
	shl64(&directory->fat_begin, 9);
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
			struct File* file = file_create_node(0, 0, &fat_file_ops);
			struct FatFile* fat_file = realloc(file, sizeof(*fat_file));

			memcpy(fat_file->name, &entry.name, 8 + 3);
			fat_file->name[8 + 3] = 0;
			fat_file->root = directory;

			// XXX: self referential pointer, invalid on realloc
			fat_file->super.super.name = fat_file->name;

			set64(&fat_file->entry_position, &position);

			return fat_file;
		}
	}

	return 0;
}

void name_to_fat(char* name) {

}

void name_from_fat(char* name) {

}

unsigned int fat_read_file(struct FileHandle* handle, char* buffer, unsigned int count) {
	struct FatFile* file = handle->super.node;
	struct DirectoryEntry entry;
	unsigned int current_cluster;
	unsigned int cluster_offset;
	Uint64 whole_clusters_left;
	Uint64 position;
	unsigned int completed = 0;

	set64(&whole_clusters_left, &handle->position);
	cluster_offset = shr64(&whole_clusters_left, file->root->cluster_pot);

	file_seek(file->root->handle, &file->entry_position);
	file_read(file->root->handle, &entry, sizeof(entry));

	current_cluster = entry.cluster_lo;

	while(completed < count) {
		unsigned int next_cluster;

		printf("cluster_offset: %d current_cluster: %d", cluster_offset, current_cluster);

		// TODO: handle bad clusters?
		if(cmp64_16(&whole_clusters_left, 0) > 0)
		{
			// just skipping
			dec64(&whole_clusters_left);
		} else {
			// cluster is at least partially to be read to buffer
			unsigned int size = (1 << file->root->cluster_pot) - cluster_offset;

			if(size > (count - completed)) {
				size = (count - completed);
			}

			init64(&position, 0, 0, 0, current_cluster - 2);
			shl64(&position, file->root->cluster_pot);
			add64(&position, &file->root->root_end);
			file_seek(file->root->handle, &position);

			printf("reading %d bytes from cluster @ %x%x\n", size, position.i[1], position.i[0]);

			size = file_read(file->root->handle, buffer, size);

			printf("read %x bytes from cluster\n", size);

			buffer += size & FILE_IO_RESULT_SIZE_MASK;
			completed += size & FILE_IO_RESULT_SIZE_MASK;

			printf("completed %x count %x \n", completed, count);

			// After reading the first partial cluster the read will always begin at the start of a cluster
			cluster_offset = 0;

			// TODO: handle file end
		}

		init64(&position, 0, 0, 0, current_cluster + (current_cluster / 2));
		add64(&position, &file->root->fat_begin);

		file_seek(file->root->handle, &position);
		file_read(file->root->handle, &next_cluster, sizeof(next_cluster));

		if(current_cluster % 2) {
			next_cluster >>= 4;
		} else {
			next_cluster &= 0x0FFF;
		}

		if (next_cluster >= 0xFF8) {
			return completed;
		}

		current_cluster = next_cluster;
	}

	return completed;
}

struct Node* fat_root_get_node_by_name(struct DirectoryHandle* handle, char* name)
{
	struct FatRootDirectory* directory = handle->super.node;
	Uint64 position;
	struct DirectoryEntry entry;

	set64(&position, &directory->root_begin);

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

		if(memcmp(entry.name, name) != 0) {
			continue;
		}

		{
			struct File* file = file_create_node(0, 0, &fat_file_ops);
			struct FatFile* fat_file = realloc(file, sizeof(*fat_file));

			memcpy(fat_file->name, &entry.name, 8 + 3);
			fat_file->name[8 + 3] = 0;

			fat_file->root = directory;

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
	fat_root_get_node_by_name,
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
