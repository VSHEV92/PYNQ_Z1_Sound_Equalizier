#ifndef sound_eq_h
#define sound_eq_h

#include "ap_fixed.h"

typedef ap_fixed<16,1> fir_coeffs_t;
typedef ap_ufixed<16,0> fir_gain_t;

typedef ap_fixed<8,1,AP_TRN,AP_SAT> io_data_t;
typedef ap_fixed<24,2> data_t;

#endif
