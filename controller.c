#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
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
			stbuf->st_mode = S_IFDIR | 0777;
			stbuf->st_nlink = 3;
			return 0;
		} else {
			printf("-----it is file\n");
			stbuf->st_mode = S_IFREG | 0666;
			stbuf->st_nlink = 1;
			stbuf->st_size = n->inode->is_file.total_size;
			return 0;
		}
	}
}

static int myfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	int pos = find_free_inode();
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
	TRACE("");
	node n = make_node_from_empty_inode(in, pos);
	TRACE("");
	cp_name(n->name, name);
	TRACE("");
	add_child(parent, n);
	TRACE("");
	return 0;
}

int myfs_setxattr(const char * a, const char * b, const char * c, size_t s, int xz) {
	return 0;
}

static int myfs_open(const char *path, struct fuse_file_info *fi) {
	node n = find_node_by_name(path);
	if (n == NULL) 
		return -ENOENT;
	if (n->inode->type != 2)
		return -ENOENT;
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

	int block_num, node_num;
	int node_capacity = 128 * 49;
	block_num = offset / node_capacity;
	offset -= block_num * node_capacity;

	node_num = offset / 128;
	offset -= node_num * 128;

	int nind = node_num;

	while (nind > 0) {
		if (file->next == NULL) return 0;
		file = file->next;
	}
	if (file->inode->is_file.data[block_num] == NULL) {
		TRACE("DATA IS NOT EXISTS");
		return 0;
	}
	file_node n = load_data_node(file->inode->is_file.data[block_num]);
	if (n == NULL) {
		TRACE("READ ERROR");
		return 0;
	}
	if (n->size == 0) {
		TRACE("EMPTY DATA");
		return 0;
	}

	TRACE("ALL GOOd");
	size_t read_size = size;
	size_t readed_size = 0;
	size_t len = n->size;
	if (offset < len) {
		do {
			// TRACE("GOOD OFFSET");
			if (offset + size > len)
				read_size = len - offset;
			// printf("REAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAD SIIIIIIIIIIIIIIIIIIIIZEEEEEEEEE %d\n", read_size);
			memcpy(buf + readed_size, n->data + offset, read_size);
			size -= read_size;
			readed_size += read_size;
			if (len < 127) return readed_size;
			printf("SIIIIIIIIIIIIIIIIIIIIZEEEEEEEEE NEEEEEEEED TOO REEEEEEEAD %d\n", size);
			// TRACE("DATA SENDED");
			if (size > 0) {
				//goto next block
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
	int block_num, node_num;
	int node_capacity = 128 * 49;
	block_num = offset / node_capacity;
	offset -= block_num * node_capacity;

	node_num = offset / 128;
	offset -= node_num * 128;

	int nind = node_num;
	while (nind > 0) {
		if (file->next == NULL) return 0;
		file = file->next;
	}
	if (file->inode->type != 2) {
		TRACE("NOT A FILE");
		return -ENOENT;
	}
	file_node n;
	if (file->inode->is_file.data[node_num] == NULL) {
		TRACE("NEW FILE");
		n = make_empty_data_node();
		file->inode->is_file.data[node_num] = find_free_data_node();
		file->inode->is_file.used_count++;
		printf("+++++++++index %d \n", file->inode->is_file.data[node_num]);
		TRACE("SPACE FINDED");
	}
	else {
		TRACE("LOAD FILE");
		n = load_data_node(file->inode->is_file.data[node_num]);
	}
	if (n == NULL) {
		TRACE("ERROR LOAD");
		return 0;
	}
	size_t write_size = size;
	size_t writted_size = 0;
	size_t len = n->size;
	if (offset <= len) {
		do {
			// TRACE("GOOD OFFSET");
			// if (offset + size > len)
			// 	write_size = len - offset;
			if (size > 128)
				write_size = 128 - offset;
			// printf("REAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAD SIIIIIIIIIIIIIIIIIIIIZEEEEEEEEE %d\n", read_size);
			memcpy(n->data + offset, buf + writted_size, write_size);
			size -= write_size;
			writted_size += write_size;
			n->size += write_size;
			len = n->size;
			save_node(file);
			save_data_node(n, file->inode->is_file.data[node_num]);
			if (len < 127) {
				file->inode->is_file.total_size = writted_size + offset;
				return writted_size;
			}
			printf("SIIIIIIIIIIIIIIIIIIIIZEEEEEEEEE NEEEEEEEED TOO REEEEEEEAD %d\n", size);
			// TRACE("DATA SENDED");
			if (size > 0) {
				//goto next block
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
	return 0;
}

int myfs_truncate(const char * path, off_t offset) {
	return 0;
}



static struct fuse_operations operations = {
	.getattr	= myfs_getattr,
	.readdir 	= myfs_readdir,
	.opendir 	= myfs_opendir,
	.mkdir 		= myfs_mkdir,
	.rmdir      = myfs_rmdir,
	// .releasedir = myfs_releasedir
	.open       = myfs_open,
	.create     = myfs_create,
	.read 		= myfs_read,
	.write 		= myfs_write,
	.unlink 	= myfs_unlink, 
	// .setxattr 	= myfs_setxattr, 
	// .chmod		= myfs_chmod, 
	.truncate 	= myfs_truncate
	// .mknod = myfs_mknod,
};

void test_mkdir() {
	inode in = malloc(sizeof(struct inode_s));
	in->type = 1;
	for (int i = 0; i < 0; i++) {
		in->is_folder.nodes[i] = NULL;
		// in->is_folder.names[i] = NULL;
	}
	in->next = NULL;

	node n = malloc(sizeof(struct node_s));
	n->index = find_free_inode();
	n->inode = in;
	n->parent = fs_cash;
	n->next = NULL;
	for (int i = 0; i < 10; i++) {
		n->childs[i] = NULL;
	}
	cp_name(n->name, "testdir");

	save_node(n);
	add_child(fs_cash, n);

	printf("added to parent\n");
	print_node(fs_cash);

	save_node(fs_cash);
}

int main(int argc, char *argv[]) {

	printf("base sizes\n");
	printf("ifolder %d\n", sizeof(struct ifolder_s));
	printf("ifile %d\n", sizeof(struct ifile_s));
	printf("inode %d\n", sizeof(struct inode_s));


	format();
	loadfs();
	// test_mkdir();
	// print_node(fs_cash);
	// if (fs_cash != NULL)
	// 	print_node(fs_cash);
	find_free_inode();
	return /*0;//*/ fuse_main(argc, argv, &operations, NULL);
}