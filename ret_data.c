/* 
   Retrieve the raw data from the song file.
*/

#include "miniaudio.c"
#include <stdio.h>









int main (int argc, char** argv) {
    ma_result result;
    ma_decoder decoder;
    ma_device_config deviceConfig;
    ma_device device;
    ma_uint64 totalFrames;
    ma_uint64 sampleRate;
    ma_uint64 channels;
    ma_uint32 bitsPerSample;
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
    format        = decoder.outputFormat;
    sampleRate    = decoder.outputSampleRate;
    channels      = decoder.outputChannels;
    bitsPerSample = ma_get_bytes_per_sample(format) * 8; 

    printf("Format: %d\n", format);
    printf("TotalFrames: %llu\n", totalFrames);
    printf("SampleRate: %llu\n", sampleRate);
    printf("Channels: %llu\n", channels);
    printf("BitsPerSample: %u\n", bitsPerSample);
    

    return 0;

}
