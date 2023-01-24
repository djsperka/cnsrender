/*
 *  dpxdemo_strabismus.c
 *
 *	Demo file for strabismus tracking using DATAPixx dual gaze-contingent display function.
 *	Created by Peter April.
 *
 */

#include <stdio.h>
#include <popt.h>
#include "libdpx.h"

int main(int argc, char **argv)
{
	// Open the Datapixx
	DPxOpen();
	if (DPxGetError() != DPX_SUCCESS)
	{
		printf("ERROR: Couldn't open a Datapixx!\n");
		return -1;
	}
	// Stop any schedules which may have been started from other applications, and were left running.
	DPxStopAllScheds();
	DPxUpdateRegCache();					// Implement immediately

	// device is open, parse arguments
	poptContext poptcon;
	int rc;
	struct poptOption optionsTable[] =
	{
		{"led-on", 'L', POPT_ARG_NONE, NULL, 'L', "Turn on LED backlight.", ""},
		{"led-off", 'l', POPT_ARG_NONE, NULL, 'l', "Turn off LED backlight.", ""},
		{"awake", 'a', POPT_ARG_NONE, NULL, 'a', "Wake up ProPIXX.", ""},
		{"sleep", 's', POPT_ARG_NONE, NULL, 's', "Put ProPIXX to sleep.", ""},
		{"front", 'f', POPT_ARG_NONE, NULL, 'f', "Set to front projection mode.", ""},
		{"rear", 'r', POPT_ARG_NONE, NULL, 'r', "Set to rear projection mode.", ""},
		{"info", 'i', POPT_ARG_NONE, NULL, 'i', "Dump PPX info.", ""},
		POPT_AUTOHELP
		POPT_TABLEEND
	};

	poptcon = poptGetContext(NULL, argc, (const char **)argv, optionsTable, 0);

    while ((rc = poptGetNextOpt(poptcon)) > 0)
    {
    	switch (rc)
    	{
    	case 'L':
    	{
    		if (DPxIsPPxLampLedEnabled())
    		{
    			printf("LED is already enabled.\n");
    		}
    		else
    		{
    			DPxEnablePPxLampLed();
        		DPxUpdateRegCache();
    		}
    		break;
    	}
    	case 'l':
    	{
    		if (!DPxIsPPxLampLedEnabled())
    		{
    			DPxDisablePPxLampLed();
        		DPxUpdateRegCache();
    		}
    		else
    		{
    			printf("LED is already disabled.\n");
    		}
    		break;
    	}
    	case 'r':
    	{
    		if (!DPxIsPPxRearProjection())
    		{
				DPxEnablePPxRearProjection();
				DPxUpdateRegCache();					// Implement immediately
    		}
    		else
    		{
    			printf("ProPIXX is already in rear projection mode.\n");
    		}
    		break;
    	}
    	case 'f':
    	{
    		if (DPxIsPPxRearProjection())
    		{
				DPxDisablePPxRearProjection();
				DPxUpdateRegCache();					// Implement immediately
    		}
    		else
    		{
    			printf("ProPIXX is already in front projection mode.\n");
    		}
    		break;
    	}
    	case 's':
    	{
    		if (DPxIsPPxAwake())
    		{
				DPxSetPPxSleep();
				DPxUpdateRegCache();
    		}
    		else
    		{
    			printf("ProPIXX is already sleeping.\n");
    		}
    		break;
    	}
    	case 'a':
    	{
    		if (!DPxIsPPxAwake())
    		{
    			DPxSetPPxAwake();
				DPxUpdateRegCache();
    		}
    		else
    		{
    			printf("ProPIXX is already awake.\n");
    		}
    		break;
    	}
    	case 'i':
    	{
    		// Dump what info we can gather about the device.
    		printf("Firmware rev %d\n", DPxGetFirmwareRev());
    		printf("Lines %d\n", DPxGetVidVActive());
    		printf("Rate %lf\n", DPxGetVidVFreq());
    		printf("Hotspot correction %d\n", DPxIsPPxHotSpotCorrection());
    		printf("Rear proj? %d\n", DPxIsPPxRearProjection());
    		break;
    	}
    	default:
    	{
    		poptPrintHelp(poptcon, stdout, 0);
    		break;
    	}
    	}
   	}


	poptFreeContext(poptcon);



	
	DPxClose();
	if (DPxGetError() != DPX_SUCCESS)
	{
		printf("ERROR: Datapixx error = %d!\n", DPxGetError());
		return -1;
	}
	else
	{
		printf("Datapixx setup done\n");
	}
	return 0;
}
