#ifndef FS
#define FS
	
#include <stdio.h>

// #define DEBUG

#ifdef DEBUG
	#define TRACE(a) printf(">>DEBUG>>line: %d, mess: %s\n", __LINE__, a);
#else
	#define TRACE(a)
#endif

#define DATASIZE 4096

typedef struct ifolder_s {
	char names[10][32];
	unsigned long nodes[10];
}ifolder;

typedef struct ifile_s {
	int used_count;
	int total_size;
	unsigned long data[49];	
}ifile;

typedef struct inode_s * inode;
struct inode_s {
	int type;
	unsigned long next;
	union {
		ifolder is_folder;
		ifile is_file;
	};
};

typedef struct file_node_s * file_node;
struct file_node_s
{
	char data[DATASIZE];
	int size;
};

typedef struct node_s * node;
struct node_s {
	unsigned long index;
	inode inode;
	node parent;
	node next;
	node childs[10];
	char name[32];

};

struct fs_info_s {
	int inode_start;
	int inode_size;
	int data_node_size;
	unsigned long dev_size;
	unsigned long data_start;
}fs_info;

extern char* filesys;
node fs_cash;

void format();
void loadfs();
void print_node(node n);
node read_node(unsigned long index);
void save_node(node n);
node find_node_by_name(char* path);
node find_node_parent(char* path);
unsigned long find_free_inode();
void add_child(node parent, node child);
void cp_name(char* dest, char* source);
char** split(char* path);
void delete_node(node n);
inode read_inode(unsigned long index);
inode make_empty_inode(int type);
node make_node_from_empty_inode(inode in, unsigned long index);
file_node make_empty_data_node();
void save_data_node(file_node n, unsigned long index);
file_node load_data_node(unsigned long index);
void delete_data_node(unsigned long index);



#endif