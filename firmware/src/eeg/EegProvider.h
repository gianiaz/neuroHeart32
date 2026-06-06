#pragma once

#include <Arduino.h>

enum EegWaveBand {
  EEG_DELTA,
  EEG_THETA,
  EEG_LO_ALPHA,
  EEG_HI_ALPHA,
  EEG_LO_BETA,
  EEG_HI_BETA,
  EEG_LO_GAMMA,
  EEG_MID_GAMMA,
  EEG_WAVE_BAND_COUNT
};

struct EegSample {
  const char *activity;
  uint8_t poorSignal;
  uint8_t attention;
  uint8_t meditation;
  unsigned long uptimeMs;
  long waves[EEG_WAVE_BAND_COUNT];
};

class EegProvider {
public:
  virtual ~EegProvider() = default;

  virtual bool readSample(const char *activity, EegSample &sample) = 0;
};
