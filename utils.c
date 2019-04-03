#include "utils.h"

int was_bitstream_loaded(){
	return (access("/tmp/lconf_ld", F_OK)==0);
}

void* file_to_mem(const char* src, long* size){
	*size = 0;
	FILE * f_src;
	size_t result;
	long src_size;
	void* buffer;

	f_src = fopen ( src , "rb" );
	if (f_src==NULL) {fprintf (stderr,"Can't open file to read: %s\n", src); return NULL;}

	// obtain file size:
	fseek (f_src , 0 , SEEK_END);
	src_size = ftell (f_src);
	rewind (f_src);

	// allocate memory to contain the whole file:
	buffer =  malloc(src_size);
	if (buffer == NULL) {fprintf(stderr, "Memory error.\n"); return NULL;}
	// copy the file into the buffer:
	result = fread (buffer,1,src_size,f_src);
	if (result != src_size) {fprintf (stderr, "Reading error.\n"); free(buffer); fclose(f_src); return NULL;}

	*size = src_size;
	fclose (f_src);

	return buffer;
}