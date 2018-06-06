#ifndef _fir_h_
#define _fir_h_

struct fir_conf_s; typedef struct fir_conf_s fir_conf_t;
struct fir_s; typedef struct fir_s fir_t;

#include "firctrl.h"
#include "fpga.h"

#define SWITCH_CON_EST 0
#define SWITCH_FIR_EN 1
#define SWITCH_FIR_UPDATE 2
#define SWITCH_FIR_SNAP 5

//offsets in fpga 32-bit addressing
#define BASE_SHIFT 7
#define FIR_COEFS_BASE 1
#define FIR_UPSAMPL_COEFS_BASE 81
#define FIR_SAMPLES_OFF 2048

#define FIR_SAMPLES_NR 10
#define FIR_SAMPLES_DEPTH 5
#define FIR_BLOCK_SAMPLES_NR 2048
#define FIR_DEBUG_BLOCKS_NR 59

#define FIR_MAX_COEFS 6400
#define FIR_MAX_UPSAMPL_COEFS 2560
#define SWITCH_ON(bit,var) ((var)|=(1<<(bit)))
#define SWITCH_OFF(bit,var) ((var)&=(~(1<<(bit))))

struct fir_conf_s{
		//read-only
/*0:1*/	char 	info [8];
/*2*/	int32_t unused2;
/*3*/	int32_t unused3;
		
/*4*/	int32_t coefs_max_nr;
/*5*/	int32_t coefs_upsamp_nr;
/*6*/	int32_t coefs_dwsamp_nr;
/*7*/	int32_t unused7;

/*8*/	int32_t tm;
/*9*/	int32_t fir_dsp_nr;
/*10*/	int32_t upsamp_dsp_nr;
/*11*/	int32_t dwsamp_dsp_nr;

/*12*/	int32_t fir_coef_mag;
/*13*/	int32_t src_coef_mag;
/*14*/	int32_t unused14;
/*15*/	int32_t unused15;

/*16*/	int32_t fir_coefs_base;
/*17*/	int32_t upsamp_coefs_base;
/*18*/	int32_t dwsamp_coefs_base;
/*19*/	int32_t unused19;
		//read|write
/*20*/	uint32_t switches;
/*21*/	int32_t coefs_crr_nr;
/*22*/	int32_t crr_debug_block;
};

struct fir_s{
	fir_conf_t* conf;
	int32_t* coefs;
	int32_t* upsamp_coefs;
	int32_t* samples;
};

fir_t open_fir(const fpga_t* fpga);
int close_fir(fir_t* fir);


#endif //_fir_h_
