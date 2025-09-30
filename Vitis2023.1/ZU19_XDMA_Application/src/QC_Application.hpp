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
bool QC_app();

//Initialize the passed unordered map with a specified schema
bool initParamMap(std::unordered_map<Params, int>& map, schema_t* schema);

bool updateParamMap(u32 paramNum, std::unordered_map<Params, int>& map, schema_t* schema);

//initialize firmware components/peripherals connected to system hardware
bool initializeHardware();

//set whether the system routes the cal output or the RF source to the system input
bool setCalMode(bool state);

//set frontend attenuation level 0-63 corresponds to 0-32.5dB in .5dB steps
bool setAttenuation(int attenuation);

//set whether the first rf switch enables bypassing filter bank 1
bool setBypassMode(bool state);

//configure the switches to route the signal through the specified filter bank
bool setFilterBank(Params filterBankParam);

//Program attenuation and path configuration into the frontend
bool programFrontend(std::unordered_map<Params, int>& map);

//program DDC firmware blocks
bool programDDC(volatile u32* bramBaseAddr, u32 mixingFreq, u32 sampleRateIndex);

bool configureHardware(std::unordered_map<Params, int>& map);

//basic setup of hardware. gpio & spi devices
bool sysInit();
