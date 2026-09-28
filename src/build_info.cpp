#include "config.h"

// BUILD_TIMESTAMP подставляет scripts/build_time.py при каждой сборке.
// Без скрипта — время последней компиляции именно этого файла.
#ifndef BUILD_TIMESTAMP
#define BUILD_TIMESTAMP __DATE__ " " __TIME__
#endif

const char FIRMWARE_BUILD_TIME[] = BUILD_TIMESTAMP;
