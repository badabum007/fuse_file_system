#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[]) {

	int offset = atoi(argv[1]);

	int block_num, node_num;
	int node_capacity = 128 * 49;
	block_num = offset / node_capacity;
	offset -= block_num * node_capacity;

	node_num = offset / 128;
	offset -= node_num * 128;

	printf("node num = %d\nblock num = %d\noffset = %d\n", node_num, block_num, offset);



	return /*0;//*/ ;
}