//Xilinx specific includes
#include "platform.h"
#include "QC_Application.hpp"


int main()
{
	init_platform();

	QC_app();

	cleanup_platform();
    return 0;
}
