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
			return -ENOENT;
		}
	}
}

static int myfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	return 0;
}

static int myfs_open(const char *path, struct fuse_file_info *fi) {
	return 0;
}

// static int myfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
// 	return 0;
// }

// static int myfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
//  return 0;
// }

// static int myfs_truncate(const char *path, off_t a) {
//  return 0;
// }

// static int myfs_release(const char *path, struct fuse_file_info *a) {
//  return 0;
// }

// static int myfs_flush(const char *path, struct fuse_file_info *a) {
//  return 0;
// }

// static int myfs_unlink(const char *path) {
//  return 0;
// }

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


static struct fuse_operations operations = {
	.getattr	= myfs_getattr,
	.readdir 	= myfs_readdir,
	.opendir 	= myfs_opendir,
	.mkdir 		= myfs_mkdir,
	.rmdir      = myfs_rmdir
	// .releasedir = myfs_releasedir
	// .open = myfs_open,
	// .create = myfs_create,
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


	// format();
	loadfs();
	// test_mkdir();
	// print_node(fs_cash);
	// if (fs_cash != NULL)
	// 	print_node(fs_cash);
	find_free_inode();
	return /*0;//*/ fuse_main(argc, argv, &operations, NULL);
}