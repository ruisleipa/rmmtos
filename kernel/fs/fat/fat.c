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

	struct FatRootDirectory* root;
	Uint64 entry_position;
	char name[8+1+3+1];
};

struct FileOps fat_file_ops = {
	0, /* open */
	fat_read_file, /* read */
	0, /* write */
};

void name_to_fat(char* name, char* fat_name) {
	int i = 0;

	//TODO: fix buffer overruns

	while(*name != '.' && i < 8) {
		fat_name[i] = toupper(*name);
		i++;
		name++;
	}

	i = 8;
	name++;

	while(*name != '.' && i < 11) {
		fat_name[i] = toupper(*name);
		i++;
		name++;
	}
}

void name_from_fat(char* fat_name, char* name) {
	//TODO: fix buffer overruns
	int i = 0;

	while(fat_name[i] != ' ' && i < 8) {
		*name = tolower(fat_name[i]);
		i++;
		name++;
	}

	i = 8;
	*name = '.';
	name++;

	while(fat_name[i] != ' ' && i < 11) {
		*name = tolower(fat_name[i]);
		i++;
		name++;
	}

	*name = 0;
}

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

			name_from_fat(&entry.name, fat_file->name);

			fat_file->root = directory;

			// XXX: self referential pointer, invalid on realloc
			fat_file->super.super.name = fat_file->name;

			set64(&fat_file->entry_position, &position);

			return fat_file;
		}
	}

	return 0;
}

unsigned int fat_read_file(struct FileHandle* handle, char* buffer, unsigned int count) {
	struct FatFile* file = handle->super.node;
	struct DirectoryEntry entry;
	unsigned int current_cluster;
	unsigned int cluster_offset;
	unsigned int end_offset;
	Uint64 whole_clusters_left;
	Uint64 position;
	unsigned int completed = 0;

	set64(&whole_clusters_left, &handle->position);
	debug_printf("whole_clusters_left1: %x%x%x%x\n", whole_clusters_left.i[3], whole_clusters_left.i[2], whole_clusters_left.i[1], whole_clusters_left.i[0]);

	cluster_offset = shr64(&whole_clusters_left, file->root->cluster_pot);
	debug_printf("whole_clusters_left2: %x%x%x%x\n", whole_clusters_left.i[3], whole_clusters_left.i[2], whole_clusters_left.i[1], whole_clusters_left.i[0]);

	file_seek(file->root->handle, &file->entry_position);
	file_read(file->root->handle, &entry, sizeof(entry));

	// TODO: support 32 bit length
	init64(&position, 0, 0, 0, ((unsigned int*) &entry.size)[0]);
	end_offset = shr64(&position, file->root->cluster_pot);


	current_cluster = entry.cluster_lo;

	while(completed < count && current_cluster < 0xFF7) {
		unsigned int next_cluster;

		debug_printf("cluster_offset: %d current_cluster: %d end_offset: %d", cluster_offset, current_cluster, end_offset);

		init64(&position, 0, 0, 0, current_cluster + (current_cluster / 2));
		add64(&position, &file->root->fat_begin);

		file_seek(file->root->handle, &position);
		file_read(file->root->handle, &next_cluster, sizeof(next_cluster));

		if(current_cluster % 2) {
			next_cluster >>= 4;
		} else {
			next_cluster &= 0x0FFF;
		}

		// TODO: handle bad clusters?
		if(cmp64_16(&whole_clusters_left, 0) != 0)
		{
			debug_printf("end_offset: %x, whole_clusters_left: %x%x%x%x, skipping\n", end_offset, whole_clusters_left.i[3], whole_clusters_left.i[2], whole_clusters_left.i[1], whole_clusters_left.i[0]);
			// just skipping
			dec64(&whole_clusters_left);
		} else {
			// cluster is at least partially to be read to buffer

			unsigned int size = (1 << file->root->cluster_pot) - cluster_offset;
			debug_printf("end_offset: %x, whole_clusters_left: %x, not skipping\n", end_offset, whole_clusters_left.i[0]);

	/*		512

			0+512 > 24

			24 - 0 = 24
*/			debug_printf("size: %x\n", size);

			if (next_cluster >= 0xFF8) {
				if((cluster_offset + size) > end_offset) {
					size = end_offset - cluster_offset;
				}
			}
			debug_printf("size: %x\n", size);

			if(size > (count - completed)) {
				size = (count - completed);
			}

			debug_printf("size: %x\n", size);


			init64(&position, 0, 0, 0, current_cluster - 2);
			shl64(&position, file->root->cluster_pot);
			add64(&position, &file->root->root_end);
			add64_16(&position, cluster_offset);
			file_seek(file->root->handle, &position);

			debug_printf("size: %x\n", size);

			size = file_read(file->root->handle, buffer, size);

			buffer += size & FILE_IO_RESULT_SIZE_MASK;
			completed += size & FILE_IO_RESULT_SIZE_MASK;

			// After reading the first partial cluster the read will always begin at the start of a cluster
			cluster_offset = 0;

			// TODO: handle file end
		}

		current_cluster = next_cluster;
	}

	add64_16(&handle->position, completed);

	return completed;
}

struct Node* fat_root_get_node_by_name(struct DirectoryHandle* handle, char* outer_name)
{
	struct FatRootDirectory* directory = handle->super.node;
	Uint64 position;
	struct DirectoryEntry entry;
	char name[8 + 3 + 2];

	name_to_fat(outer_name, name);

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

		if(memcmp(entry.name, name, 8 + 3) != 0) {
			continue;
		}

		{
			struct File* file = file_create_node(0, 0, &fat_file_ops);
			struct FatFile* fat_file = realloc(file, sizeof(*fat_file));

			name_from_fat(&entry.name, fat_file->name);

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

	debug_printf("create_fat_fs: %x %x %x\n", root_directory, params, handle);

	if(directory && params && handle)
	{
		fat_read_parameters(params, handle);
		parse_params(params, root_directory);
	}

	root_directory->handle = handle;

	free(params);

	return root_directory;
}
