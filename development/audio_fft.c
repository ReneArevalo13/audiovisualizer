/* 
    Process PCM Frames and get them to plot via raylib

    WILL BE LIMITING OUR SCOPE TO MP3 FOR NOW
*/

#include "miniaudio.c"
#include "raylib.h"
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "fft/fft.h"
#include <string.h>

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])


/*
   MP3 Data represented as float32 in miniaudio
*/

const int ARRAY_SIZE = 512;
ma_uint32 global_frames[ARRAY_SIZE] = {0};
float fft_data[ARRAY_SIZE] = {0};
float mag[ARRAY_SIZE] = {0};
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

    const int screenWidth = 800;
    const int screenHeight = 800;
    const int N = 221;    // corresponds to 22.1 KHz being upper limit
    const int sh = screenHeight;  

    // Plot the data
    InitWindow(screenWidth, screenHeight, "FOURIER MAGNITUDES");
    SetTargetFPS(60);

    
    while(!WindowShouldClose()) {

                    //          ***   Cast PCM data to floats   ***          // 
    for (int i=0; i<ARRAY_SIZE; i++) {
            float s =  *(float*)&global_frames[i];   
            printf("raw data: %f\n", s);
            fft_data[i] = s;       
        }
    
                        //          ***   FFT PROCESSING   ***          // 
        fft(fft_data, mag);
                        //          ***   DRAWING ON SCREEN   ***          // 
        BeginDrawing();
            ClearBackground(BLACK);
            float cell_width = (float)screenWidth/N;
            for (int i=0; i<N; i++) {

                float sample = fft_data[i];
                
               // printf("mag: %f\n", mag[i]);
                float s_height = sample *100;  // scaled height
                DrawRectangle(i*cell_width, sh-s_height, cell_width, s_height, VIOLET);
             
       }
        EndDrawing();
    }
    CloseWindow();

    ma_decoder_uninit(&decoder);
    ma_device_uninit(&device);
    return 0;

}

// NOTE: Only need the magnitudes up to the 221st data point
