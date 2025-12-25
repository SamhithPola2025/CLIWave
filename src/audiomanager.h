#pragma once
#include "dependencies/miniaudio.h"
#include <stdint.h>

#ifdef __cplusplus
// to ensure C compatability
extern "C" {
#endif

ma_result start_recording(const char* outputFilePath, ma_format format, uint32_t channels, uint32_t sampleRate);
ma_result stop_recording();

ma_result start_playback(const char* inputFilePath);
ma_result stop_playback();

#ifdef __cplusplus
}
#endif

