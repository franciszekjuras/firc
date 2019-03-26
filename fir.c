#include "fir.h"

fir_t open_fir(const fpga_t* fpga){
    fir_t fir;  
    fir.conf  = (fir_conf_t*)fpga->addr;
    fir.coefs = ((int32_t*)(fpga->addr)) + (fir.conf->fir_coefs_base << fir.conf->base_shift);
    fir.upsamp_coefs = ((int32_t*)(fpga->addr)) + (fir.conf->upsamp_coefs_base << fir.conf->base_shift);
    fir.dwsamp_coefs = ((int32_t*)(fpga->addr)) + (fir.conf->dwsamp_coefs_base << fir.conf->base_shift);
    fir.samples = ((int32_t*)(fpga->addr)) + fir.conf->dbg_offset;

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
        int bs = fir.conf->base_shift;
        int dsp_nr;
        dsp_nr = fir.conf->src_dsp_nr;
        for(int j = 0; j < dsp_nr; ++j)
            for(int i = 0; i < tm; ++i)
                fir.dwsamp_coefs[(j<<bs)+i] = 0;

        dsp_nr = fir.conf->src_dsp_nr;
        for(int j = 0; j < dsp_nr; ++j)
            for(int i = 0; i < tm; ++i)
                fir.upsamp_coefs[(j<<bs)+i] = 0;

        dsp_nr = fir.conf->fir_dsp_nr;
        for(int j = 0; j < dsp_nr; ++j)
            for(int i = 0; i < tm; ++i)
                fir.coefs[(j<<bs)+i] = 0;

        fprintf(stderr, "Done.\n");
    return 0;
}

int load_coefs_dw(fir_t fir, coef_t* coefs){
    fprintf(stderr, "Writing %d downsampling coefs to FPGA... ", fir.conf->src_coefs_nr);
        int k = 0; int tm = fir.conf->tm; int bs = fir.conf->base_shift;
        int dsp_nr = fir.conf->src_dsp_nr;
        coef_t mult = (coef_t)(1<<(fir.conf->src_coef_mag));
        for(int j = 0; j < dsp_nr; ++j)
            for(int i = tm-1; i >= 0; --i)
                fir.dwsamp_coefs[(j<<bs)+i] = (int32_t)round(mult*coefs[k++]);
        
        fprintf(stderr, "Done.\n");
    return 0;
}
int load_coefs_up(fir_t fir, coef_t* coefs){
    fprintf(stderr, "Writing %d upsampling coefs to FPGA... ", fir.conf->src_coefs_nr);
        int k = 0; int tm = fir.conf->tm; int bs = fir.conf->base_shift;
        int dsp_nr = fir.conf->src_dsp_nr;
        coef_t mult = (coef_t)((1<<(fir.conf->src_coef_mag)) * fir.conf->tm);
        for(int j = 0; j < dsp_nr; ++j)
            for(int i = 0; i < tm; ++i)
                fir.upsamp_coefs[(j<<bs)+i] = (int32_t)round(mult*coefs[k++]);

        fprintf(stderr, "Done.\n");
    return 0;
}
int load_coefs_fir(fir_t fir, coef_t* coefs){
    FILE* dbgWrF = fopen("/tmp/dbgwr.txt", "w");
    FILE* dbgCfF = fopen("/tmp/dbgcoefs.txt", "w");


    fprintf(stderr, "Writing %d FIR coefs to FPGA... ", fir.conf->fir_coefs_nr);
        int k = 0; int tm = fir.conf->tm; int bs = fir.conf->base_shift;
        int dsp_nr = fir.conf->fir_dsp_nr;
        coef_t mult = (coef_t)(1<<(fir.conf->fir_coef_mag));
        for(int i = 0; i < tm; ++i)
            for(int j = 0; j < dsp_nr; ++j){
                fir.coefs[(j<<bs)+i] = (int32_t)round(mult*coefs[k++]);
                fprintf(dbgWrF, "%o %d %o %d\n", (j<<bs)+i, (int32_t)round(mult*coefs[k++]), fir.conf->dbg_wr_addr, fir.conf->dbg_wr_data);
            }

        fprintf(stderr, "Done.\n");

    for(int i = 0; i < tm; ++i)
        fprintf(dbgCfF, "%d %d\n", (int32_t)round(mult*coefs[i * dsp_nr]), fir.samples[i]);
    fclose(dbgWrF);fclose(dbgCfF);

    return 0;
}