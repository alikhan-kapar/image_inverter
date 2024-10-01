#include "xaxidma.h"
#include "xparameters.h"
#include "sleep.h"
#include "xil_cache.h"
#include "xuartps.h"
#include "xil_types.h"
#include "stdlib.h"

#define imagesize 512*512
#define headersize 1080
#define filesize imagesize+headersize

u32 checkHalted(u32 baseAddress,u32 offset);

int main(){

	u8 *imagedata;
	u32 receivedbytes = 0;
	u32 totalreceivedbytes = 0;
	u32 totaltransmittedbytes = 0;
	u32 transmittedbytes = 0;
	u32 status;

	XUartPs_Config *myUartConfig;
	XUartPs myUart;
	imagedata = malloc(sizeof(u8) * filesize);
	if (imagedata <= 0){
		print ("Memory allocation failed...\n\r");
		return -1;
	}
	myUartConfig = XUartPs_LookupConfig(XPAR_PSU_UART_0_DEVICE_ID);
	status = XUartPs_CfgInitialize(&myUart, myUartConfig, myUartConfig -> BaseAddress);
	if(status != XST_SUCCESS)
		print("Uart initialization failed...\n\r");
	status = XUartPs_SetBaudRate(&myUart, 115200);
	if(status != XST_SUCCESS)
		print("Uart baudrate init failed...\n\r");

	XAxiDma_Config *myDmaConfig;
	XAxiDma myDma;

	myDmaConfig = XAxiDma_LookupConfigBaseAddr(XPAR_AXI_DMA_0_BASEADDR);

	status = XAxiDma_CfgInitialize(&myDma, myDmaConfig);
	if(status != XST_SUCCESS){
		print("DMA initialization failed\n");
		return -1;
	}
	//print("DMA initialization success..\n\r");

	while(totalreceivedbytes < filesize){
		receivedbytes = XUartPs_Recv(&myUart, (u8*)&imagedata[totalreceivedbytes], 100);
		totalreceivedbytes += receivedbytes;

	}

	Xil_DCacheFlush();

	status = XAxiDma_SimpleTransfer(&myDma, (UINTPTR)&imagedata[headersize], imagesize,XAXIDMA_DEVICE_TO_DMA);
	if(status != XST_SUCCESS){
			print("DMA initialization failed\n");
			return -1;
	}
	status = XAxiDma_SimpleTransfer(&myDma, (UINTPTR)&imagedata[headersize], imagesize,XAXIDMA_DMA_TO_DEVICE);
	if(status != XST_SUCCESS){
		print("DMA initialization failed\n");
		return -1;
	}

	status = checkHalted(XPAR_AXI_DMA_0_BASEADDR,0x4);
	while(status != 1){
	   	status = checkHalted(XPAR_AXI_DMA_0_BASEADDR,0x4);
	}
	status = checkHalted(XPAR_AXI_DMA_0_BASEADDR,0x34);
	while(status != 1){
	   	status = checkHalted(XPAR_AXI_DMA_0_BASEADDR,0x34);
	}

	/*while ((XAxiDma_Busy(&myDma,XAXIDMA_DEVICE_TO_DMA) == TRUE) ||
				(XAxiDma_Busy(&myDma,XAXIDMA_DMA_TO_DEVICE) == TRUE)) {
			}*/

	//print("DMA transfer success..\n");

	/*for (int i=headersize; i<filesize; i++){
		imagedata[i] = 255 - imagedata[i];
	}*/

	Xil_ICacheInvalidate();

	while (totaltransmittedbytes < filesize){
		transmittedbytes = XUartPs_Send(&myUart, (u8*)&imagedata[totaltransmittedbytes], 1);
		totaltransmittedbytes += transmittedbytes;
		usleep(100);
	}

}

u32 checkHalted(u32 baseAddress,u32 offset){
	u32 status;
	status = (XAxiDma_ReadReg(baseAddress,offset))&XAXIDMA_HALTED_MASK;
	return status;
}
