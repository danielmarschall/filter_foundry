

Implementation detail differences
=================================

FilterFoundry tries to be as compatible with Filter Factory as possible.
However, there are some differences which are explained in this documentation.

Various implementations
-----------------------

In the source-code file funcs.c, some functions are implemented twice:
One instance is the default implementation (older Filter Foundry versions used),
and one instance is a 100% replica of the Filter Factory code, obtained
from the "OPER" resource.
(More information at https://misc.daniel-marschall.de/projects/filter_factory/res_oper.html )
If required, the compiler-definitions "use_filterfactory_implementation_*"
can be set or unset to select the implementation.

In Filter Foundry 1.7.0.8, following functions have been updated to the Filter Factory replica:
- rnd
- cos
- sin
- tan
- r2x
- r2y
- rad
- c2d
- c2m
- sqr
- d
- m
- M


sqr(x)
------

Filter Factory:

	sqr(x)=x for x < 0
	
	Can be tested with following expression:
	sqr(-20)+21 == 1

Filter Foundry (prior to 1.7.0.8):

	sqr(x)=0 for x < 0

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

Filter Factory and FilterFoundry beginning with 1.7.0.8 accept a seed between 0 and 32767, inclusively.
If the argument is not within this range, the operation "and 0x7FFF" will be applied to it
to extract the low 15 bits.

There are many differences in the implementation between FilterFactory and FilterFoundry in regards rst(i):

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

For example, following filter would generate an one-colored picture without any randomness:
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
