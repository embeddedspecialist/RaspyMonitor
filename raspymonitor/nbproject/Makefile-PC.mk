#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_CONF=PC
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/1952676330/vcoordinator.o \
	${OBJECTDIR}/_ext/1952676330/upid2.o \
	${OBJECTDIR}/_ext/1450946203/ioutil.o \
	${OBJECTDIR}/_ext/1952676330/tempctrlhyst.o \
	${OBJECTDIR}/_ext/1952676330/temperaturecontroller.o \
	${OBJECTDIR}/_ext/1952676330/nthacc.o \
	${OBJECTDIR}/_ext/2024910124/binaryEncoder.o \
	${OBJECTDIR}/_ext/1952676330/ds18X20.o \
	${OBJECTDIR}/_ext/1462128332/ownetu.o \
	${OBJECTDIR}/_ext/1952676330/c3pointctrl.o \
	${OBJECTDIR}/_ext/2024910124/contblock.o \
	${OBJECTDIR}/_ext/1462128332/owtrnu.o \
	${OBJECTDIR}/_ext/1462128332/cownet.o \
	${OBJECTDIR}/_ext/2024910124/delayBlock.o \
	${OBJECTDIR}/_ext/1952676330/nthCnt.o \
	${OBJECTDIR}/_ext/526401590/ClockManager.o \
	${OBJECTDIR}/_ext/2024910124/satblock.o \
	${OBJECTDIR}/_ext/1952676330/ds2438.o \
	${OBJECTDIR}/_ext/1952676330/di2ao.o \
	${OBJECTDIR}/_ext/1952676330/alarmcoordinator.o \
	${OBJECTDIR}/_ext/1952676330/climaticcurve.o \
	${OBJECTDIR}/_ext/1952676330/conewirenet.o \
	${OBJECTDIR}/_ext/2024910124/BlocksCommonData.o \
	${OBJECTDIR}/_ext/2024910124/C3PointCtrlBlock.o \
	${OBJECTDIR}/_ext/1952676330/ic.o \
	${OBJECTDIR}/_ext/1450946203/timeUtil.o \
	${OBJECTDIR}/_ext/1462128332/owllu.o \
	${OBJECTDIR}/_ext/2024910124/ClockBlock.o \
	${OBJECTDIR}/_ext/1952676330/fullutactrl.o \
	${OBJECTDIR}/_ext/1952676330/floorcoord2.o \
	${OBJECTDIR}/_ext/1991509643/gio.o \
	${OBJECTDIR}/_ext/287082435/CMsgRecv.o \
	${OBJECTDIR}/_ext/1952676330/stepdigitalout.o \
	${OBJECTDIR}/_ext/1459688078/CLabel.o \
	${OBJECTDIR}/_ext/1952676330/pumpcontroller.o \
	${OBJECTDIR}/_ext/1952676330/changeovercoord.o \
	${OBJECTDIR}/_ext/1952676330/tempCondensa.o \
	${OBJECTDIR}/_ext/1952676330/humcontroller.o \
	${OBJECTDIR}/_ext/2024910124/block.o \
	${OBJECTDIR}/_ext/1952676330/ds2751.o \
	${OBJECTDIR}/_ext/1952676330/commonDefinitions.o \
	${OBJECTDIR}/_ext/1952676330/ds2408.o \
	${OBJECTDIR}/_ext/1952676330/newEngine.o \
	${OBJECTDIR}/raspymonitor.o \
	${OBJECTDIR}/_ext/2024910124/pidBlock.o \
	${OBJECTDIR}/_ext/1952676330/ibuttonreader.o \
	${OBJECTDIR}/_ext/1952676330/conewireengine.o \
	${OBJECTDIR}/_ext/1952676330/remotedido.o \
	${OBJECTDIR}/_ext/1952676330/buttoncontroller.o \
	${OBJECTDIR}/_ext/1952676330/nthvlv-adv.o \
	${OBJECTDIR}/_ext/1952676330/ds2405.o \
	${OBJECTDIR}/_ext/2024910124/costantblock.o \
	${OBJECTDIR}/_ext/1952676330/execcommands.o \
	${OBJECTDIR}/_ext/1952676330/timeddido.o \
	${OBJECTDIR}/_ext/1952676330/nthvlv.o \
	${OBJECTDIR}/_ext/2024910124/ClimaticCurveBlock.o \
	${OBJECTDIR}/_ext/2024910124/logblock.o \
	${OBJECTDIR}/_ext/461864497/xmlutil.o \
	${OBJECTDIR}/_ext/1952676330/digitalctrl.o \
	${OBJECTDIR}/_ext/782350236/cstring.o \
	${OBJECTDIR}/_ext/316114074/LibIniFile.o \
	${OBJECTDIR}/_ext/247515588/fileUtil.o \
	${OBJECTDIR}/_ext/1952676330/scheduler.o \
	${OBJECTDIR}/_ext/1952676330/vmultidido.o \
	${OBJECTDIR}/_ext/619507098/linuxlnk.o \
	${OBJECTDIR}/_ext/1952676330/digitalio.o \
	${OBJECTDIR}/_ext/2024910124/opblock.o \
	${OBJECTDIR}/_ext/1952676330/vpid.o \
	${OBJECTDIR}/_ext/1952676330/accessLog.o \
	${OBJECTDIR}/_ext/1952676330/Thu.o \
	${OBJECTDIR}/_ext/1952676330/pidsimple.o \
	${OBJECTDIR}/_ext/2024910124/triggerBlock.o \
	${OBJECTDIR}/_ext/1450946203/crcutil.o \
	${OBJECTDIR}/_ext/1462128332/owsesu.o \
	${OBJECTDIR}/_ext/1952676330/nthmgc.o \
	${OBJECTDIR}/_ext/1952676330/timemarkerctrl.o \
	${OBJECTDIR}/_ext/1952676330/pidlmd.o \
	${OBJECTDIR}/_ext/2024910124/gateblock.o \
	${OBJECTDIR}/_ext/2024910124/ifblock.o \
	${OBJECTDIR}/_ext/1952676330/nthvlv2-vav.o \
	${OBJECTDIR}/_ext/461864497/Cmd.o \
	${OBJECTDIR}/_ext/1952676330/vhystcontroller.o \
	${OBJECTDIR}/_ext/461856677/pid.o \
	${OBJECTDIR}/_ext/1952676330/tagcontrol.o \
	${OBJECTDIR}/_ext/2024910124/muxBlock.o \
	${OBJECTDIR}/_ext/1463815084/SocketPort.o \
	${OBJECTDIR}/_ext/1952676330/analogIO.o \
	${OBJECTDIR}/_ext/316114074/inifilemanager.o \
	${OBJECTDIR}/_ext/1952676330/timer.o \
	${OBJECTDIR}/_ext/1952676330/utactrl.o \
	${OBJECTDIR}/_ext/1952676330/fullutactrl2.o \
	${OBJECTDIR}/_ext/1952676330/ds2890.o \
	${OBJECTDIR}/_ext/1952676330/vdido.o \
	${OBJECTDIR}/_ext/2024910124/hystblock.o \
	${OBJECTDIR}/_ext/1952676330/digitalin2out.o \
	${OBJECTDIR}/_ext/2024910124/timerblock.o \
	${OBJECTDIR}/_ext/1952676330/nthvlv2.o \
	${OBJECTDIR}/_ext/1952676330/afoerror.o \
	${OBJECTDIR}/_ext/2024910124/BlockCoordinator.o \
	${OBJECTDIR}/_ext/316114074/IniFileHandler.o


# C Compiler Flags
CFLAGS=-O0 -g3 -Wall -gdwarf-2

# CC Compiler Flags
CCFLAGS=-O0 -g3 -Wall -gdwarf-2
CXXFLAGS=-O0 -g3 -Wall -gdwarf-2

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lpthread

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/raspymonitor

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/raspymonitor: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/raspymonitor ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/1952676330/vcoordinator.o: ../afoLibs/Moduli/vcoordinator.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/vcoordinator.o ../afoLibs/Moduli/vcoordinator.cpp

${OBJECTDIR}/_ext/1952676330/upid2.o: ../afoLibs/Moduli/upid2.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/upid2.o ../afoLibs/Moduli/upid2.cpp

${OBJECTDIR}/_ext/1450946203/ioutil.o: ../afoLibs/OWAPI/util/ioutil.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1450946203
	${RM} $@.d
	$(COMPILE.c) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I. -I../afoLibs/Blocks -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1450946203/ioutil.o ../afoLibs/OWAPI/util/ioutil.c

${OBJECTDIR}/_ext/1952676330/tempctrlhyst.o: ../afoLibs/Moduli/tempctrlhyst.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/tempctrlhyst.o ../afoLibs/Moduli/tempctrlhyst.cpp

${OBJECTDIR}/_ext/1952676330/temperaturecontroller.o: ../afoLibs/Moduli/temperaturecontroller.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/temperaturecontroller.o ../afoLibs/Moduli/temperaturecontroller.cpp

${OBJECTDIR}/_ext/1952676330/nthacc.o: ../afoLibs/Moduli/nthacc.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/nthacc.o ../afoLibs/Moduli/nthacc.cpp

${OBJECTDIR}/_ext/2024910124/binaryEncoder.o: ../afoLibs/Blocks/binaryEncoder.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2024910124
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2024910124/binaryEncoder.o ../afoLibs/Blocks/binaryEncoder.cpp

${OBJECTDIR}/_ext/1952676330/ds18X20.o: ../afoLibs/Moduli/ds18X20.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/ds18X20.o ../afoLibs/Moduli/ds18X20.cpp

${OBJECTDIR}/_ext/1462128332/ownetu.o: ../afoLibs/OWAPI/ownetu.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1462128332
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1462128332/ownetu.o ../afoLibs/OWAPI/ownetu.cpp

${OBJECTDIR}/_ext/1952676330/c3pointctrl.o: ../afoLibs/Moduli/c3pointctrl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/c3pointctrl.o ../afoLibs/Moduli/c3pointctrl.cpp

${OBJECTDIR}/_ext/2024910124/contblock.o: ../afoLibs/Blocks/contblock.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2024910124
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2024910124/contblock.o ../afoLibs/Blocks/contblock.cpp

${OBJECTDIR}/_ext/1462128332/owtrnu.o: ../afoLibs/OWAPI/owtrnu.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1462128332
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1462128332/owtrnu.o ../afoLibs/OWAPI/owtrnu.cpp

${OBJECTDIR}/_ext/1462128332/cownet.o: ../afoLibs/OWAPI/cownet.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1462128332
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1462128332/cownet.o ../afoLibs/OWAPI/cownet.cpp

${OBJECTDIR}/_ext/2024910124/delayBlock.o: ../afoLibs/Blocks/delayBlock.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2024910124
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2024910124/delayBlock.o ../afoLibs/Blocks/delayBlock.cpp

${OBJECTDIR}/_ext/1952676330/nthCnt.o: ../afoLibs/Moduli/nthCnt.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/nthCnt.o ../afoLibs/Moduli/nthCnt.cpp

${OBJECTDIR}/_ext/526401590/ClockManager.o: ../afoLibs/ClockUtil/ClockManager.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/526401590
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/526401590/ClockManager.o ../afoLibs/ClockUtil/ClockManager.cpp

${OBJECTDIR}/_ext/2024910124/satblock.o: ../afoLibs/Blocks/satblock.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2024910124
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2024910124/satblock.o ../afoLibs/Blocks/satblock.cpp

${OBJECTDIR}/_ext/1952676330/ds2438.o: ../afoLibs/Moduli/ds2438.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/ds2438.o ../afoLibs/Moduli/ds2438.cpp

${OBJECTDIR}/_ext/1952676330/di2ao.o: ../afoLibs/Moduli/di2ao.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/di2ao.o ../afoLibs/Moduli/di2ao.cpp

${OBJECTDIR}/_ext/1952676330/alarmcoordinator.o: ../afoLibs/Moduli/alarmcoordinator.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/alarmcoordinator.o ../afoLibs/Moduli/alarmcoordinator.cpp

${OBJECTDIR}/_ext/1952676330/climaticcurve.o: ../afoLibs/Moduli/climaticcurve.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/climaticcurve.o ../afoLibs/Moduli/climaticcurve.cpp

${OBJECTDIR}/_ext/1952676330/conewirenet.o: ../afoLibs/Moduli/conewirenet.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/conewirenet.o ../afoLibs/Moduli/conewirenet.cpp

${OBJECTDIR}/_ext/2024910124/BlocksCommonData.o: ../afoLibs/Blocks/BlocksCommonData.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2024910124
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2024910124/BlocksCommonData.o ../afoLibs/Blocks/BlocksCommonData.cpp

${OBJECTDIR}/_ext/2024910124/C3PointCtrlBlock.o: ../afoLibs/Blocks/C3PointCtrlBlock.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2024910124
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2024910124/C3PointCtrlBlock.o ../afoLibs/Blocks/C3PointCtrlBlock.cpp

${OBJECTDIR}/_ext/1952676330/ic.o: ../afoLibs/Moduli/ic.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/ic.o ../afoLibs/Moduli/ic.cpp

${OBJECTDIR}/_ext/1450946203/timeUtil.o: ../afoLibs/OWAPI/util/timeUtil.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1450946203
	${RM} $@.d
	$(COMPILE.c) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I. -I../afoLibs/Blocks -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1450946203/timeUtil.o ../afoLibs/OWAPI/util/timeUtil.c

${OBJECTDIR}/_ext/1462128332/owllu.o: ../afoLibs/OWAPI/owllu.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1462128332
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1462128332/owllu.o ../afoLibs/OWAPI/owllu.cpp

${OBJECTDIR}/_ext/2024910124/ClockBlock.o: ../afoLibs/Blocks/ClockBlock.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2024910124
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2024910124/ClockBlock.o ../afoLibs/Blocks/ClockBlock.cpp

${OBJECTDIR}/_ext/1952676330/fullutactrl.o: ../afoLibs/Moduli/fullutactrl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/fullutactrl.o ../afoLibs/Moduli/fullutactrl.cpp

${OBJECTDIR}/_ext/1952676330/floorcoord2.o: ../afoLibs/Moduli/floorcoord2.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/floorcoord2.o ../afoLibs/Moduli/floorcoord2.cpp

${OBJECTDIR}/_ext/1991509643/gio.o: ../afoLibs/FoxUtil/gio.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1991509643
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1991509643/gio.o ../afoLibs/FoxUtil/gio.cpp

${OBJECTDIR}/_ext/287082435/CMsgRecv.o: ../afoLibs/MsgUtil/CMsgRecv.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/287082435
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/287082435/CMsgRecv.o ../afoLibs/MsgUtil/CMsgRecv.cpp

${OBJECTDIR}/_ext/1952676330/stepdigitalout.o: ../afoLibs/Moduli/stepdigitalout.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/stepdigitalout.o ../afoLibs/Moduli/stepdigitalout.cpp

${OBJECTDIR}/_ext/1459688078/CLabel.o: ../afoLibs/Label/CLabel.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1459688078
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1459688078/CLabel.o ../afoLibs/Label/CLabel.cpp

${OBJECTDIR}/_ext/1952676330/pumpcontroller.o: ../afoLibs/Moduli/pumpcontroller.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/pumpcontroller.o ../afoLibs/Moduli/pumpcontroller.cpp

${OBJECTDIR}/_ext/1952676330/changeovercoord.o: ../afoLibs/Moduli/changeovercoord.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/changeovercoord.o ../afoLibs/Moduli/changeovercoord.cpp

${OBJECTDIR}/_ext/1952676330/tempCondensa.o: ../afoLibs/Moduli/tempCondensa.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.c) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I. -I../afoLibs/Blocks -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/tempCondensa.o ../afoLibs/Moduli/tempCondensa.c

${OBJECTDIR}/_ext/1952676330/humcontroller.o: ../afoLibs/Moduli/humcontroller.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/humcontroller.o ../afoLibs/Moduli/humcontroller.cpp

${OBJECTDIR}/_ext/2024910124/block.o: ../afoLibs/Blocks/block.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2024910124
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2024910124/block.o ../afoLibs/Blocks/block.cpp

${OBJECTDIR}/_ext/1952676330/ds2751.o: ../afoLibs/Moduli/ds2751.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/ds2751.o ../afoLibs/Moduli/ds2751.cpp

${OBJECTDIR}/_ext/1952676330/commonDefinitions.o: ../afoLibs/Moduli/commonDefinitions.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/commonDefinitions.o ../afoLibs/Moduli/commonDefinitions.cpp

${OBJECTDIR}/_ext/1952676330/ds2408.o: ../afoLibs/Moduli/ds2408.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/ds2408.o ../afoLibs/Moduli/ds2408.cpp

${OBJECTDIR}/_ext/1952676330/newEngine.o: ../afoLibs/Moduli/newEngine.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/newEngine.o ../afoLibs/Moduli/newEngine.cpp

${OBJECTDIR}/raspymonitor.o: raspymonitor.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/raspymonitor.o raspymonitor.cpp

${OBJECTDIR}/_ext/2024910124/pidBlock.o: ../afoLibs/Blocks/pidBlock.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2024910124
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2024910124/pidBlock.o ../afoLibs/Blocks/pidBlock.cpp

${OBJECTDIR}/_ext/1952676330/ibuttonreader.o: ../afoLibs/Moduli/ibuttonreader.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/ibuttonreader.o ../afoLibs/Moduli/ibuttonreader.cpp

${OBJECTDIR}/_ext/1952676330/conewireengine.o: ../afoLibs/Moduli/conewireengine.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/conewireengine.o ../afoLibs/Moduli/conewireengine.cpp

${OBJECTDIR}/_ext/1952676330/remotedido.o: ../afoLibs/Moduli/remotedido.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/remotedido.o ../afoLibs/Moduli/remotedido.cpp

${OBJECTDIR}/_ext/1952676330/buttoncontroller.o: ../afoLibs/Moduli/buttoncontroller.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/buttoncontroller.o ../afoLibs/Moduli/buttoncontroller.cpp

${OBJECTDIR}/_ext/1952676330/nthvlv-adv.o: ../afoLibs/Moduli/nthvlv-adv.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/nthvlv-adv.o ../afoLibs/Moduli/nthvlv-adv.cpp

${OBJECTDIR}/_ext/1952676330/ds2405.o: ../afoLibs/Moduli/ds2405.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/ds2405.o ../afoLibs/Moduli/ds2405.cpp

${OBJECTDIR}/_ext/2024910124/costantblock.o: ../afoLibs/Blocks/costantblock.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2024910124
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2024910124/costantblock.o ../afoLibs/Blocks/costantblock.cpp

${OBJECTDIR}/_ext/1952676330/execcommands.o: ../afoLibs/Moduli/execcommands.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/execcommands.o ../afoLibs/Moduli/execcommands.cpp

${OBJECTDIR}/_ext/1952676330/timeddido.o: ../afoLibs/Moduli/timeddido.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/timeddido.o ../afoLibs/Moduli/timeddido.cpp

${OBJECTDIR}/_ext/1952676330/nthvlv.o: ../afoLibs/Moduli/nthvlv.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/nthvlv.o ../afoLibs/Moduli/nthvlv.cpp

${OBJECTDIR}/_ext/2024910124/ClimaticCurveBlock.o: ../afoLibs/Blocks/ClimaticCurveBlock.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2024910124
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2024910124/ClimaticCurveBlock.o ../afoLibs/Blocks/ClimaticCurveBlock.cpp

${OBJECTDIR}/_ext/2024910124/logblock.o: ../afoLibs/Blocks/logblock.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2024910124
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2024910124/logblock.o ../afoLibs/Blocks/logblock.cpp

${OBJECTDIR}/_ext/461864497/xmlutil.o: ../afoLibs/XML/xmlutil.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/461864497
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/461864497/xmlutil.o ../afoLibs/XML/xmlutil.cpp

${OBJECTDIR}/_ext/1952676330/digitalctrl.o: ../afoLibs/Moduli/digitalctrl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/digitalctrl.o ../afoLibs/Moduli/digitalctrl.cpp

${OBJECTDIR}/_ext/782350236/cstring.o: ../afoLibs/Strings/cstring.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/782350236
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/782350236/cstring.o ../afoLibs/Strings/cstring.cpp

${OBJECTDIR}/_ext/316114074/LibIniFile.o: ../afoLibs/IniFile/LibIniFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/316114074
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/316114074/LibIniFile.o ../afoLibs/IniFile/LibIniFile.cpp

${OBJECTDIR}/_ext/247515588/fileUtil.o: ../afoLibs/FileUtil/fileUtil.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/247515588
	${RM} $@.d
	$(COMPILE.c) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I. -I../afoLibs/Blocks -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/247515588/fileUtil.o ../afoLibs/FileUtil/fileUtil.c

${OBJECTDIR}/_ext/1952676330/scheduler.o: ../afoLibs/Moduli/scheduler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/scheduler.o ../afoLibs/Moduli/scheduler.cpp

${OBJECTDIR}/_ext/1952676330/vmultidido.o: ../afoLibs/Moduli/vmultidido.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/vmultidido.o ../afoLibs/Moduli/vmultidido.cpp

${OBJECTDIR}/_ext/619507098/linuxlnk.o: ../afoLibs/OWAPI/userial/linuxlnk.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/619507098
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/619507098/linuxlnk.o ../afoLibs/OWAPI/userial/linuxlnk.cpp

${OBJECTDIR}/_ext/1952676330/digitalio.o: ../afoLibs/Moduli/digitalio.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/digitalio.o ../afoLibs/Moduli/digitalio.cpp

${OBJECTDIR}/_ext/2024910124/opblock.o: ../afoLibs/Blocks/opblock.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2024910124
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2024910124/opblock.o ../afoLibs/Blocks/opblock.cpp

${OBJECTDIR}/_ext/1952676330/vpid.o: ../afoLibs/Moduli/vpid.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/vpid.o ../afoLibs/Moduli/vpid.cpp

${OBJECTDIR}/_ext/1952676330/accessLog.o: ../afoLibs/Moduli/accessLog.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/accessLog.o ../afoLibs/Moduli/accessLog.cpp

${OBJECTDIR}/_ext/1952676330/Thu.o: ../afoLibs/Moduli/Thu.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/Thu.o ../afoLibs/Moduli/Thu.cpp

${OBJECTDIR}/_ext/1952676330/pidsimple.o: ../afoLibs/Moduli/pidsimple.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/pidsimple.o ../afoLibs/Moduli/pidsimple.cpp

${OBJECTDIR}/_ext/2024910124/triggerBlock.o: ../afoLibs/Blocks/triggerBlock.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2024910124
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2024910124/triggerBlock.o ../afoLibs/Blocks/triggerBlock.cpp

${OBJECTDIR}/_ext/1450946203/crcutil.o: ../afoLibs/OWAPI/util/crcutil.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1450946203
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1450946203/crcutil.o ../afoLibs/OWAPI/util/crcutil.cpp

${OBJECTDIR}/_ext/1462128332/owsesu.o: ../afoLibs/OWAPI/owsesu.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1462128332
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1462128332/owsesu.o ../afoLibs/OWAPI/owsesu.cpp

${OBJECTDIR}/_ext/1952676330/nthmgc.o: ../afoLibs/Moduli/nthmgc.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/nthmgc.o ../afoLibs/Moduli/nthmgc.cpp

${OBJECTDIR}/_ext/1952676330/timemarkerctrl.o: ../afoLibs/Moduli/timemarkerctrl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/timemarkerctrl.o ../afoLibs/Moduli/timemarkerctrl.cpp

${OBJECTDIR}/_ext/1952676330/pidlmd.o: ../afoLibs/Moduli/pidlmd.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/pidlmd.o ../afoLibs/Moduli/pidlmd.cpp

${OBJECTDIR}/_ext/2024910124/gateblock.o: ../afoLibs/Blocks/gateblock.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2024910124
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2024910124/gateblock.o ../afoLibs/Blocks/gateblock.cpp

${OBJECTDIR}/_ext/2024910124/ifblock.o: ../afoLibs/Blocks/ifblock.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2024910124
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2024910124/ifblock.o ../afoLibs/Blocks/ifblock.cpp

${OBJECTDIR}/_ext/1952676330/nthvlv2-vav.o: ../afoLibs/Moduli/nthvlv2-vav.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/nthvlv2-vav.o ../afoLibs/Moduli/nthvlv2-vav.cpp

${OBJECTDIR}/_ext/461864497/Cmd.o: ../afoLibs/XML/Cmd.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/461864497
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/461864497/Cmd.o ../afoLibs/XML/Cmd.cpp

${OBJECTDIR}/_ext/1952676330/vhystcontroller.o: ../afoLibs/Moduli/vhystcontroller.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/vhystcontroller.o ../afoLibs/Moduli/vhystcontroller.cpp

${OBJECTDIR}/_ext/461856677/pid.o: ../afoLibs/PID/pid.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/461856677
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/461856677/pid.o ../afoLibs/PID/pid.cpp

${OBJECTDIR}/_ext/1952676330/tagcontrol.o: ../afoLibs/Moduli/tagcontrol.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/tagcontrol.o ../afoLibs/Moduli/tagcontrol.cpp

${OBJECTDIR}/_ext/2024910124/muxBlock.o: ../afoLibs/Blocks/muxBlock.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2024910124
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2024910124/muxBlock.o ../afoLibs/Blocks/muxBlock.cpp

${OBJECTDIR}/_ext/1463815084/SocketPort.o: ../afoLibs/Ports/SocketPort.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1463815084
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1463815084/SocketPort.o ../afoLibs/Ports/SocketPort.cpp

${OBJECTDIR}/_ext/1952676330/analogIO.o: ../afoLibs/Moduli/analogIO.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/analogIO.o ../afoLibs/Moduli/analogIO.cpp

${OBJECTDIR}/_ext/316114074/inifilemanager.o: ../afoLibs/IniFile/inifilemanager.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/316114074
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/316114074/inifilemanager.o ../afoLibs/IniFile/inifilemanager.cpp

${OBJECTDIR}/_ext/1952676330/timer.o: ../afoLibs/Moduli/timer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/timer.o ../afoLibs/Moduli/timer.cpp

${OBJECTDIR}/_ext/1952676330/utactrl.o: ../afoLibs/Moduli/utactrl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/utactrl.o ../afoLibs/Moduli/utactrl.cpp

${OBJECTDIR}/_ext/1952676330/fullutactrl2.o: ../afoLibs/Moduli/fullutactrl2.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/fullutactrl2.o ../afoLibs/Moduli/fullutactrl2.cpp

${OBJECTDIR}/_ext/1952676330/ds2890.o: ../afoLibs/Moduli/ds2890.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/ds2890.o ../afoLibs/Moduli/ds2890.cpp

${OBJECTDIR}/_ext/1952676330/vdido.o: ../afoLibs/Moduli/vdido.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/vdido.o ../afoLibs/Moduli/vdido.cpp

${OBJECTDIR}/_ext/2024910124/hystblock.o: ../afoLibs/Blocks/hystblock.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2024910124
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2024910124/hystblock.o ../afoLibs/Blocks/hystblock.cpp

${OBJECTDIR}/_ext/1952676330/digitalin2out.o: ../afoLibs/Moduli/digitalin2out.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/digitalin2out.o ../afoLibs/Moduli/digitalin2out.cpp

${OBJECTDIR}/_ext/2024910124/timerblock.o: ../afoLibs/Blocks/timerblock.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2024910124
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2024910124/timerblock.o ../afoLibs/Blocks/timerblock.cpp

${OBJECTDIR}/_ext/1952676330/nthvlv2.o: ../afoLibs/Moduli/nthvlv2.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/nthvlv2.o ../afoLibs/Moduli/nthvlv2.cpp

${OBJECTDIR}/_ext/1952676330/afoerror.o: ../afoLibs/Moduli/afoerror.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/afoerror.o ../afoLibs/Moduli/afoerror.cpp

${OBJECTDIR}/_ext/2024910124/BlockCoordinator.o: ../afoLibs/Blocks/BlockCoordinator.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2024910124
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2024910124/BlockCoordinator.o ../afoLibs/Blocks/BlockCoordinator.cpp

${OBJECTDIR}/_ext/316114074/IniFileHandler.o: ../afoLibs/IniFile/IniFileHandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/316114074
	${RM} $@.d
	$(COMPILE.cc) -Wall -DUSE_ADV_VLV -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -I. -I../afoLibs/ClockUtil -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/316114074/IniFileHandler.o ../afoLibs/IniFile/IniFileHandler.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/raspymonitor

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
