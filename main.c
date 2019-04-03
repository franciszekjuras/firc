#include "firctrl.h"
#include <time.h>

int main(int argc, char ** argv){

	int print_status = (argc > 1 && ( strcmp(argv[1],"-s") == 0 || strcmp(argv[1],"--status") == 0) );

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
		if(strcmp(argv[i],"-tm") == 0 || strcmp(argv[i],"--time-multiplexing") == 0){
			printf("%d\n", fir.conf->tm); continue;
		} 

		if(strcmp(argv[i], "-cs") == 0 || strcmp(argv[i],"--coef-size") == 0){
			printf("%d\n", sizeof(coef_t)); continue;
		}

		if(strcmp(argv[i],"--load") == 0) {
			coef_t* src_coefs = open_coefs("/tmp/srcker.dat", fir.conf->src_coefs_nr);
			coef_t* fir_coefs = open_coefs("/tmp/firker.dat", fir.conf->fir_coefs_nr);
			if(src_coefs == NULL || fir_coefs == NULL){
				fprintf(stderr, "Error reading coefficients.\n");
				free(fir_coefs); free(src_coefs);
				return -1;
			}
			int load_stat = 0;
			load_stat += load_coefs_dw(fir, src_coefs);
			load_stat += load_coefs_up(fir, src_coefs);
			load_stat += load_coefs_fir(fir, fir_coefs);

			free(src_coefs); free(fir_coefs);
			if(load_stat != 0){
				fprintf(stderr, "Error loading coefficients.\n");
				return -1;
			}
			continue;
		}

		if(strcmp(argv[i],"-sr") == 0 || strcmp(argv[i],"--src-rank") == 0){ 
			printf("%d\n", fir.conf->src_coefs_nr); continue;
		}

		if(strcmp(argv[i],"-fr") == 0 || strcmp(argv[i],"--fir-rank") == 0){ 
			printf("%d\n",fir.conf->fir_coefs_nr); continue;
		}

		if(strcmp(argv[i],"-sb") == 0 || strcmp(argv[i],"--src-blocks") == 0){ 
			printf("%d\n", fir.conf->src_dsp_nr); continue;
		}

		if(strcmp(argv[i],"-fb") == 0 || strcmp(argv[i],"--fir-blocks") == 0){ 
			printf("%d\n",fir.conf->fir_dsp_nr); continue;
		}

		if(strcmp(argv[i],"-sp") == 0 || strcmp(argv[i],"--src-precision") == 0){ 
			printf("%d\n", fir.conf->src_coef_mag); continue;
		}

		if(strcmp(argv[i],"-fp") == 0 || strcmp(argv[i],"--fir-precision") == 0){ 
			printf("%d\n",fir.conf->fir_coef_mag);
			continue;
		}

		if(strcmp(argv[i],"-bs") == 0 || strcmp(argv[i],"--base-shift") == 0){ 
			printf("%d\n",fir.conf->base_shift);
			continue;
		}

		if(strcmp(argv[i],"--enable") == 0){
			SWITCH_ON(SWITCH_FIR_EN, fir.conf->switches);

			//debug

			SWITCH_ON(SWITCH_FIR_SNAP, fir.conf->switches);

		    nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);


		    SWITCH_OFF(SWITCH_FIR_SNAP, fir.conf->switches);


		    FILE* dbg1 = fopen("/tmp/dbg_dws_endacc_out.txt", "w");
		    FILE* dbg2 = fopen("/tmp/dbg_dws_out.txt", "w");
		    FILE* dbg3 = fopen("/tmp/dbg_fir_in.txt", "w");
		    FILE* dbg4 = fopen("/tmp/dbg_fir_sample_con0.txt", "w");
		    int tm = fir.conf->tm; 
		    for(int i = 0; i < tm; ++i)
		        fprintf(dbg1, "%d\n", fir.samples[i]);
		    for(int i = 0; i < tm; ++i)
		        fprintf(dbg2, "%d\n", fir.samples[tm+i]);
		    for(int i = 0; i < tm; ++i)
		        fprintf(dbg3, "%d\n", fir.samples[(2*tm)+i]);
		    for(int i = 0; i < tm; ++i)
		        fprintf(dbg4, "%d\n", fir.samples[(3*tm)+i]);
		    fclose(dbg1);fclose(dbg2);fclose(dbg3);fclose(dbg4);

		    //debug end
		}

		if(strcmp(argv[i],"--disable") == 0){
			SWITCH_OFF(SWITCH_FIR_EN, fir.conf->switches);
		}

		if(strcmp(argv[i],"--zero") == 0){
			zero_coefs(fir);
		}

		if(strcmp(argv[i],"help") == 0 || strcmp(argv[i],"--help") == 0){
			printf( "FIRCTRL \n"
				"\n"
				"Usage: firctrl [arguments]\n"
				"\n"
				"Arguments:\n"
				"  [-tm | --time-multiplexing] Time multiplexing rank\n"
				"  [-cs | --coef-size] Single coefficient byte-size\n"
				"  [-sr | --src-rank] Sampling rate conversion kernel rank\n"
				"  [-fr | --fir-rank] FIR kernel rank\n"
				"  [-sb | --src-blocks] Sampling rate conversion blocks per one conversion\n"
				"  [-fb | --fir-blocks] FIR blocks\n"
				"  [-sp | --src-precision] Sampling rate conversion kernel fixed point precision\n"
				"  [-fp | --fir-precision] FIR fixed point precision\n"
				"  --load:      Load kernel\n"
				"  --enable:    Enable filter\n"
				"  --disable:   Disable filter\n"
				"  --zero:      Set all coefficients to 0\n"
				"  --info:      Show info\n"
				"\n"
				);
			continue;
		} 
		if(strcmp(argv[i],"--info") == 0){
			printf("Info\n");
		}

	} 

	close_fir(&fir);
	fpga_disconnect(&fpga);
	return 0;
}