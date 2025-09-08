#pragma once

//Standard C includes
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

//Standard C++ includes
#include <iostream>
#include <unordered_map>
#include <string>

//Xilinx specific includes
#include "xparameters.h"
#include "xgpio.h"
#include "xspi.h"

//Project specific includes
#include "schema_v1.hpp"
#include "hardwareInit.hpp"
#include "parameterMappings.hpp"

#define BRAM_BASE_ADDRESS	XPAR_AXI_BRAM_CTRL_1_S_AXI_BASEADDR
#define DDC_BASE_ADDRESS	XPAR_DUT_IP_0_BASEADDR

#define FE_GPIO_ID			XPAR_GPIO_0_DEVICE_ID
#define Sys_GPIO_ID			XPAR_GPIO_1_DEVICE_ID

#define FE_SPI_ID			XPAR_SPI_0_DEVICE_ID

//Frontend control bits
#define FE_PS_BIT 		(1 << 0)
#define FE_S11_BIT 		(1 << 1)
#define FE_S12_BIT 		(1 << 2)
#define FE_S21_BIT 		(1 << 3)
#define FE_S22_BIT 		(1 << 4)
#define FE_BYP_BIT 		(1 << 5)

//Noise and cal control bits
#define NOISE_AMP_BIT 	(1 << 0)
#define CAL_SW_BIT 		(1 << 1)

//clear serial terminal screen
void clearTerminal();

//entry point to application. Separate from main because this project will become a submodule to a larger system once validated
void QC_app();

//basic setup of hardware. gpio & spi devices
bool sysInit();
