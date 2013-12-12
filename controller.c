#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include "fs.h"
/**********************************
----------file system types--------
**********************************/

/**********************************
end-------file system types--------
**********************************/


/**********************************
----------file system functions----
**********************************/

void log(char* message)
{
    FILE* f = fopen("/home/medvedmike/lodfile.log","a+");
    int i = 0;
    while(message[i] != 0) fputc((int)message[i++],f);
    fputc((int)'\n',f);
    fclose(f);
}

void open_root() {
	node n = read_node(node_start);
	if (n != NULL) {
		printf("%d, %d\n", n->index, n->type);
		for (int i = 0; i < 10; i++) {
			printf("%lu\n", n->data[i]);
		}
		printf("name %lu\n", n->name);
		name nm = read_name(n->name);
		printf("%s\n", nm->name);
	}
}


/***********************************************
---------------fuse functions here--------------
************************************************/

static int myfs_getattr(const char *path, struct stat *stbuf) {
	node n = find_node_by_name(path);
	if (n == NULL) {
		printf("-----can not find file %s\n", path);
		return -ENOENT;
	} else {
		printf("-----find file %s\n", path);
		if (n->type == 1) {
			printf("-----it is dir\n");
			stbuf->st_mode = S_IFDIR | 0777;
			stbuf->st_nlink = 3;
			return 0;
		} else {
			printf("-----it is file\n");
			return -ENOENT;
		}
	}
}

// static int myfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
// }

// static int myfs_open(const char *path, struct fuse_file_info *fi) {
// 	return 0;
// }

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

// static int myfs_mkdir(const char* path, mode_t mode) {

// }

// static int myfs_opendir(const char *path, struct fuse_file_info *a) {
// 	return 0;
// }
 
// static int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {

// }
 
// static int myfs_releasedir(const char *path, struct fuse_file_info *a) {
//  return 0;
// }


static struct fuse_operations operations = {
	.getattr	= myfs_getattr
	// .readdir = myfs_readdir,
	// //.opendir = myfs_opendir,
	// .getattr = myfs_getattr,
	// .mkdir = myfs_mkdir,  //--
	// .open = myfs_open,
	// .create = myfs_create,
	// .mknod = myfs_mknod,
};

int main(int argc, char *argv[]) {
	load_fs();
	// create_root();
	open_root();

	printf("size %lu, node %lu, name %lu, data %lu\n", size, node_start, name_start, data_start);

	printf("compare %d \n", 0 != "\0");

	return fuse_main(argc, argv, &operations, NULL);
}