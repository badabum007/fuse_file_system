#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include "fs.h"

void open_root() {
	
}


/***********************************************
---------------fuse functions here--------------
************************************************/

static int myfs_getattr(const char *path, struct stat *stbuf) {
	node n = find_node_by_name(path);
	if (n == NULL) {
		TRACE("file not found");
		return -ENOENT;
	} else {
		TRACE(path);
		printf("-----is null inode %d\n", n->inode == NULL);
		if (n->inode->type == 1) {
			TRACE("it is dir");
			stbuf->st_mode = n->inode->mode | S_IFDIR /*| 0777*/;
			stbuf->st_nlink = 3;
			return 0;
		} else {
			printf("-----it is file\n");
			stbuf->st_mode = n->inode->mode | S_IFREG/* | 0666*/;
			stbuf->st_nlink = 1;
			stbuf->st_size = n->inode->is_file.total_size;
			return 0;
		}
	}
}

int myfs_rename(const char * old, const char * new) {
	node tmp = find_node_by_name(new);
	if (tmp != NULL)
		delete_node(tmp);
	node nod = find_node_by_name(old);
	node parent = find_node_parent(old);
	if (parent->inode->type != 1)
		return -ENOENT;
	if (nod == NULL || parent == NULL)
		return -ENOENT;
	cp_name(nod->name, new+1);
	node n = parent;
	int index = -1;
	do {
		for (int i = 0; i < 10 && index < 0; i++) {
			if (n->inode->is_folder.nodes[i] == nod->index) {
				index = i;
			}
		}
		if (index < 0)
			n = n->next;
	} while (n != NULL && index < 0);
	cp_name(n->inode->is_folder.names[index], new+1);
	save_node(n);
	save_node(nod);
	return 0;
}


static int myfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	int pos = find_free_inode();
	if (pos < 0) return -ENOENT;
	TRACE(path);
	char** names = split(path);
	TRACE(path);
	int i = 0, count = 0;
	while(names[i++] != NULL) {
		TRACE(names[count]);
		count++;
	}
	char* name = names[count - 1];
	TRACE("");
	TRACE(path);

	node parent = find_node_parent(path);
	inode in = make_empty_inode(2);
	in->mode = mode;
	TRACE("");
	node n = make_node_from_empty_inode(in, pos);
	TRACE("");
	cp_name(n->name, name);
	TRACE("");
	add_child(parent, n);
	TRACE("");
	return 0;
}

static int myfs_open(const char *path, struct fuse_file_info *fi) {
	node n = find_node_by_name(path);
	if (n == NULL) 
		return -ENOENT;
	if (n->inode->type != 2)
		return -ENOENT;
	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;
	return 0;
}

static int myfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	TRACE("READ START++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
	printf("---------##size %d, offset %d \n", size, offset);
	node file = find_node_by_name(path);
	if (file->inode->type != 2) {
		TRACE("NOT IS FILE");
		return -ENOENT;
	}
	TRACE("")
	printf("-------------------------------------------------------base offset %d\n", offset);

	int block_num, node_num;
	int node_capacity = DATASIZE * 49;
	printf("-------------------------------------------------------node capacity %d\n", node_capacity);
	node_num = offset / node_capacity;
	offset -= node_num * node_capacity;

	printf("-------------------------------------------------------node_num %d\n", node_num);
	printf("-------------------------------------------------------new offset %d\n", offset);

	TRACE("")
	block_num = offset / DATASIZE;
	offset -= block_num * DATASIZE;
	TRACE("")
	printf("-------------------------------------------------------block_num %d\n", block_num);
	printf("-------------------------------------------------------new offset %d\n", offset);
	int nind = node_num;
	TRACE("")
	while (nind > 0) {
		if (file->next == NULL) {
			TRACE("next null")
			return 0;
		}
		TRACE("")
		file = file->next;
		nind--;
	}
	TRACE("")
	if (file->inode->is_file.data[block_num] == NULL) {
		TRACE("MAIN IS NOT EXISTS");
		return 0;
	}
	TRACE("")
	file_node n = load_data_node(file->inode->is_file.data[block_num]);
	if (n == NULL) {
		TRACE("READ ERROR");
		return 0;
	}
	TRACE("")
	if (n->size == 0) {
		TRACE("EMPTY DATA");
		return 0;
	}
	TRACE("")
	TRACE("ALL GOOd");
	size_t read_size = size;
	size_t readed_size = 0;
	size_t len = n->size;
	if (offset < len) {
		do {
			TRACE("GOOD OFFSET");
			if (offset + size > len)
				read_size = len - offset;
			printf("REAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAD SIIIIIIIIIIIIIIIIIIIIZEEEEEEEEE %d\n", read_size);
			memcpy(buf + readed_size, n->data + offset, read_size);
			size -= read_size;
			readed_size += read_size;
			if (len < DATASIZE - 1) return readed_size;
			printf("SIIIIIIIIIIIIIIIIIIIIZEEEEEEEEE NEEEEEEEED TOO REEEEEEEAD %d\n", size);
			// TRACE("DATA SENDED");
			if ((int)size > 0) {
				//goto next block
				if (block_num < 48) {
					TRACE("NEXT BLOCK");
					block_num++;
				} else {
					TRACE("NEXT NODE");
					file = file->next;
					block_num = 0;
					if (file == NULL)
						return readed_size;
				}
				offset = 0;
				if (file->inode->is_file.data[block_num] == NULL) {
					TRACE("NEXT DATA IS NOT EXISTS");
					return readed_size;
				}
				n = load_data_node(file->inode->is_file.data[block_num]);
				if (n == NULL) {
					TRACE("READ ERROR");
					return readed_size;
				}
				if (n->size == 0) {
					TRACE("EMPTY DATA");
					return readed_size;
				}
			}
		} while ((int)size > 0);
	} else
		readed_size = 0;
	TRACE("READ END++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
	return readed_size;
}

static int myfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	TRACE("WRITE START++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
	printf("---------##size %d, offset %d \n", size, offset);
	node file = find_node_by_name(path);
	node root = file;

	printf("-------------------------------------------------------base offset %d\n", offset);

	int block_num, node_num;
	int node_capacity = DATASIZE * 49;

	printf("-------------------------------------------------------node capacity %d\n", node_capacity);

	node_num = offset / node_capacity;
	offset -= node_num * node_capacity;

	printf("-------------------------------------------------------node_num %d\n", node_num);
	printf("-------------------------------------------------------new offset %d\n", offset);

	block_num = offset / DATASIZE;
	offset -= block_num * DATASIZE;

	printf("-------------------------------------------------------block_num %d\n", block_num);
	printf("-------------------------------------------------------new offset %d\n", offset);

	int nind = node_num;
	while (nind > 0) {
		TRACE("TO NEXT NODE")
		if (file->next == NULL) {
			// TRACE("ERROR (next == NULL)");
			// return 0;	
			TRACE("+++++++++++++GO TO NEXT INODE!!!!!!!!!!");
			inode in = make_empty_inode(2);
			TRACE("");
			unsigned long pos = find_free_inode();
			if (pos < 0) return 0;
			TRACE("");
			node new_node = make_node_from_empty_inode(in, pos);
			printf("type %d\n", new_node->inode->type);
			TRACE("");
			file->next = new_node;
			TRACE("");
			printf("type %d\n", file->next->inode->type);
			file->inode->next = pos;
			TRACE("");
			save_node(new_node);
			TRACE("");
			save_node(file);
			TRACE("");
			// file = new_node;
			TRACE("");
			// file = file->next;
			block_num = 0;
			TRACE("");
		} 
		file = file->next;
		TRACE("")
		printf("type %d\n", file->inode->type);
		printf("nind %d\n", nind);
		nind--;
		printf("nind %d\n", nind);
	}
	TRACE("");

	if (file->inode->type != 2) {
		TRACE("NOT A FILE");
		return -ENOENT;
	}
	TRACE("");
	file_node n;
	if (file->inode->is_file.data[block_num] == NULL) {
		TRACE("NEW FILE");
		n = make_empty_data_node();
		TRACE("");
		file->inode->is_file.data[block_num] = find_free_data_node();
		if (file->inode->is_file.data[block_num] < 0) return 0;
		TRACE("");
		file->inode->is_file.used_count++;
		TRACE("");
		printf("+++++++++index %d \n", file->inode->is_file.data[block_num]);
		TRACE("SPACE FINDED");
	} else {
		TRACE("LOAD NODE");
		n = load_data_node(file->inode->is_file.data[block_num]);
	}
	TRACE("");
	if (n == NULL) {
		TRACE("ERROR LOAD");
		return 0;
	}
	TRACE("");
	size_t write_size = size;
	size_t writted_size = 0;
	size_t len = n->size;
	TRACE("");
	if (offset <= len) {
		do {
			TRACE("+++++++++++++++++++++++++++++++++++++WRITE ITERATION++++++++++++++++++")
			// TRACE("GOOD OFFSET");
			// if (offset + size > len)
			// 	write_size = len - offset;
			if (size > DATASIZE)
				write_size = DATASIZE - offset;
			printf("offcet %d, size %d, datasize %d, writte size %d, writted size %d\n", offset, size, DATASIZE, write_size, writted_size);
			TRACE("+++++++++++++++++++++++++++++++++++++WRITE DATAAAAAAAAAAAAA++++++++++++++++++")
			memcpy(n->data + offset, buf + writted_size, write_size);
			size -= write_size;
			writted_size += write_size;
			printf("offcet %d, size %d, datasize %d, writte size %d, writted size %d\n", offset, size, DATASIZE, write_size, writted_size);
			n->size += write_size;
			len = n->size;
			printf("len %d", len);
			save_node(file);
			save_data_node(n, file->inode->is_file.data[block_num]);
			if (len < DATASIZE - 1) {
				root->inode->is_file.total_size += writted_size;
				save_node(root);
				return writted_size;
			}
			printf("SIIIIIIIIIIIIIIIIIIIIZEEEEEEEEE NEEEEEEEED TOO WRIIIIIIITEEEEE %d\n", size);
			// TRACE("DATA SENDED");
			if ((int)size > 0) {
				if (block_num < 48) {
					TRACE("+++++++++++++GO TO NEXT BLOCK!!!!!!!!!!");
					block_num++;
				} else {
					TRACE("+++++++++++++GO TO NEXT INODE!!!!!!!!!!");
					inode in = make_empty_inode(2);
					TRACE("");
					unsigned long pos = find_free_inode();
					if (pos < 0) return writted_size;
					TRACE("");
					node new_node = make_node_from_empty_inode(in, pos);
					TRACE("");
					file->next = new_node;
					TRACE("");
					file->inode->next = pos;
					TRACE("");
					save_node(new_node);
					TRACE("");
					save_node(file);
					TRACE("");
					file = new_node;
					TRACE("");
					// file = file->next;
					block_num = 0;
					TRACE("");
					// if (file == NULL)
					// 	return 0;
				}
				offset = 0;
				n = make_empty_data_node();
				file->inode->is_file.data[block_num] = find_free_data_node();
				if (file->inode->is_file.data[block_num] < 0) return writted_size;
				file->inode->is_file.used_count++;
				
			}
		} while ((int)size > 0);
	} else
		writted_size = 0;
	// //debug
	// 	if (size > 128) size = 128;
	// //debug
	// memcpy(n->data + offset, buf, size);
	// n->size = size + offset;
	// printf("====================================written size %d\n", n->size);
	// file->inode->is_file.total_size = size + offset;
	// file->inode->is_file.used_count = 1;
	// save_node(file);
	// save_data_node(n, file->inode->is_file.data[0]);
	TRACE("PARENT SAVED");
	TRACE("WRITE END++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
	root->inode->is_file.total_size += writted_size;
	save_node(root);
	return writted_size;
}

// static int myfs_truncate(const char *path, off_t a) {
//  return 0;
// }

// static int myfs_release(const char *path, struct fuse_file_info *a) {
//  return 0;
// }

// static int myfs_flush(const char *path, struct fuse_file_info *a) {
//  return 0;
// }

static int myfs_unlink(const char *path) {
	TRACE("unlink");
	node n = find_node_by_name(path);
	TRACE("node finded");
	print_node(n);
	delete_node(n);
	TRACE("unlink complete");
	return 0;
}

// static int myfs_mknod(const char* path, mode_t mode, dev_t dev) {

// }

static int myfs_mkdir(const char* path, mode_t mode) {
	int pos = find_free_inode();
	if (pos < 0) return -ENOENT;
	TRACE(path);
	char** names = split(path);
	TRACE(path);
	int i = 0, count = 0;
	while(names[i++] != NULL) {
		TRACE(names[count]);
		count++;
	}
	char* name = names[count - 1];
	TRACE("");
	TRACE(path);

	node parent = find_node_parent(path);
	inode in = make_empty_inode(1);
	in->mode = mode;

	TRACE("");

	node n = make_node_from_empty_inode(in, pos);

	TRACE("");
	cp_name(n->name, name);
	TRACE("");
	add_child(parent, n);
	TRACE("");
	return 0;
}

static int myfs_opendir(const char *path, struct fuse_file_info *fi) {
	node n = find_node_by_name(path);
	if (n == NULL) 
		return -ENOENT;
	if (n->inode->type != 1)
		return -ENOENT;
	return 0;
}
 
static int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	node n = find_node_by_name(path);
	printf("-------!!!!!!!!!!!--------read dir %s\n", path);
	if (n == NULL) {
		printf("-----can not find file %s\n", path);
		return -ENOENT;
	} else { 
		if (n->inode->type == 1) {
			filler(buf, ".", NULL, 0);
			filler(buf, "..", NULL, 0);
			node n1 = n;
			do {
				for (int i = 0; i < 10; i++) {
					if (n->childs[i] != NULL)
						filler(buf, n->childs[i]->name, NULL, 0);
				}
				n = n->next;
			} while (n != NULL);
			return 0;
		}
		return -ENOENT;
	}
}

static int myfs_rmdir(const char *path) {
	TRACE("rmdir");
	node n = find_node_by_name(path);
	TRACE("node finded");
	print_node(n);
	delete_node(n);
	TRACE("rmdir complete");
	return 0;
}

int myfs_chmod(const char * path, mode_t mode) {
	node n = find_node_by_name(path);
	if (n == NULL) {
		TRACE("file not found");
		return -ENOENT;
	} else {
		n->inode->mode = mode;
		save_node(n);
	}
	return 0;
}

int myfs_truncate(const char * path, off_t offset) {
	node file = find_node_by_name(path);
	node root = file;
	if (file->inode->type != 2) {
		TRACE("NOT IS FILE");
		return -ENOENT;
	}
	TRACE("")
	printf("-------------------------------------------------------base offset %d\n", offset);

	int base_offset = offset;
	int block_num, node_num;
	int node_capacity = DATASIZE * 49;
	printf("-------------------------------------------------------node capacity %d\n", node_capacity);
	node_num = offset / node_capacity;
	offset -= node_num * node_capacity;

	printf("-------------------------------------------------------node_num %d\n", node_num);
	printf("-------------------------------------------------------new offset %d\n", offset);

	TRACE("")
	block_num = offset / DATASIZE;
	offset -= block_num * DATASIZE;
	TRACE("")
	printf("-------------------------------------------------------block_num %d\n", block_num);
	printf("-------------------------------------------------------new offset %d\n", offset);
	int nind = node_num;
	TRACE("")
	while (nind > 0) {
		if (file->next == NULL) {
			TRACE("next null")
			return 0;
		}
		TRACE("")
		file = file->next;
		nind--;
	}
	TRACE("")
	if (file->inode->is_file.data[block_num] == NULL) {
		TRACE("MAIN IS NOT EXISTS");
		return 0;
	}
	TRACE("")
	file_node n = load_data_node(file->inode->is_file.data[block_num]);
	if (n == NULL) {
		TRACE("READ ERROR");
		return 0;
	}
	TRACE("")
	if (n->size == 0) {
		TRACE("EMPTY DATA");
		return 0;
	}
	root->inode->is_file.total_size = base_offset;
	save_node(root);
	block_num++;
	do {
		for (int i = block_num; i < 49; i++) {
			if (file->inode->is_file.data[i] != NULL)
				delete_data_node(file->inode->is_file.data[i]);
		}
		file = file->next;
		block_num = 0;
	} while (file != NULL);
	return 0;
}



static struct fuse_operations operations = {
	.getattr	= myfs_getattr,
	.readdir 	= myfs_readdir,
	.opendir 	= myfs_opendir,
	.mkdir 		= myfs_mkdir,
	.rmdir      = myfs_rmdir,
	.open       = myfs_open,
	.create     = myfs_create,
	.read 		= myfs_read,
	.write 		= myfs_write,
	.unlink 	= myfs_unlink, 
	.chmod		= myfs_chmod, 
	.truncate 	= myfs_truncate,
	.rename 	= myfs_rename
};

int main(int argc, char *argv[]) {

	printf("base sizes\n");
	printf("ifolder %d\n", sizeof(struct ifolder_s));
	printf("ifile %d\n", sizeof(struct ifile_s));
	printf("inode %d\n", sizeof(struct inode_s));


	char res = 0;
	char** new_argv = malloc(sizeof(char*));
	int new_argc = 1;
	new_argv[0] = argv[0];
	while ((res = getopt(argc,argv,"fn:d")) != -1) {
		switch (res) {
			case 'f':
				format();
				break;
			case 'd':
				TRACE("");
				new_argc++;
				TRACE("");
				new_argv = realloc(new_argv, sizeof(char*) * new_argc);
				TRACE("");
				new_argv[new_argc - 1] = "-d";
				TRACE("");
				break;
			case 'n':
			TRACE("");
				new_argc++;
				TRACE("");
				new_argv = realloc(new_argv, sizeof(char*) * new_argc);
				TRACE("");
				new_argv[new_argc - 1] = optarg;
				TRACE("");
				break;
		}
	}
	loadfs();
	// test_mkdir();
	// print_node(fs_cash);
	// if (fs_cash != NULL)
	// 	print_node(fs_cash);
	find_free_inode();
	return /*0;//*/ fuse_main(new_argc, new_argv, &operations, NULL);
}