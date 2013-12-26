#include "fs.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

char* filesys = "./device";

char** split(char* path) {
	char* buf = malloc((strlen(path) + 1) * sizeof(char));
	cp(buf, path);
	char** res;
	// printf("----IN SPLIT________-------%s\n", buf);
	if (strlen(buf) > 1) {
		int count = 0;
		int i = 0;
		// printf("----IN SPLIT________-------%s\n", buf);
		while(buf[i] != 0)
			if (buf[i++] == '/') 
				count++;
		// printf("----IN SPLIT________-------%s\n", buf);
		res = malloc(sizeof(char*)*(count + 2));
		res[count + 1] = 0;
		res[0] = "/";
		// printf("----IN SPLIT________-------%s\n", buf);
		char* pointer = strtok(buf, "/");
		i = 1;
		// printf("----IN SPLIT________-------%s\n", buf);
		while(pointer) {
			res[i++] = pointer;
			pointer = strtok(NULL, "/");
		}
		// printf("----IN SPLIT________-------%s\n", buf);
	} else {
		res = (char**)malloc(sizeof(char*)*2);
		res[1] = 0;
		res[0] = "/";
	} 
	// printf("----IN SPLIT________-------%s\n", buf);
	return res;
}

void cp(char* dest, char* source) {
	printf("copy name\n");
	int len = strlen(source);
	printf("source %s\n", source);
	int i = 0;
	for (; i < len; i++)
		dest[i] = source[i];
	 dest[i] = NULL;
	printf("dest %s\n", dest);
}

void cp_name(char* dest, char* source) {
	printf("copy name\n");
	int len = strlen(source);
	printf("source %s\n", source);
	len = len > 31 ? 31 : len;
	int i = 0;
	for (; i < len; i++)
		dest[i] = source[i];
	dest[i] = NULL;
	printf("dest %s\n", dest);
}

node read_nodes_tree(unsigned long index) {
	node root = read_node(index);
	node n = root;
	if (n == NULL) return NULL;
	if (n->inode->type == 1) {
		printf("read folder\n");
		do {
			printf("iter!\n");
			for (int i = 0; i < 10; i++) {
				if (n->inode->is_folder.nodes[i] != NULL) {
					printf("read child with index %lu, name %s\n", n->inode->is_folder.nodes[i], n->inode->is_folder.names[i]);
					n->childs[i] = read_nodes_tree(n->inode->is_folder.nodes[i]);
					// n->childs[i]->name = n->inode->is_folder.names[i];
					cp_name(n->childs[i]->name, n->inode->is_folder.names[i]);
					n->childs[i]->parent = n;
				}
			}
			n->next = read_node(n->inode->next);
			n = n->next;
		} while (n != NULL);
	}
	return root;
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
	fs_cash = read_nodes_tree(fs_info.inode_start);
	fs_cash->name[0] = '/';
	fs_cash->name[1] = NULL;
	print_node(fs_cash);
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
	printf("\n\nsave node");
	FILE * fs = open_fs();
	fseek(fs, n->index, SEEK_SET);
	printf("index %lu\n", n->index);
	print_node(n);
	fwrite(n->inode, sizeof(struct inode_s), 1, fs);
	printf("type = %d\n", n->inode->type);
	fclose(fs);
	print_node(read_node(n->index));
}

void add_child(node parent, node child) {
	printf("\n\nadd child %d\n", child->index);
	print_node(parent);
	int i = 0;
	while (parent->childs[i] != NULL && i < 10) i++;
	if (i == 10) //new node;
	printf("added index %d\n", i);
	parent->childs[i] = child;
	parent->inode->is_folder.nodes[i] = child->index;
	// parent->inode->is_folder.names[i] = child->name;
	cp_name(parent->inode->is_folder.names[i], child->name);
}

node read_node(unsigned long index) {
	if (index == NULL) {
		return NULL;
	}
	FILE * fs = open_fs();
	fseek(fs, index, SEEK_SET);
	node n = malloc(sizeof(struct node_s));
	n->index = index;
	inode in = malloc(sizeof(struct inode_s));
	fread(in, sizeof(struct inode_s), 1, fs);
	if (in->type == 0) return NULL;
	n->inode = in;
	n->parent = NULL;
	n->next= NULL;
	for (int i = 0; i < 10; i++)
		n->childs[i] = NULL;
	printf("type = %d\n", in->type);
	fclose(fs);
	return n;
}

void print_node(node n) {
	printf("\n\nprint node with index %d\n", n->index);
	printf("has next %d\n", n->inode->next != NULL);
	if (n->inode->type == 1) {
		printf("is folder\n");
		for (int i = 0; i < 10; i++) {
			printf("node:child index %d, is null %d\n", i, n->childs[i] == NULL);
			if (n->inode->is_folder.nodes[i] != NULL) {
				printf("inode:child index %lu, name %s\n", n->inode->is_folder.nodes[i], n->inode->is_folder.names[i]);
			}
		}

	} else if (n->inode->type == 2) {
		printf("is file\n");
	} else {
		printf("deleted\n");
	}
}

int find_free_inode() {
	FILE * fs = open_fs();
	fseek(fs, fs_info.inode_start, SEEK_SET);
	int pos = fs_info.inode_start;
	int stat;
	do {
		printf("try %d \n", pos);
		fread(&stat, 4, 1, fs);
		pos += fs_info.inode_size;
		fseek(fs, pos, SEEK_SET);
	} while (stat != NULL);
	fclose(fs);
	return pos - fs_info.inode_size;
}


node find_child_by_name(char* name, node parent) {
	node n = parent;
	do {
		for (int i = 0; i < 10; i++) {
			if (n->childs[i] != NULL) {
				if (strcmp(n->childs[i]->name, name) == 0)
					return n->childs[i];
			}
		}
		n = n->next;
	} while (n != NULL);
	return NULL;
}

node find_node_by_name(char* path) {
	char** names = split(path);
	int i = 0, count = 0;
	while(names[i++] != NULL) {
		printf("-----path element: %d : %s\n", i-1, names[i-1]);
		count++;
	}
	if (count == 1) {
		if (strcmp(path, "/") == 0) {
			printf("-----is root, inode_start %d\n", fs_info.inode_start);
			return fs_cash;
		}
		else {
			printf("-----ERROR! root is not root\n");
			return NULL;
		}
	} else {
		node folder = find_node_parent(path);
		if (folder == NULL) return NULL;
		return find_child_by_name(names[count - 1], folder);
	}
}

node find_node_parent(char* path) {
	char** names = split(path);
	int i = 0, count = 0;
	while(names[i++] != NULL) {
		printf("-----path element: %d : %s\n", i-1, names[i-1]);
		count++;
	}
	if (count == 2) {
		if (strcmp(names[0], "/") == 0) {
			printf("-----is root, inode_start %d\n", fs_info.inode_start);
			return fs_cash;
		}
		else {
			printf("-----ERROR! root is not root\n");
			return NULL;
		}
	} else {
		printf("search \n");
		node n1 = fs_cash;
		i = 1;
		while (names[i+1] != NULL) {
			n1 = find_child_by_name(names[i], n1);
			if (n1 == NULL) return NULL;
			i++;
		}
		return n1;
	}
}

