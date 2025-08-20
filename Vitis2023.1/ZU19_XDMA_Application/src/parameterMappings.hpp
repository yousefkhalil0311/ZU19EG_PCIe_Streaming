#pragma once

//Standard C++ includes
#include <iostream>
#include <unordered_map>
#include <string>

enum Params {

	//channel enable params
	CH0_EN,
	CH1_EN,
	CH2_EN,
	CH3_EN,
	CH4_EN,
	CH5_EN,
	CH6_EN,
	CH7_EN,

	//ddc enable params
	DDC0_EN,
	DDC1_EN,
	DDC2_EN,

	//ddc control params
	DDC0_FMIX,
	DDC0_SFOUT,
	DDC1_FMIX,
	DDC1_SFOUT,
	DDC2_FMIX,
	DDC2_SFOUT,

	//filter params
	LP500MHZ_EN,
	LP1GHZ_EN,
	LP2GHZ_EN,
	BYPASS_EN,

	//attenuation params
	ATTENUATION_BVAL,

	//system state params
	SYSTEM_EN,
	CAL_EN,

	//acquisition params
	ACQUIREBYSAMPLES,
	ACQUIREBYTIME_MS,
	ACQUISITIONTIME_MS,
	NUMSAMPLES_CAPTURE

};

extern std::unordered_map<std::string, Params> BRAMParamMapping;

extern std::unordered_map<Params, int> systemParamMap;
