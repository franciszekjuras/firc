#include "utils.h"

FILE* open_file_dialog(){
    FILE* file = NULL;
    char file_name[512];
    printf("Insert path to file: ");
    if(scanf("%s", file_name) == 1) 
        file = fopen(file_name, "r");
    return file;
}

// int dialog(const char* opt){//TODO: arbitrary length dialog
//     printf("Do you want to:\n1. %s\n2. %s\n3. %s\n",
//         opt1, opt2, opt3);
//     int choice = -1;
//     if(scanf("%d", &choice) < 1)
//         return -1;
//     else return choice;
// }


int read_coefs(FILE* file, int32_t* coefs, int max, int* overflow){
    int i = 0;
    int cread = 0;
    int coef_temp;
    int st;
    while((st = fscanf(file, "%d", &coef_temp)==1) && i < max){
        coefs[i] = coef_temp; i++;
    }
    if(i == 0)
        return -1;
    cread = i;
    for(;i < max;++i)
        coefs[i] = 0;
    if(st == 1)
        if(overflow != NULL)*overflow = 1;
    else
        if(overflow != NULL)*overflow = 0;

    return cread;
}

void display_info(fir_t* fir){
    printf("---------====< %s >====---------\n", fir->conf->info);
    printf("Time multiplexing rank: %d\n", fir->conf->tm);
    printf("Max. FIR coefficients: %d\n", fir->conf->coefs_max_nr);
    printf("Max. upsampling coef.: %d\n", fir->conf->coefs_upsamp_nr);
    printf("FIR coefficients loaded: %d\n", fir->conf->coefs_crr_nr);
    printf("Switches state: %d\n", fir->conf->switches);
    printf("-------------------------------------\n");
}