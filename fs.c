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

void load_fs() {
	fclose(open_fs());
	node_size = 12 * 8 + 4;
	name_size = 64;
}

void save_node(node n) {
	FILE* fs = open_fs();
	fseek(fs, n->index, SEEK_SET);
	fwrite(&n->type, sizeof(n->type), 1, fs);
	fwrite(&n->name, sizeof(n->name), 1, fs);
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
		printf("-----find here\n");
		return NULL;
	}
}