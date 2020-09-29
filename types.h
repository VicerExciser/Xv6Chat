#ifndef TYPES_XV6_H
#define TYPES_XV6_H

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef uint pde_t;
typedef uint pte_t;
// typedef uint fd_set;

#ifndef NULL
#define NULL ((void*)0)
#endif

// Represents true-or-false values
typedef _Bool bool;
enum { false, true };

// Explicitly-sized versions of integer types
typedef __signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;

// Page numbers are 32 bits long.
typedef uint32_t ppn_t;

// size_t is used for memory object sizes.
typedef uint32_t size_t;
// ssize_t is a signed version of ssize_t, used in case there might be an
// error return.
typedef int32_t ssize_t;

// off_t is used for file offsets and lengths.
//typedef int32_t off_t;

// Efficient min and max operations
#define MIN(_a, _b)                                             \
  ({                                                              \
    typeof(_a)__a = (_a);                                  \
    typeof(_b)__b = (_b);                                  \
    __a <= __b ? __a : __b;                                 \
  })

#define MAX(_a, _b)                                             \
  ({                                                              \
    typeof(_a)__a = (_a);                                  \
    typeof(_b)__b = (_b);                                  \
    __a >= __b ? __a : __b;                                 \
  })

// Rounding operations (efficient when n is a power of 2)
// Round down to the nearest multiple of n
#define ROUNDDOWN(a, n)                                         \
  ({                                                              \
    uint32_t __a = (uint32_t)(a);                          \
    (typeof(a))(__a - __a % (n));                          \
  })
// Round up to the nearest multiple of n
#define ROUNDUP(a, n)                                           \
  ({                                                              \
    uint32_t __n = (uint32_t)(n);                          \
    (typeof(a))(ROUNDDOWN((uint32_t)(a) + __n - 1, __n)); \
  })

// Return the offset of 'member' relative to the beginning of a struct type
#define offsetof(type, member)  ((size_t)(&((type*)0)->member))

// Units
#define KB (1024)
#define MB (1024 * KB)
#define GB (1024 * MB)

#endif

#ifndef FD_SET
  #undef  FD_SETSIZE
  #define FD_SETSIZE    16
  #define FD_SET(n, p)  ((p)->fd_bits[(n)/8] |=  (1 << ((n) & 7)))
  #define FD_CLR(n, p)  ((p)->fd_bits[(n)/8] &= ~(1 << ((n) & 7)))
  #define FD_ISSET(n,p) ((p)->fd_bits[(n)/8] &   (1 << ((n) & 7)))
  #define FD_ZERO(p)    memset((void*)(p),0,sizeof(*(p)))
  typedef struct fd_set {
    unsigned char fd_bits [(FD_SETSIZE+7)/8];
  } fd_set;
  struct timeval {
    long    tv_sec;         /* seconds */
    long    tv_usec;        /* and microseconds */
  };
#endif
  