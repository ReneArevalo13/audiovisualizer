/*
    Implementation of COOLEY-TUKEY FFT
    Source: https://vanhunteradams.com/FFT/FFT.html#The-Cooley-Tukey-FFT
    Fixed point used, will be adapting to float for current purposes
    Note, that audio data only contains 'real' data, i.e. no complex 
    component. We will omit the part of the algorithm that has to do with 
    the complex part of the data.

*/
#include <stdio.h>
#include <math.h>

#define PI 3.14159265358979323846
#define NUM_SAMPLES 8          // number of gathered samples
#define NUM_SAMPLES_M_1 7      // number of samples -1
#define LOG2_NUM_SAMPLES 3     // log2 of samples gathered
#define SHIFT_AMOUNT 13        // length of short mine log2 of samples

float fr[NUM_SAMPLES] = {1, 2, 3, 4, 5, 6, 7, 8};   // real part of the samples
float Sinewave[NUM_SAMPLES];



void FFT_float(float fr[]) {
    unsigned short m;         // index to be swapped
    unsigned short mr;        // the other index being swapped (reverse)
    float tr;                 // temp storage

    int i, j;     // indices being combined in the Danielson-Lanczos part  of the algorithm
    int L;        // length of FFT being combined
    int k;        // used for looking up trig values from sine table
    int istep;    // length of FFT, comes from combining two FFTs

    float wr;     // trig value from lookup table
    float qr;     // temp variable for DL part of alg

    /* BIT REVERSAL */
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
    }

    /* DANIELSON_LASCZOS */
    // Length of FFTs being combined (starts at 1)
    L = 1;
    // LOG2 of the number of samples, minus 1
    k = LOG2_NUM_SAMPLES - 1;
    // while the length of FFTs being combined is less thant the number of samples
    while (L < NUM_SAMPLES) {
        // determine the length of the FFT which will result from combining two FFTs
        istep = L<<1;     // note that this left bit shift is essentially doubling
        // for each element in the FFT that are being combined...
        for (m=0; m<L; m++) {
            // Lookup the trig values for that element
            j = m << k;      // index of the sine table
            wr = Sinewave[j + NUM_SAMPLES/4];     // cos(2pi m/N)
           // wr >>= 1;        // divide by 2
            // i gets the index of one of the FFT elements being combined
            for (i=m; i<NUM_SAMPLES; i+=istep) {
                // j gets the index of the FFT element being combined with i
                j = i + L;
                // compute the trig terms
                tr = wr * fr[j];   // ignoring the imaginary part since it's 0
                // divide the ith index elements by 2
                qr = fr[i]/2;
                // compute the new values at each index
                fr[j] = qr - tr;
                fr[i] = qr + tr;
            }
        }
        --k;
        L = istep;
    }
}


int main() {
    for (int i=0; i<NUM_SAMPLES; i++) {
        Sinewave[i] = sin(2 * PI * (float)i / NUM_SAMPLES);
    }
    FFT_float(fr);
    int size = sizeof(fr)/sizeof(fr[0]);
    for (int i=0; i<size; i++) {
        printf(" %f\n", fr[i]);
    }
    return 0;
}
