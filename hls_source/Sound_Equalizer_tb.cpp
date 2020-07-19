#include <iostream>
#include "Sound_Equalizer.h"
#include "hls_math.h"

const unsigned int array_len = 1000;
const double pi = 3.141592653589793238462643;

// тестируемая функция
void Sound_Equalizier(io_data_t  in_data,      // входные данные
					  io_data_t& out_data,     // выходные данные после фильтрации
					  fir_gain_t lpf_gain,     // коэффициент усиления ФНЧ
					  fir_gain_t bpf_gain,     // коэффициент усиления полосового фильтра
					  fir_gain_t hpf_gain      // коэффициент усиления ФВЧ
);

// функция для поиска максимального значения
double array_max_value(io_data_t* array, int start_idx){
    double max_val = (double)array[start_idx];
	for(int i = start_idx+1; i<array_len; i++){
		if ((double)array[i] > max_val)
			max_val = (double)array[i];
	}
    return max_val;
}

// функция для поиска минимального значения
double array_min_value(io_data_t* array, int start_idx){
    double min_val = (double)array[start_idx];
	for(int i = start_idx+1; i<array_len; i++){
		if ((double)array[i] < min_val)
			min_val = (double)array[i];
	}
    return min_val;
}


// функция для генерации синусоидального сигнала
// norm_freq - номированная частота, равная частоте в Гц к частоте дискретизации пополам
void generate_sin_array(double norm_freq, double amp, io_data_t* array){
    for(int i = 0; i<array_len; i++)
    	array[i] = (io_data_t)(amp*sin(pi*i*norm_freq));
}


// функция для вывода значений массива на экран
void print_array(io_data_t* array){
	for(int i = 0; i<array_len; i++)
		std::cout << "sin val["<<i<<"] = " << array[i] << std::endl;
}

// функция для вывода результатов теста
void print_test_res(fir_gain_t lpf_gain, fir_gain_t bpf_gain, fir_gain_t hpf_gain, double norm_freq, double amp, io_data_t* array){
	std::cout << "lpf gain = " << lpf_gain << std::endl;
	std::cout << "bpf gain = " << bpf_gain << std::endl;
	std::cout << "hpf gain = " << hpf_gain << std::endl;
	std::cout << "norm freq = " << norm_freq << std::endl;
	std::cout << "sin amp = " << amp << std::endl;
	std::cout << "max output value = " << array_max_value(array, 100) << std::endl;
	std::cout << "min output value = " << array_min_value(array, 100) << std::endl;
}

int main(){
	double norm_freq[3] = {0.2, 0.5, 0.8};
	double sin_amp;
	io_data_t in_data[array_len];
	io_data_t out_array[array_len];

	fir_gain_t lpf_gain;
    fir_gain_t bpf_gain;
	fir_gain_t hpf_gain;


	std::cout << "------------------------------------------------" << std::endl;
	std::cout << "----------- All Pass Test ----------------------" << std::endl;

	lpf_gain = 0.99999999999;
	bpf_gain = 0.99999999999;
	hpf_gain = 0.99999999999;
    sin_amp = 0.8;

    for (int j = 0; j<3; j++){
	    generate_sin_array(norm_freq[j], sin_amp, in_data);

	    for(int i = 0; i<array_len; i++)
	        Sound_Equalizier(in_data[i], out_array[i], lpf_gain, bpf_gain, hpf_gain);

	    print_test_res(lpf_gain, bpf_gain, hpf_gain, norm_freq[j], sin_amp, out_array);
	    std::cout << std::endl;
    }
    std::cout << std::endl;


	std::cout << "------------------------------------------------" << std::endl;
	std::cout << "----------- Low Pass Test ----------------------" << std::endl;

	lpf_gain = 0;
	bpf_gain = 0.99999999999;
	hpf_gain = 0.99999999999;
    sin_amp = 0.8;

    for (int j = 0; j<3; j++){
	    generate_sin_array(norm_freq[j], sin_amp, in_data);

	    for(int i = 0; i<array_len; i++)
	        Sound_Equalizier(in_data[i], out_array[i], lpf_gain, bpf_gain, hpf_gain);

	    print_test_res(lpf_gain, bpf_gain, hpf_gain, norm_freq[j], sin_amp, out_array);
	    std::cout << std::endl;
    }
    std::cout << std::endl;


	std::cout << "------------------------------------------------" << std::endl;
	std::cout << "----------- Band Pass Test ----------------------" << std::endl;

	lpf_gain = 0.99999999999;
	bpf_gain = 0;
	hpf_gain = 0.99999999999;
    sin_amp = 0.8;

    for (int j = 0; j<3; j++){
	    generate_sin_array(norm_freq[j], sin_amp, in_data);

	    for(int i = 0; i<array_len; i++)
	        Sound_Equalizier(in_data[i], out_array[i], lpf_gain, bpf_gain, hpf_gain);

	    print_test_res(lpf_gain, bpf_gain, hpf_gain, norm_freq[j], sin_amp, out_array);
	    std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "------------------------------------------------" << std::endl;
    std::cout << "----------- High Pass Test ----------------------" << std::endl;

    lpf_gain = 0.99999999999;
    bpf_gain = 0.99999999999;
    hpf_gain = 0;
    sin_amp = 0.8;

    for (int j = 0; j<3; j++){
        generate_sin_array(norm_freq[j], sin_amp, in_data);

        for(int i = 0; i<array_len; i++)
            Sound_Equalizier(in_data[i], out_array[i], lpf_gain, bpf_gain, hpf_gain);

    	    print_test_res(lpf_gain, bpf_gain, hpf_gain, norm_freq[j], sin_amp, out_array);
    	    std::cout << std::endl;
        }
    std::cout << std::endl;

	return 0;
}
