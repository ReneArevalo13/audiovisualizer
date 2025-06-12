/* 
   Validating fft with sine data
 
*/


// NOTE THIS VERSION IS WORKING AS INTENDED: WILL REFINE 

#include "raylib.h"
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
//#include "fft/fft.h"
#include <string.h>
#include <math.h>

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])

//#define PI 3.14159265358979323846
#define NUM_SAMPLES 512 // number of gathered samples
#define NUM_SAMPLES_M_1 511     // number of samples -1
#define LOG2_NUM_SAMPLES 9      // log2 of samples gathered
#define SHIFT_AMOUNT 7          // length of short (16) minus log2 of samples


float fr[NUM_SAMPLES] = {};   // real part of the samples
float fi[NUM_SAMPLES] = {};   // imaginary part of the samples
float mag[NUM_SAMPLES] = {};   // magnitude of fft data 
float Sinewave[NUM_SAMPLES];

void magnitude(float fr[], float fi[], float mag[]) {
    for (int i=0; i<NUM_SAMPLES; i++) {
        if (i == 0) {
            mag[i] = sqrt(fr[i]*fr[i] + fi[i]*fi[i])/NUM_SAMPLES;
        }
        mag[i] = sqrt(fr[i]*fr[i] + fi[i]*fi[i])/NUM_SAMPLES * 2;

       // printf("fr[%d]:%f fi[%d]: %f\n", i, fr[i], i, fi[i]);
        //printf("MAG[%d]: %f\n", i, mag[i]);
    }
}

void clear(float fr[], float fi[], float mag[]) {
    memset(fr, 0, NUM_SAMPLES*sizeof(fr[0]));
    memset(fi, 0, NUM_SAMPLES*sizeof(fi[0]));
    memset(mag, 0, NUM_SAMPLES*sizeof(mag[0]));
}

void fft(float fr[], float fi[], float mag[]) {
    unsigned short m;         // index to be swapped
    unsigned short mr;        // the other index being swapped (reverse)
    float tr, ti;                 // temp storage

    int i, j;     // indices being combined in the Danielson-Lanczos part  of the algorithm
    int L;        // length of FFT being combined
    int k;        // used for looking up trig values from sine table
    int istep;    // length of FFT, comes from combining two FFTs

    float wr, wi;     // trig value from lookup table
    float qr, qi;     // temp variable for DL part of alg

    
                       /*   CAST 2 FLOAT   */
   // __cast2Float(data);

                       /*   BIT REVERSAL   */
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

                       /*   DANIELSON_LASCZOS   */

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
            wr =  Sinewave[j + NUM_SAMPLES/4];     // cos(2pi m/N) 
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

int main (int argc, char** argv) {
                                  //          *** RayLib Setup ***          // 

    const int screenWidth = 800;
    const int screenHeight = 700;
    const int N = 512;    
    const int Nyquist = N/2;    
    const int sh = screenHeight;  
    const int sw = screenWidth;  
    const int interval = sw/16;
    const int f_size = 13;
    // Plot the data
    InitWindow(screenWidth, screenHeight, "FOURIER MAGNITUDES");
    SetTargetFPS(60);

    
    while(!WindowShouldClose()) {
        clear(fr, fi, mag);

                   //          ***   Cast PCM data to floats   ***          // 
    for (int i=0; i<NUM_SAMPLES; i++) {
        float t = (float) i/NUM_SAMPLES;
        Sinewave[i] = sinf(((2*PI)* ((float)i/NUM_SAMPLES))) ;
        fr[i] = sinf((2*PI)* t * 210 ) + 3*sinf((2*PI)* t * 54 ) + 1.3*sinf((2*PI)* t * 139);
    }

                            //          ***   FFT PROCESSING   ***          // 
        fft(fr, fi, mag);
        magnitude(fr, fi, mag);
                        //          ***   DRAWING ON SCREEN   ***          // 
        BeginDrawing();
        ClearBackground(BLACK);
            float cell_width = (float)screenWidth/Nyquist;
            for (int i=0; i<Nyquist; i++) {
                float sample = mag[i];
               // printf("mag: %f\n", mag[i]);
                float s_height = sample * 200;  // scaled height
                DrawRectangle(i*cell_width, sh-s_height, cell_width, s_height - 30, VIOLET);
                DrawText("16hz",1*interval-35 ,sh-f_size , f_size , WHITE);
                DrawText("32hz",2*interval-35 ,sh-f_size , f_size , WHITE);
                DrawText("48hz",3*interval-35 ,sh-f_size , f_size , WHITE);
                DrawText("64hz",4*interval-35 ,sh-f_size , f_size , WHITE);
                DrawText("80hz",5*interval-35 ,sh-f_size , f_size , WHITE);
                DrawText("96hz",6*interval-35 ,sh-f_size , f_size , WHITE);
                DrawText("112hz",7*interval-35 , sh-f_size,f_size , WHITE);
                DrawText("128hz",8*interval-35 , sh-f_size,f_size , WHITE);
                DrawText("144hz",9*interval-35 , sh-f_size,f_size , WHITE);
                DrawText("160hz",10*interval-38 , sh-f_size ,f_size , WHITE);
                DrawText("176hz",11*interval-38 , sh-f_size ,f_size , WHITE);
                DrawText("192hz",12*interval-38 , sh-f_size ,f_size , WHITE);
                DrawText("208hz",13*interval-38 , sh-f_size ,f_size , WHITE);
                DrawText("224hz",14*interval-38 , sh-f_size ,f_size , WHITE);
                DrawText("240hz",15*interval-38 , sh-f_size ,f_size , WHITE);
                DrawText("256hz",16*interval-38 , sh-f_size ,f_size , WHITE);

                DrawLine(0, sh-30, sw, sh-30, WHITE);
                DrawLine(interval, 0, interval, sh, WHITE);
                DrawLine(2*interval, 0, 2*interval, sh, WHITE);
                DrawLine(3*interval, 0, 3*interval, sh, WHITE);
                DrawLine(4*interval, 0, 4*interval, sh, WHITE);
                DrawLine(5*interval, 0, 5*interval, sh, WHITE);
                DrawLine(6*interval, 0, 6*interval, sh, WHITE);
                DrawLine(7*interval, 0, 7*interval, sh, WHITE);
                DrawLine(8*interval, 0, 8*interval, sh, WHITE);
                DrawLine(9*interval, 0, 9*interval, sh, WHITE);
                DrawLine(10*interval, 0, 10*interval, sh, WHITE);
                DrawLine(11*interval, 0, 11*interval, sh, WHITE);
                DrawLine(12*interval, 0, 12*interval, sh, WHITE);
                DrawLine(13*interval, 0, 13*interval, sh, WHITE);
                DrawLine(14*interval, 0, 14*interval, sh, WHITE);
                DrawLine(15*interval, 0, 15*interval, sh, WHITE);
                DrawLine(16*interval, 0, 16*interval, sh, WHITE);
             
       }
        EndDrawing();
    }
    CloseWindow();
    return 0;

}

// NOTE: Only need the magnitudes up to the 221st data point
