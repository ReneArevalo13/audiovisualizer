#define MA_NO_ENGINE        /* We're intentionally not using the ma_engine API here. */
#include "miniaudio.c"


void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_data_source_read_pcm_frames((ma_data_source*)pDevice->pUserData, pOutput, frameCount, NULL);

    (void)pInput;
}

int main(int argc, char** argv)
{
    ma_result result;
    ma_device_config deviceConfig;
    ma_device device;
    ma_resource_manager_config resourceManagerConfig;
    ma_resource_manager resourceManager;
    ma_resource_manager_data_source dataSource;

    if (argc < 2) {
        printf("No input file.");
        return -1;
    }


    /* We'll initialize the device first. */
    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.dataCallback = data_callback;
    deviceConfig.pUserData    = &dataSource;    /* <-- We'll be reading from this in the data callback. */

    result = ma_device_init(NULL, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize device.");
        return -1;
    }


    /*
    We have the device so now we want to initialize the resource manager. We'll use the resource manager to load a
    sound based on the command line.
    */
    resourceManagerConfig = ma_resource_manager_config_init();
    resourceManagerConfig.decodedFormat     = device.playback.format;
    resourceManagerConfig.decodedChannels   = device.playback.channels;
    resourceManagerConfig.decodedSampleRate = device.sampleRate;

    /*
    We're not supporting threading with Emscripten so go ahead and disable threading. It's important
    that we set the appropriate flag and also the job thread count to 0.
    */
#ifdef __EMSCRIPTEN__
    resourceManagerConfig.flags |= MA_RESOURCE_MANAGER_FLAG_NO_THREADING;
    resourceManagerConfig.jobThreadCount = 0;
#endif

    result = ma_resource_manager_init(&resourceManagerConfig, &resourceManager);
    if (result != MA_SUCCESS) {
        ma_device_uninit(&device);
        printf("Failed to initialize the resource manager.");
        return -1;
    }

    /* Now that we have a resource manager we can load a sound. */
    result = ma_resource_manager_data_source_init(
        &resourceManager,
        argv[1],
        MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE | MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC | MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_STREAM,
        NULL,   /* Async notification. */
        &dataSource);
    if (result != MA_SUCCESS) {
        printf("Failed to load sound \"%s\".", argv[1]);
        return -1;
    }

    /* In this example we'll enable looping. */
    ma_data_source_set_looping(&dataSource, MA_TRUE);


    /* Now that we have a sound we can start the device. */
    result = ma_device_start(&device);
    if (result != MA_SUCCESS) {
        ma_device_uninit(&device);
        printf("Failed to start device.");
        return -1;
    }

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(main_loop__em, &resourceManager, 0, 1);
#else
    printf("Press Enter to quit...\n");
    getchar();
#endif

    /* Teardown. */

    /* Uninitialize the device first to ensure the data callback is stopped and doesn't try to access any data. */
    ma_device_uninit(&device);

    /*
    Before uninitializing the resource manager we need to uninitialize every data source. The data source is owned by
    the caller which means you're responsible for uninitializing it.
    */
    ma_resource_manager_data_source_uninit(&dataSource);

    /* Uninitialize the resource manager after each data source. */
    ma_resource_manager_uninit(&resourceManager);

    return 0;
}
