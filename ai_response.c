#include "miniaudio.h"

int main() {
    ma_context context;
    ma_result result = ma_context_init(NULL, 0, NULL, &context);
    if (result != MA_SUCCESS) {
        return 1;
    }

    ma_file_info fileInfo;
    result = ma_context_get_file_info(context, "example.wav", &fileInfo);
    if (result != MA_SUCCESS) {
        ma_context_uninit(&context);
        return 1;
    }

    ma_decoder_config decoderConfig = ma_decoder_config_init(fileInfo.format);
    decoderConfig.on_data = on_data;
    decoderConfig.on_seek = on_seek;

    ma_decoder decoder;
    result = ma_decoder_init(context, &decoderConfig, &decoder);
    if (result != MA_SUCCESS) {
        ma_context_uninit(&context);
        return 1;
    }

    ma_uint64 totalFrames = fileInfo.lengthInFrames;
    ma_uint32 sampleRate = fileInfo.sampleRate;
    ma_uint32 channels = fileInfo.channels;
    ma_uint32 bitsPerSample = fileInfo.bitsPerSample;

    printf("Total Frames: %llu\n", totalFrames);
    printf("Sample Rate: %u\n", sampleRate);
    printf("Channels: %u\n", channels);
    printf("Bits Per Sample: %u\n", bitsPerSample);

    void* pData = malloc(totalFrames * channels * (bitsPerSample / 8));
    if (pData == NULL) {
        ma_decoder_uninit(&decoder);
        ma_context_uninit(&context);
        return 1;
    }

    result = ma_decoder_start(&decoder);
    if (result != MA_SUCCESS) {
        free(pData);
        ma_decoder_uninit(&decoder);
        ma_context_uninit(&context);
        return 1;
    }

    while (ma_decoder_is_running(&decoder)) {
        ma_decoder_process(&decoder, NULL);
    }

    // pData now contains the raw audio data
    // You can process it as needed

    free(pData);
    ma_decoder_uninit(&decoder);
    ma_context_uninit(&context);

    return 0;
}

void on_data(ma_decoder* decoder, const void* pData, ma_uint32 frames) {
    // pData contains the raw audio data for the current frame
    // You can process it as needed
}

void on_seek(ma_decoder* decoder, ma_uint64 frame) {
    // The decoder has seeked to the specified frame
    // You can process it as needed
}

