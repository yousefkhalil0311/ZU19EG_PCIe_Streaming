#include "schema_v1.hpp"

schema_t QC_SCHEMA = {
		.startToken = {START_ADR + 0x00, HEADER_TOKEN},
		.status = START_ADR + 0x01,
		.version = {START_ADR + 0x02, SCHEMA_VERSION},
		.hostTime = START_ADR + 0x03,
		.deviceTime = START_ADR + 0x04,
		.numParams = START_ADR + 0x05,
		.lastParam = LAST_PARAM_END_TOKEN,
		.paramStart = PARAM_START_TOKEN,
		.paramEnd = BRAM_END_TOKEN,
		.endToken = BRAM_END_TOKEN,
		.params = {nullptr}
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

//call any time schema differs from expected results. Will signal to host that schema setup failed
void failSchemaSetup(){
	QC_print("Schema Setup Failed\n");
	setStatusBit(BRAM_SCHEMA_RETURN);
	resetStatusBit(BRAM_SCHEMA_VALID);
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

	for (int param = 0; param < *(QC_SCHEMA.numParams); param++){

	}


}
