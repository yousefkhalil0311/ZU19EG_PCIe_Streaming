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

unordered_map<string, int> paramMap;

/*
 * entry point to application.
 * Separate from main because this project will
 * be a submodule to a larger system once validated
 *
 */

void QC_app(){

	paramMap["param1"] = 1;

	xil_printf("Value written to paramMap['param1']: %d\n", paramMap["param1"]);


	xil_printf("Writing values to Ram\n");
	int index = 0x100>>2;
	DDC[index] = 12;
	DDC[index + 1] = 11;

	while(1){
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

void sysInit(){

	cout << "Initializing" << endl;

	GPIO_Init(&FE_GPIO, FE_GPIO_ID, 1, 0x0, 0x0);
	GPIO_Init(&Sys_GPIO, Sys_GPIO_ID, 1, 0x0, 0x0);

	SPI_Init(&FE_SPI, FE_SPI_ID);

}
