#ifndef _fir_h_
#define _fir_h_

struct fir_conf_s; typedef struct fir_conf_s fir_conf_t;
struct fir_s; typedef struct fir_s fir_t;

#include "firctrl.h"
#include "utils.h"
#include "fpga.h"

#define SWITCH_CON_EST 0
#define SWITCH_FIR_EN 1
#define SWITCH_FIR_UPDATE 2
#define SWITCH_FIR_SNAP 5

//offsets in fpga 32-bit addressing
#define BASE_SHIFT 7
#define FIR_COEFS_BASE 1
#define FIR_UPSAMPL_COEFS_BASE 81
#define FIR_DWSAMPL_COEFS_BASE 101
#define FIR_SAMPLES_OFF 1024

#define FIR_SAMPLES_NR 10
#define FIR_SAMPLES_DEPTH 5
#define FIR_BLOCK_SAMPLES_NR 1024
#define FIR_DEBUG_BLOCKS_NR 20

#define FIR_MAX_COEFS 10000
#define FIR_MAX_SRC_COEFS 3000
#define SWITCH_ON(bit,var) ((var)|=(1<<(bit)))
#define SWITCH_OFF(bit,var) ((var)&=(~(1<<(bit))))

struct fir_conf_s{
		//read-only
/*0:2*/	char 	info [12];
/*3*/	int32_t unused3;
		
/*4*/	int32_t coefs_max_nr; 
/*5*/	int32_t coefs_upsamp_nr;
/*6*/	int32_t coefs_dwsamp_nr;
/*7*/	int32_t unused7;

/*8*/	int32_t tm; // rząd multipleksowania
/*9*/	int32_t fir_dsp_nr;
/*10*/	int32_t upsamp_dsp_nr;
/*11*/	int32_t dwsamp_dsp_nr;

/*12*/	int32_t fir_coef_mag; // przecinek
/*13*/	int32_t src_coef_mag;
/*14*/	int32_t unused14;
/*15*/	int32_t unused15;

/*16*/	int32_t fir_coefs_base;
/*17*/	int32_t upsamp_coefs_base;
/*18*/	int32_t dwsamp_coefs_base;
/*19*/	int32_t unused19;
		//read|write
/*20*/	uint32_t switches; // zmienna zawierajaca bity kontrolne
/*21*/	int32_t coefs_crr_nr; // do wywalenia 
/*22*/	int32_t unused22;
/*23*/	int32_t unused23;

/*24*/	int32_t crr_debug_block;
/*25:27*/	int32_t debug_source[3];
};

struct fir_s{
	fir_conf_t* conf;
	int32_t* coefs;
	int32_t* dwsamp_coefs;
	int32_t* upsamp_coefs;
	int32_t* samples;
};

fir_t open_fir(const fpga_t* fpga);
int close_fir(fir_t* fir);

coef_t* open_coefs(const char* src, int coefs_nr);

int zero_coefs(fir_t fir);

int load_coefs_dw(fir_t fir, coef_t* coefs);
int load_coefs_up(fir_t fir, coef_t* coefs);
int load_coefs_fir(fir_t fir, coef_t* coefs);


#endif //_fir_h_
