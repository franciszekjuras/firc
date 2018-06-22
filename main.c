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

int main(void){
    //general variables:
    int32_t coefs[FIR_MAX_COEFS];
    int32_t upsamp_coefs[FIR_MAX_UPSAMPL_COEFS];
    int32_t* fir_coefs;
    fir_t fir;
    fpga_t fpga;

    //mapping fpga memory:
    fpga= fpga_connect(FPGA_BASE_ADDR ,FPGA_MEM_SIZE);
    if(!fpga.valid){
        fprintf(stderr, "Couldn't establish connection with FPGA.\n"); return -1;
    }
    fir = open_fir(&fpga);

    display_info(&fir);

    int choice; int cont = 1;

    if(FIR_MAX_COEFS < fir.conf->coefs_max_nr){
        fprintf(stderr, 
            "Only %d coefficients can be read,\
while FIR filter can take %d.\n\
Change internal constant FIR_MAX_COEFS.\n",
            FIR_MAX_COEFS, fir.conf->coefs_max_nr);
        close_fir(&fir); fpga_disconnect(&fpga);
        return -1;
    }

    if(FIR_MAX_UPSAMPL_COEFS < fir.conf->coefs_upsamp_nr){
        fprintf(stderr, 
            "Only %d coefficients can be read,\
while upsampling filter can take %d.\n\
Change internal constant FIR_MAX_UPSAMPL_COEFS.\n",
            FIR_MAX_UPSAMPL_COEFS, fir.conf->coefs_upsamp_nr);
        close_fir(&fir); fpga_disconnect(&fpga);
        return -1;
    }

    while(cont){
        //TODO: add some tests for fpga connection
        printf("Do you want to:\n\
0. Update upsampling coefficients\n\
1. Update FIR coefficients\n\
2. Enable filter\n\
3. Disable filter\n\
4. Display info\n\
5. Clear filter\n\
6. Choose snap source\n\
7. Enable snap record\n\
8. Disable snap record and take snapshot\n\
9. Block ram snapshot\n");
        if(scanf("%d", &choice) < 1){
            cont = 0;
            continue;
        }

        if(choice == 0){
            FILE* coef_file = open_file_dialog();
            if(coef_file == NULL){
                printf("Error: Couldn't open coefficients' file\n");
                continue;
            }

            int overflow; 
            int coefs_read = read_coefs(coef_file, upsamp_coefs, fir.conf->coefs_upsamp_nr, &overflow); 
            fclose(coef_file);       
            if(coefs_read < 0){
                fprintf(stderr, "Error: Failed reading coefficients.\n");
                continue;
            }
            fprintf(stdout, "Read %d coefficients.\n", coefs_read);
            if(overflow == 1)
                fprintf(stderr, "Warning: Not all coefficients have been read.\n");
            if(coefs_read != fir.conf->coefs_upsamp_nr){
                fprintf(stderr, "Error: Number of coefs %d doesn't match expected number of coefs %d.\n", coefs_read, fir.conf->coefs_upsamp_nr);
            }
            else{
                fprintf(stdout, "Writing %d coefs to FPGA... ", coefs_read);
                int k = 0; int tm = fir.conf->tm; int upsamp_dsp_nr = fir.conf->upsamp_dsp_nr;
                for(int j = 0; j < upsamp_dsp_nr; ++j)
                    for(int i = 0; i < tm; ++i)
                        fir.upsamp_coefs[(j<<BASE_SHIFT)+i] = upsamp_coefs[k++];
                
                fprintf(stdout, "Done.\n");
            }   
        }
        else if(choice == 1){
            FILE* coef_file = open_file_dialog();
            if(coef_file == NULL){
                printf("Error: Couldn't open coefficients' file\n");
                continue;
            }

            int overflow; 
            int coefs_read = read_coefs(coef_file, coefs, fir.conf->coefs_max_nr, &overflow); 
            fclose(coef_file);       
            if(coefs_read < 0){
                fprintf(stderr, "Error: Failed reading coefficients.\n");
                continue;
            }
            fprintf(stdout, "Read %d coefficients.\n", coefs_read);
            if(overflow == 1)
                fprintf(stderr, "Warning: Not all coefficients have been read.\n");
            
            fprintf(stdout, "Writing %d coefs to FPGA... ", coefs_read);
            int k = 0; int tm = fir.conf->tm; int fir_dsp_nr = fir.conf->fir_dsp_nr;
            for(int i = 0; i < tm; ++i)
                for(int j = 0; j < fir_dsp_nr; ++j)
                    fir.coefs[(j<<BASE_SHIFT)+i] = coefs[k++];
            fir.conf->coefs_crr_nr = coefs_read;
            fprintf(stdout, "Done.\n");
        }

        else if(choice == 2){ //merge into switch en/dis
            SWITCH_ON(SWITCH_FIR_EN, fir.conf->switches);
            fprintf(stdout, "FIR enabled.\n");
        }

        else if(choice == 3){
            SWITCH_OFF(SWITCH_FIR_EN, fir.conf->switches);
            fprintf(stdout, "FIR disabled.\n");
        }
        else if(choice == 4){
            display_info(&fir);
        }
        else if(choice == 5){ //clear filter
            for(int i = 0; i < fir.conf->coefs_max_nr; i++)
                fir.coefs[i] = 0;
        }
        else if(choice == 6){
            int debug_source;
            fprintf(stdout, "Choose debug source: ");
            if(scanf("%d", &debug_source) == 1)
                fir.conf->debug_source = debug_source;
            else
                fprintf(stdout, "Debug source must be an integer\n");
        }
        else if(choice == 7){
            SWITCH_ON(SWITCH_FIR_SNAP, fir.conf->switches);
        }
        else if(choice == 8){
            SWITCH_OFF(SWITCH_FIR_SNAP, fir.conf->switches);
            FILE* snaps_file = fopen("snaps.dat","w");
            if(snaps_file == NULL){
                printf("Couldn't open file snaps.dat for writing.\n");
                continue;
            }
            for(int i = 0; i < FIR_SAMPLES_NR; ++i){
                for(int j = 0; j < FIR_SAMPLES_DEPTH; ++j){
                    fprintf(snaps_file, "%d\t", fir.samples[(i*FIR_SAMPLES_DEPTH)+j]);
                }
                fprintf(snaps_file, "\n");
            }
            fclose(snaps_file);
        }
        else if(choice == 9){
            SWITCH_OFF(SWITCH_FIR_SNAP, fir.conf->switches);
            FILE* snaps_file = fopen("snaps.dat","w");
            if(snaps_file == NULL){
                printf("Couldn't open file snaps.dat for writing.\n");
                continue;
            }
            for(int i = 0; i < FIR_DEBUG_BLOCKS_NR; ++i){
                fir.conf->crr_debug_block = i;
                for(int j = 0; j < FIR_BLOCK_SAMPLES_NR; ++j){
                    fprintf(snaps_file, "%d\n", fir.samples[j]);
                }
            }
            fclose(snaps_file);
        }
        // else if(choice == 7){
        //     printf("SIGNAL\n-----------------\n");
        //     SWITCH_ON(SWITCH_FIR_TRIGGER, fir.conf->switches);
        //     SWITCH_OFF(SWITCH_FIR_TRIGGER, fir.conf->switches);
        //     for(int i = 0; i < fir.conf->coefs_max_nr; i++){
        //         fprintf(data_file,"%f\n",((double)fir.samples[i])/(1<<13));
        //     }
        //     fprintf(data_file, "0\n0.02\n0.04\n0.06\n0.04\n0.02\n0\n-0.02\n-0.04\n-0.06\n-0.04\n-0.02\n0\n");
        // }
        else
            cont = 0;

    }
    close_fir(&fir);
    fpga_disconnect(&fpga);

    return 0;
}