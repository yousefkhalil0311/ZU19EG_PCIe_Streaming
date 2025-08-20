#include "schema_v1.hpp"

schema_t QC_SCHEMA = {
		.startToken = {START_ADR + 0x00, HEADER_TOKEN},
		.status 	= START_ADR + 0x01,
		.version 	= {START_ADR + 0x02, SCHEMA_VERSION},
		.hostTime 	= START_ADR + 0x03,
		.deviceTime = START_ADR + 0x04,
		.numParams 	= START_ADR + 0x05,
		.endHeader 	= {START_ADR + 0x06, HEADER_TOKEN},
		.paramStart = PARAM_START_TOKEN,
		.params 	= {nullptr},
		.keyValSep	= KEY_VALUE_SEP_TOKEN,
		.paramEnd 	= BRAM_END_TOKEN,
		.lastParam 	= LAST_PARAM_END_TOKEN,
		.endToken 	= BRAM_END_TOKEN
};

//setup QC to operate without a host or safely shut down
bool enableIndependentOp(){
	QC_print("Enabling independent operation.\n");
	setStatusBit(IND_OP_ACK);

	//TODO ->> return false for now
	resetStatusBit(IND_OP_ONLINE);
	return false;
}

//get status bit from bram status register
bool getStatusBit(uint32_t mask){
	return *(QC_SCHEMA.status) | mask;
}

//set status bit in bram status register
void setStatusBit(uint32_t mask){
	QC_print("Enabling independent operation.\n");
	*(QC_SCHEMA.status) |= mask;

}

//reset status bit in bram status register
void resetStatusBit(uint32_t mask){
	*(QC_SCHEMA.status) &= ~mask;
}

//returns whether the 2 params are equal
bool assertEquals(uint32_t value, uint32_t expected){
	if (value != expected){
		QC_print("assertEquals Failed: got 0x%08X and expected 0x%08X\n", value, expected);
		return false;
	}
	return true;
}

//returns whether the 2 params are equal
bool assertInRange(uint32_t value, uint32_t low, uint32_t high){
	if (value < low || value > high){
		QC_print("assertInRange Failed: got 0x%08X but expected range was (0x%08X, 0x%08X)\n", value, low, high);
		return false;
	}
	return true;
}

//call any time schema differs from expected results. Will signal to host that schema setup failed
void failSchemaSetup(){
	QC_print("Schema Setup Failed\n");
	setStatusBit(BRAM_SCHEMA_RETURN);
	resetStatusBit(BRAM_SCHEMA_VALID);
}

//store characters from bram register into parameter's keyString element
void bramCharsToKeyStrings(int param, volatile uint32_t* ptr, uint32_t numChars){

	for (uint32_t charNum = 0; charNum < numChars; charNum++){

		//4 chars per address (4 chars per uint32_t)
		uint32_t addrOffset = charNum / 4;

		uint32_t byteOffset = charNum % 4;

		QC_SCHEMA.params[param]->keyString[charNum] = (char)((*(ptr + addrOffset) >> (8 * (3 - byteOffset))) & 0xFF);
	}

}

bool initSchema(){

	if (*(QC_SCHEMA.status) | HOST_IND_OP_REQUEST){
		return enableIndependentOp();
	}

	//set bram register for device version for host to read
	*(QC_SCHEMA.version.bramData) = QC_SCHEMA.startToken.targetData;

	//Request host to populate bram param contents
	setStatusBit(BRAM_SETUP_REQUEST);

	//wait for host to declare parameter contents have been set
	while(!getStatusBit(HOST_SETUP_DONE));

	resetStatusBit(BRAM_SETUP_REQUEST);

	//begin param validation & populate schema values
	bool Status;

	//check start token
	Status = assertEquals(*(QC_SCHEMA.startToken.bramData), QC_SCHEMA.startToken.targetData);
	if(Status == FAILURE){
		failSchemaSetup();
		return Status;
	}

	//make sure # of params is valid
	Status = assertInRange(*(QC_SCHEMA.numParams), 0, MAX_PARAMS);
	if(Status == FAILURE){
		QC_print("Invalid numParams from host\n");
		failSchemaSetup();
		return Status;
	}

	//Verify header end Token present
	Status = assertEquals(*(QC_SCHEMA.endHeader.bramData), QC_SCHEMA.endHeader.targetData);
	if(Status == FAILURE){
		QC_print("Invalid header end Token from host\n");
		failSchemaSetup();
		return Status;
	}

	//next location after header end will be param data space
	volatile uint32_t* bramPtr = QC_SCHEMA.endHeader.bramData + 1;

	//validate each parameter
	for (uint32_t param = 0; param < *(QC_SCHEMA.numParams); param++){

		Status = assertEquals(*bramPtr, QC_SCHEMA.paramStart);
		if(Status == FAILURE){
			QC_print("Invalid paramStart Token from host at param index %d\n", param);
			failSchemaSetup();
			return Status;
		}

		//Instantiate param pointer
		QC_SCHEMA.params[param] = new bram_param_t;

		//Set param pointers
		QC_SCHEMA.params[param]->address = bramPtr++;
		QC_SCHEMA.params[param]->paramID = bramPtr++;
		QC_SCHEMA.params[param]->keySize = bramPtr++;

		//bram for keyData will hold offset from current location to start of keyString
		//following was done to get global location of keyString
		QC_SCHEMA.params[param]->keyData = bramPtr + *(bramPtr++);
		QC_SCHEMA.params[param]->valData = bramPtr++;

		//make sure key size is within limits
		Status = assertInRange(*(QC_SCHEMA.params[param]->keySize), 0, KEYSTRING_MAX_LENGTH);
		if(Status == FAILURE){
			QC_print("Invalid key length from host at param index %d\n", param);
			failSchemaSetup();
			return Status;
		}

		//make sure param separater token seen after valData and before key string
		Status = assertEquals(*bramPtr++, QC_SCHEMA.keyValSep);
		if(Status == FAILURE){
			QC_print("Invalid key value separator from host at param index %d\n", param);
			failSchemaSetup();
			return Status;
		}

		//make sure param key string is at the host specified location by comparing first 4 bytes
		Status = assertEquals(*bramPtr, *(QC_SCHEMA.params[param]->keyData));
		if(Status == FAILURE){
			QC_print("Invalid key String address at param index %d\n", param);
			failSchemaSetup();
			return Status;
		}

		bramCharsToKeyStrings(param, QC_SCHEMA.params[param]->keyData, *(QC_SCHEMA.params[param]->keySize));

		//set ptr to endParamToken
		bramPtr += KEYSTRING_MAX_LENGTH / 4;

		//make sure the parameter ends with and endParam token
		Status = assertEquals(*bramPtr++, QC_SCHEMA.paramEnd);
		if(Status == FAILURE){
			QC_print("Invalid end token at param index %d\n", param);
			failSchemaSetup();
			return Status;
		}

	}

	//bramPtr should be 1 address past the last param. We compare to make sure this location contains the last param token
	Status = assertEquals(*bramPtr, QC_SCHEMA.lastParam);
	if(Status == FAILURE){
		QC_print("Invalid end token after last param\n");
		failSchemaSetup();
		return Status;
	}


	return SUCCESS;
}
