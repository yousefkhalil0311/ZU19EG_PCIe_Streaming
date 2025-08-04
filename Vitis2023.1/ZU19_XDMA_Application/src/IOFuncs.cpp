#include "IOFuncs.hpp"

//isolates program from depending on specific print style statements
void QC_print(const char* format, ...){
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}
