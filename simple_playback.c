/*
   Load a sound file and play it back using the low-level api.

   using ma_decoder api to load a sound and play it back.
*/

#include "miniaudio.c"
#include <stdio.h>

void data_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount) {
    //printf("Invoking data callback\n");
    ma_uint64 framesRead;
    ma_decoder *pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return ;
    }

    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, &framesRead);
   // printf("%d\t", &pOutput);
       // printf("Framecount: %u/n", frameCount);
   // (void)pInput;
}

int main(int argc, char** argv) {
    ma_result result;
    ma_decoder decoder;
    ma_device_config deviceConfig;
    ma_device device;
    char input;

    if (argc < 2) {
        printf("No input file");
        return -1;
    }
    result = ma_decoder_init_file(argv[1], NULL, &decoder);
    if (result != MA_SUCCESS) {
        printf("Could not load file: %s\n", argv[1]);
        return -2;
    }

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format     = decoder.outputFormat;
    deviceConfig.playback.channels   = decoder.outputChannels;
    deviceConfig.sampleRate          = decoder.outputSampleRate;
    deviceConfig.dataCallback        = data_callback;
    deviceConfig.pUserData           = &decoder;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        ma_decoder_uninit(&decoder);
        return -3;
    } 
    printf("Starting Device\n");
    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_decoder_uninit(&decoder);
        return -4;
    }

    printf("SAMPLE RATE: %u\n", deviceConfig.sampleRate);
    printf("LETS PARTY\n");
    printf("Press Enter to quit ...");
    getchar();
    printf("Quitting program\n");
    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);
    return 0; 
}
