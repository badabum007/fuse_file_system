#include "fs.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

char* filesys = "./device";

char** split(char* path) {
	char** res;
	if (strlen(path) > 1) {
		int count = 0;
		int i = 0;
		while(path[i] != 0)
			if (path[i++] == '/') 
				count++;
		res = malloc(sizeof(char*)*(count + 2));
		res[count + 1] = 0;
		res[0] = "/";
		char* pointer = strtok(path, "/");
		i = 1;
		while(pointer) {
			res[i++] = pointer;
			pointer = strtok(NULL, "/");
		}
	} else {
		res = (char**)malloc(sizeof(char*)*2);
		res[1] = 0;
		res[0] = "/";
	} 
	return res;
}

FILE* open_fs() {
	FILE* fs = fopen(filesys, "rb+");
	fseek(fs, 0, SEEK_SET);
	return fs;
}

void loadfs() {
	FILE * fs = open_fs();
	fread(&fs_info, sizeof(struct fs_info_s), 1, fs);
	printf("readed size %d, start %d\n", fs_info.inode_size, fs_info.inode_start);
	printf("readed data_start %lu, dev_size %lu\n", fs_info.data_start, fs_info.dev_size);
	fclose(fs);
}

void format() {
	FILE * fs = open_fs();
	// int zero = 0x00000000;
	int zero = NULL;
	fpos_t pos;
	fgetpos(fs, &pos);
	// while (!feof(fs)) {
	// 	fwrite(&zero, 4, 1, fs);
	// 	printf("%d\n", pos);
	// }
	for (int i = 0; i < 100000; i++) {
		fwrite(&zero, 4, 1, fs);
	}

	fseek(fs,0,SEEK_END);   
	unsigned long size = ftell(fs); 

	fseek(fs, 0, SEEK_SET);
	fs_info.inode_size = sizeof(struct inode_s);
	fs_info.inode_start = sizeof(struct fs_info_s);
	fs_info.dev_size = size;
	fs_info.data_start = (unsigned long)(size * 0.05);
	printf("written size %d, start %d\n", fs_info.inode_size, fs_info.inode_start);
	fwrite(&fs_info, sizeof(struct fs_info_s), 1, fs);

	inode in = malloc(sizeof(struct inode_s));
	in->type = 1;
	for (int i = 0; i < 0; i++) {
		in->is_folder.nodes[i] = NULL;
		// in->is_folder.names[i] = NULL;
	}
	in->next = NULL;

	node n = malloc(sizeof(struct node_s));
	n->index = fs_info.inode_start;
	n->inode = in;
	save_node(n);

	fclose(fs);
}

void save_node(node n) {
	FILE * fs = open_fs();
	fseek(fs, n->index, SEEK_SET);
	fwrite(n->inode, sizeof(struct inode_s), 1, fs);
	printf("type = %d\n", n->inode->type);
	fclose(fs);
}

void add_child(node parent, node child) {
	int i = 0;
	while (parent->inode->is_folder.nodes[i] != NULL && i < 10) i++;
	parent->inode->is_folder.nodes[i] = child->index;
	parent->inode->is_folder.names[i][0] = 'a';
}

node read_node(unsigned long index) {
	FILE * fs = open_fs();
	fseek(fs, index, SEEK_SET);
	node n = malloc(sizeof(struct node_s));
	n->index = index;
	inode in = malloc(sizeof(struct inode_s));
	fread(in, sizeof(struct inode_s), 1, fs);
	n->inode = in;
	printf("type = %d\n", in->type);
	fclose(fs);
	return n;
}

void print_node(node n) {
	printf("print node with index %d\n", n->index);
	printf("has next %d\n", n->inode->next != NULL);
	if (n->inode->type == 1) {
		printf("is folder\n");
	} else if (n->inode->type == 2) {
		printf("is file\n");
	} else {
		printf("deleted\n");
	}
}

unsigned long find_free_inode() {
	FILE * fs = open_fs();
	fseek(fs, index, SEEK_SET);
	unsigned long pos = fs_info.inode_start;
	node n;
	do {
		printf("try %lu \n", pos);
		n = read_node(pos);
		pos += fs_info.inode_size;
	} while (n->inode->type != NULL);
	fclose(fs);
	return pos - fs_info.inode_size;
}

node find_node_by_name(char* path) {
	char** p = split(path);
	int i = 0, count = 0;
	while(p[i++] != NULL) {
		printf("-----path element: %d : %s\n", i-1, p[i-1]);
		count++;
	}
	if (count == 1) {
		printf("-----root: %s\n", p[0]);
		if (strcmp(path, "/") == 0) {
			printf("-----is root, inode_start %d\n", fs_info.inode_start);
			return read_node(fs_info.inode_start);
		}
		else {
			printf("-----ERROR! root is not root\n");
			return NULL;
		}
	} else {
		printf("-----search now\n");
		node root = read_node(fs_info.inode_start);
		printf("-----get root %lu\n", root->index);
		node n = root;
		node current = NULL;
		i = 1;
		while (p[i] != NULL) {
			for (int j = 0; j < 10; j++) {
				if (n->inode->is_folder.nodes[j] != NULL) {
					if (strcmp(p[i], n->inode->is_folder.names[j]) == 0) {
						current = read_node(n->inode->is_folder.nodes[j]);
						break;
					}
				}
			}
			if (current == NULL) return NULL;
			i++;
			n = current;
		}
		return n;
	}
}

node find_node_parent(char* path) {
			return read_node(fs_info.inode_start);
}

