/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <linux/ioctl.h>

#define BINBFLEN 1024
#define TXTBFLEN 2*BINBFLEN
#define FLNAMELEN 1024

// len is for Binary
void Two2One(unsigned char *tbuffer, unsigned char *bbuffer, int len)
{
  int i;
  for(i=0; i<len; i++) {
    *(bbuffer + i) = (unsigned char)(*(tbuffer + 2*i)) - 0x41 + (unsigned char)((*(tbuffer + 2*i + 1) - 0x41) << 4);  
  }
}

int txt2bin(char **argv)
{
  char fname[FLNAMELEN] = "./";
  char ofname[FLNAMELEN];
  FILE *bFile = NULL;
  FILE *tFile = NULL;
  unsigned char bbuffer[BINBFLEN];
  unsigned char tbuffer[TXTBFLEN];
  int nSize, nCount, idx;
  int rtnlen;
  
  if(strlen(*argv)>FLNAMELEN/2) return -1;
  strcat(fname, *argv);
  strcpy(ofname, fname);
  strcat(ofname, ".bin");    

  tFile = fopen(fname, "rb");
  if (tFile == NULL) {
    printf("Couldn't open target file: %s\n", fname);
    goto txt2binexit;
  }  

  bFile = fopen(ofname, "w");
  if (bFile == NULL) {
    printf("Couldn't open target file: %s\n", ofname);
    goto txt2binexit;
  }

  fseek (tFile , 0 , SEEK_END);
  nSize = ftell (tFile);
  nCount = nSize / sizeof(tbuffer);
  fseek(tFile, 0, SEEK_SET);  

  rtnlen = fread(tbuffer, 1, 1,tFile);
  if(rtnlen != 1) goto txt2binexit;

  if(tbuffer[0] != '\n') {
      fseek(tFile, 0, SEEK_SET);
  } else {
      nSize--;
      nCount = nSize / sizeof(tbuffer);
  }

  for (idx = 0; idx < nCount; idx ++) {
    rtnlen = fread(tbuffer, sizeof(tbuffer),1,tFile);
    if(rtnlen != 1) goto txt2binexit;
    Two2One(tbuffer, bbuffer, sizeof(bbuffer));
    fwrite(bbuffer, sizeof(tbuffer)/2, 1, bFile);
  }

  nCount = nSize % sizeof(tbuffer);
  if (nCount) {
    rtnlen = fread(tbuffer, nCount,1, tFile);
    if(rtnlen != 1) goto txt2binexit;
    Two2One(tbuffer, bbuffer, nCount/2);
    fwrite(bbuffer, nCount/2, 1, bFile);
  }

txt2binexit:       
  if(bFile) fclose(bFile);
  if(tFile) fclose(tFile);
}

// len is for Binary
void One2Two(unsigned char *bbuffer, unsigned char *tbuffer, int len)
{
  int i;
  for(i=0; i<len; i++) {
    *(tbuffer + 2*i) = 0x41 + (unsigned char)((*(bbuffer + i))&0xF);
    *(tbuffer + 2*i + 1) = 0x41 + (unsigned char)(((*(bbuffer + i))&0xF0)>>4);
  }
}

int bin2txt(char **argv)
{
  char fname[FLNAMELEN] = "./";
  char ofname[FLNAMELEN];
  FILE *bFile = NULL;
  FILE *tFile = NULL;
  unsigned char bbuffer[BINBFLEN];
  unsigned char tbuffer[TXTBFLEN];
  int nSize, nCount, idx;
  int rtnlen;
  
  if(strlen(*argv)>FLNAMELEN/2) return -1;
  strcat(fname, *argv);
  strcpy(ofname, fname);
  strcat(ofname, ".txt");    

  bFile = fopen(fname, "rb");
  if (bFile == NULL) {
    printf("Couldn't open target file: %s\n", fname);
    goto bin2txtexit;
  }  
  
  tFile = fopen(ofname, "wb");
  if (tFile == NULL) {
    printf("Couldn't open target file: %s\n", ofname);
    goto bin2txtexit;
  }
  
  fseek (bFile , 0 , SEEK_END);
  nSize = ftell (bFile);
  nCount = nSize / sizeof(bbuffer);
  rewind (bFile);
  for (idx = 0; idx < nCount; idx ++) {
    rtnlen = fread(bbuffer, sizeof(bbuffer),1,bFile);
    if(rtnlen != 1) goto bin2txtexit;
    One2Two(bbuffer, tbuffer, sizeof(bbuffer));
    fwrite(tbuffer, 2*sizeof(bbuffer), 1, tFile);
  }
  nCount = nSize % sizeof(bbuffer);
  if (nCount) {
    rtnlen = fread(bbuffer, nCount,1, bFile);
    if(rtnlen != 1) goto bin2txtexit;
    One2Two(bbuffer, tbuffer, nCount);
    fwrite(tbuffer, 2*nCount, 1, tFile);
  }  

bin2txtexit:  
  if(bFile) fclose(bFile);
  if(tFile) fclose(tFile);
}

void ShowHelp (void)
{
  printf("bin_txt -t TextFileName\n");  
  printf("bin_txt -b BinFileName\n");      
}

int main (int argc, char **argv)
{
  if(argc == 3) {
    if(!strcmp(argv[1], "-t")) {
      if(txt2bin(argv + 2)) return -1;
    } else if(!strcmp(argv[1], "-b")) {
      if(bin2txt(argv + 2)) return -1;
    } else ShowHelp();
  } else ShowHelp();
  return 0;
}
    

