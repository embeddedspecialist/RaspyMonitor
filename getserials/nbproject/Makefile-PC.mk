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
	${OBJECTDIR}/_ext/1450946203/ioutil.o \
	${OBJECTDIR}/_ext/1462128332/owtrnu.o \
	${OBJECTDIR}/_ext/1462128332/cownet.o \
	${OBJECTDIR}/_ext/1462128332/ownetu.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/_ext/1450946203/timeUtil.o \
	${OBJECTDIR}/_ext/1462128332/owllu.o \
	${OBJECTDIR}/_ext/1991509643/gio.o \
	${OBJECTDIR}/_ext/287082435/CMsgRecv.o \
	${OBJECTDIR}/_ext/1459688078/CLabel.o \
	${OBJECTDIR}/_ext/1952676330/commonDefinitions.o \
	${OBJECTDIR}/_ext/247515588/fileUtil.o \
	${OBJECTDIR}/_ext/461864497/xmlutil.o \
	${OBJECTDIR}/_ext/782350236/cstring.o \
	${OBJECTDIR}/_ext/316114074/LibIniFile.o \
	${OBJECTDIR}/_ext/619507098/linuxlnk.o \
	${OBJECTDIR}/_ext/1462128332/owsesu.o \
	${OBJECTDIR}/_ext/1450946203/crcutil.o \
	${OBJECTDIR}/_ext/461864497/Cmd.o \
	${OBJECTDIR}/_ext/1463815084/SocketPort.o \
	${OBJECTDIR}/_ext/316114074/inifilemanager.o \
	${OBJECTDIR}/_ext/1952676330/afoerror.o \
	${OBJECTDIR}/_ext/316114074/IniFileHandler.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/getserials

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/getserials: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/getserials ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/1450946203/ioutil.o: ../afoLibs/OWAPI/util/ioutil.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1450946203
	${RM} $@.d
	$(COMPILE.c) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1450946203/ioutil.o ../afoLibs/OWAPI/util/ioutil.c

${OBJECTDIR}/_ext/1462128332/owtrnu.o: ../afoLibs/OWAPI/owtrnu.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1462128332
	${RM} $@.d
	$(COMPILE.cc) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1462128332/owtrnu.o ../afoLibs/OWAPI/owtrnu.cpp

${OBJECTDIR}/_ext/1462128332/cownet.o: ../afoLibs/OWAPI/cownet.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1462128332
	${RM} $@.d
	$(COMPILE.cc) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1462128332/cownet.o ../afoLibs/OWAPI/cownet.cpp

${OBJECTDIR}/_ext/1462128332/ownetu.o: ../afoLibs/OWAPI/ownetu.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1462128332
	${RM} $@.d
	$(COMPILE.cc) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1462128332/ownetu.o ../afoLibs/OWAPI/ownetu.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/_ext/1450946203/timeUtil.o: ../afoLibs/OWAPI/util/timeUtil.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1450946203
	${RM} $@.d
	$(COMPILE.c) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1450946203/timeUtil.o ../afoLibs/OWAPI/util/timeUtil.c

${OBJECTDIR}/_ext/1462128332/owllu.o: ../afoLibs/OWAPI/owllu.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1462128332
	${RM} $@.d
	$(COMPILE.cc) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1462128332/owllu.o ../afoLibs/OWAPI/owllu.cpp

${OBJECTDIR}/_ext/1991509643/gio.o: ../afoLibs/FoxUtil/gio.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1991509643
	${RM} $@.d
	$(COMPILE.cc) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1991509643/gio.o ../afoLibs/FoxUtil/gio.cpp

${OBJECTDIR}/_ext/287082435/CMsgRecv.o: ../afoLibs/MsgUtil/CMsgRecv.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/287082435
	${RM} $@.d
	$(COMPILE.cc) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/287082435/CMsgRecv.o ../afoLibs/MsgUtil/CMsgRecv.cpp

${OBJECTDIR}/_ext/1459688078/CLabel.o: ../afoLibs/Label/CLabel.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1459688078
	${RM} $@.d
	$(COMPILE.cc) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1459688078/CLabel.o ../afoLibs/Label/CLabel.cpp

${OBJECTDIR}/_ext/1952676330/commonDefinitions.o: ../afoLibs/Moduli/commonDefinitions.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/commonDefinitions.o ../afoLibs/Moduli/commonDefinitions.cpp

${OBJECTDIR}/_ext/247515588/fileUtil.o: ../afoLibs/FileUtil/fileUtil.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/247515588
	${RM} $@.d
	$(COMPILE.c) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/247515588/fileUtil.o ../afoLibs/FileUtil/fileUtil.c

${OBJECTDIR}/_ext/461864497/xmlutil.o: ../afoLibs/XML/xmlutil.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/461864497
	${RM} $@.d
	$(COMPILE.cc) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/461864497/xmlutil.o ../afoLibs/XML/xmlutil.cpp

${OBJECTDIR}/_ext/782350236/cstring.o: ../afoLibs/Strings/cstring.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/782350236
	${RM} $@.d
	$(COMPILE.cc) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/782350236/cstring.o ../afoLibs/Strings/cstring.cpp

${OBJECTDIR}/_ext/316114074/LibIniFile.o: ../afoLibs/IniFile/LibIniFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/316114074
	${RM} $@.d
	$(COMPILE.cc) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/316114074/LibIniFile.o ../afoLibs/IniFile/LibIniFile.cpp

${OBJECTDIR}/_ext/619507098/linuxlnk.o: ../afoLibs/OWAPI/userial/linuxlnk.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/619507098
	${RM} $@.d
	$(COMPILE.cc) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/619507098/linuxlnk.o ../afoLibs/OWAPI/userial/linuxlnk.cpp

${OBJECTDIR}/_ext/1462128332/owsesu.o: ../afoLibs/OWAPI/owsesu.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1462128332
	${RM} $@.d
	$(COMPILE.cc) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1462128332/owsesu.o ../afoLibs/OWAPI/owsesu.cpp

${OBJECTDIR}/_ext/1450946203/crcutil.o: ../afoLibs/OWAPI/util/crcutil.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1450946203
	${RM} $@.d
	$(COMPILE.cc) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1450946203/crcutil.o ../afoLibs/OWAPI/util/crcutil.cpp

${OBJECTDIR}/_ext/461864497/Cmd.o: ../afoLibs/XML/Cmd.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/461864497
	${RM} $@.d
	$(COMPILE.cc) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/461864497/Cmd.o ../afoLibs/XML/Cmd.cpp

${OBJECTDIR}/_ext/1463815084/SocketPort.o: ../afoLibs/Ports/SocketPort.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1463815084
	${RM} $@.d
	$(COMPILE.cc) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1463815084/SocketPort.o ../afoLibs/Ports/SocketPort.cpp

${OBJECTDIR}/_ext/316114074/inifilemanager.o: ../afoLibs/IniFile/inifilemanager.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/316114074
	${RM} $@.d
	$(COMPILE.cc) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/316114074/inifilemanager.o ../afoLibs/IniFile/inifilemanager.cpp

${OBJECTDIR}/_ext/1952676330/afoerror.o: ../afoLibs/Moduli/afoerror.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1952676330
	${RM} $@.d
	$(COMPILE.cc) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1952676330/afoerror.o ../afoLibs/Moduli/afoerror.cpp

${OBJECTDIR}/_ext/316114074/IniFileHandler.o: ../afoLibs/IniFile/IniFileHandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/316114074
	${RM} $@.d
	$(COMPILE.cc) -g -I../afoLibs -I../afoLibs/FoxUtil -I../afoLibs/IniFile -I../afoLibs/Label -I../afoLibs/Moduli -I../afoLibs/MsgUtil -I../afoLibs/OWAPI -I../afoLibs/OWAPI/userial -I../afoLibs/OWAPI/util -I../afoLibs/Ports -I../afoLibs/Strings -I../afoLibs/XML -I../afoLibs/PID -I../afoLibs/FileUtil -I../afoLibs/Blocks -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/316114074/IniFileHandler.o ../afoLibs/IniFile/IniFileHandler.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/getserials

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
