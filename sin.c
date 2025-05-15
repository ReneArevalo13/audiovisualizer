#define MA_NO_DECODING
#define MA_NO_ENCODING
#include "miniaudio.c"

#include <stdio.h>
#include "raylib.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>

void main_loop__em()
{
}
#endif

#define DEVICE_FORMAT       ma_format_f32
#define DEVICE_CHANNELS     2
#define DEVICE_SAMPLE_RATE  48000

ma_uint32 data[441] = {0};


void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_waveform* pSineWave;

    MA_ASSERT(pDevice->playback.channels == DEVICE_CHANNELS);

    pSineWave = (ma_waveform*)pDevice->pUserData;
    MA_ASSERT(pSineWave != NULL);

    ma_waveform_read_pcm_frames(pSineWave, pOutput, frameCount, NULL);

    memcpy(data, pOutput, sizeof(ma_format_f32)*frameCount);
    (void)pInput;   /* Unused. */
}

int main(int argc, char** argv)
{
    ma_waveform sineWave;
    ma_device_config deviceConfig;
    ma_device device;
    ma_waveform_config sineWaveConfig;

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = DEVICE_FORMAT;
    deviceConfig.playback.channels = DEVICE_CHANNELS;
    deviceConfig.sampleRate        = DEVICE_SAMPLE_RATE;
    deviceConfig.dataCallback      = data_callback;
    deviceConfig.pUserData         = &sineWave;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        return -4;
    }

    printf("Device Name: %s\n", device.playback.name);

    sineWaveConfig = ma_waveform_config_init(device.playback.format, device.playback.channels, device.sampleRate, ma_waveform_type_square, 0.2, 220);
    ma_waveform_init(&sineWaveConfig, &sineWave);

    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        return -5;
    }
    
    // RAYLIB STUFF
    const int screenWidth = 800;
    const int screenHeight = 800;
    const int N = 441;
    const int hh = screenHeight/2;  // half height

    // Plot the data
    InitWindow(screenWidth, screenHeight, "SIN DATA");
    SetTargetFPS(60);

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        float cell_width = (float)screenWidth/N;
        for (int i=0; i<N; i++) {

            float sample = *(float*)&data[i];
            if (sample >= 0) {
                float s_height = hh*sample;  // scaled height
                DrawRectangle(i*cell_width, hh-s_height, cell_width, s_height, VIOLET);
            } else {
                float s_height = -1.0*hh*sample;  // scaled height
                DrawRectangle(i*cell_width, hh, cell_width, s_height, SKYBLUE);
            }
       }
        EndDrawing();
    }
    CloseWindow();

    ma_device_uninit(&device);
    
    (void)argc;
    (void)argv;
    return 0;
}
