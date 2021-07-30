// From: Ron_Hunsinger@bmug.org (Ron Hunsinger)

#define BITS 32
// BITS is the smallest even integer such that 2^BITS > ULONG_MAX.
// If you port this routine to a computer where 2^(BITS-1) > ULONG_MAX
// (Unisys A-series, for example, where integers would generally be
// stored in the 39-bit mantissa of a 48-bit word), unwind the first 
// iteration of the loop, and special-case references to 'half' in that 
// first iteration so as to avoid overflow.

unsigned long lsqrt (unsigned long n) {
    // Compute the integer square root of the integer argument n
    // Method is to divide n by x computing the quotient x and remainder r
    // Notice that the divisor x is changing as the quotient x changes

    // Instead of shifting the dividend/remainder left, we shift the
    // quotient/divisor right.  The binary point starts at the extreme
    // left, and shifts two bits at a time to the extreme right.

    // The residue contains n-x^2.  (Within these comments, the ^ operator
    // signifies exponentiation rather than exclusive or.  Also, the /
    // operator returns fractions, rather than truncating, so 1/4 means
    // one fourth, not zero.)

    // Since (x + 1/2)^2 == x^2 + x + 1/4,
    //   n - (x + 1/2)^2 == (n - x^2) - (x + 1/4)
    // Thus, we can increase x by 1/2 if we decrease (n-x^2) by (x+1/4)

    unsigned long residue;   // n - x^2
    unsigned long root;      // x + 1/4
    unsigned long half;      // 1/2

    residue = n;             // n - (x = 0)^2, with suitable alignment
    root = 1 << (BITS - 2);  // x + 1/4, shifted left BITS places
    half = root + root;      // 1/2, shifted left BITS places
    
    do {
        if (root <= residue) {   // Whenever we can,
            residue -= root;         // decrease (n-x^2) by (x+1/4)
            root += half; }          // increase x by 1/2
        half >>= 2;          // Shift binary point 2 places right
        root -= half;        // x{+1/2}+1/4 - 1/8 == x{+1/2}+1/8
        root >>= 1;          // 2x{+1}+1/4, shifted right 2 places
        } while (half);      // When 1/2 == 0, binary point is at far right
    
    // Omit the following line to truncate instead of rounding
    if (root < residue) ++root;
    
    return root;   // Guaranteed to be correctly rounded (or truncated)
    }
