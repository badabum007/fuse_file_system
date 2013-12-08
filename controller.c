#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>


typedef int file_t;
typedef struct node_s *node;
typedef struct node_s {
	char* name;s
	file_t node_type;
	node parent;
	node* childs;
	int child_count;
	char* data;
};
node fs;

void log(char* message)
{
    FILE* f = fopen("/home/medvedmike/lodfile.log","a+");
    int i = 0;
    while(message[i] != 0) fputc((int)message[i++],f);
    fputc((int)'\n',f);
    fclose(f);
}

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

node find_node(node tree, char* path) {
	char** pp = split(path);
	int count = 0, i = 0;
	while(pp[i++] != NULL) count++;
	if (count == 1) 
		return tree;
	node p = tree;
	i = 1;
	while (pp[i] != NULL) {
		if (p->childs != NULL) {
			for (int j = 0; j < p->child_count; j++) {
				if (strcmp(pp[i], p->childs[j]->name) == 0) {
					p = p->childs[j];
					break;
				}
			}
		}
		i++;
	}
	return p;
}

node create_node(char* name, int type, char* data) {
	node n = (node)malloc(sizeof(struct node_s));
	n->name = name;
	n->node_type = type;
	n->parent = NULL;
	n->childs = NULL;
	n->child_count = 0;
	n->data = data;
	return n;
}

void add_child(node parent, node child) {
	if (parent->child_count == 0) {
		parent->childs = (node)malloc(sizeof(struct node_s));
		parent->childs[0] = child;
		child->parent = parent;
		parent->child_count++;
	} else {
		parent->childs = (node)realloc(parent->childs, sizeof(struct node_s) * (parent->child_count + 1));
		parent->childs[parent->child_count] = child;
		child->parent = parent;
		parent->child_count++;
	}
}

node create_root() {
	node root = (node)malloc(sizeof(struct node_s));
	root->name = "/";
	root->node_type = 0;
	root->parent = NULL;
	root->childs = NULL;//malloc(sizeof(node));
	root->child_count = 0;
	root->data = NULL;

	node new = create_node("loh", 0, NULL);
	add_child(root, new);

	new = create_node("pidr", 1, "");
	add_child(root, new);
}

/***********************************************
---------------fuse functions here--------------
************************************************/

static int myfs_getattr(const char *path, struct stat *stbuf) {
	memset(stbuf, 0, sizeof(struct stat));
	node p = find_node(fs, path);
	if (p == NULL)
		return -ENOENT;
	if (p->node_type == 0) {
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 3;
	} else {
		stbuf->st_mode = S_IFREG | 0666;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(p->data);
	}
	return 0;
}

static int myfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	char** pp = split(path);
	int count = 0, i = 0;
	while(pp[i++] != NULL) count++;
	node new = create_node(pp[count - 1], 1, "");
	log(new->name);
	node parent = find_node(fs, path);
	log(parent->name);
	add_child(parent, new);
	return 0;
}

static int myfs_open(const char *path, struct fuse_file_info *fi) {
	return 0;
}

static int myfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	return 0;
}

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

static int myfs_mknod(const char* path, mode_t mode, dev_t dev)
{
	char** pp = split(path);
	int count = 0, i = 0;
	while(pp[i++] != NULL) count++;
	node new = create_node(pp[count - 1], 1, "");
	log(new->name);
	node parent = find_node(fs, path);
	log(parent->name);
	add_child(parent, new);
	return 0;
}

static int myfs_mkdir(const char* path, mode_t mode) {
	char** pp = split(path);
	int count = 0, i = 0;
	while(pp[i++] != NULL) count++;
	log(new->name);
	node parent = find_node(fs, path);
	log(parent->name);
	add_child(parent, new);
	return 0;
}

static int myfs_opendir(const char *path, struct fuse_file_info *a) {
	return 0;
}
 
static int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	log(path);
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	node n = find_node(fs, path);
	for (int i = 0; i < n->child_count; i++) {
		filler(buf, n->childs[i]->name, NULL, 0);
	}
	return 0;
}
 
// static int myfs_releasedir(const char *path, struct fuse_file_info *a) {
//  return 0;
// }


static struct fuse_operations hello_oper = {
	// .getattr	= hello_getattr,
	.readdir = myfs_readdir,
	//.opendir = myfs_opendir,
	.getattr = myfs_getattr,
	.mkdir = myfs_mkdir,  //--
	.open = myfs_open,
	.create = myfs_create,
	.mknod = myfs_mknod,
};

int main(int argc, char *argv[]) {
	fs = create_root();
	// char* path = (char*)malloc(sizeof(char) * 3);
	// path[0] = '/';
	// path[1] = 'a';
	// path[2] = 'b';
	// node n = find_node(fs, path);
	// printf("%d\n", n == NULL);
	// printf("%s\n", n->name);
	printf("%d\n", fs->child_count);
	printf("%s\n", fs->childs[0]->name);
	printf("%s\n", fs->childs[1]->name);
	return fuse_main(argc, argv, &hello_oper, NULL);
}