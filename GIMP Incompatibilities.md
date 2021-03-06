Incompatibilities with GIMP / PSPI
----------------------------------

1. Filter cannot be loaded by PSPI, because of a memory error.

	Status: FIXED (Bug in FF) in FilterFoundry 1.7
	
	FilterFoundry 1.6 used wrong length values for the TLV (Type, Length, Value) members
	of the PIPL structure (in RC files, and in fixpipl()).
	The length value must include everything, including paddings of strings.
	
	In regards propertyLength, Adobe writes in SPPiPL.h:
	"Number of characters in the data array. Rounded to a multiple of 4."
	
	On the other hand, the 1997 PICA documentation (page 23) and
	1996 "Cross-Application Plug-in Development Resource Guide" describes:
	"[propertyLength] contains the length of the propertyData field. It does not include any padding bytes after
	propertyData to achieve four byte alignment. This field may be zero."
	
	I think this is not correct, since even official plugins of Adobe (e.g. "3D Transform.8bf") and cnvtpipl
	are rounding the length to a multiple of 4 (actually, rounding to the next possible multiple 4,
	so that padding is always guaranteed).
	Photoshop (tested with Photoshop 7) will crash if the propertyLength follows the definition of PICA.

2. Filters will only fill the bottom of the picture, not the whole canvas.

	Status: FIXED (Workaround) in FilterFoundry 1.7
	
	The reason is that OnContinue/main.c:RequestNext() processes the image in chunks,
	and probably due to a bug, PSPI only applies the image data of the last chunk.

	Workaround applied in FF 1.7: If the host signature is "GIMP" (PMIG), then we set
	needall=1 to disable chunked processing.

3. When you re-open the main window, the formulas of the previous session vanish.

	Status: FIXED (Workaround) in FilterFoundry 1.7

	The reason is a bug in PSPI: The host should preserve the value of pb->parameters, which PSPI doesn't do.
	Also, all global variables are unloaded, so the plugin cannot preserve any data.

	Workaround in FF 1.7: If the host GIMP is detected, the new function "host_preserves_parameters" returns "true".
	This mode saves the filter data into a temporary AFS file and loads it
	when the window is opened again.

	Exactly the same problem applies to the IfanView 8BF host.

4. Saving does not work

	Status: FIXED (Workaround) in FilterFoundry 1.7

	Filters could not be saved as AFS files because PSPI can't handle zero-sized handles
	(PINEWHANDLE), so FF 1.7 is now assigning 1 byte for these.

5. For a 1920x1080 picture, the fitzoom in GIMP/PSPI is not correct. It is 8% on GIMP but should be 7%.

	Status: FIXED (Workaround) in FilterFoundry 1.7
	
	The reason is that FilterFoundry adjusts the zoom level if the available memory is low.
	PSPI sets maxSpace to 100 MB hardcoded. It is probably only a dummy value and doesn't
	resemble the actual space GIMP can assign to the plugin.
	In Photoshop, maxSpace is 825 MB.

	Workaround in FF 1.7: If GIMP/PSPI is detected, the adjustment of the zoom level is disabled.

6. Standalone filters don't work correctly

	Status: FIXED (Workaround) in FilterFoundry 1.7

	PSPI does not preserve the data of the EntryPoint argument "intptr_t *data".
	Therefore, the global data (gdata) object cannot preserved,
	and the standalone plugin doesn't "know" that it is a standalone plugin.
	Exactly the same problem applies to the IfanView 8BF host.
	=> The handling in checkandinitparams() has been changed to address this issue.

	Furthermore, the preserved "pb->parameters" are in a TEMP file which has
	the same name for each FilterFoundry plugin. Therefore, the "pb->parameters"
	are shared between main plugin and all standalone plugins.
	=> In FilterFoundry 1.7.0.4, the file name will contain a hash of the plugin
	   or 0 for the main plugin.
