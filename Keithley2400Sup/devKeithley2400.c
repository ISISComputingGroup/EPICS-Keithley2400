/*
 * Keithley2400 device support
 */

#include <epicsStdio.h>
#include <devCommonGpib.h>


/******************************************************************************
 *
 * The following define statements are used to declare the names to be used
 * for the dset tables.   
 *
 * A DSET_AI entry must be declared here and referenced in an application
 * database description file even if the device provides no AI records.
 *
 ******************************************************************************/
#define DSET_AI     devAiKeithley2400
#define DSET_AO     devAoKeithley2400
#define DSET_BI     devBiKeithley2400
#define DSET_BO     devBoKeithley2400
#define DSET_EV     devEvKeithley2400
#define DSET_LI     devLiKeithley2400
#define DSET_LO     devLoKeithley2400
#define DSET_MBBI   devMbbiKeithley2400
#define DSET_MBBID  devMbbidKeithley2400
#define DSET_MBBO   devMbboKeithley2400
#define DSET_MBBOD  devMbbodKeithley2400
#define DSET_SI     devSiKeithley2400
#define DSET_SO     devSoKeithley2400
#define DSET_WF     devWfKeithley2400

#include <devGpib.h> /* must be included after DSET defines */

#define TIMEOUT     1.0    /* I/O must complete within this time */
#define TIMEWINDOW  2.0    /* Wait this long after device timeout */


/******************************************************************************
 * Strings used by the init routines to fill in the znam,onam,...
 * fields in BI and BO record types.
 ******************************************************************************/

static char  *offOnList[] = { "Off","On" };
static struct devGpibNames offOn = { 2,offOnList,0,1 };

static char  *resetList[] = { "Reset","Reset" };
static struct devGpibNames reset = { 2,resetList,0,1 };

static char  *clearList[] = { "Clear","Clear" };
static struct devGpibNames clear = { 2,clearList,0,1 };

static char  *tripList[] = { "OK","Trip" };
static struct devGpibNames trip = { 2,tripList,0,1 };


/******************************************************************************
 * Structures used by the init routines to fill in the onst,twst,... and the
 * onvl,twvl,... fields in MBBI and MBBO record types.
 *
 * Note that the intExtSsBm and intExtSsBmStop structures use the same
 * intExtSsBmStopList and intExtSsBmStopVal lists but have a different number
 * of elements in them that they use... The intExtSsBm structure only represents
 * 4 elements,while the intExtSsBmStop structure represents 5.
 ******************************************************************************/

static char *sourceFunctionList[] = {
    "VOLT","CURR" };
static unsigned long sourceFunctionVal[] = {
    0x1,0x2 }; 
static struct devGpibNames sourceFunctionValues = {
    2,sourceFunctionList,sourceFunctionVal,1 };

static char *senseFunctionList[] = {
    "VOLT","CURR","RES" };
static unsigned long senseFunctionVal[] = {
    0x1,0x2,0x3 }; 
static struct devGpibNames senseFunctionValues = {
    3,senseFunctionList,senseFunctionVal,2 };

static char *voltageRangeList[] = {
    "200mV","2V","20V","200V","AUTO" };
static unsigned long voltageRangeVal[] = { 
    0x0,0x1,0x2,0x3,0x4 };
static struct devGpibNames voltageValues = {
    5,voltageRangeList,voltageRangeVal,3 };

static char *currentRangeList[] = { 
    "1uA","10uA","100uA","1mA","10mA","100mA","1A","AUTO" };
static unsigned long currentRangeVal[] = { 
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7 };
static struct devGpibNames currentValues = {
    8,currentRangeList,currentRangeVal,4 };

static char *resistanceRangeList[] = { 
    "20","200","2k","20k","200k","2Meg",
    "20Meg","200Meg","AUTO" };
static unsigned long resistanceRangeVal[] = { 
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8 };
static struct devGpibNames resistanceValues = {
    9,resistanceRangeList,resistanceRangeVal,4 };

/******************************************************************************
 * String arrays for EFAST operations. The last entry must be 0.
 *
 * On input operations,only as many bytes as are found in the string array
 * elements are compared.  Additional bytes are ignored.
 * The first matching string  will be used as a match.
 *
 * For the input operations,the strings are compared literally!  This
 * can cause problems if the instrument is returning things like \r and \n
 * characters.  When defining input strings so you include them as well.
 ******************************************************************************/

static char *sourceFunctionFList[] = {
    ":SOUR:FUNC VOLT",
    ":SOUR:FUNC CURR",
    NULL
    };

static char *senseFunctionFList[] = {
    ":SENS:FUNC 'VOLT'",
    ":SENS:FUNC 'CURR'",
    ":SENS:FUNC 'RES'",
    NULL
    };

static char *voltageFList[] = {
    ":SOUR:VOLT:RANG .2",
    ":SOUR:VOLT:RANG 2",
    ":SOUR:VOLT:RANG 20",
    ":SOUR:VOLT:RANG 200",
    ":SOUR:VOLT:RANG:AUTO ON", 
    NULL
    };

static char *currentFList[] = {
    ":SOUR:CURR:RANG .000001",
    ":SOUR:CURR:RANG .00001",
    ":SOUR:CURR:RANG .0001",
    ":SOUR:CURR:RANG .001",
    ":SOUR:CURR:RANG .01",
    ":SOUR:CURR:RANG .1",
    ":SOUR:CURR:RANG 1",
    ":SOUR:CURR:RANG:AUTO ON",
    NULL
    };

static char *resistanceFList[] = {
    ":SENS:RES:RANG 20",
    ":SENS:RES:RANG 200",
    ":SENS:RES:RANG 2000",
    ":SENS:RES:RANG 20000",
    ":SENS:RES:RANG 200000",
    ":SENS:RES:RANG 2000000",
    ":SENS:RES:RANG 20000000",
    ":SENS:RES:RANG 200000000",
    ":SENS:RES:RANG:AUTO ON",
    NULL
    };


/******************************************************************************
 * Array of structures that define all GPIB messages
 * supported for this type of instrument.
 ******************************************************************************/
/*  Generic SCPI commands  */

static struct gpibCmd gpibCmds[] = {
    /* Param 0 -- Read SCPI identification string */
    {&DSET_SI, GPIBREAD, IB_Q_HIGH, "*IDN?", "%39[^\r\n]", 
         0, 200, NULL, 0, 0, NULL, NULL, NULL},

    /* Param 1 - SCPI reset command */
    {&DSET_BO, GPIBCMD, IB_Q_HIGH, "*RST", NULL, 
         0, 80, NULL, 0, 0, NULL, &reset, NULL},

    /* Param 2 - SCPI clear status command */
    {&DSET_BO, GPIBCMD, IB_Q_HIGH, "*CLS", NULL, 
         0, 80, NULL, 0, 0, NULL, &clear, NULL},

    /* Param 3 - Read SCPI status byte */
    {&DSET_LI, GPIBREAD, IB_Q_HIGH, "*STB?", "%d", 
         0, 80, NULL, 0, 0, NULL, NULL, NULL},

    /* Param 4 - Read SCPI event register */
    {&DSET_LI, GPIBREAD, IB_Q_HIGH, "*ESR?", "%d", 
         0, 80, NULL, 0, 0, NULL, NULL, NULL},

    /* Param 5 - Enable SCPI events */
    {&DSET_LO, GPIBWRITE, IB_Q_HIGH, NULL, "*ESE %d", 
         0, 80, NULL, 0, 0, NULL, NULL, NULL},

    /* Param 6 - Read back SCPI enabled events */
    {&DSET_LI, GPIBREAD, IB_Q_HIGH, "*ESE?", "%d", 
         0, 80, NULL, 0, 0, NULL, NULL, NULL},

    /* Param 7 - Enable SCPI service request sources */
    {&DSET_LO, GPIBWRITE, IB_Q_HIGH, NULL, "*SRE %d", 
         0, 80, NULL, 0, 0, NULL, NULL, NULL},

    /* Param 8 - Read back SCPI enabled service request sources */
    {&DSET_LI, GPIBREAD, IB_Q_HIGH, "*SRE?", "%d", 
         0, 80, NULL, 0, 0, NULL, NULL, NULL},

    /* Param 9 - Read SCPI output completion status */
    {&DSET_LI, GPIBREAD, IB_Q_HIGH, "*OPC?", "%d", 
         0, 80, NULL, 0, 0, NULL, NULL, 0},

/*

  Device specific commands  

 */

    /* Param 10 - Turn output on/off */
    {&DSET_BO, GPIBEFASTO, IB_Q_MEDIUM, ":OUTP ", NULL,
         0, 80, NULL, 0 ,0, offOnList, &offOn, NULL},

    /* Param 11 - Read state of output switch */
    {&DSET_BI, GPIBREAD, IB_Q_HIGH, ":OUTP?", "%lu",
         0, 80, NULL, 0, 0, NULL, &offOn, NULL},

    /* Param 12 - Set output voltage level */
    {&DSET_AO, GPIBWRITE, IB_Q_LOW, NULL, ":SOUR:VOLT:LEV %g",
         0, 80, NULL, 0, 0, NULL, NULL, NULL},

    /* Param 13 - Read voltage setting */ 
    {&DSET_AI, GPIBREAD, IB_Q_LOW, ":SOUR:VOLT:LEV?", "%lf",
         0, 80, NULL, 0, 0, NULL, NULL, NULL},

    /* Param 14 - Read voltage measurement */ 
    {&DSET_AI, GPIBREAD, IB_Q_LOW, ":FORM:ELEM VOLT;:READ?", "%lf",
         0, 80, NULL, 0, 0, NULL, NULL, NULL},

    /* Param 15 - Set current level */
    {&DSET_AO, GPIBWRITE, IB_Q_LOW, NULL,":SOUR:CURR:LEV %g",
         0, 80, NULL, 0, 0, NULL, NULL, NULL},

    /* Param 16 - Read current setting */
    {&DSET_AI, GPIBREAD, IB_Q_LOW, ":SOUR:CURR:LEV?", "%lf",
         0, 80, NULL, 0, 0, NULL, NULL, NULL},

    /* Param 17 - Read current measurement */
    {&DSET_AI, GPIBREAD, IB_Q_LOW, 
         ":FORM:ELEM CURR;:READ?", "%lf",
         0, 80 , NULL, 0, 0, NULL, NULL, NULL},

    /* Param 18 - Set voltage range */
    {&DSET_MBBO, GPIBEFASTO, IB_Q_LOW, 0, NULL,
         0, 32, 0, 0, 0, voltageFList, &voltageValues, 0},

    /* Param 19 - Set current range */
    {&DSET_MBBO, GPIBEFASTO, IB_Q_LOW, 0, NULL,
         0, 32, 0, 0, 0, currentFList, &currentValues, 0},

    /* Param 20 - Set resistance range */
    {&DSET_MBBO, GPIBEFASTO, IB_Q_LOW, 0, NULL,
         0, 32, 0, 0, 0, resistanceFList, &resistanceValues, 0},

    /* Param 21 - Set source function */
    {&DSET_MBBO, GPIBEFASTO, IB_Q_LOW, 0, NULL,
         0, 32, 0, 0, 0, sourceFunctionFList, &sourceFunctionValues, 0},

    /* Param 22 - Set sense function */
    {&DSET_MBBO, GPIBEFASTO, IB_Q_LOW, 0, NULL,
         0, 32, 0, 0, 0, senseFunctionFList, &senseFunctionValues, 0},

    /* Param 23 - Read measurement */
    {&DSET_AI, GPIBREAD, IB_Q_LOW, ":SENS:DATA:LAT?", "%lf",
         0, 80, NULL, 0, 0, NULL, NULL, 0},

    /* Param 24 - Set voltage compliance level */
    {&DSET_AO, GPIBWRITE, IB_Q_LOW, NULL, ":SENS:VOLT:PROT:LEV %g",
         0, 80, NULL, 0, 0, NULL, NULL, NULL},

    /* Param 25 - Read voltage compliance setting */ 
    {&DSET_AI, GPIBREAD, IB_Q_LOW, ":SENS:VOLT:PROT:LEV?", "%lf",
         0, 80, NULL, 0, 0, NULL, NULL, NULL},

    /* Param 26 - Read voltage compliance trip state */ 
    {&DSET_BI, GPIBREAD, IB_Q_LOW, ":SENS:VOLT:PROT:TRIP?", "%lu",
         0, 80, NULL, 0, 0, NULL, &trip, NULL},

    /* Param 27 - Set current compliance level */
    {&DSET_AO, GPIBWRITE, IB_Q_LOW, NULL, ":SENS:CURR:PROT:LEV %g",
         0, 80, NULL, 0, 0, NULL, NULL, NULL},

    /* Param 28 - Read current compliance setting */ 
    {&DSET_AI, GPIBREAD, IB_Q_LOW, ":SENS:CURR:PROT:LEV?", "%lf",
         0, 80, NULL, 0, 0, NULL, NULL, NULL},

    /* Param 29 - Read current compliance trip state */ 
    {&DSET_BI, GPIBREAD, IB_Q_LOW, ":SENS:CURR:PROT:TRIP?", "%lu",
         0, 80, NULL, 0, 0, NULL, &trip, NULL},

    /* Param 30 - Read resistance measurement */
    {&DSET_AI, GPIBREAD, IB_Q_LOW, 
         ":FORM:ELEM RES;:READ?", "%lf",
         0, 80 , NULL, 0, 0, NULL, NULL, NULL},

};

/* The following is the number of elements in the command array above.  */
#define NUMPARAMS sizeof(gpibCmds)/sizeof(struct gpibCmd)


/******************************************************************************
 * Initialize device support parameters
 *
 *****************************************************************************/
static long init_ai(int parm)
{
    if(parm==0) {
        devSupParms.name = "devKeithley2400";
        devSupParms.gpibCmds = gpibCmds;
        devSupParms.numparams = NUMPARAMS;
        devSupParms.timeout = TIMEOUT;
        devSupParms.timeWindow = TIMEWINDOW;
        devSupParms.respond2Writes = -1;
    }
    return(0);
}
