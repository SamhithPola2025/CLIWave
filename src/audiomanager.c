#include "dependencies/miniaudio.h"

#include <stdio.h>
#include <stdbool.h>
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

typedef struct {
    ma_device device;
    ma_decoder decoder;
    bool isPlaying;
    bool isInitialized;
} AudioPlayer;

static AudioPlayer g_player = {0};

void playback_callback(ma_device* pDevice, void* pOutput, const void* pInput, uint32_t frameCount)
{
    (void)pDevice;
    (void)pInput;
    if (!g_player.isInitialized) return;
    ma_decoder_read_pcm_frames(&g_player.decoder, pOutput, frameCount, NULL);
}

ma_result stop_playback()
{
    if (!g_player.isInitialized) {
        return MA_INVALID_OPERATION;
    }

    g_player.isPlaying = false;

    ma_device_stop(&g_player.device);
    ma_device_uninit(&g_player.device);
    ma_decoder_uninit(&g_player.decoder);

    g_player.isInitialized = false;
    printf("Playback stopped.\n");
    return MA_SUCCESS;
}

ma_result start_playback(const char* inputFilePath)
{
    ma_result result;

    if (g_player.isInitialized) {
        stop_playback();
    }

    result = ma_decoder_init_file(inputFilePath, NULL, &g_player.decoder);
    if (result != MA_SUCCESS) {
        printf("Failed to init decoder: %d\n", result);
        return result;
    }

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = g_player.decoder.outputFormat;
    deviceConfig.playback.channels = g_player.decoder.outputChannels;
    deviceConfig.sampleRate        = g_player.decoder.outputSampleRate;
    deviceConfig.dataCallback      = playback_callback;
    deviceConfig.pUserData         = &g_player;

    result = ma_device_init(NULL, &deviceConfig, &g_player.device);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize playback device: %d\n", result);
        ma_decoder_uninit(&g_player.decoder);
        return result;
    }

    result = ma_device_start(&g_player.device);
    if (result != MA_SUCCESS) {
        printf("Failed to start playback device: %d\n", result);
        ma_device_uninit(&g_player.device);
        ma_decoder_uninit(&g_player.decoder);
        return result;
    }

    g_player.isPlaying = true;
    g_player.isInitialized = true;

    printf("Playback started: %s\n", inputFilePath);
    return MA_SUCCESS;
}
