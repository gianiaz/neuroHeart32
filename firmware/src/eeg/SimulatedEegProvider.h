#pragma once

#include "EegProvider.h"

struct SimulatedEegProviderConfig {
  uint8_t initialAttention;
  uint8_t initialMeditation;
  float initialWaves[EEG_WAVE_BAND_COUNT];
};

class SimulatedEegProvider : public EegProvider {
public:
  explicit SimulatedEegProvider(const SimulatedEegProviderConfig &config);

  bool readSample(const char *activity, EegSample &sample) override;

private:
  void chooseActivityTargets(const char *activity, int &targetAttention, int &targetMeditation);
  long simulatedWaveForBand(const char *activity, uint8_t bandIndex);

  uint8_t _attention;
  uint8_t _meditation;
  float _waves[EEG_WAVE_BAND_COUNT];
};
