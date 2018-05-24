#ifndef _fpga_h_
#define _fpga_h_

#define FPGA_BASE_ADDR 0x43C00000
#define FPGA_MEM_SIZE 1024*64 //64K

struct fpga_s;
typedef struct fpga_s fpga_t;

#include "firctrl.h"

typedef struct fpga_s{
    int valid;
    int fd;
    void* addr;
    long mem_size;
    long mem_len;
}fpga_t;

int fpga_disconnect(fpga_t* fpga);
fpga_t fpga_connect(long base, long mem_size);

#endif //_fpga_h_