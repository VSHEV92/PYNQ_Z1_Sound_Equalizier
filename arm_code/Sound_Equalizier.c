#include "xgpio.h"
#include "xsound_equalizier.h"
#include "xscugic.h"
#include "xscutimer.h"
#include "xsdps.h"
#include "xuartps.h"
#include "platform.h"

XGpio GPIO_In;
XGpio GPIO_Out;
XSdPs SD_Card;
XUartPs Uart_Port;
XScuGic InterruptController;
XScuTimer Sample_Timer;
XSound_equalizier Equalizier;

// SD Card
#define SD_Block_Size 512
u8 SD_Ping_Buffer[512];
u8 SD_Pong_Buffer[512];
u8* SD_Addr;
u8 Ping_Pong_Flag = 0;

// входной буфер для UART
static u8 Uart_RX_Buffer[7];

// флаги для работы с sdcard
u8 Rec_Flag = 0;
u8 Play_Flag = 0;
u8 SD_Read = 0;
u8 SD_Write = 0;

// счетчики работы с sdcard
u16 Max_Block_Count = 0;
u16 Sample_Count = 0;
static u16 SD_Block_Count = 0;

// обработчик прерываний от таймера
void Timer_Handler(XScuTimer * TimerInstance)
{
    // запись на SD
	if (Rec_Flag == 1){
		// записываем отсчеты в буфер
		if (Ping_Pong_Flag == 0)
        	SD_Ping_Buffer[Sample_Count] = XGpio_DiscreteRead(&GPIO_In, 1);
		else
			SD_Pong_Buffer[Sample_Count] = XGpio_DiscreteRead(&GPIO_In, 1);

		Sample_Count++;
		if (Sample_Count == SD_Block_Size){
			Sample_Count = 0;
            // записываем блок в sd card
			if (Ping_Pong_Flag == 0){
				SD_Addr = SD_Ping_Buffer;
				SD_Write = 1;
				Ping_Pong_Flag = 1;
			} else {
				SD_Addr = SD_Pong_Buffer;
				SD_Write = 1;
				Ping_Pong_Flag = 0;
			}
			SD_Block_Count++;
			XUartPs_Send(&Uart_Port, (u8*)(&SD_Block_Count), 2);
		}
	}

	// чтение с SD
	if (Play_Flag == 1){
		// выдаем отсчеты из буфера
		if (Ping_Pong_Flag == 0)
		    XGpio_DiscreteWrite(&GPIO_Out, 1, SD_Ping_Buffer[Sample_Count]);
		else
			XGpio_DiscreteWrite(&GPIO_Out, 1, SD_Pong_Buffer[Sample_Count]);

		Sample_Count++;
		if (Sample_Count == SD_Block_Size){
			Sample_Count = 0;
			SD_Block_Count++;
			if (SD_Block_Count == Max_Block_Count)
				SD_Block_Count = 0;
			// считываем следующий блок из SD
			if (Ping_Pong_Flag == 0){
				SD_Addr = SD_Ping_Buffer;
				SD_Read = 1;
				Ping_Pong_Flag = 1;
			} else {
				SD_Addr = SD_Pong_Buffer;
				SD_Read = 1;
				Ping_Pong_Flag = 0;
			}
			XUartPs_Send(&Uart_Port, (u8*)(&SD_Block_Count), 2);
		}
	}

	XScuTimer_ClearInterruptStatus(TimerInstance);
}


// обработчик прерывания при поступлении данные UART
void Uart_Intr_Handler(void *CallBackRef, u32 Event, unsigned int EventData)
{
	u16 lpf_gain;
	u16 bpf_gain;
	u16 hpf_gain;

	if (Event == XUARTPS_EVENT_RECV_DATA) {
	    XUartPs_Recv(&Uart_Port, Uart_RX_Buffer, 7);
	    // управление фильтрами
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
	    // управление мультиплексором
	    } else if (Uart_RX_Buffer[0] == 254){
		    XGpio_DiscreteWrite(&GPIO_Out, 2, Uart_RX_Buffer[1]);
		// старт записи на SD
	    } else if (Uart_RX_Buffer[0] == 253){
		    Rec_Flag = Uart_RX_Buffer[1];
		    Sample_Count = 0;
		    SD_Block_Count = 0;
		    Ping_Pong_Flag = 0;
		// старт чтения с SD
	    } else if (Uart_RX_Buffer[0] == 252){
		    Play_Flag = Uart_RX_Buffer[1];
		    Sample_Count = 0;
		    Ping_Pong_Flag = 0;
		    XSdPs_ReadPolled(&SD_Card, 0*SD_Block_Size, 1, SD_Ping_Buffer);
		    XSdPs_ReadPolled(&SD_Card, 1*SD_Block_Size, 1, SD_Pong_Buffer);
		    SD_Block_Count = 1;
		// число циклически считываемых блоков
	    } else if (Uart_RX_Buffer[0] == 251){
	    	Max_Block_Count = ((u16)Uart_RX_Buffer[1]) << 8;
	    	Max_Block_Count = Max_Block_Count | Uart_RX_Buffer[2];
	    }
	}
}

int main(){

	// инициализация контроллера прерываний и включаем прерывания
	init_intr(&InterruptController);

	// инициализация таймера для получения отсчетов через GPIO
	init_timer(&Sample_Timer);

	// подключаем обработчик прерываний для таймера
	XScuGic_Connect(&InterruptController, XPAR_SCUTIMER_INTR, (Xil_ExceptionHandler) Timer_Handler, (void *) &Sample_Timer);
	XScuGic_Enable(&InterruptController, XPAR_SCUTIMER_INTR);
	XScuTimer_EnableInterrupt(&Sample_Timer);

	// инициализация UART контроллера
	init_UART(&Uart_Port);

	// подключаем обработчик прерываний для UART
	XScuGic_Connect(&InterruptController, XPAR_XUARTPS_0_INTR, (Xil_ExceptionHandler) XUartPs_InterruptHandler, (void *) &Uart_Port);
	XUartPs_SetHandler(&Uart_Port, (XUartPs_Handler)Uart_Intr_Handler, &Uart_Port);
	XScuGic_Enable(&InterruptController, XPAR_XUARTPS_0_INTR);

	// инициализация SD Card
	init_SDCard(&SD_Card);

	// инициализация GPIO для выходных данных и управления мультиплексором
	init_GPIO_Out(&GPIO_Out);

	// инициализация GPIO для входных данных
	init_GPIO_In(&GPIO_In);

	// инициализация блока фильтрации
	init_Sound_Equalizier(&Equalizier);

	// запускаем таймер
	XScuTimer_Start(&Sample_Timer);

	while(1){

		if (SD_Write == 1) {
			XSdPs_WritePolled(&SD_Card, SD_Block_Count*SD_Block_Size, 1, SD_Addr);
			SD_Write = 0;
		}

		if (SD_Read == 1) {
			XSdPs_ReadPolled(&SD_Card, SD_Block_Count*SD_Block_Size, 1, SD_Addr);
			SD_Read = 0;
		}
	}

	return 0;
}
