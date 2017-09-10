/**
*
* Copyright 2008 Novatel Wireless Inc.
*
*/

#ifndef _PLATFORM_INC_H
#define _PLATFORM_INC_H

//Detect linux
#ifdef __linux__
  #ifndef _LINUX_
    #define _LINUX_
  #endif
  #ifndef _LINUX_
  	#define _LINUX_
  #endif
#endif

//Detect android 
#ifdef __android__
  #ifndef _ANDROID_
    #define _ANDROID_
  #endif
#endif

//Detect windows
#if defined(_WIN32)
	#ifndef _NVTL_WINDOWS_
		#define _NVTL_WINDOWS_
	#endif
#endif
#if defined(_WIN64)
	#ifndef _NVTL_WINDOWS_
		#define _NVTL_WINDOWS_
	#endif
	#ifndef NVTL_64_BIT
		#define NVTL_64_BIT
	#endif
#endif

//must be Mac then
#if !defined(_LINUX_) && !defined(_NVTL_WINDOWS_)
#define _MAC_OS_X_
#endif

#define DEFINED_LINUX defined(_LINUX_)
#define DEFINED_MAC defined(_MAC_OS_X_)
#define DEFINED_UNIX_BASED DEFINED_LINUX || DEFINED_MAC
#define DEFINED_ANDROID   defined(_ANDROID_)
//Windows defines and includes
#ifdef _NVTL_WINDOWS_

    #define DEFINED_NVTL_WINDOWS _NVTL_WINDOWS_

    //control symbol exports
    #define NOVATEL_CALL __cdecl
	
    #ifdef NOVATEL_EXPORTS
		#define NOVATEL_API __declspec(dllexport)
	#else
		#define NOVATEL_API __declspec(dllimport)
	#endif

    #ifndef WINVER				// Allow use of features specific to Windows XP or later.
		#define WINVER 0x0500		// Change this to the appropriate value to target other versions of Windows.
	#endif

	#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
		#define _WIN32_WINNT 0x0500	// Change this to the appropriate value to target other versions of Windows.
	#endif	

	#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
		#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
	#endif						

	#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
	#define _CRT_SECURE_NO_WARNINGS		// disable deprecated security warnings

	#include <windows.h>
	#include <tchar.h>
	#include <process.h>
    #include <time.h>

	#ifndef PACKED
		#define PACKED 
	#endif

  #if defined(_DEBUG)
	#define Dbg printf
  #else 
	#define Dbg
  #endif
#endif


//common defines and includes
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//linux defines and includes
#ifdef _LINUX_
  #define NOVATEL_API  __attribute__ ((visibility("default")))
  #define NOVATEL_CALL
  #define PACKED
  #define Dbg printf
#endif

//MAC defines and includes
#ifdef _MAC_OS_X_
  #define NOVATEL_API
  #define NOVATEL_CALL
  #define PACKED
  #define Dbg printf
#include <stdint.h>
#endif





#endif
