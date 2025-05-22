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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"
#include <time.h>


//#define PI 3.14159265358979323846
#define NUM_SAMPLES 64          // number of gathered samples
#define NUM_SAMPLES_M_1 63      // number of samples -1
#define LOG2_NUM_SAMPLES 6     // log2 of samples gathered
#define SHIFT_AMOUNT 10     // length of short minus log2 of samples
#define NYQUIST NUM_SAMPLES/2 

float fr[NUM_SAMPLES] = {};   // real part of the samples
float fi[NUM_SAMPLES] = {};   // imaginary part of the samples
float mag[NUM_SAMPLES] = {};   // magnitude of fft data 
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

void magnitude(float fr[], float fi[], float mag[]) {
    // calculate magnitude of signal (Euclidean Norm)
    // with normalization
    for (int i=0; i<NUM_SAMPLES; i++) {
    // without normalization
     //   mag[i] = sqrt(fr[i]*fr[i] + fi[i]*fi[i]);
    // with normalization
        // zero frequency is not doubled 
        if (i == 0) {
            mag[i] = ((sqrt(fr[i]*fr[i] + fi[i]*fi[i])) / NUM_SAMPLES);
        }
        mag[i] = ((sqrt(fr[i]*fr[i] + fi[i]*fi[i])) / NUM_SAMPLES) * 2;
        printf("MAG[%d]: %f\n", i, mag[i]);
    }
}
/* generate varying sine waves */
void gen_data(float fr[]) {
    int rand1 = rand() % 10;
    int rand2 = rand() % 20;
    int rand3 = rand() % 13;

    for (int i=0; i<NUM_SAMPLES; i++) {
        float t = (float) i/NUM_SAMPLES;
        fr[i] =  sinf((2*PI)* t + rand1) + 30*sinf(rand1*PI*t + rand2) + sinf(3*PI*t + rand3);
        printf("data: %f\n", fr[i]);
    }

}
void clear_data(float fr[], float fi[], float mag[]) {
    for (int i=0; i<NUM_SAMPLES; i++) {
        fr[i] = 0;  
        fi[i] = 0;  
        mag[i] = 0;  
    }
}
int main() {
    srand(time(NULL));
    for (int i=0; i<NUM_SAMPLES; i++) {
        float t = (float) i/NUM_SAMPLES;
      //  printf("t = %f\n", t);
        Sinewave[i] = sinf(((2*PI)* ((float)i/NUM_SAMPLES))) ;
       // fr[i] = sinf((2*PI)* t + 3) + sinf(PI*t + 2) + sinf(3*PI*t + 1);
       // printf("data[%d]: %f\n", i, fr[i]);
    }

    

    // Raylib plotting
    const int screenWidth = 800;
    const int screenHeight = 800;
    const int sh = screenHeight;
    const int hh = screenHeight/2;
    const int N = NUM_SAMPLES;
  //  const int N = NYQUIST;
    InitWindow(screenWidth, screenHeight, "Frequency Domain data");

    SetTargetFPS(60);
    float cell_width = (float)screenWidth/N;

    while(!WindowShouldClose()){
        clear_data(fr,fi,mag);
        gen_data(fr);
        // FFT transform
        FFT_float(fr);
        magnitude(fr, fi, mag);
        BeginDrawing();

            ClearBackground(WHITE);
            for (int i=0; i < N; i++) {
                
                float s = fr[i]*1;
                if (s >= 0) {
                    float s_height = hh*s;
                DrawRectangle(i*cell_width, hh - s_height, cell_width, s_height, VIOLET);
                } else {
                    float s_height = -1.0*hh*s;
                DrawRectangle(i*cell_width, hh, cell_width, s_height, VIOLET);
                }

            }

        EndDrawing();
    }
    return 0;
}
