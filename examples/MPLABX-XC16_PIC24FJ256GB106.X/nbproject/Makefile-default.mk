#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/MPLABX-XC16_PIC24FJ256GB106.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/MPLABX-XC16_PIC24FJ256GB106.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../../source/nOSEvent.c ../../source/nOSFlag.c ../../source/nOSList.c ../../source/nOSMem.c ../../source/nOSMutex.c ../../source/nOSQueue.c ../../source/nOSSched.c ../../source/nOSSem.c ../../source/nOSThread.c ../../source/nOSTime.c ../../source/nOSTimer.c ../../source/port/XC16/PIC24/nOSPort.c ../../source/nOSSignal.c main.c ../../source/nOSAlarm.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/870914629/nOSEvent.o ${OBJECTDIR}/_ext/870914629/nOSFlag.o ${OBJECTDIR}/_ext/870914629/nOSList.o ${OBJECTDIR}/_ext/870914629/nOSMem.o ${OBJECTDIR}/_ext/870914629/nOSMutex.o ${OBJECTDIR}/_ext/870914629/nOSQueue.o ${OBJECTDIR}/_ext/870914629/nOSSched.o ${OBJECTDIR}/_ext/870914629/nOSSem.o ${OBJECTDIR}/_ext/870914629/nOSThread.o ${OBJECTDIR}/_ext/870914629/nOSTime.o ${OBJECTDIR}/_ext/870914629/nOSTimer.o ${OBJECTDIR}/_ext/1743343961/nOSPort.o ${OBJECTDIR}/_ext/870914629/nOSSignal.o ${OBJECTDIR}/main.o ${OBJECTDIR}/_ext/870914629/nOSAlarm.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/870914629/nOSEvent.o.d ${OBJECTDIR}/_ext/870914629/nOSFlag.o.d ${OBJECTDIR}/_ext/870914629/nOSList.o.d ${OBJECTDIR}/_ext/870914629/nOSMem.o.d ${OBJECTDIR}/_ext/870914629/nOSMutex.o.d ${OBJECTDIR}/_ext/870914629/nOSQueue.o.d ${OBJECTDIR}/_ext/870914629/nOSSched.o.d ${OBJECTDIR}/_ext/870914629/nOSSem.o.d ${OBJECTDIR}/_ext/870914629/nOSThread.o.d ${OBJECTDIR}/_ext/870914629/nOSTime.o.d ${OBJECTDIR}/_ext/870914629/nOSTimer.o.d ${OBJECTDIR}/_ext/1743343961/nOSPort.o.d ${OBJECTDIR}/_ext/870914629/nOSSignal.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/_ext/870914629/nOSAlarm.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/870914629/nOSEvent.o ${OBJECTDIR}/_ext/870914629/nOSFlag.o ${OBJECTDIR}/_ext/870914629/nOSList.o ${OBJECTDIR}/_ext/870914629/nOSMem.o ${OBJECTDIR}/_ext/870914629/nOSMutex.o ${OBJECTDIR}/_ext/870914629/nOSQueue.o ${OBJECTDIR}/_ext/870914629/nOSSched.o ${OBJECTDIR}/_ext/870914629/nOSSem.o ${OBJECTDIR}/_ext/870914629/nOSThread.o ${OBJECTDIR}/_ext/870914629/nOSTime.o ${OBJECTDIR}/_ext/870914629/nOSTimer.o ${OBJECTDIR}/_ext/1743343961/nOSPort.o ${OBJECTDIR}/_ext/870914629/nOSSignal.o ${OBJECTDIR}/main.o ${OBJECTDIR}/_ext/870914629/nOSAlarm.o

# Source Files
SOURCEFILES=../../source/nOSEvent.c ../../source/nOSFlag.c ../../source/nOSList.c ../../source/nOSMem.c ../../source/nOSMutex.c ../../source/nOSQueue.c ../../source/nOSSched.c ../../source/nOSSem.c ../../source/nOSThread.c ../../source/nOSTime.c ../../source/nOSTimer.c ../../source/port/XC16/PIC24/nOSPort.c ../../source/nOSSignal.c main.c ../../source/nOSAlarm.c


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/MPLABX-XC16_PIC24FJ256GB106.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=24FJ256GB106
MP_LINKER_FILE_OPTION=,--script=p24FJ256GB106.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/870914629/nOSEvent.o: ../../source/nOSEvent.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSEvent.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSEvent.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSEvent.c  -o ${OBJECTDIR}/_ext/870914629/nOSEvent.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSEvent.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSEvent.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSFlag.o: ../../source/nOSFlag.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSFlag.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSFlag.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSFlag.c  -o ${OBJECTDIR}/_ext/870914629/nOSFlag.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSFlag.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSFlag.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSList.o: ../../source/nOSList.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSList.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSList.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSList.c  -o ${OBJECTDIR}/_ext/870914629/nOSList.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSList.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSList.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSMem.o: ../../source/nOSMem.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSMem.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSMem.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSMem.c  -o ${OBJECTDIR}/_ext/870914629/nOSMem.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSMem.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSMem.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSMutex.o: ../../source/nOSMutex.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSMutex.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSMutex.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSMutex.c  -o ${OBJECTDIR}/_ext/870914629/nOSMutex.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSMutex.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSMutex.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSQueue.o: ../../source/nOSQueue.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSQueue.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSQueue.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSQueue.c  -o ${OBJECTDIR}/_ext/870914629/nOSQueue.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSQueue.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSQueue.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSSched.o: ../../source/nOSSched.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSSched.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSSched.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSSched.c  -o ${OBJECTDIR}/_ext/870914629/nOSSched.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSSched.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSSched.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSSem.o: ../../source/nOSSem.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSSem.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSSem.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSSem.c  -o ${OBJECTDIR}/_ext/870914629/nOSSem.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSSem.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSSem.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSThread.o: ../../source/nOSThread.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSThread.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSThread.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSThread.c  -o ${OBJECTDIR}/_ext/870914629/nOSThread.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSThread.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSThread.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSTime.o: ../../source/nOSTime.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSTime.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSTime.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSTime.c  -o ${OBJECTDIR}/_ext/870914629/nOSTime.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSTime.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSTime.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSTimer.o: ../../source/nOSTimer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSTimer.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSTimer.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSTimer.c  -o ${OBJECTDIR}/_ext/870914629/nOSTimer.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSTimer.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSTimer.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1743343961/nOSPort.o: ../../source/port/XC16/PIC24/nOSPort.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1743343961" 
	@${RM} ${OBJECTDIR}/_ext/1743343961/nOSPort.o.d 
	@${RM} ${OBJECTDIR}/_ext/1743343961/nOSPort.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/port/XC16/PIC24/nOSPort.c  -o ${OBJECTDIR}/_ext/1743343961/nOSPort.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1743343961/nOSPort.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/1743343961/nOSPort.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSSignal.o: ../../source/nOSSignal.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSSignal.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSSignal.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSSignal.c  -o ${OBJECTDIR}/_ext/870914629/nOSSignal.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSSignal.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSSignal.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/main.o: main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/main.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSAlarm.o: ../../source/nOSAlarm.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSAlarm.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSAlarm.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSAlarm.c  -o ${OBJECTDIR}/_ext/870914629/nOSAlarm.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSAlarm.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSAlarm.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
else
${OBJECTDIR}/_ext/870914629/nOSEvent.o: ../../source/nOSEvent.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSEvent.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSEvent.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSEvent.c  -o ${OBJECTDIR}/_ext/870914629/nOSEvent.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSEvent.o.d"        -g -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSEvent.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSFlag.o: ../../source/nOSFlag.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSFlag.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSFlag.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSFlag.c  -o ${OBJECTDIR}/_ext/870914629/nOSFlag.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSFlag.o.d"        -g -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSFlag.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSList.o: ../../source/nOSList.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSList.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSList.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSList.c  -o ${OBJECTDIR}/_ext/870914629/nOSList.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSList.o.d"        -g -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSList.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSMem.o: ../../source/nOSMem.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSMem.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSMem.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSMem.c  -o ${OBJECTDIR}/_ext/870914629/nOSMem.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSMem.o.d"        -g -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSMem.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSMutex.o: ../../source/nOSMutex.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSMutex.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSMutex.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSMutex.c  -o ${OBJECTDIR}/_ext/870914629/nOSMutex.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSMutex.o.d"        -g -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSMutex.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSQueue.o: ../../source/nOSQueue.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSQueue.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSQueue.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSQueue.c  -o ${OBJECTDIR}/_ext/870914629/nOSQueue.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSQueue.o.d"        -g -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSQueue.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSSched.o: ../../source/nOSSched.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSSched.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSSched.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSSched.c  -o ${OBJECTDIR}/_ext/870914629/nOSSched.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSSched.o.d"        -g -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSSched.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSSem.o: ../../source/nOSSem.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSSem.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSSem.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSSem.c  -o ${OBJECTDIR}/_ext/870914629/nOSSem.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSSem.o.d"        -g -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSSem.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSThread.o: ../../source/nOSThread.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSThread.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSThread.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSThread.c  -o ${OBJECTDIR}/_ext/870914629/nOSThread.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSThread.o.d"        -g -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSThread.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSTime.o: ../../source/nOSTime.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSTime.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSTime.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSTime.c  -o ${OBJECTDIR}/_ext/870914629/nOSTime.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSTime.o.d"        -g -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSTime.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSTimer.o: ../../source/nOSTimer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSTimer.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSTimer.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSTimer.c  -o ${OBJECTDIR}/_ext/870914629/nOSTimer.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSTimer.o.d"        -g -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSTimer.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1743343961/nOSPort.o: ../../source/port/XC16/PIC24/nOSPort.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1743343961" 
	@${RM} ${OBJECTDIR}/_ext/1743343961/nOSPort.o.d 
	@${RM} ${OBJECTDIR}/_ext/1743343961/nOSPort.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/port/XC16/PIC24/nOSPort.c  -o ${OBJECTDIR}/_ext/1743343961/nOSPort.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1743343961/nOSPort.o.d"        -g -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/1743343961/nOSPort.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSSignal.o: ../../source/nOSSignal.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSSignal.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSSignal.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSSignal.c  -o ${OBJECTDIR}/_ext/870914629/nOSSignal.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSSignal.o.d"        -g -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSSignal.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/main.o: main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/main.o.d"        -g -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/main.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/870914629/nOSAlarm.o: ../../source/nOSAlarm.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/870914629" 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSAlarm.o.d 
	@${RM} ${OBJECTDIR}/_ext/870914629/nOSAlarm.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../source/nOSAlarm.c  -o ${OBJECTDIR}/_ext/870914629/nOSAlarm.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/870914629/nOSAlarm.o.d"        -g -omf=elf -O0 -I"." -I"../../include" -I"../../include/port/XC16/PIC24" -msmart-io=1 -Wall -msfr-warn=off
	@${FIXDEPS} "${OBJECTDIR}/_ext/870914629/nOSAlarm.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemblePreproc
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/MPLABX-XC16_PIC24FJ256GB106.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/MPLABX-XC16_PIC24FJ256GB106.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -omf=elf  -mreserve=data@0x800:0x81F -mreserve=data@0x820:0x821 -mreserve=data@0x822:0x823 -mreserve=data@0x824:0x825 -mreserve=data@0x826:0x84F   -Wl,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK3=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem$(MP_EXTRA_LD_POST) 
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/MPLABX-XC16_PIC24FJ256GB106.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/MPLABX-XC16_PIC24FJ256GB106.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -Wl,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem$(MP_EXTRA_LD_POST) 
	${MP_CC_DIR}\\xc16-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/MPLABX-XC16_PIC24FJ256GB106.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf  
	
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/default
	${RM} -r dist/default

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
