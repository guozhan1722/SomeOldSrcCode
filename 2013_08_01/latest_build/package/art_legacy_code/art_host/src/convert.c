#include "project.h"

#include <string.h>

int strnicmp(const char *s1, const char *s2, size_t len)
{
  /* Yes, i am keeping 'em unsigned too */
  unsigned char c1=0, c2=0;
  unsigned count = 0;
  int flag = -1;

//  printf("s1=%s, s2=%s, len=%d\n", s1, s2, len);
  if(len > 0)
  {
    for(count=0; count<len; count++)
    {
      c1 = tolower(s1[count]); 
      c2 = tolower(s2[count]);    

      if(c1 != c2)
      {
	flag = 1;
	return flag;	  
      }
     }     
  }
  if(count == len) flag = 0; 
  return flag; // return 0 for matching and
} 

//void  *lfind(const void *key, const void *base,       size_t *nmemb,       size_t size,  int(*compar)(const void *, const void *));
//void *_lfind(const void *key, const void *base, unsigned int *num,   unsigned int width, int (__cdecl *compare)(const void *, const void *));

#include <search.h>

void *_lfind(const void *key, const void *base, unsigned int *num, unsigned int width, int(*compare)(const void *, const void *))
{
  return lfind(key, base, num, width, compare);
}

/*   string   to   low*/
char*   strlwr(   char*   str   )
{
      char*   orig   =   str;
      //   process   the   string
      for   (   ;   *str   !=   '\0 ';   str++   )
              *str   =   tolower(*str);
      return   orig;
} 

//need fix
unsigned long int milliTime (void)
{
//	A_UINT32 timeMilliSeconds;
//	A_UINT64 tempTime = 0;

//	tempTime = uclock() * 1000;
//	timeMilliSeconds = (tempTime)/UCLOCKS_PER_SEC;
//	return timeMilliSeconds;
	return 0;
}

