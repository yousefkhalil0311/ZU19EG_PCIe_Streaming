#pragma once

//standard C includes
#include <stdint.h>

//Xilinx specific includes
#include <xparameters.h>

//Project specific includes
#include "IOFuncs.hpp"


#define SCHEMA_VERSION			0x00000001	//advertise to host schema version for compatibility

#define START_ADR				(volatile uint32_t*) XPAR_AXI_BRAM_CTRL_1_S_AXI_BASEADDR //base address to map bram

#define HEADER_TOKEN			0xDEADBEEF	//Indicate header/metadata start/end sequence
#define KEY_VALUE_SEP_TOKEN		0xBBBBBBBB	//Separator between key/value data in parameter
#define PARAM_START_TOKEN		0xCCCCCCCC	//Indicate start of individual parameter data
#define PARAM_END_TOKEN			0xAAAAAAAA	//Indicate end of individual parameter data
#define LAST_PARAM_END_TOKEN	0xABABABAB	//Indicate end of parameter space in bram
#define BRAM_END_TOKEN			0xEEEEEEEE	//Indicate end of bram usage (host has not written past this point)

#define KEYSTRING_MAX_LENGTH 	20
#define MAX_PARAMS				50

//host communication status register
#define HOST_PARAM_CHANGE	(1 << 31)
#define PARAM_CHANGE_ACK	(1 << 30)
#define PARAM_CHANGE_DONE	(1 << 29)
#define PARAM_CHANGE_STAT	(1 << 28)
#define BRAM_SETUP_REQUEST	(1 << 27)
#define HOST_SETUP_DONE		(1 << 26)
#define BRAM_SCHEMA_RETURN	(1 << 25)
#define BRAM_SCHEMA_VALID	(1 << 24)
#define HOST_IND_OP_REQUEST	(1 << 23)
#define IND_OP_ACK			(1 << 22)
#define IND_OP_ONLINE		(1 << 21)

#define SUCCESS	1
#define FAILURE 0

typedef struct{

	//data pointer to bram location
	volatile uint32_t* bramData;

	//expected value or data to store in bram location
	uint32_t targetData;

} bram_data_t;

typedef struct{

	//memory location of bram_param_t in bram
	volatile uint32_t* address;

	//address to id of param
	volatile uint32_t* paramID;

	//key length in chars (max 20)
	volatile uint32_t* keySize;

	//address to key string of param in bram
	volatile uint32_t* keyData;

	//address to value of param in bram
	volatile uint32_t* valData;
i
	//store key. Used for validation when edited
	char keyString[KEYSTRING_MAX_LENGTH];

} bram_param_t;

typedef struct{

	//Start token to validate BRAM contents are initialized
	bram_data_t startToken;

	//shared status reg to indicate device/host state
	//status Bit| 		Description
	//31		|		1: Host changed param; 0 otherwise
	//30		|		1: Device ack change request; 0 otherwise
	//29		|		1: Device ready to respond; 0 otherwise
	//28		|		1: param change success; 0 otherwise
	//27		|		1: Device requesting host to setup BRAM data
	//26		|		1: Host BRAM setup complete
	//25		|		1: Device done checking schema validity
	//24		|		1: BRAM schema matches device schema
	//23		|		1: Hosting requesting Device operate independently
	//22		|		1: Device ack independent mode request
	//21		|		1: independent mode successfully set up
	//15:0		|		On param change request; # of param changed

	volatile uint32_t* status;

	//Device Schema version used
	bram_data_t	version;

	//host PC timestamp
	volatile uint32_t* hostTime;

	//device timestamp
	volatile uint32_t* deviceTime;

	//number of parameters
	volatile uint32_t* numParams;

	//number of parameters
	bram_data_t endHeader;

	//Start param token
	uint32_t paramStart;

	//array pointing to parameter structs
	bram_param_t* params[MAX_PARAMS];

	//keyValue separator
	uint32_t keyValSep;

	//End param token
	uint32_t paramEnd;

	uint32_t lastParam;

	//token to specify end of bram usage
	uint32_t endToken;

} schema_t;

extern schema_t QC_SCHEMA;

//setup QC to operate without a host or safely shut down
bool enableIndependentOp();

//get status bit from bram status register
bool getStatusBit(uint32_t mask);

//set status bit in bram status register
void setStatusBit(uint32_t mask);

//reset status bit in bram status register
void resetStatusBit(uint32_t mask);

//returns whether the 2 params are equal
bool assertEquals(uint32_t value, uint32_t expected);

//call any time schema differs from expected results. Will signal to host that schema setup failed
void failSchemaSetup();


bool initSchema();

