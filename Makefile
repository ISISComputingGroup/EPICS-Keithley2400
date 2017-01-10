# Makefile for Asyn keithley2400 support
#
# Created by afp on Fri Aug 17 14:05:45 2007
# Based on the Asyn devGpib template

TOP = .
include $(TOP)/configure/CONFIG

DIRS := configure
DIRS += $(wildcard *[Ss]up)
DIRS += $(wildcard *[Aa]pp)
DIRS += $(wildcard ioc[Bb]oot)

include $(TOP)/configure/RULES_TOP
