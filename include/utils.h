#ifndef _utils_h_
#define _utils_h_



#include "firctrl.h"
#include "fir.h"

//FILE* open_file_br(char* file_name);
//int read_coefs(FILE* file, int32_t* coefs, int max, int* overflow);
//int tridialog(const char* opt1, const char* opt2, const char* opt3);
//void display_info(fir_t* fir);
void* file_to_mem(const char* src, long* size);



#endif //_utils_h_