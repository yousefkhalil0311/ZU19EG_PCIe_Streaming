#include "hardwareInit.hpp"

//initializes AXI GPIO bus given XGpio instance and sets pin direction (I/O) for each channel (0 for output, 1 for input)
int GPIO_Init (XGpio *instance, u16 busID, u8 channel, u32 ch_direction, u32 ch_initState){

	XGpio_Initialize (instance, busID);
	XGpio_SetDataDirection(instance, channel, ch_direction);
	XGpio_DiscreteWrite(instance, channel, ch_initState);
	return XST_SUCCESS;

}

//Initializes SPI device
int SPI_Init (XSpi *instance, uint16_t bus_id){

	int Status;
	XSpi_Config *ConfigPtr;

	ConfigPtr = XSpi_LookupConfig(bus_id);
	if (ConfigPtr == NULL){
		return XST_DEVICE_NOT_FOUND;
	}

	Status = XSpi_CfgInitialize(instance, ConfigPtr, ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	Status = XSpi_SetOptions(instance, XSP_MASTER_OPTION | XSP_MANUAL_SSELECT_OPTION);
	if (Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	XSpi_Start(instance);
	XSpi_IntrGlobalDisable(instance);

	//SPI1_LSDAC initialized. Usage:
	//Set CS pin: 	XSpi_SetSlaveSelect(Instance, cs pin)
	//Transfer data:XSpi_Transfer(Instance, Tx_Data_buffer, Rx_Data_buffer, Buffer size in bytes)
	//Transfer function return XST_SUCCESS and XST_FAILURE

	return XST_SUCCESS;

}
