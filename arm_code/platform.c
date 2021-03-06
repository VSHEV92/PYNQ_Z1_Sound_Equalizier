#include "xscugic.h"
#include "xscutimer.h"
#include "xuartps.h"
#include "xgpio.h"
#include "xsdps.h"
#include "xparameters.h"
#include "xsound_equalizier.h"

// инициализация контроллера прерываний и включаем прерывания
void init_intr(XScuGic* IntrInstPtr){
	XScuGic_Config *Intc_Config;
	Intc_Config = XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);
	XScuGic_CfgInitialize(IntrInstPtr, Intc_Config, Intc_Config->CpuBaseAddress);

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler,	IntrInstPtr);
	Xil_ExceptionEnable();
}

// инициализация таймера для получения отсчетов через GPIO
void init_timer(XScuTimer* TimerInstancePtr){

	XScuTimer_Config *ConfigPtr;
	ConfigPtr = XScuTimer_LookupConfig(XPAR_PS7_SCUTIMER_0_DEVICE_ID);
	XScuTimer_CfgInitialize(TimerInstancePtr, ConfigPtr, ConfigPtr->BaseAddr);
	XScuTimer_EnableAutoReload(TimerInstancePtr);

    // устанавливаем частоту дискретизации в 24000 Гц
	int TimerLoadValue =  XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ/2/24000;
	XScuTimer_LoadTimer(TimerInstancePtr, TimerLoadValue);
}

// инициализация SD Card
void init_SDCard(XSdPs* InstancePtr){
    XSdPs_Config *SD_ConfigPtr;
	SD_ConfigPtr = XSdPs_LookupConfig(XPAR_PS7_SD_0_DEVICE_ID);
	XSdPs_CfgInitialize(InstancePtr, SD_ConfigPtr, SD_ConfigPtr->BaseAddress);
	XSdPs_CardInitialize(InstancePtr);
}
// инициализация UART контроллера
void init_UART(XUartPs* InstPtr){
	XUartPs_Config *Uart_Config;
	Uart_Config = XUartPs_LookupConfig(XPAR_PS7_UART_0_DEVICE_ID);
	XUartPs_CfgInitialize(InstPtr, Uart_Config, Uart_Config->BaseAddress);

	// настраиваем параметры работы UART
	XUartPsFormat Uart_Configuration;
	Uart_Configuration.BaudRate = 9600;
	Uart_Configuration.DataBits = XUARTPS_FORMAT_8_BITS;
	Uart_Configuration.Parity = XUARTPS_FORMAT_NO_PARITY;
	Uart_Configuration.StopBits = XUARTPS_FORMAT_1_STOP_BIT;
	XUartPs_SetDataFormat(InstPtr, &Uart_Configuration);

	// устанавливаем, чтобы прерывание срабатывало при поступлении 7 байт
	XUartPs_SetInterruptMask(InstPtr, XUARTPS_IXR_RXOVR);
    XUartPs_SetFifoThreshold(InstPtr, 7);
}


// инициализация GPIO для выходных данных и управления мультиплексором
void init_GPIO_Out(XGpio* InstPtr){
	XGpio_Initialize(InstPtr, XPAR_AXI_GPIO_1_DEVICE_ID);
	// первый канал данные для блока фильтрации, второй - для управления мультплексором
	XGpio_DiscreteWrite(InstPtr, 1, 0);
	XGpio_DiscreteWrite(InstPtr, 2, 0); // данные идут от микрофона
}

// инициализация GPIO для входных данных
void init_GPIO_In(XGpio* InstPtr){
	XGpio_Initialize(InstPtr, XPAR_AXI_GPIO_0_DEVICE_ID);
}

// инициализация блока фильтрации
void init_Sound_Equalizier(XSound_equalizier* InstPtr){
	XSound_equalizier_Initialize(InstPtr, XPAR_SOUND_EQUALIZIER_0_DEVICE_ID);
	// устанавливаем максимальные коэффициенты усиления
	XSound_equalizier_Set_lpf_gain_V(InstPtr, 0xFFFF);
	XSound_equalizier_Set_bpf_gain_V(InstPtr, 0xFFFF);
	XSound_equalizier_Set_hpf_gain_V(InstPtr, 0xFFFF);
    // стартуем блок
	XSound_equalizier_EnableAutoRestart(InstPtr);
	XSound_equalizier_Start(InstPtr);
}
