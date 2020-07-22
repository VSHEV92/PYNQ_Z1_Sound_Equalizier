#include "xil_printf.h"
#include "xgpio.h"
#include "xsound_equalizier.h"
#include "xscugic.h"
#include "xscutimer.h"
#include "xuartps.h"
#include "platform.h"

XGpio GPIO_In;
XGpio GPIO_Out;
XUartPs Uart_Port;
XScuGic InterruptController;
XScuTimer Sample_Timer;
XSound_equalizier Equalizier;

// входной буфер для UART
static u8 Uart_RX_Buffer[7];

// обработчик прерываний от таймера
void Timer_Handler(XScuTimer * TimerInstance)
{
	u32 Sound_Sample;
	//xil_printf("Hello\n");
	Sound_Sample = XGpio_DiscreteRead(&GPIO_In, 1);

	XGpio_DiscreteWrite(&GPIO_Out, 1, Sound_Sample);

	XScuTimer_ClearInterruptStatus(TimerInstance);
}


// обработчик прерывания при поступлении данные UART
void Uart_Intr_Handler(void *CallBackRef, u32 Event, unsigned int EventData)
{
	u16 lpf_gain;
	u16 bpf_gain;
	u16 hpf_gain;

	XUartPs_Recv(&Uart_Port, Uart_RX_Buffer, 7);
	if (Uart_RX_Buffer[0] == 255){
		lpf_gain = ((u16)Uart_RX_Buffer[1]) << 8;
		lpf_gain = lpf_gain | Uart_RX_Buffer[2];
		XSound_equalizier_Set_lpf_gain_V(&Equalizier, lpf_gain);

		bpf_gain = ((u16)Uart_RX_Buffer[3]) << 8;
		bpf_gain = bpf_gain | Uart_RX_Buffer[4];
		XSound_equalizier_Set_bpf_gain_V(&Equalizier, bpf_gain);

		hpf_gain = ((u16)Uart_RX_Buffer[5]) << 8;
		hpf_gain = hpf_gain | Uart_RX_Buffer[6];
		XSound_equalizier_Set_hpf_gain_V(&Equalizier, hpf_gain);
	} else if (Uart_RX_Buffer[0] == 254){
		XGpio_DiscreteWrite(&GPIO_Out, 2, Uart_RX_Buffer[1]);
	}
}

int main(){

	// инициализация контроллера прерываний и включаем прерывания
	init_intr(&InterruptController);

	// инициализация таймера для получения отсчетов через GPIO
	init_timer(&Sample_Timer);
	XScuGic_Connect(&InterruptController, XPAR_SCUTIMER_INTR, (Xil_ExceptionHandler) Timer_Handler, (void *) &Sample_Timer);
	XScuGic_Enable(&InterruptController, XPAR_SCUTIMER_INTR);
	XScuTimer_EnableInterrupt(&Sample_Timer);

	// подключаем обработчик прерываний для таймера

	// инициализация UART контроллера
	init_UART(&Uart_Port);

	// подключаем обработчик прерываний для UART
	XScuGic_Connect(&InterruptController, XPAR_XUARTPS_0_INTR, (Xil_ExceptionHandler) XUartPs_InterruptHandler, (void *) &Uart_Port);
	XUartPs_SetHandler(&Uart_Port, (XUartPs_Handler)Uart_Intr_Handler, &Uart_Port);
	XScuGic_Enable(&InterruptController, XPAR_XUARTPS_0_INTR);

	// инициализация GPIO для выходных данных и управления мультиплексором
	init_GPIO_Out(&GPIO_Out);

	// инициализация GPIO для входных данных
	init_GPIO_In(&GPIO_In);

	// инициализация блока фильтрации
	init_Sound_Equalizier(&Equalizier);

	// запускаем таймер
	XScuTimer_Start(&Sample_Timer);

	while(1){}

	return 0;
}
