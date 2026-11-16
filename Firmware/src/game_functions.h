#ifndef GAME_FUNCTIONS_H
#define GAME_FUNCTIONS_H

#include <lvgl.h>


#ifdef __cplusplus
extern "C" {
#endif

// UI event handler function declarations
void funcCalibrateZero(lv_event_t * e);
void funcKnownSet(lv_event_t * e);
void fctStart(lv_event_t * e);
void updateCurrentScreen();

extern uint8_t playerCount;
extern uint8_t selectedGameMode;


enum GameType {
  GAME_DARTS,
  GAME_TARGET_WEIGHT,
  GAME_NONE
};

enum DartsState {
  DARTS_PLAYER_SPLASH,
  DARTS_WAITING,
  DARTS_TARE,
  DARTS_PRE_SCORING,
  DARTS_SCORING,
  DARTS_NEXT_PLAYER,
  DARTS_CALC_SCORES,
  DARTS_SHOW_WINNER
};

enum TargetState {
  TARGET_PLAYER_SPLASH,
  TARGET_SETTINGS,
  TARGET_WAITING,
  TARGET_TARE,
  TARGET_PRE_SCORING,
  TARGET_SCORING,
  TARGET_NEXT_PLAYER,
  TARGET_CALC_SCORES,
  TARGET_SHOW_WINNER
};

#ifdef __cplusplus
}
#endif



#endif // GAME_FUNCTIONS_H