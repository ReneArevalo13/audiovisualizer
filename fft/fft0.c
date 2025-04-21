// Learning FFT 
// https://vanhunteradams.com/FFT/FFT.html#The-Cooley-Tukey-FFT

#include <limits.h>
#include <stdio.h>
#define NUM_SAMPLES        8 // number of gatherd samples (power of 2)
#define NUM_SAMPLES_M_1    7 // number of gathered samples minus 1
#define LOG2_NUM_SAMPLES   3 // log2 of the number of gathered samples
#define SHIFT_AMOUNT     13  // length of short (16bits) minus log2 of number of samples

float fr[NUM_SAMPLES] = {9.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};     // array of the real part of the sample
float fi[NUM_SAMPLES];     // array of the imaginary part of the sample


void bitReversalPermutation() {
    unsigned short m;    // one of the indices being swapped
    unsigned short mr;   // the other index being swapped (r for reversed)
    int tr, ti;               // temporary storage while swapping

    // code below based on stanford graphics lab
    // https://graphics.stanford.edu/~seander/bithacks.html#BitReverseObvious
    for (m=1; m<NUM_SAMPLES_M_1; m++) {
       // printf("Initial Index: %hu\n", m);

        // swap odd and even bits
        mr =  ((m >> 1) & 0x5555) | ((m & 0x5555) << 1);
        // swap consecutive pairs
        mr = ((mr >> 2) & 0x3333) | ((mr & 0x3333) << 2);
        // swap nibbles ...
        mr = ((mr >> 4) & 0x0F0F) | ((mr & 0x0F0F) << 4);
        // swap bytes
        mr = ((mr >> 8) & 0x00FF) | ((mr & 0x00FF) << 8);
        // shift down mr
        mr >>= SHIFT_AMOUNT;
        // don't swap that which has already been swapped
        if (mr <= m) continue;
        // swap the bit-reversed indices
        tr = fr[m];
        fr[m] = fr[mr];
        fr[mr] = tr;

       // ti = fi[m];
       // fi[m] = fi[mr];
       // fr[mr] = ti;
       // printf("SWAPPED Index: %hu\n", mr);

   }
}


void reverseBits(unsigned int v) {

    unsigned int r = v;  // r will be reversed bits of v; first get LSB of v
    int s = sizeof(v) * CHAR_BIT - 1;  // extra shift needed at the end

    for (v >>= 1; v; v>>=1) {
        r <<= 1;
        r |= v & 1;
        s--;
    }
    r <<= s; // shift when v's highest bits are 0
}

int main() {
    unsigned int v = 11;
    //reverseBits(v);
    bitReversalPermutation();
    int size = sizeof(fr)/sizeof(fr[0]);
    for (int i=0; i<size; i++) {
        printf("%f\n", fr[i]);
    }
    return 0;
}
