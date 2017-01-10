TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# build an ioc application

PROD_IOC = Keithley2400Test

DBD += Keithley2400Test.dbd
# Keithley2400Test.dbd will be made up from these files:
Keithley2400Test_DBD += base.dbd
#include definitions for any other support applications needed
Keithley2400Test_DBD += devKeithley2400.dbd
Keithley2400Test_DBD += drvAsynSerialPort.dbd
Keithley2400Test_DBD += asynRecord.dbd

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
Keithley2400Test_SRCS += Keithley2400Test_registerRecordDeviceDriver.cpp
Keithley2400Test_SRCS_DEFAULT += Keithley2400TestMain.cpp
Keithley2400Test_SRCS_vxWorks += -nil-

#The following adds support from base/src/vxWorks
Keithley2400Test_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

#add a definition for each support application used by this application
Keithley2400Test_LIBS += devKeithley2400 asyn


#NOTES:
#      1)It is not possible to build sncExample both as a component of Keithley2400Test
#        and standalone. You must choose only one.
#      2)To build sncExample SNCSEQ must be defined in <top>/configure/RELEASE

#The following builds sncExample as a component of Keithley2400Test
#Keithley2400Test_SRCS += sncExample.stt
#Keithley2400Test_LIBS += seq pv

Keithley2400Test_LIBS += $(EPICS_BASE_IOC_LIBS)

#The following builds sncExample as a standalone application
#PROD_HOST += sncExample
#sncExample_SNCFLAGS += +m
#sncExample_SRCS += sncExample.stt
#sncExample_LIBS += seq pv
#sncExample_LIBS += $(EPICS_BASE_HOST_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

