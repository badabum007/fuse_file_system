#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "fs.h"

void gen(char* name, long size) {
	FILE* file = fopen(name, "wb+");
	char data = 0;
	for (int i = 0; i < size * 1024; i++)
		fwrite(&data, 1, 1024, file);
	fclose(file);
}

void regen(char* name) {
	FILE* file = fopen(name, "rb+");
	fseek(file, 0, SEEK_SET);
	char data = 0;
	while (!feof(file))
		fwrite(&data, 1, 1024, file);
	fclose(file);
}

void write_fs_data(char* filename) {
	FILE* file = fopen(filename, "rb+");
	fseek(file,0,SEEK_END);   
	unsigned long size = ftell(file); 
	printf("size %lu\n", size);
	fseek(file, 0, SEEK_SET);
	unsigned long node_start = sizeof(unsigned long) * 4;
	unsigned long name_start = (int)size * 0.03 + sizeof(unsigned long) * 4;
	unsigned long data_start = (int)size * 0.05 + sizeof(unsigned long) * 4;
	printf("node %lu\nname %lu\ndata %lu\n", node_start, name_start, data_start);
	printf("in this fs %f nodes; %f fnames\n", (name_start - node_start) / 100., (data_start - name_start) / 64.);
	fwrite(&size, sizeof(unsigned long), 1, file);
	fwrite(&node_start, sizeof(unsigned long), 1, file);
	fwrite(&name_start, sizeof(unsigned long), 1, file);
	fwrite(&data_start, sizeof(unsigned long), 1, file);
	fclose(file);
	filesys = filename;
	node n = malloc(sizeof(struct fs_node_s));
	n->index = node_start;
	n->type = 1;
	n->name = name_start;
	for (int i = 0; i < 10; i++)
		n->data[i] = 0;

	name nm = malloc(sizeof(struct fs_name_s));
	nm->index = n->name;
	nm->name = "/";

	save_name(nm);
	save_node(n);
	free(n);
	
}

int main(int argc, char* argv[]) {
	int rez = 0;
	long size = 0;
	int vs_name = 0;
	int is_new = 0;
	char* name;
	while ((rez = getopt(argc,argv,"s:n:c")) != -1) {
		switch (rez) {
			case 's':
				size = atol(optarg);
			//	printf("size = %l\n", size);
				break;
			case 'n':
				name = optarg;
				vs_name = 1;
			//	printf("name = %s\n", name);
			    break;
			case 'c':
				is_new = 1;
				break;

//	 		case '?': printf("Error found !\n");break;
		 }
	}
	if (size > 0 && vs_name) {
		if (is_new)
			gen(name, size);
		else
			regen(name);
		write_fs_data(name);
	}
	return 0;
}
