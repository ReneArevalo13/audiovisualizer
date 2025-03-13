/*
   Testing out how to read in a song and extract the data

   Will try decoding from a file
   want to extract: format, channel, samplerate

   Want to extract the raw PCM data.
*/


#include "miniaudio.c"
#include <stdio.h>


int main(int argc, char** argv) {

    ma_result result;
    ma_decoder decoder;
    ma_device_config deviceConfig;
    ma_device device;
    ma_uint64 framesRead;

    if (argc<2) {
        printf("No input file");
        return -1;

    }
    
    result = ma_decoder_init_file(argv[1], NULL, &decoder);
    if (result != MA_SUCCESS) {
        printf("Could not load file: %s\n", argv[1]);
        return -2;
    }
    // print values of decoded file
    printf("Output Format %d\n", decoder.outputFormat);
    printf("Output Channels %d\n", decoder.outputChannels);
    printf("Output Sample Rate %d\n", decoder.outputSampleRate);

    result = ma_data_source_read_pcm_frames(&decoder, pFrameOut,
            frameCount, &framesRead);
    printf("Frames Read: %d\n", framesRead);


    
    return 0;
}
