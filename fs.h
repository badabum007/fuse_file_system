#ifndef FS
#define FS
	
#include <stdio.h>

typedef struct fs_node_s * node;
struct fs_node_s {
	unsigned long index;
	int type; //1 - directory, 2 - file, 0 - deleted
	unsigned long name;
	unsigned long data[10];
};

typedef struct fs_name_s * name;
struct fs_name_s
{
	unsigned long index;
	char* name;
};

struct fs_data_s {
	unsigned long index;
	char* data;
};

extern char* filesys;
unsigned long node_start;
int node_size;
unsigned long name_start;
int name_size;
unsigned long data_start;
int data_size;
unsigned long size;

char** split(char* path);
FILE * open_fs();
void load_fs();
void save_node(node n);
node read_node(unsigned long index);
node find_node_by_name(char* path);
void save_name(name n);
name read_name(unsigned long index);

#endif