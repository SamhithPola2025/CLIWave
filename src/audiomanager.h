#pragma once
#include "dependencies/miniaudio.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

ma_result start_recording(const char* outputFilePath, ma_format format, uint32_t channels, uint32_t sampleRate);
ma_result stop_recording();

#ifdef __cplusplus
}
#endif
