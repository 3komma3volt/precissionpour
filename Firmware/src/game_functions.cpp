#include "game_functions.h"
#include "ui.h"
#include <Arduino.h>
#include <EEPROM.h>
#include "ui_functions.h"
#include "sounds.h"

#include <HX711_ADC.h>
extern HX711_ADC LoadCell;
extern QueueHandle_t weightQueue;

uint8_t selectedGameMode = 0;
uint8_t playerCount = 2;
uint8_t activeGameState = DARTS_PLAYER_SPLASH;

unsigned long now;

uint16_t playerDartsScores[] = {301, 301, 301, 301, 301, 301, 301, 301}; // Max 8 players
uint16_t playerTareValues[] = {0, 0, 0, 0, 0, 0, 0, 0};
uint16_t playerScores[8];
uint8_t currentPlayerIndex = 0;
uint16_t currentPlayerScore = 0;
uint8_t gameRounds = 0;
uint8_t targetMin = 0;
uint8_t targetMax = 0;
uint8_t currentRound = 1;
uint8_t currentTarget = 0;
uint16_t currentMalus = 0;

        uint32_t dartDebugTime;
            
            uint8_t dartDebug;

lv_obj_t* table = NULL;

// Use C linkage for LVGL event handler functions
extern "C"
{

    void funcCalibrateZero(lv_event_t *e)
    {
            playTouchSound();
        Serial.println("Zero calibration button clicked!");
        LoadCell.tare();
        lv_img_set_src(ui_imgZero, &ui_img_tick_png);
    }

    void funcKnownSet(lv_event_t *e)
    {
            playTouchSound();
        float known_mass = (float)lv_spinbox_get_value(ui_spinCalibWeight);
        LoadCell.setCalFactor(1.0);
        Serial.println(known_mass);
        LoadCell.refreshDataSet();
        float newCalibrationValue = LoadCell.getNewCalibration(known_mass);
        Serial.print("New calibration value has been set to: ");
        Serial.println(newCalibrationValue);
        EEPROM.writeFloat(0, newCalibrationValue);
        EEPROM.commit();
        lv_img_set_src(ui_imgWeight, &ui_img_tick_png);
    }

    void fctTargetControl(lv_event_t *e)
    {
            playTouchSound();
        switch (activeGameState)
        {
        case TARGET_WAITING:
            activeGameState = TARGET_TARE;
            lv_obj_clear_flag(ui_imgResultTarget, LV_OBJ_FLAG_HIDDEN);
            lv_img_set_src(ui_imgResultTarget, &ui_img_clock_png);
          //  lv_obj_add_flag(ui_lblResultTextTarget, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(ui_btnControlTarget, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(ui_panInfoTarget, LV_OBJ_FLAG_HIDDEN);
            now = millis();
            break;
        case TARGET_PRE_SCORING:
            now = millis();
            lv_obj_clear_flag(ui_imgResultTarget, LV_OBJ_FLAG_HIDDEN);
            lv_img_set_src(ui_imgResultTarget, &ui_img_clock_png);
        
            lv_obj_add_flag(ui_btnControlTarget, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(ui_panInfoTarget, LV_OBJ_FLAG_HIDDEN);
            activeGameState = TARGET_SCORING;
            break;
        case TARGET_NEXT_PLAYER:
            currentPlayerIndex++;
            if (currentPlayerIndex >= playerCount)
            {
                currentPlayerIndex = 0;
                currentRound++;
                lv_label_set_text_fmt(ui_lblTargetRound, "Round: %d", currentRound);
            }
            lv_label_set_text_fmt(ui_lblPlayerTarget, "Player: %d", currentPlayerIndex + 1);
            lv_label_set_text_fmt(ui_lblMalus, "Malus: %d", playerScores[currentPlayerIndex]);
            lv_label_set_text(ui_lblResultTextTarget, "Weight");
            lv_label_set_text(ui_lblBtnControlTarget, "Weight");
            lv_obj_add_flag(ui_imgResultTarget, LV_OBJ_FLAG_HIDDEN);
            activeGameState = TARGET_PLAYER_SPLASH;
            now = millis();
        break;
        case TARGET_SHOW_WINNER:
                lv_scr_load(ui_screenTargetWinner);
                
                table = lv_table_create(ui_panScore);
                lv_table_set_col_cnt(table, 2);
                lv_table_set_row_cnt(table, playerCount + 1);
                lv_obj_set_height(table, 128);
                lv_obj_center(table);
                lv_obj_set_scroll_dir(table, LV_DIR_VER);
                lv_obj_add_flag(table, LV_OBJ_FLAG_SCROLLABLE);
                lv_obj_clear_flag(table, LV_OBJ_FLAG_SCROLL_ELASTIC);
                // Set header
                //lv_table_set_cell_value(table, 0, 0, "Player");
                //lv_table_set_cell_value(table, 0, 1, "Malus");

                struct PlayerScore {
                    uint8_t playerNumber;
                    uint16_t score;
                } sortedPlayers[8];
                
                for (uint8_t i = 0; i < playerCount; i++)
                {
                    sortedPlayers[i].playerNumber = i + 1;  // Player numbers 1-based
                    sortedPlayers[i].score = playerScores[i];
                }
                for (uint8_t i = 0; i < playerCount - 1; i++)
                {
                    for (uint8_t j = 0; j < playerCount - i - 1; j++)
                    {
                        if (sortedPlayers[j].score > sortedPlayers[j + 1].score)
                        {
                            // Swap both player number and score
                            struct PlayerScore temp = sortedPlayers[j];
                            sortedPlayers[j] = sortedPlayers[j + 1];
                            sortedPlayers[j + 1] = temp;
                        }
                    }
                }
                                
                for (uint8_t i = 0; i < playerCount; i++)
                {
                    lv_table_set_cell_value_fmt(table, i, 0, "Player %d", sortedPlayers[i].playerNumber);
                    lv_table_set_cell_value_fmt(table, i, 1, "%d", sortedPlayers[i].score);
                }
        break;
        }
    }

    void fctStartTarget(lv_event_t *e)
    {

        for (uint8_t i = 0; i < 8; i++)
        {
            playerScores[i] = 0;
        }
        gameRounds = (uint8_t)lv_slider_get_value(ui_sliderRounds);
        Serial.print("Number of rounds selected: ");
        Serial.println(gameRounds);
        targetMax = (uint8_t)lv_slider_get_value(ui_sliderAmount);
        targetMin = (uint8_t)lv_slider_get_left_value(ui_sliderAmount);
        activeGameState = TARGET_PLAYER_SPLASH;
        currentRound = 1;
        currentPlayerIndex = 0;

        lv_scr_load(ui_screenTarget);
        lv_label_set_text_fmt(ui_lblTargetRound, "Round: %d", currentRound);
        lv_label_set_text_fmt(ui_lblPlayerTarget, "Player: %d", currentPlayerIndex + 1);
        lv_label_set_text_fmt(ui_lblMalus, "Malus: 0");
        lv_obj_add_flag(ui_imgResultTarget, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(ui_lblResultTextTarget, "Weight");
        now = millis();
    }

    void fctAmountChanged(lv_event_t *e)
    {
        uint8_t maxVal = (uint8_t)lv_slider_get_value(ui_sliderAmount);
        uint8_t minVal = (uint8_t)lv_slider_get_left_value(ui_sliderAmount);
        lv_label_set_text_fmt(ui_lblAmountMax, "%.0f", (float)maxVal);
        lv_label_set_text_fmt(ui_lblAmountMin, "%.0f", (float)minVal);
    }

    void fctStart(lv_event_t *e)
    {
        Serial.println("Start button clicked!");
        playerCount = (uint8_t)lv_slider_get_value(ui_sliderPlayerCnt);
        Serial.print("Number of players selected: ");
        Serial.println(playerCount);
        selectedGameMode = (uint8_t)lv_roller_get_selected(ui_rollerGame);
        Serial.print("Game mode selected: ");
        Serial.println(selectedGameMode);

        switch (selectedGameMode)
        {
        case GAME_TARGET_WEIGHT:
            playerCount = (uint8_t)lv_slider_get_value(ui_sliderPlayerCnt);
            lv_scr_load(ui_screenTargetSettings);

            break;
        case GAME_DARTS:
            Serial.println("Darts game selected.");
            lv_scr_load(ui_screenDarts);
            for (uint8_t i = 0; i < 8; i++)
            {
                playerDartsScores[i] = 301; // Reset scores to 301
                playerScores[i] = 0;
                playerTareValues[i] = 0;
            }
            currentPlayerIndex = 0;
            lv_label_set_text_fmt(ui_lblScore, "%d", playerDartsScores[currentPlayerIndex]);
            lv_label_set_text_fmt(ui_lblPlayer, "Player %d", currentPlayerIndex + 1);
            lv_label_set_text(ui_lblResultText, "Weight");
            lv_obj_add_flag(ui_imgResult, LV_OBJ_FLAG_HIDDEN);
            activeGameState = DARTS_PLAYER_SPLASH;
            now = millis();

            break;
        default:
            Serial.println("Unknown game selected.");
            break;
        }
    }

    void fctScrollTableUp(lv_event_t *e)
    {
            playTouchSound();
        if (table != NULL)
        {
            lv_coord_t current_scroll = lv_obj_get_scroll_y(table);
            
            if (current_scroll >= 30)
            {
                lv_obj_scroll_by(table, 0, 30, LV_ANIM_ON); 
            }
        }
    }

    void fctScrollTableDown(lv_event_t *e)
    {
            playTouchSound();
        if (table != NULL)
        {
            lv_coord_t current_scroll = lv_obj_get_scroll_y(table);
            lv_coord_t max_scroll = lv_obj_get_scroll_bottom(table);
            
               if (current_scroll < (max_scroll + (30 * (playerCount + 1))))
            {
                lv_obj_scroll_by(table, 0, -30, LV_ANIM_ON); 
            }
        }
    }
    
    void fctDartsControl(lv_event_t *e)
    {
            playTouchSound();
        Serial.println("Darts control button clicked!");
        switch (activeGameState)
        {

        case DARTS_WAITING:

            activeGameState = DARTS_TARE;
            lv_obj_clear_flag(ui_imgResult, LV_OBJ_FLAG_HIDDEN);
            lv_img_set_src(ui_imgResult, &ui_img_clock_png);
            lv_obj_add_flag(ui_lblResultText, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(ui_btnControl, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(ui_panInfoDarts, LV_OBJ_FLAG_HIDDEN);
            now = millis();

            break;
        case DARTS_PRE_SCORING:
            now = millis();
            lv_obj_clear_flag(ui_imgResult, LV_OBJ_FLAG_HIDDEN);
            lv_img_set_src(ui_imgResult, &ui_img_clock_png);
            lv_obj_add_flag(ui_lblResultText, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(ui_btnControl, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(ui_panInfoDarts, LV_OBJ_FLAG_HIDDEN);
            activeGameState = DARTS_SCORING;
            break;

        case DARTS_NEXT_PLAYER:
            currentPlayerIndex++;
            if (currentPlayerIndex >= playerCount)
            {
                currentPlayerIndex = 0;
            }
            lv_label_set_text_fmt(ui_lblScore, "%d", playerDartsScores[currentPlayerIndex]);
            lv_label_set_text_fmt(ui_lblPlayer, "Player %d", currentPlayerIndex + 1);
            lv_label_set_text(ui_lblResultText, "Weight");
            lv_label_set_text(ui_lblBtnControl, "Weight");

            activeGameState = DARTS_PLAYER_SPLASH;
            now = millis();
            break;
        }
    }

} // extern "C"

void updateCurrentScreen()
{
    lv_obj_t *current_screen = lv_scr_act();
    float weight;

    unsigned long static lastDartsUpdate = 0;

    if (current_screen == ui_screenSettings)
    {
        // Update weight display on settings screen
        if (xQueueReceive(weightQueue, &weight, 0) == pdTRUE)
        {
            lv_label_set_text_fmt(ui_lblScaleValue, "%.2f", weight);
        }
    }
    else if (current_screen == ui_screenDarts)
    {
        switch (activeGameState)
        {
        case DARTS_PLAYER_SPLASH:
            showDartArrows(100); // Hide arrows
            lv_label_set_text_fmt(ui_lblPanPlayerOverlay, "Get Ready\nPlayer %d", currentPlayerIndex + 1);
            lv_obj_clear_flag(ui_panPlayerOverlay, LV_OBJ_FLAG_HIDDEN);
            if (millis() - now > 2000)
            {
                lv_obj_add_flag(ui_panPlayerOverlay, LV_OBJ_FLAG_HIDDEN);
                activeGameState = DARTS_WAITING;
            }
    
            dartDebugTime = millis();
            break;
        case DARTS_WAITING:
            lv_obj_add_flag(ui_imgResult, LV_OBJ_FLAG_HIDDEN);
            if (xQueueReceive(weightQueue, &weight, 0) == pdTRUE)
            {
                if (weight < 0.0)
                {
                    weight = 0.0;
                }
                lv_label_set_text_fmt(ui_lblResult, "%.0f", weight);
            }

            break;

        case DARTS_TARE:
            if (xQueueReceive(weightQueue, &weight, 0) == pdTRUE)
            {
                if (weight < 0.0)
                {
                    weight = 0.0;
                }
                lv_label_set_text_fmt(ui_lblResult, "%.0f", weight);
            }
            if (millis() - now > 3000)
            {
                lv_label_set_text(ui_lblBtnControl, "Drink & Weight");
                lv_obj_clear_flag(ui_lblResultText, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(ui_btnControl, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(ui_panInfoDarts, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(ui_imgResult, LV_OBJ_FLAG_HIDDEN);
                playerTareValues[currentPlayerIndex] = round(weight);
                activeGameState = DARTS_PRE_SCORING;
            }
            break;
        case DARTS_SCORING:

            // Todo: Add some waiting
            if (xQueueReceive(weightQueue, &weight, 0) == pdTRUE)
            {
                if (weight < 0.0)
                {
                    weight = 0.0;
                }
                lv_label_set_text_fmt(ui_lblResult, "%.0f", weight);
            }
            if (millis() - now > 3000)
            {
                if (playerTareValues[currentPlayerIndex] < round(weight))
                {
                    currentPlayerScore = 0;
                }
                else
                {
                    currentPlayerScore = playerTareValues[currentPlayerIndex] - (uint16_t)round(weight);
                }

                lv_obj_clear_flag(ui_imgResult, LV_OBJ_FLAG_HIDDEN);
                if (currentPlayerScore > 60 || currentPlayerScore == 0 || currentPlayerScore > playerDartsScores[currentPlayerIndex])
                {
                    lv_img_set_src(ui_imgResult, &ui_img_cross_png);
                    showDartArrows(0);
                    playFailSound();
                }
                else
                {
                    lv_img_set_src(ui_imgResult, &ui_img_tick_png);
                    playerDartsScores[currentPlayerIndex] -= currentPlayerScore;
                    showDartArrows(currentPlayerScore);
                    playOKSound();
                }

                // playerDartsScores[currentPlayerIndex] = 0; // DEBUG
                

                lv_label_set_text_fmt(ui_lblResult, "%d", currentPlayerScore);
                lv_label_set_text_fmt(ui_lblScore, "%d", playerDartsScores[currentPlayerIndex]);
                lv_obj_clear_flag(ui_lblResultText, LV_OBJ_FLAG_HIDDEN);
                lv_label_set_text(ui_lblResultText, "Score");
                lv_obj_clear_flag(ui_btnControl, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(ui_panInfoDarts, LV_OBJ_FLAG_HIDDEN);
                if (playerDartsScores[currentPlayerIndex] == 0)
                {
                    // lv_label_set_text(ui_lblBtnControl, "Game Won");
                    lv_scr_load(ui_screenWinner);

                    lv_label_set_text_fmt(ui_lblWinner, "Player %d", currentPlayerIndex + 1);

                    activeGameState = DARTS_SHOW_WINNER;
                }
                else
                {
                    lv_label_set_text(ui_lblBtnControl, "Next Player");
                    activeGameState = DARTS_NEXT_PLAYER;
                }
            }
            break;
        case DARTS_SHOW_WINNER:

            break;
        }
    }
    else if (current_screen == ui_screenTarget)
    {
        switch (activeGameState)
        {
        case TARGET_PLAYER_SPLASH:
            lv_label_set_text_fmt(ui_lblPanPlayerOverlayTarget, "Get Ready\nPlayer %d", currentPlayerIndex + 1);
            lv_obj_clear_flag(ui_panPlayerOverlayTarget, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(ui_panInfoTarget, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(ui_lblBtnControlTarget, "Weight");
            if (millis() - now > 2000)
            {
                lv_obj_add_flag(ui_panPlayerOverlayTarget, LV_OBJ_FLAG_HIDDEN);
                
                activeGameState = TARGET_WAITING;
            }
            currentTarget = random(targetMin, targetMax + 1);
            lv_label_set_text_fmt(ui_lblTarget, "Target: %d", currentTarget);

            break;
        case TARGET_WAITING:
            lv_obj_add_flag(ui_imgResultTarget, LV_OBJ_FLAG_HIDDEN);
            if (xQueueReceive(weightQueue, &weight, 0) == pdTRUE)
            {
                if (weight < 0.0)
                {
                    weight = 0.0;
                }
                lv_label_set_text_fmt(ui_lblResultTextTarget, "Weight: %.0f", weight);
            }

            break;
        case TARGET_TARE:
            if (xQueueReceive(weightQueue, &weight, 0) == pdTRUE)
            {
                if (weight < 0.0)
                {
                    weight = 0.0;
                }
                lv_label_set_text_fmt(ui_lblResultTextTarget, "Weight: %.0f", weight);
            }
            if (millis() - now > 3000)
            {
                lv_label_set_text(ui_lblBtnControlTarget, "Drink & Weight");
                lv_obj_clear_flag(ui_lblResultTextTarget, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(ui_btnControlTarget, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(ui_panInfoTarget, LV_OBJ_FLAG_HIDDEN);

                lv_obj_add_flag(ui_imgResultTarget, LV_OBJ_FLAG_HIDDEN);
                playerTareValues[currentPlayerIndex] = round(weight);
                activeGameState = TARGET_PRE_SCORING;
            }
            break;
        case TARGET_SCORING:

            // Todo: Add some waiting
            if (xQueueReceive(weightQueue, &weight, 0) == pdTRUE)
            {
                if (weight < 0.0)
                {
                    weight = 0.0;
                }
               // lv_label_set_text_fmt(ui_lblResultTarget, "%.0f", weight);
                lv_label_set_text_fmt(ui_lblResultTextTarget, "Weight: %.0f", weight);
                


            }
            if (millis() - now > 3000)
            {
                if (playerTareValues[currentPlayerIndex] < round(weight))
                {
                    currentPlayerScore = 0;
                }
                else
                {
                    currentPlayerScore = playerTareValues[currentPlayerIndex] - (uint16_t)round(weight);
                }

                lv_obj_clear_flag(ui_imgResultTarget, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(ui_panInfoTarget, LV_OBJ_FLAG_HIDDEN);

                if (currentPlayerScore > (currentTarget + 1))
                {
                    currentMalus = (currentPlayerScore - currentTarget);
                    lv_img_set_src(ui_imgResultTarget, &ui_img_cross_png);
                    playFailSound();
                }
                else if (currentPlayerScore < (currentTarget - 1))
                {
                    currentMalus = (currentTarget - currentPlayerScore);
                    lv_img_set_src(ui_imgResultTarget, &ui_img_cross_png);
                    playFailSound();
                }
                else
                {
                    currentMalus = 0;
                    lv_img_set_src(ui_imgResultTarget, &ui_img_tick_png);
                    playOKSound();
                }
                playerScores[currentPlayerIndex] += currentMalus;
                // playerDartsScores[currentPlayerIndex] = 0; // DEBUG

               // lv_label_set_text_fmt(ui_lblResultTarget, "%d", currentMalus);
                lv_label_set_text_fmt(ui_lblResultTextTarget, "Drank: %d Malus: %d", currentPlayerScore, currentMalus);

                lv_label_set_text_fmt(ui_lblMalus, "Malus: %d", playerScores[currentPlayerIndex]);
               // lv_obj_clear_flag(ui_lblResultTextTarget, LV_OBJ_FLAG_HIDDEN);
              //  lv_label_set_text(ui_lblResultTextTarget, "Malus");
                lv_obj_clear_flag(ui_btnControlTarget, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(ui_panInfoTarget, LV_OBJ_FLAG_HIDDEN);
                //lv_obj_add_flag(ui_imgResultTarget, LV_OBJ_FLAG_HIDDEN);

                if ((currentRound) == gameRounds && (currentPlayerIndex + 1) == playerCount)
                {
                    lv_label_set_text(ui_lblBtnControlTarget, "Game Finished");
                    //  lv_scr_load(ui_screenWinner);
                    activeGameState = TARGET_SHOW_WINNER;
                }
                else
                {
                    lv_label_set_text(ui_lblBtnControlTarget, "Next Player");
                    activeGameState = TARGET_NEXT_PLAYER;
                 //   activeGameState = TARGET_SHOW_WINNER; //debug
                }
            }
            break;
        }
    }
}
