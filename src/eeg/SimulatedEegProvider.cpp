#include "SimulatedEegProvider.h"

SimulatedEegProvider::SimulatedEegProvider(const SimulatedEegProviderConfig &config)
    : _attention(config.initialAttention),
      _meditation(config.initialMeditation) {
  for (uint8_t i = 0; i < EEG_WAVE_BAND_COUNT; i++) {
    _waves[i] = config.initialWaves[i];
  }
}

bool SimulatedEegProvider::readSample(const char *activity, EegSample &sample) {
  int targetAttention;
  int targetMeditation;
  chooseActivityTargets(activity, targetAttention, targetMeditation);

  if (_attention < targetAttention) {
    _attention += 2;
  } else if (_attention > targetAttention) {
    _attention -= 2;
  }

  if (_meditation < targetMeditation) {
    _meditation += 2;
  } else if (_meditation > targetMeditation) {
    _meditation -= 2;
  }

  _attention = constrain(_attention, 0, 100);
  _meditation = constrain(_meditation, 0, 100);

  sample.activity = activity;
  sample.poorSignal = 0;
  sample.attention = _attention;
  sample.meditation = _meditation;
  sample.uptimeMs = millis();

  for (uint8_t i = 0; i < EEG_WAVE_BAND_COUNT; i++) {
    sample.waves[i] = simulatedWaveForBand(activity, i);
  }

  return true;
}

void SimulatedEegProvider::chooseActivityTargets(const char *activity, int &targetAttention, int &targetMeditation) {
  if (strcmp(activity, "meditazione") == 0) {
    targetAttention = random(20, 50);
    targetMeditation = random(70, 95);
  } else if (strcmp(activity, "lettura") == 0) {
    targetAttention = random(65, 85);
    targetMeditation = random(40, 60);
  } else if (strcmp(activity, "videogame") == 0) {
    targetAttention = random(80, 100);
    targetMeditation = random(20, 45);
  } else {
    targetAttention = random(40, 60);
    targetMeditation = random(40, 60);
  }
}

long SimulatedEegProvider::simulatedWaveForBand(const char *activity, uint8_t bandIndex) {
  const float multiplier = random(95, 106) / 100.0;
  _waves[bandIndex] *= multiplier;
  _waves[bandIndex] = constrain(_waves[bandIndex], 1000.0, 500000.0);

  float activityBoost = 1.0;
  if (strcmp(activity, "meditazione") == 0 && (bandIndex == EEG_LO_ALPHA || bandIndex == EEG_HI_ALPHA)) {
    activityBoost = 1.4;
  } else if (strcmp(activity, "lettura") == 0 && (bandIndex == EEG_LO_BETA || bandIndex == EEG_HI_BETA)) {
    activityBoost = 1.2;
  } else if (strcmp(activity, "videogame") == 0 &&
             (bandIndex == EEG_LO_BETA || bandIndex == EEG_HI_BETA || bandIndex == EEG_LO_GAMMA)) {
    activityBoost = 1.35;
  }

  return (long)(_waves[bandIndex] * activityBoost);
}
