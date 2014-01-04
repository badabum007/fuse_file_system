#include "fs.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

char* filesys = "./device";
// char* filesys = "/dev/sdc4";

char** split(char* path) {
	char* buf = malloc((strlen(path) + 1) * sizeof(char));
	cp(buf, path);
	char** res;
	if (strlen(buf) > 1) {
		int count = 0;
		int i = 0;
		while(buf[i] != 0)
			if (buf[i++] == '/') 
				count++;
		res = malloc(sizeof(char*)*(count + 2));
		res[count + 1] = 0;
		res[0] = "/";
		char* pointer = strtok(buf, "/");
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
	// free(buf);
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

node read_nodes_tree(unsigned long index, node parent, char* name) {
	if (index == NULL) return NULL;
	TRACE("read inode");
	inode in = read_inode(index);
	TRACE("node readed");
	if (in == NULL) {
		TRACE("!!readed node is NULL");
		return NULL;	
	} 
	TRACE("node not null");
	printf("type %d", in->type);
	TRACE("type printed");
	inode p = in;
	node head = malloc(sizeof(struct node_s));
	node h = head;
	h->parent = parent;
	h->index = index;
	h->inode = p;
	cp_name(h->name, name);
	h->next = read_nodes_tree(p->next, parent, name);
	if (p->type == 1) {
		for (int i = 0; i < 10; i++) {
			if (p->is_folder.nodes[i] != NULL) {
				h->childs[i] = read_nodes_tree(p->is_folder.nodes[i], head, p->is_folder.names[i]);
			} else {
				h->childs[i] = NULL;
			}
		}
	}
	return h;
}

FILE* open_fs() {
	FILE* fs = fopen(filesys, "rb+");
	fseek(fs, 0, SEEK_SET);
	return fs;
}

void loadfs() {
	FILE * fs = open_fs();
	TRACE("");
	fread(&fs_info, sizeof(struct fs_info_s), 1, fs);
	printf("readed size %d, start %d\n", fs_info.inode_size, fs_info.inode_start);
	printf("readed data_start %lu, dev_size %lu\n", fs_info.data_start, fs_info.dev_size);
	TRACE("");
	fs_cash = read_nodes_tree(fs_info.inode_start, NULL, "/");
	TRACE("readed fs");
	// fs_cash->name[0] = '/';
	// fs_cash->name[1] = NULL;
	TRACE("start print");
	print_node(fs_cash);
	TRACE("printed");
	fclose(fs);
	TRACE("");
}

inode make_empty_inode(int type) {
	inode in = malloc(sizeof(struct inode_s));
	in->type = type;
	in->next = NULL;
	if (type == 1) {
		for (int i = 0; i < 10; i++) {
			in->is_folder.nodes[i] = NULL;
		}
	} else if (type == 2) {
		for (int i = 0; i < 49; i++) {
			in->is_file.data[i] = NULL;
		}
		in->is_file.used_count = 0;
		in->is_file.total_size = 0;
	} else {
		return NULL;
	}
	return in;
}

node make_node_from_empty_inode(inode in, unsigned long index) {
	node head = malloc(sizeof(struct node_s));
	head->index = index;
	head->inode = in;
	head->parent = NULL;
	head->next = NULL;
	for (int i = 0; i < 10; i++) {
		head->childs[i] = NULL;
	}
	return head;
}

node make_new_node_from_empty_inode(inode in) {
	return make_node_from_empty_inode(in, find_free_inode());
}

file_node make_empty_data_node() {
	file_node fn = malloc(sizeof(struct file_node_s));
	for (int i = 0; i < 128; i++) 
		fn->data[i] = NULL;
	fn->size = 0;
}

void save_data_node(file_node n, unsigned long index) {
	FILE * fs = open_fs();
	fseek(fs, index, SEEK_SET);
	fwrite(n, sizeof(struct file_node_s), 1, fs);
	fclose(fs);
}

file_node load_data_node(unsigned long index) {
	FILE * fs = open_fs();
	file_node n = malloc(sizeof(struct file_node_s));
	fseek(fs, index, SEEK_SET);
	int stat;
	fread(&stat, 4, 1, fs);
	if (stat == NULL) return NULL;
	fseek(fs, index, SEEK_SET);
	fread(n, sizeof(struct file_node_s), 1, fs);
	fclose(fs);
	return n;
}

void format() {
	FILE * fs = open_fs();
	TRACE("");
	// int zero = 0x00000000;
	int zero = NULL;
	fpos_t pos;
	fgetpos(fs, &pos);

	fseek(fs,0,SEEK_END);   
	unsigned long size = ftell(fs); 
	fseek(fs,0,SEEK_SET);   
	for (unsigned long i = 0; i < size; i+=4) {
		fwrite(&zero, 4, 1, fs);
	}
	TRACE("fomated");
	fseek(fs, 0, SEEK_SET);
	fs_info.inode_size = sizeof(struct inode_s);
	fs_info.inode_start = sizeof(struct fs_info_s);
	fs_info.dev_size = size;
	fs_info.data_node_size = sizeof(struct file_node_s);
	fs_info.data_start = (unsigned long)(size * 0.05);
	printf("written size %d, start %d\n", fs_info.inode_size, fs_info.inode_start);
	fwrite(&fs_info, sizeof(struct fs_info_s), 1, fs);
	TRACE("fs info written");

	inode in = make_empty_inode(1);
	TRACE("");

	node n = make_node_from_empty_inode(in, fs_info.inode_start);
	print_node(n);
	TRACE("");
	save_node(n);
	TRACE("");
	print_node(n);

	fclose(fs);
	TRACE("format successful");
}

void save_node(node n) {
	FILE * fs = open_fs();
	fseek(fs, n->index, SEEK_SET);
	if (n->inode->type != 0)
		fwrite(n->inode, sizeof(struct inode_s), 1, fs);
	else {
		int stat = NULL;
		fwrite(&stat, 4, 1, fs);
	}
	fclose(fs);
}

inode read_inode(unsigned long index) {
	TRACE("read inode");
    if (index == NULL) {
    	TRACE("index null");
        return NULL;
    }
    TRACE("index not null");
	FILE * fs = open_fs();
	fseek(fs, index, SEEK_SET);
	inode in = malloc(sizeof(struct inode_s));
    fread(in, sizeof(struct inode_s), 1, fs);
	TRACE("inode readed");
    if (in->type == 0) {
    	TRACE("inode type is null");
    	return NULL;
    }
    TRACE("inode type not null");
    printf("readed inode type is %d\n", in->type);
	fclose(fs);
	return in;
}

void add_child(node parent, node child) {
	int i = 0;
	while (i < 10 && parent->childs[i] != NULL) i++;
	if (i < 10) {
		parent->childs[i] = child;
		child->parent = parent;
		parent->inode->is_folder.nodes[i] = child->index;
		cp_name(parent->inode->is_folder.names[i], child->name);
		save_node(child);
		save_node(parent);
	}
}

// file_node read_file_node(unsigned long index) {

// }

// void save_file_node(file_node n) {

// }

int write_data(node n, char *buf, size_t size, off_t offset) {
	// int bl = (int)(offset / 128);
	// printf("++++++++++++++++++++++++++++++blocks offset %d\n", bl);
	// if (bl >= 50) return 0;
	// offset = offset - bl * 128;
	// int fsize = 128 - offset;
	// if (fsize < size) {
	// 	file_node fn = read_file_node(n->inode->is_file.data[bl]);
	// 	memcpy(buf, fn->data + offset, fsize);
	// 	save_node(fn);
	// 	return size;
	// } else {
	// 	;
	// }

}

int read_data(node n, char *buf, size_t size, off_t offset) {

}

void forget_child(node parent, node child) {
	if (parent == NULL) return;
	node n = parent;
	do {
		for (int i = 0; i < 10; i++) {
			if (n->inode->is_folder.nodes[i] == child->index) {
				n->inode->is_folder.nodes[i] = NULL;
				n->childs[i] = NULL;
				save_node(n);
				return;
			}
		}
		n = n->next;
	} while(n != NULL);
}

node read_node(unsigned long index) {
	
}

void print_node(node n) {
	printf("print node\n");
	printf("~index: %lu\n", n->index);
	printf("~name: %s\n", n->name);
	if (n->inode->type == 1) {
		printf("~is directory\n");
		for (int i = 0; i < 10; i++) {
			printf("~~node child [%d] is null %d\n", i, n->childs[i] == NULL);
			printf("~~node child from inode [%d] is null %d\n", i, n->inode->is_folder.nodes[i] == NULL);
			if (n->inode->is_folder.nodes[i] != NULL) 
				printf("~~name child [%d] %s\n", i, n->inode->is_folder.names[i]);
		}
	}
}

unsigned long find_free_inode() {
	FILE * fs = open_fs();
	fseek(fs, fs_info.inode_start, SEEK_SET);
	unsigned long pos = fs_info.inode_start;
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

unsigned long find_free_data_node() {
	FILE * fs = open_fs();
	fseek(fs, fs_info.data_start, SEEK_SET);
	unsigned long pos = fs_info.data_start;
	int stat;
	do {
		printf("try %d \n", pos);
		fread(&stat, 4, 1, fs);
		pos += fs_info.data_node_size;
		fseek(fs, pos, SEEK_SET);
	} while (stat != NULL);
	fclose(fs);
	return pos - fs_info.data_node_size;
}

////TODO
void delete_node(node n) {
	forget_child(n->parent, n);
	free_node(n);
}

void free_node(node n) {
	do {
		n->inode->type = 0;
		save_node(n);
		if (n->inode->type == 1) {
			for (int i = 0; i < 0; i++) {
				if (n->childs[i] != NULL) {
					free_node(n->childs[i]);
				}
			}
		}
		free(n->inode);
		node next = n->next;
		free(n);
		n = next;
	} while (n != NULL);
}

node find_child_by_name(char* name, node parent) {
	node n = parent;
	TRACE("!~!~!!~!~!~!!~~~~~~~~~~~~~~~~");
	TRACE(parent->name);
	TRACE(name);
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
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!COUNT %d\n", count);
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

