#include "StdAfx.h"
#include "GobiError.h"
#include "GobiConnectionMgmtAPI.h"
#include "common.h"
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <string>
#include <iostream>
#include <getopt.h>

char *version = "1.0.0";

void usage(FILE *fp, int rc)
{
	fprintf(fp, "Usage: tip [-?hvcd]"
		"\t-h?\tthis help\n"
        "\t-s\tCheck connection status.\n"
		"\t-c\tConnect to carrier.\n"
		"\t-d\tDisconnect from carrier.\n");
	exit(rc);
}

int main(int argc, char ** argv)
{
  int c, connect;
  ULONG ret;

  while ((c = getopt(argc, argv, "?hcdsv")) > 0) {
    switch (c) {
    case 'v':
      printf("%s: version %s\n", argv[0], version);
      exit(0);
    case 'c':
      connect = 1;
      break;
    case 'd':
      connect = 0;
      break;
    case 's':
      connect = 2;
      break;
    case 'h':
    case '?':
      usage(stdout, 0);
      break;
    default:
      fprintf(stderr, "ERROR: unkown option '%c'\n", c);
      usage(stderr, 1);
      break;
    }
  }

  if (connect == 1)
  {
    connect_unlock();
    ret = SetDefaultProfile(0, NULL, NULL, NULL, NULL, NULL, NULL,
                            (char *)APN, (char *)"", (char *)"");
    if (ret)
    {
      fail(ret, "SetDefaultProfile");
    }   
    ULONG state;
    ret = GetSessionState(&state);
    if (ret)
    {
      fail(ret, "GetSessionState");
    }
    printf("State: %s\n",
           state == 1 ? "Disconnected" :
           state == 2 ? "Connected" :
           state == 3 ? "Suspended (not supported)" :
           state == 4 ? "Authenticating" :
           "Unknown");
  
    if (state != 2)
    {
      printf("Enabling enhanced autoconnect\n");
      ULONG foo = 1;
      ret = SetEnhancedAutoconnect(1, &foo);
      if (ret)
      {
        fail(ret, "SetEnhancedAutoconnect");
      }
    }

  } else if(connect == 0)
  {

    connect_unlock();
    ret = SetDefaultProfile(0, NULL, NULL, NULL, NULL, NULL, NULL,
                            (char *)APN, (char *)"", (char *)"");
    if (ret)
    {
      fail(ret, "SetDefaultProfile");
    }   
    ULONG state;
    ret = GetSessionState(&state);
    if (ret)
    {
      fail(ret, "GetSessionState");
    }
    printf("State: %s\n",
           state == 1 ? "Disconnected" :
           state == 2 ? "Connected" :
           state == 3 ? "Suspended (not supported)" :
           state == 4 ? "Authenticating" :
           "Unknown");

    if (state != 1)
    {
      printf("Disabling enhanced autoconnect\n");
      ULONG foo = 1;
      ret = SetEnhancedAutoconnect(0, &foo);
      if (ret)
      {
        fail(ret, "SetEnhancedAutoconnect");
      }
    }
  } else {
      connect_unlock();
      ret = SetDefaultProfile(0, NULL, NULL, NULL, NULL, NULL, NULL,
                              (char *)APN, (char *)"", (char *)"");
      if (ret)
      {
        fail(ret, "SetDefaultProfile");
      }   
      ULONG state;
      ret = GetSessionState(&state);
      if (ret)
      {
        fail(ret, "GetSessionState");
      }
      printf("State: %s\n",
             state == 1 ? "Disconnected" :
             state == 2 ? "Connected" :
             state == 3 ? "Suspended (not supported)" :
             state == 4 ? "Authenticating" :
             "Unknown");
  }

  ret = QCWWANDisconnect();
  if (ret)
  {
    fail(ret, "QCWWANDisconnect");
  }

  return 0;
}

