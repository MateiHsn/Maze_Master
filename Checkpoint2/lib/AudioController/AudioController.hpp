#ifndef AUDIO_CONTROLLER_HPP
#define AUDIO_CONTROLLER_HPP

#include <Defaults.hpp>

enum class SoundEffect {
  MENU_MOVE,
  MENU_SELECT,
  COLLECT_CUP,
  PLAYER_DEATH,
  LEVEL_COMPLETE,
  GAME_OVER,
  VICTORY,
  STARTUP
};

struct ToneSequence {
  uint32_t frequency;
  uint32_t duration;
};

class AudioController {
private:
  uint8_t buzzerPin;
  bool soundEnabled;

  bool isPlayingSequence;
  uint8_t currentToneIndex;
  uint8_t totalTones;
  uint32_t toneStartTime;
  uint32_t toneDuration;
  const ToneSequence * currentSequence;

  static const ToneSequence menuMoveSeq[1];
  static const ToneSequence menuSelectSeq[2];
  static const ToneSequence collectCupSeq[2];
  static const ToneSequence playerDeathSeq[3];
  static const ToneSequence levelCompleteSeq[4];
  static const ToneSequence gameOverSeq[3];
  static const ToneSequence victorySeq[4];
  static const ToneSequence startupSeq[3];

  void startSequence( const ToneSequence * sequence, uint8_t numTones);
  void playNextToneInSequence();

public:
  AudioController();

  void begin();
  void update();
  void playSound(SoundEffect effect);
  void stopSound();

  void enableSound();
  void disableSound();
  bool isSoundEnabled() const;
  bool isPlaying() const;
};

#endif