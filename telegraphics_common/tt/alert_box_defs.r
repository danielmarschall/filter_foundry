/* from HIN 010 */
// The first set of definitions are not actual coordinates, but instead are intermediate values used to derive them:

#define A                    13    // white space between most elements
#define B                    23    // white space to left and right of icon
#define NumTextLines          3    // number of lines of text in the alert
#define LineHeight           16    // height of a single line of Chicago-12
#define ButtonHeight         20    // standard button height
#define LongestButtonName    41    // width of “Cancel” in Chicago-12
#define ButtonWidth          59    // (LongestButtonName + 18)

// The rest of the definitions are actual coordinates defining the window size
// (AlertWidth and AlertHeight) and the icon, text, and button locations:

#define AlertWidth          341    // chosen to make the right margin = A

#define IconLeft             20    // (B - 3)
#define IconRight            52    // (IconLeft + 32)
#define IconTop              10    // (A - 3)
#define IconBottom           42    // (IconTop + 32)

#define TextLeft             74    // (IconRight + (B - 1))
#define TextRight           331    // (AlertWidth - (A - 3))
#define TextTop               7    // (A - 6)
#define TextBottom           55    // (TextTop + (NumTextLines * LineHeight))

#define ButtonTop            68    // (TextBottom + A)
#define ButtonBottom         88    // (ButtonTop + ButtonHeight)
#define ActionButtonRight   331    // (AlertWidth - (A - 3))
#define ActionButtonLeft    272    // (ActionButtonRight - ButtonWidth)
#define CancelButtonRight   259    // (ActionButtonLeft - A)
#define CancelButtonLeft    200    // (CancelButtonRight - ButtonWidth)

#define AlertHeight          98    // (ButtonBottom + (A - 3))
