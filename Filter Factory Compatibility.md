
Implementation detail differences
=================================

FilterFoundry tries to be as compatible with Filter Factory as possible.
However, results are usually not 100% equal because functions like
cos, sin, sqr, etc., have different accuracy due to the underlying
implementation.

Furthermore, there are the following known differences between
Filter Foundry and Filter Factory:

i, u, v (Testcase iuv.afs)
-------

Filter Foundry <1.7 uses the same formulas as in Filter Factory:

    i=((76*r)+(150*g)+(29*b))/256            // Output range is 0..254
    u=((-19*r)+(-37*g)+(56*b))/256           // Output range is -55..55
    v=((78*r)+(-65*g)+(-13*b))/256           // Output range is -77..77

Filter Foundry 1.7 uses more accurate formulas:

    i=(299*r+587*g+114*b)/1000               // Output range is 0..255
    u=(-147407*r-289391*g+436798*b)/2000000  // Output range is -55..55
    v=614777*r-514799*g-99978*b)/2000000     // Output range is -78..78

Both formulas follow the same YUV standard but have different accuracy.


get(i) (Testcase getput.afs)
------

Filter Foundry:

    get(x)=0 if x>255 or x<0

Filter Factory:

    get(x)=x if x>255 or x<0

Note: The result "x" was most likely not intended but a result of an undefined behavior


r, g, b at empty canvas (Testcase emptycanvas.afs)
-----------------------

In Filter Factory, an empty (transparent) canvas of a new file is initialized as `r=g=b=0`

Filter Foundry initializes it as `r=g=b=255`


rnd(a,b) and rst(i) (Testcases rst_*.afs)
-------------------

Filter Foundry's implementation of rst(i) (an undocumented function that sets the seed for the PRG)
and rnd(a,b) (generate a random number between a and b, inclusively)
differs from the implementation of Filter Factory in many ways.

0. Beginning with Filter Foundry 1.7.0.8, the same compiler-independant algorithm like
   in Filter Factory is used.

1. In Filter Factory, the random seed is automatically initialized with seed 0.   
   Filter Foundry 1.7.0.8 does the same.

2. In Filter Factory, the argument i must be between 0 and 32767, inclusively.
   If the argument is not within this range, the operation "and 0x7FFF" will be applied to it
   to extract the low 15 bits.
   Filter Foundry 1.7.0.8 does the same.

3. In Filter Foundry, the function rnd(a,b) retrieves a random number in realtime; therefore, if the
   seed is changed via rst(i), there is an immediate effect on the next call of the rnd(a,b) function.
   For example, following filter would generate an one-colored picture without any randomness:
        R: rst(123), rnd(0,255)
        G: rnd(0,255)
        B: rnd(0,255)

   If you want to generate a random pixel image with a non-default seed, you need to make sure
   that rst(i) is called only once at the beginning (channel 0, coordinate 0|0):
        R: (x==0 && y==0) ? rst(123) : 0, rnd(0,255)
        G: rnd(0,255)
        B: rnd(0,255)

   In Filter Factory, the rnd(a,b) function is more complex.
   As soon as the function rnd(a,b) is used once, rst(i) will not have any effect.
   So, if you want to use rst(i), you must make sure to call it before using rnd(a,b).
   Following filter will generate a random pixel picture:
        R: rst(123), rnd(0,255)
        G: rnd(0,255)
        B: rnd(0,255)
   Following filter would generate a different random pixel picture:
        R: rst(456), rnd(0,255)
        G: rnd(0,255)
        B: rnd(0,255)
   Following filter would generate the same random pixel picture:
        R: rst(456), rnd(0,255)+rst(123)  // note that rst() always returns 0, so the '+' operator is OK
        G: rnd(0,255)
        B: rnd(0,255)+rst(456)  // the last rst(456) call is to mitigate a bug; see below.

4. In Filter Factory, due to a bug, the random state is not reset to its initial state (0) before the filter is applied:
   The preview image processing will modify the random state, and once the filter is actually applied (pressing "OK"),
   the random state that was set in the preview picture, will be used.
   Example:
        R: rnd(0,255), rst(123)   // note that the rst(123) is ignored because rnd() was already called.
        G: rnd(0,255)
        B: rnd(0,255)
   This filter will produce a random pixel picture with the initial default random seed 0,
   but only for the first calculation (i.e. in the preview picture processing, or in a standalone filter without dialog).
   Any further calculation will result in a random pixel picture with random seed 123,
   since the random seed 123 will be taken from the previous run.

   Furthermore, the random state can't be changed again, not even at the beginning of the red channel before any rnd() call.
   Example:
        R: rst(333), rnd(0,255)
        G: rnd(0,255)
        B: rnd(0,255)+rst(555)
   This filter will produce a picture with random seed 333 on the first calculation,
   and at every further calculation, a random picture with seed 555.
   Another example:
        R: rst(ctl(0)), rnd(0,255)
        G: rnd(0,255)
        B: rnd(0,255)+rst(555)
   In this filter, the slider value is ignored and the resulting picture will always be the same.


Evaluation of conditional branches
----------------------------------

**Filter Foundry:**

Only the branches which will be chosen due to the conditional expression will be evaluated.

This means that following filter would generate a black canvas: (Testcase conditional_eval_1.afs)

        R: 1==0 ? put(255,0) : 0
        G: get(0)
        B: 0

In boolean expressions, the evaluation will be aborted if the result is already determined.

So, this will also generate a black canvas: (Testcase conditional_eval_2.afs)

        R: 1==0 && put(255,0) ? 0: 0
        G: get(0)
        B: 0

This will also generate a black canvas: (Testcase conditional_eval_3.afs)

        R: 1==1 || put(255,0) ? 0 : 0
        G: get(0)
        B: 0

**Filter Factory:**

Each branch inside a if-then-else expression will be evaluated.
This means that following filter would generate a green canvas: (Testcase conditional_eval_1.afs)

        R: 1==0 ? put(255,0) : 0
        G: get(0)
        B: 0

Also, all arguments of an boolean expression will be fully evaluated.
So, this will also generate a green canvas: (Testcase conditional_eval_2.afs)

        R: 1==0 && put(255,0) ? 0: 0
        G: get(0)
        B: 0

This will also generate a green canvas: (Testcase conditional_eval_3.afs)

        R: 1==1 || put(255,0) ? 0 : 0
        G: get(0)
        B: 0
