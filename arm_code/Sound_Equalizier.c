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
u16 TransferMode;
#define ADDRESS_BEYOND_32BIT	0x100000000U
s32 XSdPs_Write(XSdPs *InstancePtr, u32 Arg, u32 BlkCnt, const u8 *Buff);
s32 XSdPs_Read(XSdPs *InstancePtr, u32 Arg, u32 BlkCnt, u8 *Buff);
u8 SD_Ping_Buffer[512];
u8 SD_Pong_Buffer[512];
u8 Ping_Pong_Flag = 0;

u8 Test_Buffer[512];

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
				XSdPs_Write(&SD_Card, SD_Block_Count*SD_Block_Size, 1, SD_Ping_Buffer);
				Ping_Pong_Flag = 1;
			} else {
				XSdPs_Write(&SD_Card, SD_Block_Count*SD_Block_Size, 1, SD_Pong_Buffer);
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
				XSdPs_Read(&SD_Card, SD_Block_Count*SD_Block_Size, 1, SD_Ping_Buffer);
				Ping_Pong_Flag = 1;
			} else {
				XSdPs_Read(&SD_Card, SD_Block_Count*SD_Block_Size, 1, SD_Pong_Buffer);
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
		    XSdPs_Read(&SD_Card, 0*SD_Block_Size, 1, SD_Ping_Buffer);
		    XSdPs_Read(&SD_Card, 1*SD_Block_Size, 1, SD_Pong_Buffer);
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

	for(int i = 0; i<256; i++){
		Test_Buffer[i] = i;
		Test_Buffer[256+i] = i;
		}

	while(1){
	}

	return 0;
}

/*****************************************************************************/
/**
* This function performs SD write in polled mode.
*
* @param	InstancePtr is a pointer to the instance to be worked on.
* @param	Arg is the address passed by the user that is to be sent as
* 		argument along with the command.
* @param	BlkCnt - Block count passed by the user.
* @param	Buff - Pointer to the data buffer for a DMA transfer.
*
* @return
* 		- XST_SUCCESS if initialization was successful
* 		- XST_FAILURE if failure - could be because another transfer
* 		is in progress or command or data inhibit is set
*
******************************************************************************/
s32 XSdPs_Write(XSdPs *InstancePtr, u32 Arg, u32 BlkCnt, const u8 *Buff)
{
	s32 Status;
	u32 PresentStateReg;
	u32 StatusReg;

	if ((InstancePtr->HC_Version != XSDPS_HC_SPEC_V3) ||
				((InstancePtr->Host_Caps & XSDPS_CAPS_SLOT_TYPE_MASK)
				!= XSDPS_CAPS_EMB_SLOT)) {
		if(InstancePtr->Config.CardDetect != 0U) {
			/* Check status to ensure card is initialized */
			PresentStateReg = XSdPs_ReadReg(InstancePtr->Config.BaseAddress,
					XSDPS_PRES_STATE_OFFSET);
			if ((PresentStateReg & XSDPS_PSR_CARD_INSRT_MASK) == 0x0U) {
				Status = XST_FAILURE;
				goto RETURN_PATH;
			}
		}
	}

	/* Set block size to 512 if not already set */
	if( XSdPs_ReadReg(InstancePtr->Config.BaseAddress,
			XSDPS_BLK_SIZE_OFFSET) != XSDPS_BLK_SIZE_512_MASK ) {
		Status = XSdPs_SetBlkSize(InstancePtr,
			XSDPS_BLK_SIZE_512_MASK);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}

	}

	if (InstancePtr->Dma64BitAddr >= ADDRESS_BEYOND_32BIT) {
		XSdPs_SetupADMA2DescTbl64Bit(InstancePtr, BlkCnt);
	} else {
		XSdPs_SetupADMA2DescTbl(InstancePtr, BlkCnt, Buff);
		if (InstancePtr->Config.IsCacheCoherent == 0U) {
			Xil_DCacheFlushRange((INTPTR)Buff,
				BlkCnt * XSDPS_BLK_SIZE_512_MASK);
		}
	}

	if (BlkCnt == 1U) {
		TransferMode = XSDPS_TM_BLK_CNT_EN_MASK | XSDPS_TM_DMA_EN_MASK;

		/* Send single block write command */
		Status = XSdPs_CmdTransfer(InstancePtr, CMD24, Arg, BlkCnt);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}
	} else {
		TransferMode = XSDPS_TM_AUTO_CMD12_EN_MASK |
			XSDPS_TM_BLK_CNT_EN_MASK |
			XSDPS_TM_MUL_SIN_BLK_SEL_MASK | XSDPS_TM_DMA_EN_MASK;

		/* Send multiple blocks write command */
		Status = XSdPs_CmdTransfer(InstancePtr, CMD25, Arg, BlkCnt);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}
	}

	/* Write to clear bit */
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			XSDPS_NORM_INTR_STS_OFFSET, XSDPS_INTR_TC_MASK);

	Status = XST_SUCCESS;

	RETURN_PATH:
		return Status;
}

//*****************************************************************************/
///**
//* This function performs SD read in polled mode.
//*
//* @param	InstancePtr is a pointer to the instance to be worked on.
//* @param	Arg is the address passed by the user that is to be sent as
//* 		argument along with the command.
//* @param	BlkCnt - Block count passed by the user.
//* @param	Buff - Pointer to the data buffer for a DMA transfer.
//*
//* @return
//* 		- XST_SUCCESS if initialization was successful
//* 		- XST_FAILURE if failure - could be because another transfer
//* 		is in progress or command or data inhibit is set
//*
//******************************************************************************/
s32 XSdPs_Read(XSdPs *InstancePtr, u32 Arg, u32 BlkCnt, u8 *Buff)
{
	s32 Status;
	u32 PresentStateReg;
	u32 StatusReg;

	if ((InstancePtr->HC_Version != XSDPS_HC_SPEC_V3) ||
				((InstancePtr->Host_Caps & XSDPS_CAPS_SLOT_TYPE_MASK)
				!= XSDPS_CAPS_EMB_SLOT)) {
		if(InstancePtr->Config.CardDetect != 0U) {
			/* Check status to ensure card is initialized */
			PresentStateReg = XSdPs_ReadReg(InstancePtr->Config.BaseAddress,
					XSDPS_PRES_STATE_OFFSET);
			if ((PresentStateReg & XSDPS_PSR_CARD_INSRT_MASK) == 0x0U) {
				Status = XST_FAILURE;
				goto RETURN_PATH;
			}
		}
	}

	/* Set block size to 512 if not already set */
	if( XSdPs_ReadReg(InstancePtr->Config.BaseAddress,
			XSDPS_BLK_SIZE_OFFSET) != XSDPS_BLK_SIZE_512_MASK ) {
		Status = XSdPs_SetBlkSize(InstancePtr,
			XSDPS_BLK_SIZE_512_MASK);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}
	}

	if (InstancePtr->Dma64BitAddr >= ADDRESS_BEYOND_32BIT) {
		XSdPs_SetupADMA2DescTbl64Bit(InstancePtr, BlkCnt);
	} else {
		XSdPs_SetupADMA2DescTbl(InstancePtr, BlkCnt, Buff);
		if (InstancePtr->Config.IsCacheCoherent == 0U) {
			Xil_DCacheInvalidateRange((INTPTR)Buff,
				BlkCnt * XSDPS_BLK_SIZE_512_MASK);
		}
	}

	if (BlkCnt == 1U) {
		TransferMode = XSDPS_TM_BLK_CNT_EN_MASK |
			XSDPS_TM_DAT_DIR_SEL_MASK | XSDPS_TM_DMA_EN_MASK;

		/* Send single block read command */
		Status = XSdPs_CmdTransfer(InstancePtr, CMD17, Arg, BlkCnt);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}
	} else {
		TransferMode = XSDPS_TM_AUTO_CMD12_EN_MASK |
			XSDPS_TM_BLK_CNT_EN_MASK | XSDPS_TM_DAT_DIR_SEL_MASK |
			XSDPS_TM_DMA_EN_MASK | XSDPS_TM_MUL_SIN_BLK_SEL_MASK;

		/* Send multiple blocks read command */
		Status = XSdPs_CmdTransfer(InstancePtr, CMD18, Arg, BlkCnt);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}
	}

	/* Write to clear bit */
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			XSDPS_NORM_INTR_STS_OFFSET, XSDPS_INTR_TC_MASK);

	if (InstancePtr->Config.IsCacheCoherent == 0U) {
		Xil_DCacheInvalidateRange((INTPTR)Buff,
				BlkCnt * XSDPS_BLK_SIZE_512_MASK);
	}

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}
