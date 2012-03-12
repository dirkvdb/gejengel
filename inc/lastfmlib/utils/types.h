#ifndef GEJENGEL_INT_TYPES_H
#define GEJENGEL_INT_TYPES_H

#ifndef WIN32
#include <inttypes.h>
#else
typedef long long int int64_t;
typedef unsigned long long int uint64_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef char int8_t;
typedef unsigned char uint8_t;
#endif

#endif
