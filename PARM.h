/*
http://visca.com/ffactory/archives/7-99/msg00011.html

Hi Dave,

Here's my reverse-engineered version of the Filter Factory PARM
resource block.  Is this what you need?  It seems to be good for
most known (PC) versions of FF.  You can use the Win32 FindResource
API to locate the PARM resource.  E-mail me privately if you need
more details.

Cheers,
-Alex Hunter
alex@afh.com
FilterMeister Developer 
*/

#define PARM_SIZE 8296
#define PARM_SIG 7272 // don't know WHERE this value comes from...but it's in valid FF resources

#ifdef Rez

type 'PARM' {
	longint = PARM_SIG; // cbSize;    //size of this structure
	longint FilterFactory,standaloneFilter; // nVersion;  //0=original FF, 1=standalone filter
	array[8] { longint; }; // val[8];    //initial values of controls
	longint noParameters,parametersDialog; // popDialog; //true if need to pop a parameter dialog
	longint; // unknown1;
	longint; // unknown2;
	longint; // unknown3;
	array[4] { longint; }; // map_used[4];   //true if map(n) is used
	array[8] { longint; }; // ctl_used[8];   //true if ctl(n) is used
	pstring[251];    //Category name
	// Michael Johannhanwahr's protect flag...
	longint notProtected,isProtected; // iProtected;            // == 1 means protected
	pstring[255];       //Filter title
	pstring[255];   //Copyright info
	pstring[255];      //Filter author(s)
	array[4] { pstring[255]; };      //4 map labels
	array[8] { pstring[255]; };      //8 control labels
	array[4] { cstring[1024]; }; //4 channel formulas
};

#else

/* N.B. under Windows, the strings are all C strings (!) */

typedef struct {   //structure of FF PARM resource
     long cbSize;    //size of this structure
     long nVersion;  //0=original FF, 1=standalone filter
     long val[8];    //initial values of controls
     long popDialog; //true if need to pop a parameter dialog
     long unknown1;
     long unknown2;
     long unknown3;
     long map_used[4];   //true if map(n) is used
     long ctl_used[8];   //true if ctl(n) is used
     unsigned char category[252];    //Category name
     // Michael Johannhanwahr's protect flag...
     long iProtected;            // == 1 means protected
     unsigned char title[256];       //Filter title
     unsigned char copyright[256];   //Copyright info
     unsigned char author[256];      //Filter author(s)
     unsigned char map[4][256];      //4 map labels
     unsigned char ctl[8][256];      //8 control labels
     char formula[4][1024]; //4 channel formulas
 } PARM_T;

#endif
