#include "types.h"
#include "x86.h"

void*
memset(void *dst, int c, uint n)
{
  if ((int)dst%4 == 0 && n%4 == 0){
    c &= 0xFF;
    stosl(dst, (c<<24)|(c<<16)|(c<<8)|c, n/4);
  } else
    stosb(dst, c, n);
  return dst;
}

int
memcmp(const void *v1, const void *v2, uint n)
{
  const uchar *s1, *s2;

  s1 = v1;
  s2 = v2;
  while(n-- > 0){
    if(*s1 != *s2)
      return *s1 - *s2;
    s1++, s2++;
  }

  return 0;
}

void*
memmove(void *dst, const void *src, uint n)
{
  const char *s;
  char *d;

  s = src;
  d = dst;
  if(s < d && s + n > d){
    s += n;
    d += n;
    while(n-- > 0)
      *--d = *--s;
  } else
    while(n-- > 0)
      *d++ = *s++;

  return dst;
}

// memcpy exists to placate GCC.  Use memmove.
void*
memcpy(void *dst, const void *src, uint n)
{
  return memmove(dst, src, n);
}

int
strncmp(const char *p, const char *q, uint n)
{
  while(n > 0 && *p && *p == *q)
    n--, p++, q++;
  if(n == 0)
    return 0;
  return (uchar)*p - (uchar)*q;
}

char*
strncpy(char *s, const char *t, int n)
{
  char *os;

  os = s;
  while(n-- > 0 && (*s++ = *t++) != 0)
    ;
  while(n-- > 0)
    *s++ = 0;
  return os;
}

 int 
 strnlen(const char *s, uint size) 
 { 
   int n; 
  
   for (n = 0; size > 0 && *s != '\0'; s++, size--) 
     n++; 
   return n; 
 } 
 


// Like strncpy but guaranteed to NUL-terminate.
char*
safestrcpy(char *s, const char *t, int n)
{
  char *os;

  os = s;
  if(n <= 0)
    return os;
  while(--n > 0 && (*s++ = *t++) != 0)
    ;
  *s = 0;
  return os;
}

int
strlen(const char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}


int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

// Return a pointer to the first occurrence of 'c' in 's',
 // or a pointer to the string-ending null character if the string has no 'c'.
 char *
 strfind(const char *s, char c)
 {
   for (; *s; s++)
     if (*s == c)
       break;
   return (char*)s;
 }

char*
strcat(char *dst, const char *src)
{
  char *ptr = dst + strlen(dst);
  while (*src)
    *ptr++ = *src++;
  *ptr = '\0';
  return dst;
}


// inline function to swap two numbers
static inline void 
swap(char *x, char *y) {
  char t = *x; *x = *y; *y = t;
}

// function to reverse buffer[i..j]
static char* 
reverse(char *buffer, int i, int j)
{
  while (i < j)
    swap(&buffer[i++], &buffer[j--]);

  return buffer;
}

// Iterative function to implement itoa() function in C
char* 
itoa(int value, char* buffer, int base)
{
  // invalid input
  if (base < 2 || base > 32)
    return buffer;

  // consider absolute value of number
  int n = value;
  if (n < 0)
    n *= (-1);

  int i = 0;
  while (n)
  {
    int r = n % base;

    if (r >= 10) 
      buffer[i++] = 65 + (r - 10);
    else
      buffer[i++] = 48 + r;

    n = n / base;
  }

  // if number is 0
  if (i == 0)
    buffer[i++] = '0';

  // If base is 10 and value is negative, the resulting string 
  // is preceded with a minus sign (-)
  // With any other base, value is always considered unsigned
  if (value < 0 && base == 10)
    buffer[i++] = '-';

  buffer[i] = '\0'; // null terminate string

  // reverse the string and return it
  return reverse(buffer, 0, i - 1);
}

