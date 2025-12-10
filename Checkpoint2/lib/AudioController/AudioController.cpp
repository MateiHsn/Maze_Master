#include <AudioController.hpp>

const ToneSequence AudioController::menuMoveSeq[1] = {
  {800, 50}
};

const ToneSequence AudioController::menuSelectSeq[2] = {
  {1200, 100},
  {1500, 150}
};

const ToneSequence AudioController::collectCupSeq[2] = {
  {1000, 80},
  {1200, 80}
};

const ToneSequence AudioController::playerDeathSeq[3] = {
  {800, 150},
  {600, 150},
  {400, 300}
};

const ToneSequence AudioController::levelCompleteSeq[4] = {
  {1000, 100},
  {1200, 100},
  {1500, 100},
  {2000, 200}
};

const ToneSequence AudioController::gameOverSeq[3] = {
  {600, 200},
  {500, 200},
  {400, 400}
};

const ToneSequence AudioController::victorySeq[4] = {
  {1500, 100},
  {1800, 100},
  {2100, 100},
  {2500, 300}
};

const ToneSequence AudioController::startupSeq[3] = {
  {1000, 100},
  {1500, 100},
  {2000, 150}
};

AudioController::AudioController() {
  buzzerPin = BUZZER_PIN;
  soundEnabled = true;
  isPlayingSequence = false;
  currentToneIndex = 0;
  totalTones = 0;
  toneStartTime = 0;
  toneDuration = 0;
  currentSequence = nullptr;
}

void AudioController::begin() {
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
}

void AudioController::update() {
  if(!soundEnabled || !isPlayingSequence) return;

  uint32_t currentTime = millis();

  if(currentTime - toneStartTime >= toneDuration) {
    noTone(buzzerPin);
    currentToneIndex++;

    if(currentToneIndex >= totalTones) {
      isPlayingSequence = false;
      currentSequence = nullptr;
      return;
    }
    playNextToneInSequence();
  }
}

void AudioController::startSequence(const ToneSequence * sequence, uint8_t numTones) {
  if(!soundEnabled) return;

  currentSequence = sequence;
  totalTones = numTones;
  currentToneIndex = 0;
  isPlayingSequence = true;

  playNextToneInSequence();
}

void AudioController::playNextToneInSequence() {
  if(currentToneIndex >= totalTones) return;

  const ToneSequence & currentTone = currentSequence[currentToneIndex];
  tone(buzzerPin, currentTone.frequency, currentTone.duration);
  toneStartTime = millis();
  toneDuration = currentTone.duration;
}

void AudioController::playSound(SoundEffect effect) {
  if(!soundEnabled) return;

  stopSound();

  switch(effect) {
    case SoundEffect::STARTUP:
      startSequence(startupSeq, 3);
    break;
    case SoundEffect::MENU_MOVE:
      startSequence(menuMoveSeq, 1);
    break;
    case SoundEffect::MENU_SELECT:
      startSequence(menuSelectSeq, 2);
    break;
    case SoundEffect::COLLECT_CUP:
      startSequence(collectCupSeq, 2);
    break;
    case SoundEffect::PLAYER_DEATH:
      startSequence(playerDeathSeq, 3);
    break;
    case SoundEffect::LEVEL_COMPLETE:
      startSequence(levelCompleteSeq, 4);
    break;
    case SoundEffect::GAME_OVER:
      startSequence(gameOverSeq, 3);
    break;
    case SoundEffect::VICTORY:
      startSequence(victorySeq, 4);
    break;
  }
}

void AudioController::stopSound() {
  isPlayingSequence = false;
  currentToneIndex = 0;
  currentSequence = nullptr;
  noTone(buzzerPin);
}

void AudioController::enableSound() {
  soundEnabled = true;
}

void AudioController::disableSound() {
  soundEnabled = false;
  stopSound();
}

bool AudioController::isSoundEnabled() const {
  return soundEnabled;
}

bool AudioController::isPlaying() const {
  return isPlayingSequence;
}

