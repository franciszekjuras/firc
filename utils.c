#include "utils.h"

// FILE* open_file_br(char* file_name){
//     FILE* file = NULL;
//     //char file_name[512];
//     //printf("Insert path to file: ");
//     //if(scanf("%s", file_name) == 1) 
//         file = fopen(file_name, "r");
//         if(file == NULL)
//         	fprintf(stderr, "Error: Couldn't open file: %s\n", file_name);
//     return file;
// }

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


 // read_coefs chyba zwraca liczbę wgranych współczynników
// int read_coefs(FILE* file, int32_t* coefs, int max, int* overflow){ // nie rozumiem do czego złuży overflow
//     int i = 0;
//     int cread = 0;
//     int coef_temp;
//     int st;
//     while((st = fscanf(file, "%d", &coef_temp)==1) && i < max){
//         coefs[i] = coef_temp; i++;
//     }
//     if(i == 0)
//         return -1;
//     cread = i;
//     for(;i < max;++i)
//         coefs[i] = 0;
//     if(st == 1)
//         if(overflow != NULL)*overflow = 1;
//     else
//         if(overflow != NULL)*overflow = 0;

//     return cread; 
// }

// void display_info(fir_t* fir){
//     printf("---------====< %s >====---------\n", fir->conf->info);
//     printf("Time multiplexing rank: %d\n", fir->conf->tm);
//     printf("Max. FIR coefficients: %d\n", fir->conf->coefs_max_nr);
//     printf("Max. upsampling coef.: %d\n", fir->conf->coefs_upsamp_nr);
//     printf("FIR coefficients loaded: %d\n", fir->conf->coefs_crr_nr);
//     printf("Switches state: %d\n", fir->conf->switches);
//     printf("-------------------------------------\n");
// }