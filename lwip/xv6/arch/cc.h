#ifndef XV6_LWIP_CC_H_
#define XV6_LWIP_CC_H_
 
#include "../../../assert.h"
// #include "../../../stdio.h"
// extern int cprintf(const char *fmt, ...);
 
typedef unsigned char u8_t;
typedef signed char s8_t;
typedef unsigned short u16_t;
typedef signed short s16_t;
typedef unsigned int u32_t;
typedef signed short s32_t;
typedef unsigned int mem_ptr_t;

#define LWIP_PROVIDE_ERRNO

#define PACK_STRUCT_FIELD(x)    x
#define PACK_STRUCT_STRUCT      __attribute__((__packed__))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

#define LWIP_PLATFORM_DEBUG(x)  x  //cprintf x
#define LWIP_PLATFORM_DIAG(x)  x //cprintf x
// #define LWIP_PLATFORM_DIAG2(x,y)  cprintf((x),(y))
#define LWIP_PLATFORM_ASSERT(x) assert(x)

#endif // XV6_LWIP_CC_H_
