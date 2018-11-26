#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#include "firctrl.h"

int main(int argc, char ** argv){

	int32_t coefs[FIR_MAX_COEFS];
	int32_t upsamp_coefs[FIR_MAX_SRC_COEFS];
	int32_t dwsamp_coefs[FIR_MAX_SRC_COEFS];

	fpga_t fpga;
	fir_t fir;

	fpga = fpga_connect( FPGA_BASE_ADDR, FPGA_MEM_SIZE );
			if( !fpga.valid ) {
				fprintf(stderr, "Error: Couldn't establish connection with FPGA.\n");
				return -1;
			}
			fir = open_fir(&fpga); 

	for(int i=1; i < argc; i++) {

		if(strcmp(argv[i],"-p") == 0) { // docelowo - wpisać konfigurację do pliku
			continue;
		} // -p

		if(strcmp(argv[i],"-t") == 0) { // zwraca rząd multipleksowania w czasie
			printf("%d\n", fir.conf->tm);
			continue;
		} // -t

		if(strcmp(argv[i],"-l") == 0) { // ładowanie współczynników
			FILE* up_coefs_file = open_file_dialog("tmp/firctrl/up.dat");
			FILE* dw_coefs_file = open_file_dialog("tmp/firctrl/dw.dat");
			FILE* fir_coefs_file = open_file_dialog("tmp/firctrl/cf.dat");
			if(up_coefs_file == NULL || dw_coefs_file == NULL || fir_coefs_file == NULL)
				return -1;

			int up_overflow;  // nie rozumiem do czego to jest
			int dw_overflow;
			int fir_overflow;

			// wczytanie up_coefs
            int up_coefs_read = read_coefs(up_coefs_file, upsamp_coefs, fir.conf->coefs_upsamp_nr, &up_overflow); 
            fclose(up_coefs_file);    
        	// ewentualne błędy:   
            if(up_coefs_read < 0) {
                fprintf(stderr, "Error: Failed reading upsampling coefficients (even though the file was succesfully opened). \n");
                return -1;
            } // if
            if(up_overflow == 1) {
            	fprintf(stderr, "Error: Number of upsampling coefficients is larger than expectd.\n");
  				return -1;
            } // if
            if(up_coefs_read != fir.conf->coefs_upsamp_nr){ 
                fprintf(stderr, "Error: Number of upsampling coefficients doesn't match expected number.\n");
                return -1;
            } // if
            //wgranie współczynników
            load_coefs(fir, upsamp_coefs, fir.conf->upsamp_dsp_nr);

        	// wczytanie dw_coefs
            int dw_coefs_read = read_coefs(dw_coefs_file, dwsamp_coefs, fir.conf->coefs_dwsamp_nr, &dw_overflow); 
            fclose(dw_coefs_file);  
        	// ewentualne błędy:     
            if(dw_coefs_read < 0){
                fprintf(stderr, "Error: Failed reading downsampling coefficients.\n");
                return -1;
            } // if
            if(dw_overflow == 1) {
            	fprintf(stderr, "Error: Number of downsampling coefficients is larger than expetced.\n");
  				return -1;
            } // if
            if(dw_coefs_read != fir.conf->coefs_dwsamp_nr){ 
                fprintf(stderr, "Error: Number of downsampling coefficients doesn't match expected number.\n");
                return -1;
            } // if
            //wgranie współczynników
            load_coefs(fir, dwsamp_coefs, fir.conf->dwsamp_dsp_nr);

        	// wczytanie fir_coefs
            int coefs_read = read_coefs(fir_coefs_file, coefs, fir.conf->coefs_max_nr, &fir_overflow); 
            fclose(fir_coefs_file);  
        	// ewentualne błędy:     
            if(coefs_read < 0){
                fprintf(stderr, "Error: Failed reading FIR coefficients.\n");
            	return -1;
            } // if
            if(fir_overflow == 1) {
            	fprintf(stderr, "Error: Number of fir coefficients is larger than expected.\n");
  				return -1;
            } // if
            if(coefs_read != fir.conf->coefs_max_nr){ 
                fprintf(stderr, "Error: Number of fir coefficients doesn't match expected number.\n");
                return -1;
            } // if
            //wgranie współczynników 
            load_coefs(fir, coefs, fir.conf->fir_dsp_nr);
            fir.conf->coefs_crr_nr = coefs_read; // nie wiem do czego to służy...

		} // -l

		if(strcmp(argv[i],"-c") == 0) { // wypisanie maksymalnej liczby współczynników
			printf("%d\n", fir.conf->coefs_dwsamp_nr); 
			printf("%d\n", fir.conf->coefs_upsamp_nr);
			continue;
		} // -c

		if(strcmp(argv[i],"-r") == 0) { // wypisanie maksymalnej liczby współczynników FIR
			printf("%d\n",fir.conf->coefs_max_nr);
			continue;
		} // -r

		if(strcmp(argv[i],"-m") == 0) { // zwraca pozycję przecinka; nie wiem która to zmienna 
			printf("%d\n", fir.conf->fir_coef_mag);
			continue;
		} // -m

		if(strcmp(argv[i],"-o") == 0) { // włącza filtr
			SWITCH_ON(SWITCH_FIR_EN, fir.conf->switches);
		} // -o

		if(strcmp(argv[i],"-x") == 0) { // wyłącza filtr
			SWITCH_OFF(SWITCH_FIR_EN, fir.conf->switches);
		} // -x

		if(strcmp(argv[i],"-z") == 0) { // zeruje wszystkie współczynniki
			zero_coefs(upsamp_coefs, fir.conf->coefs_upsamp_nr);
			load_coefs(fir, upsamp_coefs, fir.conf->upsamp_dsp_nr);

			zero_coefs(dwsamp_coefs, fir.conf->coefs_dwsamp_nr);
			load_coefs(fir, dwsamp_coefs, fir.conf->dwsamp_dsp_nr);

			zero_coefs(coefs, fir.conf->coefs_max_nr);
			load_coefs(fir, coefs, fir.conf->fir_dsp_nr);
		} // -z

		if(strcmp(argv[i],"-i") == 0 || strcmp(argv[i],"help") == 0 || strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"-help") == 0 || strcmp(argv[i],"--help") == 0) {
			printf( "FIRCTRL \n"
				"\n"
				"Usage: ./firctrl [arguments]\n"
				"\n"
				"Arguments:\n"
				"  -p: 	Check beatstreamem consistency\n"
				"  -t: 	Show time multiplexing rank\n"
				"  -l: 	Load coefficients\n"
				"  -c: 	Show maximum number of downsampling and upsampling coefficients\n"
				"  -r: 	Show maximum number of FIR coefficients\n"
				"  -m: 	Show comma position\n"
				"  -o:	Enable filter\n"
				"  -x: 	Disable filter\n"
				"  -z: 	Set all coefficients to 0\n"
				"  -i: 	Show info\n"
				"\n"
				); // printf
			continue;
		} // info

	} // for i

	close_fir(&fir);
	fpga_disconnect(&fpga);
	return 0;
}