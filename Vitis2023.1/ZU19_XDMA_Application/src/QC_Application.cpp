#include "QC_Application.hpp"

using namespace std;

u32 FE_GPIO_CACHE = 0x00000000;
u32 Sys_GPIO_CACHE = 0x00000000;

//AXI GPIO instances
XGpio FE_GPIO;
XGpio Sys_GPIO;

//AXI SPI instances
XSpi  FE_SPI;


//Pointer to BRAM base address where PCIe commands will be stored
volatile u32* bram = (volatile u32*)BRAM_BASE_ADDRESS;

//Pointer to DDC block register space
volatile u32* DDC = (volatile u32*)DDC_BASE_ADDRESS;

unordered_map<Params, int> paramMap;

/*
 * entry point to application.
 * Separate from main because this project will
 * be a submodule to a larger system once validated
 *
 */

//clear serial terminal screen
void clearTerminal(){
	printf("\033[2J\033[H");
	return;
}

void QC_app(){

	bool status = sysInit();

	while(status == SUCCESS){
		clearTerminal();
		for (int offset = 0; offset < 150; offset+=3){
			QC_print("BRAM[%d] = 0x%X\t\t\tBRAM[%d] = 0x%X\t\t\tBRAM[%d] = 0x%X\n", offset, bram[offset], offset+1, bram[offset+1], offset+2, bram[offset+2]);
		}
		sleep(1);
	}

	xil_printf("Writing values to Ram\n");
	int index = 0x100>>2;
	DDC[index] = 12;
	DDC[index + 1] = 11;

	while(0){
		for(uint16_t i = 1; i < 600; i+=10){
			DDC[index] = i;
			usleep(3000000);
			xil_printf("Writing values to Ram: %d MHz\n", i);
		}
	}

	while(0) {
		xil_printf("Value stored into BRAM at address offset 0: %d, %d, %d, %d, %d, %d, %d, %d, %d\n", bram[0], bram[1], bram[2], bram[3], bram[4], bram[5], bram[6], bram[7], bram[8]);

		int aquisition_time_ms = bram[0];
		int fe_attenuation = bram[1];
		int fe_first_stage_bypass = bram[2];
		int fe_second_stage_filter_setting = bram[3];
		int cal_en = bram[4];
		int sys_active = bram[5];
		int num_samples = bram[6];
		int center_frequency = bram[7];
		int sample_rate = bram[8];

		if (fe_first_stage_bypass == 1){
			FE_GPIO_CACHE |= FE_BYP_BIT;
		}
		else{
			FE_GPIO_CACHE &= ~FE_BYP_BIT;
		}

		switch(fe_second_stage_filter_setting){
		case 1:
			FE_GPIO_CACHE &= ~FE_S11_BIT;
			FE_GPIO_CACHE &= ~FE_S12_BIT;
			FE_GPIO_CACHE |= FE_S21_BIT;
			FE_GPIO_CACHE &= ~FE_S22_BIT;
			break;
		case 2:
			FE_GPIO_CACHE &= ~FE_S11_BIT;
			FE_GPIO_CACHE |= FE_S12_BIT;
			FE_GPIO_CACHE |= FE_S21_BIT;
			FE_GPIO_CACHE |= FE_S22_BIT;
			break;
		case 3:
			FE_GPIO_CACHE |= FE_S11_BIT;
			FE_GPIO_CACHE &= ~FE_S12_BIT;
			FE_GPIO_CACHE &= ~FE_S21_BIT;
			FE_GPIO_CACHE &= ~FE_S22_BIT;
			break;
		case 4:
			FE_GPIO_CACHE |= FE_S11_BIT;
			FE_GPIO_CACHE |= FE_S12_BIT;
			FE_GPIO_CACHE &= ~FE_S21_BIT;
			FE_GPIO_CACHE |= FE_S22_BIT;
			break;
		default:
			break;
		}

		XGpio_DiscreteWrite(&FE_GPIO, 1, FE_GPIO_CACHE);

		if(cal_en){
			Sys_GPIO_CACHE |= NOISE_AMP_BIT;
			Sys_GPIO_CACHE |= CAL_SW_BIT;
		}
		else{
			Sys_GPIO_CACHE &= ~NOISE_AMP_BIT;
			Sys_GPIO_CACHE &= ~CAL_SW_BIT;
		}

		XGpio_DiscreteWrite(&Sys_GPIO, 1, Sys_GPIO_CACHE);

		XSpi_SetSlaveSelect(&FE_SPI, 1);

		u8 receiveBuffer[1];
		u8 transmitBuffer[1];

		transmitBuffer[0] = (u8)fe_attenuation;

		XSpi_Transfer(&FE_SPI, transmitBuffer, receiveBuffer, 1);

	}
}


//Initialized the passed unordered map with a specified schema
bool initParamMap(unordered_map<Params, int>& map, schema_t* schema){

	int numParams = *schema->numParams;
	QC_print("params on BRAM: %d\n", numParams);
	if(!assertInRange(numParams, 0, MAX_PARAMS)){
		QC_print("Corrupt number of params on BRAM: %d\n", numParams);
		return false;
	}

	for(int paramNum = 0; paramNum < numParams; paramNum++){

		QC_print("ParamNum %d\n", paramNum);

		//get param string from schema struct
		string paramName = schema->params[paramNum].keyString;

		//get param value from schema struct
		int paramValue = static_cast<int>(*schema->params[paramNum].valData);

		//find the enum Params corresponding to param String
		auto it = BRAMParamMapping.find(paramName);

		QC_print("paramName: %s\nparamVal: %d\n", paramName.c_str(), paramValue);


		//Verify param is expected
		if (it != BRAMParamMapping.end()){

			//set param value in the map
			map[it->second] = paramValue;

		}
		else{

			cout << "Failed to map parameter " << paramNum << ": " << paramName << " : unexpected param." << endl;

			return false;

		}
	}

	return true;

}

bool sysInit(){

	QC_print("Initializing...\n");

	GPIO_Init(&FE_GPIO, FE_GPIO_ID, 1, 0x0, 0x0);
	GPIO_Init(&Sys_GPIO, Sys_GPIO_ID, 1, 0x0, 0x0);

	SPI_Init(&FE_SPI, FE_SPI_ID);

	QC_print("Initializing Schema\n");
	int Status = FAILURE;
	int attemptNum = 0;

	while(attemptNum++ < 10 && Status == FAILURE){

		QC_print(".\n");

		Status = initSchema();

	}

	if (Status == SUCCESS){
		cout << "SCHEMA Init SUCCESS" << endl;
	}
	else{

		cout << "FAILURE to intialize SCHEMA" << endl;

		return FAILURE;

	}

	Status = initParamMap(systemParamMap, &QC_SCHEMA);

	return Status;


}
