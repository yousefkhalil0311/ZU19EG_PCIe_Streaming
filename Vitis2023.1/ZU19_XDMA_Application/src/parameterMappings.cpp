#include "parameterMappings.hpp"

using namespace std;

/*
 * Maps the strings stored in the BRAM to parameters that this application can use.
 * Companion application must have the same mapping.
*/
unordered_map<string, Params> BRAMParamMapping = {
		{"CH0_EN", CH0_EN},
		{"CH1_EN", CH1_EN},
		{"CH2_EN", CH2_EN},
		{"CH3_EN", CH3_EN},
		{"CH4_EN", CH4_EN},
		{"CH5_EN", CH5_EN},
		{"CH6_EN", CH6_EN},
		{"CH7_EN", CH7_EN},

		{"DDC0_EN", DDC0_EN},
		{"DDC1_EN", DDC1_EN},
		{"DDC2_EN", DDC2_EN},

		{"DDC0_FMIX", DDC0_FMIX},
		{"DDC0_SFOUT", DDC0_SFOUT},
		{"DDC1_FMIX", DDC1_FMIX},
		{"DDC1_SFOUT", DDC1_SFOUT},
		{"DDC2_FMIX", DDC2_FMIX},
		{"DDC2_SFOUT", DDC2_SFOUT},

		{"ATTENUATION_BVAL", ATTENUATION_BVAL},

		{"LP500MHZ_EN", LP500MHZ_EN},
		{"LP1GHZ_EN", LP1GHZ_EN},
		{"LP2GHZ_EN", LP2GHZ_EN},
		{"BYPASS_EN", BYPASS_EN},

		{"SYSTEM_EN", SYSTEM_EN},
		{"CAL_EN", CAL_EN},

		{"ACQUIREBYSAMPLES", ACQUIREBYSAMPLES},
		{"ACQUIREBYTIME_MS", ACQUIREBYTIME_MS},
		{"ACQUISITIONTIME_MS", ACQUISITIONTIME_MS},
		{"NUMSAMPLES_CAPTURE", NUMSAMPLES_CAPTURE}
};



unordered_map<Params, int> systemParamMap = {
		{CH0_EN, 0},
		{CH1_EN, 0},
		{CH2_EN, 0},
		{CH3_EN, 0},
		{CH4_EN, 0},
		{CH5_EN, 0},
		{CH6_EN, 0},
		{CH7_EN, 0},

		{DDC0_EN, 0},
		{DDC1_EN, 0},
		{DDC2_EN, 0},

		{DDC0_FMIX, 0},
		{DDC0_SFOUT, 0},
		{DDC1_FMIX, 0},
		{DDC1_SFOUT, 0},
		{DDC2_FMIX, 0},
		{DDC2_SFOUT, 0},

		{ATTENUATION_BVAL, 64},

		{LP500MHZ_EN, 1},
		{LP1GHZ_EN, 0},
		{LP2GHZ_EN, 0},
		{BYPASS_EN, 0},

		{SYSTEM_EN, 0},
		{CAL_EN, 0},

		{ACQUIREBYSAMPLES, 0},
		{ACQUIREBYTIME_MS, 1},
		{ACQUISITIONTIME_MS, 0},
		{NUMSAMPLES_CAPTURE, 0}
};
