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
#define SHIFT_AMOUNT 13        // length of short minus log2 of samples

float fr[NUM_SAMPLES] = {};   // real part of the samples
float fi[NUM_SAMPLES] = {};   // imaginary part of the samples
float mag[NUM_SAMPLES] = {};   // magnitude of signal 
float Sinewave[NUM_SAMPLES];

void FFT_float(float fr[]) {
    unsigned short m;         // index to be swapped
    unsigned short mr;        // the other index being swapped (reverse)
    float tr, ti;                 // temp storage

    int i, j;     // indices being combined in the Danielson-Lanczos part  of the algorithm
    int L;        // length of FFT being combined
    int k;        // used for looking up trig values from sine table
    int istep;    // length of FFT, comes from combining two FFTs

    float wr, wi;     // trig value from lookup table
    float qr, qi;     // temp variable for DL part of alg

    /* BIT REVERSAL */
    // code below based on stanford graphics lab
    // https://graphics.stanford.edu/~seander/bithacks.html#BitReverseObvious
    for (m=1; m<NUM_SAMPLES_M_1; m++) {

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
        ti = fi[m];
        fi[m] = fi[mr];
        fi[mr] = ti;
    }

    // DANIELSON_LASCZOS 
    // Length of FFTs being combined (starts at 1)
    L = 1;
    // LOG2 of the number of samples, minus 1
    k = LOG2_NUM_SAMPLES - 1;
    // while the length of FFTs being combined is less thant the number of samples
        while (L < NUM_SAMPLES) {
        // determine the length of the FFT which will result from combining two FFTs
        
        istep = L<<1;             // note that this left bit shift is doubling
        // for each element in the FFT that are being combined...
        for (m=0; m<L; ++m) {
            // Lookup the trig values for that element
            j = m << k;            // index of the sine table
            wr = Sinewave[j + NUM_SAMPLES/4];     // cos(2pi m/N) 
            wi = -Sinewave[j];     // cos(2pi m/N) 
            // i gets the index of one of the FFT elements being combined
            for (i=m; i<NUM_SAMPLES; i+=istep) {
                // j gets the index of the FFT element being combined with i
                j = i + L;
                // compute the trig terms
                tr = (wr * fr[j]) - (wi * fi[j]);   
                ti = (wr * fi[j]) + (wi * fr[j]);   
                // divide the ith index elements by 2
                qr = fr[i];
                qi = fi[i];
                // compute the new values at each index
                fr[j] = qr - tr;
                fr[i] = qr + tr;
                fi[j] = qi - ti;
                fi[i] = qi + ti;
            }
        }
        --k;
        L = istep;
    }
    
}


int main() {
    for (int i=0; i<NUM_SAMPLES; i++) {
        float t = (float) i/NUM_SAMPLES;
      //  printf("t = %f\n", t);
        Sinewave[i] = sinf(((2*PI)* ((float)i/NUM_SAMPLES))) ;
        fr[i] = sinf((2*PI)* t * 2 ) + sinf((2*PI)* t * 3 );
       // printf("data[%d]: %f\n", i, fr[i]);
    }


    /*
    printf("PRIOR TO FFT\n");
    for (int i=0; i<NUM_SAMPLES; i++) {
       printf("%f\n", fr[i]);
    }

    */

    FFT_float(fr);
    for (int i=0; i<NUM_SAMPLES; i++) {
        printf("FFT DATA[%d]: %f + %fi\n", i, fr[i], fi[i]);
        mag[i] = sqrt(fr[i]*fr[i] + fi[i]*fi[i]);
        printf("Magintude [%d]: %f\n\n", i, mag[i]);
    }
    return 0;
}
