type'DRVR'{
	boolean = 0; /* unused */
	boolean dontNeedLock,needLock; /* lock drvr in memory */
	boolean dontNeedTime,needTime; /* for periodic action */
	boolean dontNeedGoodbye,needGoodbye; /* call before heap reinit*/
	boolean noStatusEnable,statusEnable; /* responds to status */
	boolean noCtlEnable,ctlEnable; /* responds to control */
	boolean noWriteEnable,writeEnable; /* responds to write */
	boolean noReadEnable,readEnable; /* responds to read */
	byte = 0;
	
	integer; /* driver delay (ticks) */
	integer; /* desk acc event mask */
	integer; /* driver menu ID */
	
	integer = (c>>3);    /* offset to DRVRRuntime open */
	integer = (c>>3)+4;  /* offset to DRVRRuntime prime */
	integer = (c>>3)+8;  /* offset to DRVRRuntime control*/
	integer = (c>>3)+12; /* offset to DRVRRuntime status */
	integer = (c>>3)+16; /* offset to DRVRRuntime close */
	
	pstring; /* driver name; IM 4 says THIS IS REQUIRED */
	align word;
c:	hex string; /* driver code (use $$resource(...)) */
};
