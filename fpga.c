#include "fpga.h"

fpga_t fpga_connect(long base, long mem_size){
	fpga_t fpga;
	//mapping fpga memory
	fpga.fd = open("/dev/mem", O_RDWR | O_SYNC);
	if(fpga.fd < 0) {
		fprintf(stderr, "open(/dev/mem) failed: %s\n", strerror(errno));
		fpga.valid = 0;
		return fpga;
	}
	long page_size = sysconf(_SC_PAGESIZE);
	long page_addr = base & (~(page_size-1));
	long page_off = base - page_addr;
	long map_length = page_off + mem_size;
	void* page_ptr = mmap(NULL, map_length, PROT_READ | PROT_WRITE,
						  MAP_SHARED, fpga.fd, page_addr);
	//break if mapping failed
	//fprintf(stderr, "Pointer: %p\n", page_ptr);
	if(page_ptr == MAP_FAILED) {
		fprintf(stderr, "mmap() failed: %s\n", strerror(errno));
		fpga.valid = 0;
		return fpga;
	}
	//calculate pointers -- fpga_mem is base pointer of fpga memory
	//set as uint32_t pointer type for pointer addition compling
	//with fpga 32-bit addressing
	fpga.mem_size = mem_size;
	fpga.mem_len = mem_size/sizeof(uint32_t);
	fpga.addr = page_ptr + page_off;
	fpga.valid = 1;
	return fpga;
}

int fpga_disconnect(fpga_t* fpga){
	
	if (munmap(fpga->addr, fpga->mem_size) < 0) {
		fprintf(stderr, "munmap() failed: %s\n", strerror(errno));
		return -1;
	}
	
	fpga->addr = NULL;
	fpga->mem_size = 0;
	fpga->mem_len = 0;
	//TODO: close file des
	return 0;
}