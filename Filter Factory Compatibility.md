Implementation detail differences
=================================

Filter Foundry tries to be as compatible with Filter Factory as possible.
However, there are some differences that are explained in this documentation.

Various implementations
-----------------------

In the source-code file funcs.c, some functions are implemented twice:
One instance is the default implementation (older Filter Foundry versions used),
and one instance is a 100% replica of the Filter Factory code, obtained
from the "OPER" resource.
(More information at https://misc.daniel-marschall.de/projects/filter_factory/res_oper.html )
If required, the compiler-definitions `use_filterfactory_implementation_*`
can be set or unset to select the implementation.

In Filter Foundry 1.7.0.8, the following functions have been updated to the Filter Factory replica:
- `rnd(x)`
- `cos(x)`
- `sin(x)`
- `tan(x)`
- `r2x(d,m)`
- `r2y(d,m)`
- `rad(d,m,z)`
- `c2d(x,y)`
- `c2m(x,y)`
- `sqr(x)`
- `d`
- `m`
- `M`


sqr(x)
------

Filter Factory:

	sqr(x)=x for x < 0
	
	Can be tested with the following expression:
	sqr(-20)+21 == 1

Filter Foundry (prior to 1.7.0.8):

	sqr(x)=0 for x < 0

Beginning with Filter Foundry 1.7.0.8, the behavior of Filter Factory was implemented.


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


I, U, V, imin, umin, vmin (Testcase iuv_minmax.afs)
-------------------------

In Filter Foundry 1.7.0.8, the previously undocumented variables I, U, V as well as imin, umin, vmin
have been changed to represent the actual results of the i,u,v variables:

    imax = 255 (stayed the same)
    umax = 55 (was 255 in Filter Factory)
    vmax = 78 (was 255 in Filter Factory)

    imin = 0 (stayed the same)
    umin = -55 (was 0 in Filter Factory)
    vmin = -78 (was 0 in Filter Factory)

It is questionable if `I` was meant to be a synonym of `imax`, or if `I` was meant to be `I := imax - imin`.
We have chosen the latter in Filter Foundry 1.7.0.9. Same thing with `U` and `V`.

Therefore:

    I := imax-imin = 255
    U := umax-umin = 110
    V := vmax-vmin = 156


dmin, D (Testcase d_minmax.afs)
-------

**The Filter Factory manual writes:**

0 corresponds to the 3 o'clock position
256 to the 6 o'clock position,
512 to the 9 o'clock position,
768 to the 12 o'clock position,
and 1024 to the full rotation back to the 3 o'clock position

But this does not match the Windows implementation of Filter Factory
(maybe it is true to the Mac implementation?)

**In the original Windows implementation we can observe:**

d=-512 is at 9 o'clock position
d=-256 is at 12 o'clock position
d=0 is at 3 o'clock position
d=256 is at 6 o'clock position
d=512 is the full rotation back to 3 o'clock position

Therefore, `dmin` has been changed from 0 to -512,
and `D`, `dmax` has been changed from 1024 to 512.


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


rnd(a,b) (Testcases rnd*)
--------

Filter Factory uses Donald E. Knuth's subtractive random number generator algorithm,
which has been published in "The Art of Computer Programming, volume 2: Seminumerical Algorithms".
Addison-Wesley, Reading, MA, second edition, 1981.

Beginning with Filter Foundry 1.7.0.8, the same PRNG was implemented,
so that the output of rnd(a,b) is exactly the same now.


rst(i) (Testcases rnd*.afs and rst_*.afs)
------

Filter Factory contains an undocumented function that sets the seed for the random number generator.

Filter Factory and Filter Foundry beginning with 1.7.0.8 accept a seed between 0 and 32767, inclusively.
If the argument is not within this range, the operation lowest 15 bits are taken.

There are many differences in the implementation between Filter Factory and Filter Foundry in regards rst(i):

**Filter Factory:**

If rst(i) is called in Filter Factory, an internal Seed-Variable is set.
It does NOT influence any calls of rnd(a,b), because a lookup-table needs to be built first.
The building of the lookup-table is probably done before the processing of the first pixel (x,y,z=0).
It is suspected that the call of rst(i) will take effect on the next calculation.
Due to a bug (or feature?), the random state is not reset to its initial state (0) before the
filter is applied. The preview image processing will modify the random state, and once the filter
is actually applied (pressing "OK"), the random state that was set in the preview picture, will be used.
This could be considered as a bug, but it is probably required, otherwise the call of rst(i)
(inside the preview calculation) won't affect the rnd(a,b) in the real run.
However, in a standalone filter without dialog/preview, there is no preview that could set
the internal seed, so the rnd(a,b) functions will always work using the default seed 0,
and only the subsequent calls will use the rst(i) of the previous call.

**Filter Foundry:**

In Filter Foundry, the function rnd(a,b) retrieves a random number in "realtime"; therefore, if the
seed is changed via rst(i), there is an immediate effect on the next call of the rnd(a,b) function.

For example, the following filter would generate an one-colored picture without any randomness:
        R: rst(123), rnd(0,255)
        G: rnd(0,255)
        B: rnd(0,255)

If you want to generate a random pixel image with a non-default seed, you need to make sure
that rst(i) is called only once at the beginning (channel 0, coordinate 0|0):
        R: (x== 0 && y ==0) ? rst(123) : 0, rnd(0,255)
        G: rnd(0,255)
        B: rnd(0,255)

In Filter Foundry, rst(i) can be called by branches and variables/sliders can
be used as arguments of rst(i).


Evaluation of conditional branches
----------------------------------

**Filter Foundry:**

Only the branches which will be chosen due to the conditional expression will be evaluated.

This means that the following filter would generate a black canvas: (Testcase conditional_eval_1.afs)

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

Each branch inside an if-then-else expression will be evaluated.
This means that the following filter would generate a green canvas: (Testcase conditional_eval_1.afs)

        R: 1==0 ? put(255,0) : 0
        G: get(0)
        B: 0

Also, all arguments of a boolean expression will be fully evaluated.
So, this will also generate a green canvas: (Testcase conditional_eval_2.afs)

        R: 1==0 && put(255,0) ? 0: 0
        G: get(0)
        B: 0

This will also generate a green canvas: (Testcase conditional_eval_3.afs)

        R: 1==1 || put(255,0) ? 0 : 0
        G: get(0)
        B: 0
