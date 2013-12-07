#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

static int myfs_getattr(const char *path, struct stat *stbuf) {

}

static int myfs_create(const char *path, mode_t a, struct fuse_file_info *b) {

}

static int myfs_open(const char *path, struct fuse_file_info *fi) {

}

static int myfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {

}

static int myfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {

}

static int myfs_truncate(const char *path, off_t a) {

}

static int myfs_release(const char *path, struct fuse_file_info *a) {

}

static int myfs_flush(const char *path, struct fuse_file_info *a) {

}

static int myfs_unlink(const char *path) {

}

static int myfs_opendir(const char *path, struct fuse_file_info *a) {

}
 
static int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {

}
 
static int myfs_releasedir(const char *path, struct fuse_file_info *a) {

}


static struct fuse_operations hello_oper = {
	// .getattr	= hello_getattr,
};

int main(int argc, char *argv[]) {
	return fuse_main(argc, argv, &hello_oper, NULL);
}