#include "AEUserTermTypes.r"

#define MSWindows 0
#include "piactions.h"

#include "ui.h"
#include "version.h"

//-------------------------------------------------------------------------------
//	Dictionary (scripting) resource
//-------------------------------------------------------------------------------

resource 'aete' (AETE_ID, "", purgeable)
{
	1, 0, english, roman,				/* aete version and language specifiers */
	{
		vendorName,						/* vendor suite name */
		"",								/* optional description */
		plugInSuiteID,					/* suite ID */
		1,								/* suite code, must be 1 */
		1,								/* suite level, must be 1 */
		{								/* structure for filters */
			plugInName,					/* unique filter name */
			"",							/* optional description */
			plugInClassID,				/* class ID, must be unique or Suite ID */
			plugInEventID,				/* event ID, must be unique to class ID */
			
			NO_REPLY,					/* never a reply */
			IMAGE_DIRECT_PARAMETER,		/* direct parameter, used by Photoshop */
			{	/* parameters here, if any */
				"R", PARAM_R_KEY, typeChar, "R channel expression", flagsOptionalSingleParameter,
				"G", PARAM_G_KEY, typeChar, "G channel expression", flagsOptionalSingleParameter,
				"B", PARAM_B_KEY, typeChar, "B channel expression", flagsOptionalSingleParameter,
				"A", PARAM_A_KEY, typeChar, "A channel expression", flagsOptionalSingleParameter,
				"ctl0", PARAM_CTL0_KEY, typeInteger, "ctl(0) setting", flagsOptionalSingleParameter,
				"ctl1", PARAM_CTL1_KEY, typeInteger, "ctl(1) setting", flagsOptionalSingleParameter,
				"ctl2", PARAM_CTL2_KEY, typeInteger, "ctl(2) setting", flagsOptionalSingleParameter,
				"ctl3", PARAM_CTL3_KEY, typeInteger, "ctl(3) setting", flagsOptionalSingleParameter,
				"ctl4", PARAM_CTL4_KEY, typeInteger, "ctl(4) setting", flagsOptionalSingleParameter,
				"ctl5", PARAM_CTL5_KEY, typeInteger, "ctl(5) setting", flagsOptionalSingleParameter,
				"ctl6", PARAM_CTL6_KEY, typeInteger, "ctl(6) setting", flagsOptionalSingleParameter,
				"ctl7", PARAM_CTL7_KEY, typeInteger, "ctl(7) setting", flagsOptionalSingleParameter,
			}
		},
		{ },
		{ },
		{ }
	}
};
