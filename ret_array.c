/* 
   Retrieve the raw data from the song file.
   put data into an array

*/

#include "miniaudio.c"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#define ARRAY_LEN(xs)   sizeof(xs)/sizeof(xs[0])

int callbacks = 0;
float data[1000] = {};
size_t count = 0;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
   // printf("datacallback call %d", callbacks);
    ma_uint64 framesRead;
    ma_decoder *pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }
    ma_format format         = pDecoder->outputFormat;
    ma_uint32 bytesPerSample = ma_get_bytes_per_sample(format); 
    ma_uint64 channels       = pDecoder->outputChannels;

   // printf("\nFRAMECOUNT %d\n", frameCount);
    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, &framesRead);

    memcpy(data+callbacks, pOutput, sizeof(format)*frameCount);
    count = (size_t)frameCount;

    callbacks++;
   
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
   

    printf("Press Enter to quit...\n");
   
    
    getchar();
    ma_decoder_uninit(&decoder);
    ma_device_uninit(&device);

    printf("\ncallbacks %d\n", callbacks);

    for (size_t i=0; i<callbacks; ++i) {
        float sample = *(float*)&data[i];
        printf("%f  ", sample);
    }

    /*
    ma_decoder_uninit(&decoder);
    ma_device_uninit(&device);
    return 0;
    */

    return 0;
   
}
