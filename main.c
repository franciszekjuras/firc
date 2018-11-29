#include "firctrl.h"

int main(int argc, char ** argv){

	int print_status = (argc > 1 && strcmp(argv[1],"-s") == 0);

	if(!was_bitstream_loaded()){
		if(print_status) printf("FPGA_NO_BITSTREAM\n");
		printf("Error: No bitstream was loaded (or it wasn't loaded by lconf).\n");
		return -1;
	}

	fpga_t fpga = fpga_connect( FPGA_BASE_ADDR, FPGA_MEM_SIZE );
	if( !fpga.valid ) {
		if(print_status) printf("FPGA_NO_CONNECTION\n");
		fprintf(stderr, "Error: Couldn't establish connection with FPGA.\n");
		return -1;
	}
	fir_t fir = open_fir(&fpga); 

	if(strncmp(fir.conf->info, FIR_VER, strlen(FIR_VER)) != 0){
		if(print_status) printf("FPGA_BAD_BITSTREAM\n");
		fprintf(stderr, "FIR version is %.12s while %s* is expected\n", fir.conf->info, FIR_VER);
		return -1;
	}
	if(print_status) printf("FPGA_OK\n");



	for(int i=1; i < argc; i++) {
		if(strcmp(argv[i],"-t") == 0 || strcmp(argv[i],"--tm") == 0) { // zwraca rząd multipleksowania w czasie
			printf("%d\n", fir.conf->tm); continue;
		} // -t

		if(strcmp(argv[i], "--coef-sizeof") == 0) {
			printf("%d\n", sizeof(coef_t)); continue;
		}

		if(strcmp(argv[i],"-l") == 0 || strcmp(argv[i],"--load") == 0) { // ładowanie współczynników
			coef_t* dw_coefs = open_coefs("/tmp/firctrl/dw.dat", fir.conf->coefs_dwsamp_nr);
			coef_t* up_coefs= open_coefs("/tmp/firctrl/up.dat", fir.conf->coefs_upsamp_nr);
			coef_t* fir_coefs = open_coefs("/tmp/firctrl/cf.dat", fir.conf->coefs_max_nr);
			if(up_coefs == NULL || dw_coefs == NULL || fir_coefs == NULL){
				fprintf(stderr, "Error reading coefficients.\n");
				free(up_coefs); free(dw_coefs); free(fir_coefs);
				return -1;
			}
			int load_stat = 0; 
			load_stat += load_coefs_dw(fir, dw_coefs);
			load_stat += load_coefs_up(fir, up_coefs);
			load_stat += load_coefs_fir(fir, fir_coefs);

			free(up_coefs); free(dw_coefs); free(fir_coefs);
			if(load_stat != 0){
				fprintf(stderr, "Error loading coefficients.\n");
				return -1;
			}
			continue;
		}

		if(strcmp(argv[i],"--ndw") == 0) { // wypisanie maksymalnej liczby współczynników
			printf("%d\n", fir.conf->coefs_dwsamp_nr); continue;
		}

		if(strcmp(argv[i],"--nup") == 0) { // wypisanie maksymalnej liczby współczynników
			printf("%d\n", fir.conf->coefs_upsamp_nr); continue;
		}

		if(strcmp(argv[i],"--ncf") == 0 || strcmp(argv[i],"-n") == 0) { // wypisanie maksymalnej liczby współczynników
			printf("%d\n",fir.conf->coefs_max_nr); continue;
		}

		if(strcmp(argv[i],"--pdw") == 0) { // wypisanie precyzji współczynników
			printf("%d\n", fir.conf->src_coef_mag); continue;
		}

		if(strcmp(argv[i],"--pup") == 0) { // wypisanie precyzji współczynników
			printf("%d\n", fir.conf->src_coef_mag); continue;
		} // -c

		if(strcmp(argv[i],"--pcf") == 0 || strcmp(argv[i],"-p") == 0) { // wypisanie precyzji współczynników
			printf("%d\n",fir.conf->fir_coef_mag);
			continue;
		} // -c

		if(strcmp(argv[i],"-o") == 0) { // włącza filtr
			SWITCH_ON(SWITCH_FIR_EN, fir.conf->switches);
		} // -o

		if(strcmp(argv[i],"-x") == 0) { // wyłącza filtr
			SWITCH_OFF(SWITCH_FIR_EN, fir.conf->switches);
		} // -x

		if(strcmp(argv[i],"-z") == 0) { // zeruje wszystkie współczynniki
			zero_coefs(fir);
		} // -z

		if(strcmp(argv[i],"help") == 0 || strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"-help") == 0 || strcmp(argv[i],"--help") == 0) {
			printf( "FIRCTRL \n"
				"\n"
				"Usage: firctrl [arguments]\n"
				"\n"
				"Arguments:\n"
				"  -p: 	Check bitstreamem consistency\n"
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
		if(strcmp(argv[i],"-i") == 0 || strcmp(argv[i],"--info") == 0){
			printf("Info\n");
		}

	} // for i

	close_fir(&fir);
	fpga_disconnect(&fpga);
	return 0;
}