TOP=..
include $(TOP)/configure/CONFIG
#=======================================

# Build devKeithley2400 as a library for an IOC:
LIBRARY_IOC += devKeithley2400

# Library Source files
devKeithley2400_SRCS += devKeithley2400.c

# Link with the asyn and base libraries
devKeithley2400_LIBS += asyn
devKeithley2400_LIBS += $(EPICS_BASE_IOC_LIBS)

# Install .dbd and .db files
DBD += devKeithley2400.dbd
DB += devKeithley2400.vdb
DATA += Keithley2400.proto

#=======================================
include $(TOP)/configure/RULES
