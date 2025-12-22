#include "dependencies/miniaudio.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

// global vars to manage recording
typedef struct {
    ma_device device;
    ma_encoder encoder;
    bool isRecording;
    bool isInitialized;
} AudioRecorder;

void sleep_ms(long ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

static AudioRecorder g_recorder = {0};

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, uint32_t frameCount)
{
    AudioRecorder* pRecorder = (AudioRecorder*)pDevice->pUserData;
    
    if (pRecorder->isRecording && pInput != NULL) {
        ma_encoder_write_pcm_frames(&pRecorder->encoder, pInput, frameCount, NULL);
    }
    
    (void)pOutput;
}

ma_result start_recording(const char* outputFilePath, ma_format format, uint32_t channels, uint32_t sampleRate)
{
    ma_result result;
    
    if (g_recorder.isInitialized) {
        printf("Already initialized. Stop current recording first.\n");
        return MA_INVALID_OPERATION;
    }
    
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_capture);
    deviceConfig.capture.format   = format;
    deviceConfig.capture.channels = channels;
    deviceConfig.sampleRate       = sampleRate;
    deviceConfig.dataCallback     = data_callback;
    deviceConfig.pUserData        = &g_recorder;

    result = ma_device_init(NULL, &deviceConfig, &g_recorder.device);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize capture device: %d\n", result);
        return result;
    }
    
    ma_encoder_config encoderConfig = ma_encoder_config_init(
        ma_encoding_format_wav,
        format,
        channels,
        sampleRate
    );
    
    result = ma_encoder_init_file(outputFilePath, &encoderConfig, &g_recorder.encoder);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize encoder: %d\n", result);
        ma_device_uninit(&g_recorder.device);
        return result;
    }
    
    result = ma_device_start(&g_recorder.device);
    if (result != MA_SUCCESS) {
        printf("Failed to start device: %d\n", result);
        ma_encoder_uninit(&g_recorder.encoder);
        ma_device_uninit(&g_recorder.device);
        return result;
    }
    
    g_recorder.isRecording = MA_TRUE;
    g_recorder.isInitialized = MA_TRUE;
    
    printf("Recording started to: %s\n", outputFilePath);
    return MA_SUCCESS;
}

ma_result stop_recording()
{
    if (!g_recorder.isInitialized) {
        printf("No active recording to stop.\n");
        return MA_INVALID_OPERATION;
    }
    
    g_recorder.isRecording = MA_FALSE;
    
    ma_device_stop(&g_recorder.device);
    ma_device_uninit(&g_recorder.device);
    
    ma_encoder_uninit(&g_recorder.encoder);
    
    g_recorder.isInitialized = MA_FALSE;
    
    printf("Recording stopped and saved.\n");
    return MA_SUCCESS;
}

int main()
{
    ma_result result = start_recording(
        "recording.wav",
        ma_format_s16,
        2,
        44100
    );
    
    if (result != MA_SUCCESS) {
        return -1;
    }

    // test:
    
    printf("Recording for 5 seconds...\n");
//    ma_sleep(5000);
    sleep_ms(5000); 
    stop_recording();
    
    printf("Done! File saved as recording.wav\n");
    return 0;
}