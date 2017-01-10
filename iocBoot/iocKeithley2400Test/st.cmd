#!../../bin/linux-x86/Keithley2400Test

## You may have to change Keithley2400Test to something else
## everywhere it appears in this file

< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase("dbd/Keithley2400Test.dbd",0,0)
Keithley2400Test_registerRecordDeviceDriver(pdbbase)

#The following commands are for a local serial line
drvAsynSerialPortConfigure("L0","/dev/ttyS0",0,0,0)
asynSetOption("L0", -1, "baud", "9600")
asynSetOption("L0", -1, "bits", "8")
asynSetOption("L0", -1, "parity", "none")
asynSetOption("L0", -1, "stop", "1")
asynSetOption("L0", -1, "clocal", "Y")
asynSetOption("L0", -1, "crtscts", "N")
asynOctetSetInputEos("L0", -1, "\r")
asynOctetSetOutputEos("L0", -1, "\n")

asynSetTraceFile("L0",-1,"")
#asynSetTraceMask("L0",-1,0x09)
asynSetTraceIOMask("L0",-1,0x2) 

## Load record instances
dbLoadRecords("db/devKeithley2400.vdb","P=keithley,R=2400:,L=0,A=-1")
dbLoadRecords("db/asynRecord.db","P=keithley,R=2400,PORT=L0,ADDR=-1,OMAX=0,IMAX=0")

cd ${TOP}/iocBoot/${IOC}
iocInit()

## Start any sequence programs
#seq sncxxx,"user=afpHost"
