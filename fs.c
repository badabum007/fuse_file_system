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
	fread(&size, sizeof(unsigned long), 1, fs);
	fread(&node_start, sizeof(unsigned long), 1, fs);
	fread(&name_start, sizeof(unsigned long), 1, fs);
	fread(&data_start, sizeof(unsigned long), 1, fs);
	return fs;
}

unsigned long find_empty_node() {
	FILE * fs = open_fs();
	unsigned long pos = node_start;
	char stat;
	do {
		pos += node_size;
		printf("find space for node. try %lu\n", pos);
		fseek(fs, pos, SEEK_SET);
		fread(&stat, sizeof(char), 1, fs);
	} while (stat != 0 && pos < name_start);
	return pos;
}

unsigned long find_empty_name() {
	FILE * fs = open_fs();
	unsigned long pos = name_start;
	char stat;
	do {
		pos += name_size;
		printf("find space for name. try %lu\n", pos);
		fseek(fs, pos, SEEK_SET);
		fread(&stat, sizeof(char), 1, fs);
	} while (stat != 0 && pos < data_start);
	return pos;
}

unsigned long find_empty_data() {

}


void load_fs() {
	fclose(open_fs());
	node_size = 97;
	name_size = 64;
	data_size = 128;
}

void save_node(node n) {
	FILE* fs = open_fs();
	fseek(fs, n->index, SEEK_SET);
	int b = fwrite(&n->type, sizeof(n->type), 1, fs);
	printf("written type size %d\n", b);
	b = fwrite(&n->name, sizeof(n->name), 1, fs);
	printf("written name size %d\n", b);
	printf("--savename %d\n", n->name);
	fwrite(n->data, sizeof(n->data), 1, fs);
	fclose(fs);
}

node read_node(unsigned long index) {
	FILE* fs = open_fs();
	fseek(fs, index, SEEK_SET);
	int t;
	fread(&t, sizeof(int), 1, fs);
	if (t == 0) 
		return NULL;
	node n = malloc(sizeof(struct fs_node_s));
	n->index = index;
	n->type = t;
	fread(&n->name, sizeof(n->name), 1, fs);
	printf("--readname %d\n", n->name);
	fread(n->data, sizeof(n->data), 1, fs);
	return n;
}

void save_name(name n) {
	FILE* fs = open_fs();
	fseek(fs, n->index, SEEK_SET);
	for (int i = 0; i < 63 && n->name[i] != 0; i++) {
		fwrite(n->name + i, sizeof(char), 1, fs);
	}
	char a = 0;
	fwrite(&a, sizeof(char), 1, fs);
	fclose(fs);
}

name read_name(unsigned long index) {
	FILE* fs = open_fs();
	fseek(fs, index, SEEK_SET);
	name n = malloc(sizeof(struct fs_name_s));
	n->index = index;
	n->name = malloc(sizeof(char) * 64);
	fread(n->name, sizeof(char) * 64, 1, fs);
	fclose(fs);
	return n;
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
			printf("-----is root\n");
			return read_node(node_start);
		}
		else {
			printf("-----ERROR! root is not root\n");
			return NULL;
		}
	} else {
		printf("-----search now\n");
		node root = read_node(node_start);
		printf("-----get root %lu\n", root->index);
		node n = root;
		node current = NULL;
		i = 1;
		printf("-----start loop\n");;
		while (p[i] != NULL) {
			printf("-----current name pattern %s\n", p[i]);
			for (int j = 0; j < 9; j++) {
				printf("-----check %d cell data %lu\n", j, n->data[j]);
				if (n->data[j] != 0) {
					node child = read_node(n->data[j]);
					name nm = read_name(child->name);
					printf("-------compare %s pat %s name\n", p[i], nm->name);
					if (strcmp(p[i], nm->name) == 0) {
						printf("-------find ! %s pat %s name\n", p[i], nm->name);
						current = child;
						printf("!!!!!!!TYPE!!!!!-------type %d name\n", current->type);
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