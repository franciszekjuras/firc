#include "fir.h"

fir_t open_fir(const fpga_t* fpga){
    fir_t fir;  
    fir.conf  = (fir_conf_t*)fpga->addr;
    fir.coefs = ((int32_t*)(fpga->addr)) + (fir.conf->fir_coefs_base << BASE_SHIFT);
    fir.upsamp_coefs = ((int32_t*)(fpga->addr)) + (fir.conf->upsamp_coefs_base << BASE_SHIFT);
    fir.dwsamp_coefs = ((int32_t*)(fpga->addr)) + (fir.conf->dwsamp_coefs_base << BASE_SHIFT);
    fir.samples = ((int32_t*)(fpga->addr)) + FIR_SAMPLES_OFF;

    SWITCH_ON(SWITCH_CON_EST, fir.conf->switches);

    return fir;
}

int close_fir(fir_t* fir){
    SWITCH_OFF(SWITCH_CON_EST, fir->conf->switches);
    fir->conf = NULL; fir->coefs = NULL;
    return 0;
}


coef_t* open_coefs(const char* src, int coefs_nr){
    long size = 0;
    void* mem = file_to_mem(src, &size);
    if(size != (coefs_nr * sizeof(coef_t))){
        fprintf(stderr, "Number of read coefs (%ld) doesn't match expected number (%d)\nError reading coefs from: %s\n",(size/sizeof(coef_t)), coefs_nr, src);
        free(mem);
        return NULL;
    }
    return (coef_t*) mem;
}


int zero_coefs(fir_t fir){
    fprintf(stderr, "Clearing coefs... ");
        int tm = fir.conf->tm;
        int dsp_nr;
        dsp_nr = fir.conf->dwsamp_dsp_nr;
        for(int j = 0; j < dsp_nr; ++j)
            for(int i = 0; i < tm; ++i)
                fir.dwsamp_coefs[(j<<BASE_SHIFT)+i] = 0;

        dsp_nr = fir.conf->upsamp_dsp_nr;
        for(int j = 0; j < dsp_nr; ++j)
            for(int i = 0; i < tm; ++i)
                fir.upsamp_coefs[(j<<BASE_SHIFT)+i] = 0;

        dsp_nr = fir.conf->fir_dsp_nr;
        for(int j = 0; j < dsp_nr; ++j)
            for(int i = 0; i < tm; ++i)
                fir.coefs[(j<<BASE_SHIFT)+i] = 0;

        fprintf(stderr, "Done.\n");
    return 0;
}

int load_coefs_dw(fir_t fir, coef_t* coefs){
    fprintf(stderr, "Writing %d downsampling coefs to FPGA... ", fir.conf->coefs_dwsamp_nr);
        int k = 0; int tm = fir.conf->tm; 
        int dsp_nr = fir.conf->dwsamp_dsp_nr;
        coef_t mult = (coef_t)(1<<(fir.conf->src_coef_mag));
        for(int j = 0; j < dsp_nr; ++j)
            for(int i = tm-1; i >= 0; --i)
                fir.dwsamp_coefs[(j<<BASE_SHIFT)+i] = (int32_t)round(mult*coefs[k++]);
        
        fprintf(stderr, "Done.\n");
    return 0;
}
int load_coefs_up(fir_t fir, coef_t* coefs){
    fprintf(stderr, "Writing %d upsampling coefs to FPGA... ", fir.conf->coefs_upsamp_nr);
        int k = 0; int tm = fir.conf->tm; 
        int dsp_nr = fir.conf->upsamp_dsp_nr;
        coef_t mult = (coef_t)(1<<(fir.conf->src_coef_mag));
        for(int j = 0; j < dsp_nr; ++j)
            for(int i = 0; i < tm; ++i)
                fir.upsamp_coefs[(j<<BASE_SHIFT)+i] = (int32_t)round(mult*coefs[k++]);

        fprintf(stderr, "Done.\n");
    return 0;
}
int load_coefs_fir(fir_t fir, coef_t* coefs){
    fprintf(stderr, "Writing %d FIR coefs to FPGA... ", fir.conf->coefs_max_nr);
        int k = 0; int tm = fir.conf->tm;
        int dsp_nr = fir.conf->fir_dsp_nr;
        coef_t mult = (coef_t)(1<<(fir.conf->fir_coef_mag));
        for(int i = 0; i < tm; ++i)
            for(int j = 0; j < dsp_nr; ++j)
                fir.coefs[(j<<BASE_SHIFT)+i] = (int32_t)round(mult*coefs[k++]);

        fprintf(stderr, "Done.\n");
    return 0;
}