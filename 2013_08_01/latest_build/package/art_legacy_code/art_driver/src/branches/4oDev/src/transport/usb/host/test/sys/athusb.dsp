# Microsoft Developer Studio Project File - Name="athusb" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=athusb - Win32 WDM Checked Trace
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "athusb.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "athusb.mak" CFG="athusb - Win32 WDM Checked Trace"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "athusb - Win32 WDM Checked Trace" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "athusb - Win32 WDM Free" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName "deepak_dev"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "athusb - Win32 WDM Checked Trace"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "athusb___Win32_WDM_Checked_Trace"
# PROP BASE Intermediate_Dir "athusb___Win32_WDM_Checked_Trace"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WDM_Checked_Trace\output"
# PROP Intermediate_Dir "WDM_Checked_Trace\output"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gz /W3 /Z7 /Oi /Ob2 /Gy /I "$(BASEDIR)\inc" /I "$(BASEDIR)\inc\ddk" /FI"$(BASEDIR)\inc\warning.h" /D "DEBUG" /D "_DEBUG" /D DBG=1 /D FPO=0 /D "_IDWBUILD" /D WIN32=100 /D "_WINDOWS" /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0500 /D WIN32_LEAN_AND_MEAN=1 /D DEVL=1 /D _DLL=1 /D _X86_=1 /D $(CPU)=1 /D BINARY_COMPATIBLE=1 /FR /FD /Zel -cbstring /QIfdiv- /QIf /c
# ADD CPP /nologo /Gz /W3 /WX /Z7 /Oi /Ob1 /Gy /I "." /I "include" /I "..\..\include" /I "..\..\..\..\..\include" /I "$(DDK_INC_PATH)" /I "$(CRT_INC_PATH)" /I "$(WDM_INC_PATH)" /FI"$(DDK_INC_PATH)\warning.h" /D "DEBUG" /D "CHIP_TEST" /D "DEPRECATE_DDK_FUNCTIONS" /D "_DEBUG" /D DBG=1 /D FPO=0 /D "_IDWBUILD" /D WIN32=100 /D "_WINDOWS" /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0500 /D WINVER=0x0500 /D WIN32_LEAN_AND_MEAN=1 /D DEVL=1 /D _DLL=1 /D _X86_=1 /D $(CPU)=1 /D BINARY_COMPATIBLE=1 /D "USB2" /D "WMI_SUPPORT" /D "BOOTLOADER" /FR /FD /I /I /Zel -cbstring /QIfdiv- /QIf /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /i "$(BASEDIR)\inc" /i "..\include" /d "_DEBUG" /d "NDIS_MINIPORT_DRIVER"
# ADD RSC /l 0x409 /i "$(BASEDIR)\inc" /i "$(CRT_INC_PATH)" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 int64.lib ntoskrnl.lib usbd.lib hidclass.lib wdm.lib wmilib.lib/nologo /base:"0x10000" /version:4.0 /entry:"DriverEntry" /incremental:no /pdb:"Checked_Trace\output/../athusb.pdb" /map /debug /debugtype:both /machine:IX86 /nodefaultlib /out:"Checked_Trace/output/../athusb.sys" /libpath:"$(BASEDIR)\libchk\i386" /libpath:"$(BASEDIR)\lib\i386\checked" /driver /debug:notmapped,FULL /IGNORE:4001,4037,4039,4065,4078,4087,4089,4096 /MERGE:_PAGE=PAGE /MERGE:_TEXT=.text /SECTION:INIT,d /MERGE:.rdata=.text /FULLBUILD /RELEASE /FORCE:MULTIPLE /OPT:REF /OPTIDATA /align:0x20 /osversion:4.00 /subsystem:native /MAPINFO:EXPORTS /MAPINFO:FIXUPS /MAPINFO:LINES
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 ntoskrnl.lib usbd.lib hidclass.lib wdm.lib wmilib.lib /nologo /base:"0x10000" /entry:"DriverEntry" /incremental:no /pdb:"WDM_Checked_Trace\output/../athusb.pdb" /map /debug /machine:IX86 /nodefaultlib /out:"WDM_Checked_Trace/output/../athusb.sys" /libpath:"$(DDK_LIB_DEST)\$(Cpu)" /libpath:"WDM_Checked_Trace\lib" /debug:FULL /driver /IGNORE:4001,4037,4039,4065,4078,4087,4089,4096 /MERGE:_PAGE=PAGE /MERGE:_TEXT=.text /SECTION:INIT,d /MERGE:.rdata=.text /FULLBUILD /RELEASE /FORCE:MULTIPLE /OPT:REF /align:0x80 /osversion:5.1 /subsystem:native /MAPINFO:EXPORTS /MAPINFO:FIXUPS /MAPINFO:LINES
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "athusb - Win32 WDM Free"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "athusb___Win32_NDIS_51_Free"
# PROP BASE Intermediate_Dir "athusb___Win32_NDIS_51_Free"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WDM_Free\output"
# PROP Intermediate_Dir "WDM_Free\output"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gz /W3 /Z7 /O2 /Ob2 /I "." /I "$(BASEDIR)\inc" /I "$(BASEDIR)\inc\ddk" /FI"$(BASEDIR)\inc\warning.h" /D FPO=1 /D "PCI_INTERFACE" /D "NDEBUG" /D "_IDWBUILD" /D WIN32=100 /D "_WINDOWS" /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0500 /D WIN32_LEAN_AND_MEAN=1 /D DEVL=1 /D _DLL=1 /D _X86_=1 /D $(CPU)=1 /D BINARY_COMPATIBLE=1 /FR /FD /I /Oxs /Zel -cbstring /QIfdiv- /QIf /c
# ADD CPP /nologo /Gz /W3 /WX /Z7 /O2 /Ob2 /I "." /I "include" /I "..\..\include" /I "..\..\..\..\..\include" /I "$(DDK_INC_PATH)" /I "$(CRT_INC_PATH)" /I "$(WDM_INC_PATH)" /FI"$(DDK_INC_PATH)\warning.h" /D "CHIP_TEST" /D "DEPRECATE_DDK_FUNCTIONS" /D FPO=1 /D "NDEBUG" /D "_IDWBUILD" /D WIN32=100 /D "_WINDOWS" /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0500 /D WINVER=0x0500 /D WIN32_LEAN_AND_MEAN=1 /D DEVL=1 /D _DLL=1 /D _X86_=1 /D $(CPU)=1 /D BINARY_COMPATIBLE=1 /D "USB2" /D "WMI_SUPPORT" /D "BOOTLOADER" /FR /FD /I /Oxs /Zel -cbstring /QIfdiv- /QIf /c
# ADD BASE MTL /nologo /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /i "$(BASEDIR)\inc" /i "..\include" /d "NDEBUG" /d "NDIS_MINIPORT_DRIVER"
# SUBTRACT BASE RSC /x
# ADD RSC /l 0x409 /fo"WDM_Free\output/bulkusb.res" /i "$(BASEDIR)\inc" /i "$(CRT_INC_PATH)" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ndis.lib int64.lib ntoskrnl.lib hal.lib /nologo /base:"0x10000" /version:4.0 /entry:"DriverEntry" /pdb:"WDM_Free\output/../athusb.pdb" /map /machine:IX86 /nodefaultlib /out:"WDM_Free\output/../athusb.sys" /libpath:"$(BASEDIR)\libfre\i386" /libpath:"$(BASEDIR)\lib\i386\free" /driver /debug:notmapped,MINIMAL /IGNORE:4001,4037,4039,4065,4070,4078,4087,4089,4096 /MERGE:_PAGE=PAGE /MERGE:_TEXT=.text /SECTION:INIT,d /MERGE:.rdata=.text /FULLBUILD /RELEASE /FORCE:MULTIPLE /OPT:REF /OPTIDATA /align:0x20 /osversion:4.00 /subsystem:native /MAPINFO:EXPORTS /MAPINFO:FIXUPS /MAPINFO:LINES
# SUBTRACT BASE LINK32 /pdb:none /debug /force
# ADD LINK32 ntoskrnl.lib usbd.lib hidclass.lib wdm.lib wmilib.lib /nologo /base:"0x10000" /version:5.1 /entry:"DriverEntry" /pdb:"WDM_Free\output/../athusb.pdb" /map /machine:IX86 /nodefaultlib /out:"WDM_Free\output/../athusb.sys" /libpath:"$(DDK_LIB_DEST)\$(Cpu)" /libpath:"NDIS5_Free\lib" /driver /debug:MINIMAL /IGNORE:4001,4037,4039,4065,4070,4078,4087,4089,4096 /MERGE:_PAGE=PAGE /MERGE:_TEXT=.text /SECTION:INIT,d /MERGE:.rdata=.text /FULLBUILD /RELEASE /FORCE:MULTIPLE /OPT:REF /align:0x80 /osversion:5.1 /subsystem:native /MAPINFO:EXPORTS /MAPINFO:FIXUPS /MAPINFO:LINES
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
TargetDir=.\WDM_Free
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(TargetDir)\athusb.sys $(TargetDir)\athfmwdl.sys
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "athusb - Win32 WDM Checked Trace"
# Name "athusb - Win32 WDM Free"
# Begin Group "Source Files"

# PROP Default_Filter ".c;.cpp"
# Begin Group "drv"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\drv\athusbconfig.c
# End Source File
# Begin Source File

SOURCE=..\..\drv\athusbdrv.c
# End Source File
# Begin Source File

SOURCE=..\..\drv\athusbRxTx.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\athdev.c
# End Source File
# Begin Source File

SOURCE=.\athfmw.c
# End Source File
# Begin Source File

SOURCE=.\athpnp.c
# End Source File
# Begin Source File

SOURCE=.\athpwr.c
# End Source File
# Begin Source File

SOURCE=.\athrwr.c
# End Source File
# Begin Source File

SOURCE=.\athssm.c
# End Source File
# Begin Source File

SOURCE=.\athusb.c
# End Source File
# Begin Source File

SOURCE=.\athusbtst.c
# End Source File
# Begin Source File

SOURCE=.\athwmi.c
# End Source File
# Begin Source File

SOURCE=.\util.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Group "include"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\include\athusbapi.h
# End Source File
# Begin Source File

SOURCE=..\..\include\athusbconfig.h
# End Source File
# Begin Source File

SOURCE=..\..\include\athusbdrv.h
# End Source File
# Begin Source File

SOURCE=..\..\include\athusbRxTx.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\athdev.h
# End Source File
# Begin Source File

SOURCE=.\athfmw.h
# End Source File
# Begin Source File

SOURCE=.\athpnp.h
# End Source File
# Begin Source File

SOURCE=.\athpwr.h
# End Source File
# Begin Source File

SOURCE=.\athrwr.h
# End Source File
# Begin Source File

SOURCE=.\athssm.h
# End Source File
# Begin Source File

SOURCE=.\athusb.h
# End Source File
# Begin Source File

SOURCE=.\athusbtst.h
# End Source File
# Begin Source File

SOURCE=.\athusr.h
# End Source File
# Begin Source File

SOURCE=.\athwmi.h
# End Source File
# Begin Source File

SOURCE=.\precomp.h
# End Source File
# Begin Source File

SOURCE=.\util.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\athusb.bmf
# End Source File
# Begin Source File

SOURCE=.\athusb.rc
# End Source File
# End Group
# End Target
# End Project
