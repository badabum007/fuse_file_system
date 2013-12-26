#ifndef FS
#define FS
	
#include <stdio.h>

#define DEBUG

#ifdef DEBUG
	#define TRACE(a) printf(">>DEBUG>>line: %d, mess: %s\n", __LINE__, a);
	//#define PRINT(k, v) printf(">>DEBUG>> %s, mess: %s\n", __LINE__, a);
#else
	#define TRACE(a)
#endif

typedef struct ifolder_s {
	char names[10][32];
	unsigned long nodes[10];
}ifolder;

typedef struct ifile_s {
	unsigned long data[50];
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



#endif