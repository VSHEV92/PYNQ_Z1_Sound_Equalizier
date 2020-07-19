#include "Sound_Equalizer.h"
#include "FIR_Coeffs.h"

void Sound_Equalizier(io_data_t  in_data,      // входные данные
					  io_data_t& out_data,     // выходные данные после фильтрации
					  fir_gain_t lpf_gain,     // коэффициент усиления ФНЧ
					  fir_gain_t bpf_gain,     // коэффициент усиления полосового фильтра
					  fir_gain_t hpf_gain      // коэффициент усиления ФВЧ
){

// настроаиваем интерфейсы
#pragma HLS INTERFACE s_axilite port=return bundle=ctrl
#pragma HLS INTERFACE s_axilite port=lpf_gain bundle=ctrl
#pragma HLS INTERFACE s_axilite port=bpf_gain bundle=ctrl
#pragma HLS INTERFACE s_axilite port=hpf_gain bundle=ctrl

#pragma HLS INTERFACE ap_vld port=in_data
#pragma HLS INTERFACE ap_vld port=out_data

// тактовая частота 100 МГц, данные поступают со скоростью 24 кГц, поэтому требуемый II 4166
// он и так дастигается, поэтому ненакладаывем pragma на II

	data_t fir_lpf_output;
	data_t fir_bpf_output;
	data_t fir_hpf_output;
	static data_t fir_lpf_shift_reg[47] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	static data_t fir_bpf_shift_reg[47] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	static data_t fir_hpf_shift_reg[47] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	// фильтр нижних частот
	fir_lpf_output = 0;
	lpf_loop: for(int i = 46; i>0; i--){
		fir_lpf_shift_reg[i] = fir_lpf_shift_reg[i-1];
		fir_lpf_output += fir_lpf_shift_reg[i-1]*lpf_coeffs[i];
	}
	fir_lpf_shift_reg[0] = in_data;
	fir_lpf_output += in_data*lpf_coeffs[0];

	// полосовой фильтр
	fir_bpf_output = 0;
	bpf_loop: for(int i = 46; i>0; i--){
		fir_bpf_shift_reg[i] = fir_bpf_shift_reg[i-1];
		fir_bpf_output += fir_bpf_shift_reg[i-1]*bpf_coeffs[i];
	}
	fir_bpf_shift_reg[0] = in_data;
	fir_bpf_output += in_data*bpf_coeffs[0];

	// фильтр верхних частот
	fir_hpf_output = 0;
	hpf_loop: for(int i = 46; i>0; i--){
		fir_hpf_shift_reg[i] = fir_hpf_shift_reg[i-1];
		fir_hpf_output += fir_hpf_shift_reg[i-1]*hpf_coeffs[i];
	}
	fir_hpf_shift_reg[0] = in_data;
	fir_hpf_output += in_data*hpf_coeffs[0];

    //  взвешенная сумма выходных сигналов фильтров
	out_data = fir_lpf_output*lpf_gain + fir_bpf_output*bpf_gain + fir_hpf_output*hpf_gain;
}
