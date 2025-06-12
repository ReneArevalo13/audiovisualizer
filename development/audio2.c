/* 
    Process PCM Frames and get them to plot via raylib

    WILL BE LIMITING OUR SCOPE TO MP3 FOR NOW
*/


// NOTE THIS VERSION IS WORKING AS INTENDED: WILL REFINE 

#include "miniaudio.c"
#include "raylib.h"
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
//#include "fft/fft.h"
#include <string.h>

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])

//#define PI 3.14159265358979323846
#define NUM_SAMPLES 512         // number of gathered samples
#define NUM_SAMPLES_M_1 511     // number of samples -1
#define LOG2_NUM_SAMPLES 9      // log2 of samples gathered
#define SHIFT_AMOUNT 7          // length of short (16) minus log2 of samples

/*
   MP3 Data represented as float32 in miniaudio
*/
ma_uint32 global_frames[NUM_SAMPLES] = {0};

float fr[NUM_SAMPLES] = {};   // real part of the samples
float fi[NUM_SAMPLES] = {};   // imaginary part of the samples
float mag[NUM_SAMPLES] = {};   // magnitude of fft data 
float Sinewave[NUM_SAMPLES];


size_t count = 0;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {

    ma_uint64 framesRead;
    ma_decoder *pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }
    ma_format format         = pDecoder->outputFormat;
    ma_uint32 bytesPerSample = ma_get_bytes_per_sample(format); 
    ma_uint64 channels       = pDecoder->outputChannels;

    // read data
    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, &framesRead);


    // copying data
    memcpy(global_frames, pOutput, sizeof(format)*frameCount);
    count += (size_t)frameCount;
}

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

    ma_result result;
    ma_decoder decoder;
    ma_decoder_config decoderConfig;
    ma_device device;
    ma_device_config deviceConfig;
    ma_uint64 totalFrames;
    ma_uint64 sampleRate;
    ma_uint64 channels;
    ma_uint32 bytesPerSample;
    ma_format format;

    
               //          *** ERROR CHECKING ***          // 
    if (argc < 2) {
        printf("No input file");
        return -1;
    }

    result = ma_decoder_init_file(argv[1], NULL, &decoder);
    if (result != MA_SUCCESS) {
        printf("Could not load file: %s\n", argv[1]);
        return -2;
    }

               //          *** MiniAudio Info ***          // 
    result = ma_decoder_get_length_in_pcm_frames(&decoder, &totalFrames);
    format         = decoder.outputFormat;
    sampleRate     = decoder.outputSampleRate;
    channels       = decoder.outputChannels;
    bytesPerSample = ma_get_bytes_per_sample(format); 

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format     = format;
    deviceConfig.playback.channels   = channels;
    deviceConfig.sampleRate          = sampleRate;
    deviceConfig.dataCallback        = data_callback;
    deviceConfig.pUserData           = &decoder;


    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        ma_decoder_uninit(&decoder);
        return -4;
    }
    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_decoder_uninit(&decoder);
        return -5;
    }

                            //          *** RayLib Setup ***          // 

    const int screenWidth = 1105;
    const int screenHeight = 850;
    const int N = 221;    // corresponds to 22.1 KHz being upper limit
    const int sh = screenHeight;  
    // Plot the data
    InitWindow(screenWidth, screenHeight, "FOURIER MAGNITUDES");
    SetTargetFPS(60);

    
    while(!WindowShouldClose()) {
        clear(fr, fi, mag);

                    //          ***   Cast PCM data to floats   ***          // 
    for (int i=0; i<NUM_SAMPLES; i++) {
            float s =  *(float*)&global_frames[i]; 
            Sinewave[i] = sinf(((2*PI)* ((float)i/NUM_SAMPLES))) ;
            fr[i] = s;       
          //  printf("raw data: %f\n", fr[i]);
        }
    
                        //          ***   FFT PROCESSING   ***          // 
        fft(fr, fi, mag);
        magnitude(fr, fi, mag);
                        //          ***   DRAWING ON SCREEN   ***          // 
        BeginDrawing();
            ClearBackground(BLACK);
            float cell_width = (float)screenWidth/N;
            for (int i=0; i<N; i++) {
                float sample = mag[i];
               // printf("mag: %f\n", mag[i]);
                float s_height = sample *5500;  // scaled height
                DrawRectangle(i*cell_width, sh-s_height, cell_width, s_height - 30, VIOLET);
                DrawText("100 HZ", 20, 830, 20, WHITE);
                DrawLine(20, 0, 20, s_height, BLUE);
             
       }
        EndDrawing();
    }
    CloseWindow();

    ma_decoder_uninit(&decoder);
    ma_device_uninit(&device);
    return 0;

}

// NOTE: Only need the magnitudes up to the 221st data point
