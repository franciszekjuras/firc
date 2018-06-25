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