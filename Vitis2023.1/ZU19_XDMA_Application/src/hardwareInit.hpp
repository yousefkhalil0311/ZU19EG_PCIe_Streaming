#pragma once

//Xilinx specific includes
#include "xparameters.h"
#include "xgpio.h"
#include "xspi.h"

//C++ standard includes
#include <cstdint>

//initializes AXI GPIO bus given XGpio instance and sets pin direction (I/O) for each channel (0 for output, 1 for input)
int GPIO_Init (XGpio *instance, u16 busID, u8 channel, u32 ch_direction, u32 ch_initState);

//Initializes SPI device
int SPI_Init (XSpi *instance, uint16_t bus_id);
