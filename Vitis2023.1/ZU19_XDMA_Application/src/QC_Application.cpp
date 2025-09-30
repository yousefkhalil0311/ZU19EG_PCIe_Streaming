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

bool QC_app(){

	//communicate to host and initialize parameter control
	bool status = sysInit();
	if(status == FAILURE){
		QC_print("Failed to initialize system.\n");
		return status;
	}

	while(1){

		while (!getStatusBit(HOST_PARAM_CHANGE));

		setStatusBit(PARAM_CHANGE_ACK);

		while (!getStatusBit(PARAM_CHANGE_DONE));

		resetStatusBit(PARAM_CHANGE_ACK);

		u32 paramNumChanged = static_cast<u32> (getStatusParamChanged());

		//param changed should be parameter that exists
		int status = assertInRange(paramNumChanged, 0, getNumParams());
		if(status == FAILURE){
			QC_print("Parameter ID does not exist\n");
			return status;
		}

		//param changed should be parameter that exists
		status = updateParamMap(paramNumChanged, systemParamMap, &QC_SCHEMA);
		if(status == FAILURE){
			QC_print("Parameter does not exist\n");
			return status;
		}

		configureHardware(systemParamMap);

	}

}


//Initialize the passed unordered map with a specified schema
bool initParamMap(unordered_map<Params, int>& map, schema_t* schema){

	int numParams = *schema->numParams;
	QC_print("params on BRAM: %d\n", numParams);
	if(!assertInRange(numParams, 0, MAX_PARAMS)){
		QC_print("Corrupt number of params on BRAM: %d\n", numParams);
		return FAILURE;
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

			return FAILURE;

		}
	}

	return SUCCESS;

}

bool updateParamMap(u32 paramNum, unordered_map<Params, int>& map, schema_t* schema){

	QC_print("Changing param %d\n", paramNum);

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

		QC_print("Failed to find parameter\n");

		return FAILURE;

	}

	return SUCCESS;

}

//initialize firmware components/peripherals connected to system hardware
bool initializeHardware(){

	QC_print("Initializing hardware components...\n");

	GPIO_Init(&FE_GPIO, FE_GPIO_ID, 1, 0x0, 0x0);
	GPIO_Init(&Sys_GPIO, Sys_GPIO_ID, 1, 0x0, 0x0);

	SPI_Init(&FE_SPI, FE_SPI_ID);


	return SUCCESS;
}

//set whether the system routes the cal output or the RF source to the system input
bool setCalMode(bool state){

	QC_print("Changing Calibration Mode Settings to %d\n", (int)state);

	if(systemParamMap[CAL_EN]){
		Sys_GPIO_CACHE |= NOISE_AMP_BIT;
		Sys_GPIO_CACHE |= CAL_SW_BIT;
	}
	else{
		Sys_GPIO_CACHE &= ~NOISE_AMP_BIT;
		Sys_GPIO_CACHE &= ~CAL_SW_BIT;
	}

	XGpio_DiscreteWrite(&Sys_GPIO, 1, Sys_GPIO_CACHE);

	return SUCCESS;

}

//set frontend attenuation level 0-63 corresponds to 0-32.5dB in .5dB steps
bool setAttenuation(int attenuation){

	QC_print("Setting frontend attenuation to %d dB\n", attenuation);

	XSpi_SetSlaveSelect(&FE_SPI, 1);

	u8 receiveBuffer[1];
	u8 transmitBuffer[1];

	transmitBuffer[0] = (u8)attenuation;

	XSpi_Transfer(&FE_SPI, transmitBuffer, receiveBuffer, 1);

	return SUCCESS;
}

//set whether the first rf switch enables bypassing filter bank 1
bool setBypassMode(bool state){

	QC_print("Changing Bypass Mode Settings to %d\n", (int)state);

	if (systemParamMap[BYPASS_EN] == 1){
		FE_GPIO_CACHE |= FE_BYP_BIT;
	}
	else{
		FE_GPIO_CACHE &= ~FE_BYP_BIT;
	}

	XGpio_DiscreteWrite(&FE_GPIO, 1, FE_GPIO_CACHE);

	return SUCCESS;

}

//configure the switches to route the signal through the specified filter bank
bool setFilterBank(Params filterBankParam){

	QC_print("Changing Filter Mode Settings to %d\n", (int)filterBankParam);

	//TODO: check that this mapping is correct
	switch(filterBankParam){
	case BYPASS_EN:
			FE_GPIO_CACHE &= ~FE_S11_BIT;
			FE_GPIO_CACHE &= ~FE_S12_BIT;
			FE_GPIO_CACHE |= FE_S21_BIT;
			FE_GPIO_CACHE &= ~FE_S22_BIT;
			break;
	case LP500MHZ_EN:
			FE_GPIO_CACHE &= ~FE_S11_BIT;
			FE_GPIO_CACHE |= FE_S12_BIT;
			FE_GPIO_CACHE |= FE_S21_BIT;
			FE_GPIO_CACHE |= FE_S22_BIT;
			break;
	case LP1GHZ_EN:
			FE_GPIO_CACHE |= FE_S11_BIT;
			FE_GPIO_CACHE &= ~FE_S12_BIT;
			FE_GPIO_CACHE &= ~FE_S21_BIT;
			FE_GPIO_CACHE &= ~FE_S22_BIT;
			break;
	case LP2GHZ_EN:
			FE_GPIO_CACHE |= FE_S11_BIT;
			FE_GPIO_CACHE |= FE_S12_BIT;
			FE_GPIO_CACHE &= ~FE_S21_BIT;
			FE_GPIO_CACHE |= FE_S22_BIT;
			break;
	default:
		QC_print("Bad filter param given\n");
		break;
	}

	XGpio_DiscreteWrite(&FE_GPIO, 1, FE_GPIO_CACHE);

	return SUCCESS;

}

//Program attenuation and path configuration into the frontend
bool programFrontend(unordered_map<Params, int>& map){

	QC_print("Changing Frontend Settings\n");

	setBypassMode(map[BYPASS_EN]);

	setAttenuation(map[ATTENUATION_BVAL]);

	//only 1 should be true. In case of multiple being true by error, fall through safest state
	if(map[BYPASS_EN])setFilterBank(BYPASS_EN);

	if(map[LP2GHZ_EN])setFilterBank(LP2GHZ_EN);

	if(map[LP1GHZ_EN])setFilterBank(LP1GHZ_EN);

	if(map[LP500MHZ_EN])setFilterBank(LP500MHZ_EN);

	return SUCCESS;

}

//program DDC firmware blocks
bool programDDC(volatile u32* bramBaseAddr, u32 mixingFreq, u32 sampleRateIndex){

	QC_print("DDC register write: MIXING_FREQ Setting: %d MHz, SAMPLE_RATE Setting: %d\n");

	int index = 0x100>>2;

	//write registers in DDC block to set center frequency and sample rate
	DDC[index] = mixingFreq;
	DDC[index + 1] = sampleRateIndex;

	return SUCCESS;

}

bool configureHardware(unordered_map<Params, int>& map){

	programFrontend(map);

	setCalMode(map[CAL_EN]);

	int CF = map[DDC0_FMIX];
	int SR = map[DDC0_SFOUT];

	programDDC(DDC, CF, SR);

	return SUCCESS;

}

bool sysInit(){

	//intialize spi and gpio peripherals
	QC_print("Initializing hardware\n");
	initializeHardware();

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

	//copy BRAM contents into the systemParamMap. This protects against BRAM corruption due to host.
	Status = initParamMap(systemParamMap, &QC_SCHEMA);

	if (Status == SUCCESS){
		cout << "Param Map Init SUCCESS" << endl;
	}
	else{

		cout << "FAILURE to intialize parameter map" << endl;

		return FAILURE;

	}

	//Program hardware based on parameters stored in systemParamMap
	configureHardware(systemParamMap);

	return Status;


}
