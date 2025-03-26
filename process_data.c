/* 
    Process PCM Frames and get them to plot via raylib

*/

#include "miniaudio.c"
#include "raylib.h"
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>



void* pData;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {

    ma_uint64 framesRead;
    ma_decoder *pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }
    ma_format format         = pDecoder->outputFormat;
    ma_uint32 bytesPerSample = ma_get_bytes_per_sample(format); 
    ma_uint64 channels       = pDecoder->outputChannels;

    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, &framesRead);

    memcpy(pData, pOutput,bytesPerSample*sizeof(format)*channels);

    // data type should be taken into account
    if (format == 5) {
        float frame = *(float*)pData;
        printf("%f ", frame);
    } else if (format == 3) {
        int32_t frame = *(int32_t*)pData;
        printf("%d ", frame);
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

    // RAYLIB STUFF
    const int screenWidth = 800;
    const int screenHeight = 500;
    const int N = 150;
    const int hh = screenHeight/2;  // half height

    
    if (argc < 2) {
        printf("No input file");
        return -1;
    }

    result = ma_decoder_init_file(argv[1], NULL, &decoder);
    if (result != MA_SUCCESS) {
        printf("Could not load file: %s\n", argv[1]);
        return -2;
    }

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

    pData = malloc(totalFrames * channels * bytesPerSample);
    printf("Memory allocated: %llu\n", (totalFrames*channels*bytesPerSample));
    if (pData == NULL) {
        printf("malloc failed\n");
        ma_decoder_uninit(&decoder);    
        return -3;
    }

    if(ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        ma_decoder_uninit(&decoder);
        return -4;
    }
    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_decoder_uninit(&decoder);
        return -5;
    }
    // Plot the data
    InitWindow(screenWidth, screenHeight, "PCM DATA");
    SetTargetFPS(60);
    srand(time(NULL));

    // random rects

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        float cell_width = (float)screenWidth/N;
        for (int i=0; i<N; i++) {
            float rand_height = (float)rand() / RAND_MAX; // 0.0-1.0
            float s_height = hh*rand_height;  // scaled height
            DrawRectangle(i*cell_width, hh-s_height, 5, s_height, GREEN);
        }
        EndDrawing();
    }

    CloseWindow();
    ma_decoder_uninit(&decoder);
    ma_device_uninit(&device);
    return 0;

}
