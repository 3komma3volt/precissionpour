#include "ui_functions.h"
#include "ui.h"


struct DartPosition {
    int16_t x;
    int16_t y;
};

struct ScoreArrows {
    uint8_t dartCount;
    DartPosition darts[2]; // Max 2 darts 
};

// Pre-calculated arrow positions for each score (0-60)
const ScoreArrows scoreArrowPositions[61] = {
    {1, {{-30, -90}}}, // Score 0
    {1, {{-61, -86}}}, // Score 1
    {1, {{-52, -19}}}, // Score 2
    {1, {{-75, -12}}}, // Score 3
    {1, {{-40, -73}}}, // Score 4
    {1, {{-86, -87}}}, // Score 5
    {1, {{-39, -50}}}, // Score 6
    {1, {{-97, -16}}}, // Score 7
    {1, {{-112, -37}}}, // Score 8
    {1, {{-106, -72}}}, // Score 9
    {1, {{-39, -39}}}, // Score 10
    {1, {{-114, -50}}}, // Score 11
    {1, {{-97, -81}}}, // Score 12
    {1, {{-37, -62}}}, // Score 13
    {1, {{-111, -63}}}, // Score 14
    {1, {{-41, -26}}}, // Score 15
    {1, {{-103, -26}}}, // Score 16
    {1, {{-61, -13}}}, // Score 17
    {1, {{-50, -81}}}, // Score 18
    {1, {{-85, -11}}}, // Score 19
    {1, {{-73, -89}}}, // Score 20
    {1, {{-91, -25}}}, // Score 21
    {1, {{-124, -55}}}, // Score 22
    {2, {{-73, -89}, {-75, -12}}}, // Score 23
    {1, {{-91, -25}}}, // Score 24
    {1, {{-71, -52}}}, // Score 25
    {1, {{-45, -59}}}, // Score 26
    {1, {{-96, -66}}}, // Score 27
    {1, {{-120, -64}}}, // Score 28
    {2, {{-73, -89}, {-106, -72}}}, // Score 29
    {1, {{-45, -39}}}, // Score 30
    {2, {{-73, -89}, {-114, -50}}}, // Score 31
    {1, {{-116, -24}}}, // Score 32
    {1, {{-104, -51}}}, // Score 33
    {1, {{-60, -4}}}, // Score 34
    {2, {{-73, -89}, {-41, -26}}}, // Score 35
    {1, {{-45, -88}}}, // Score 36
    {2, {{-45, -39}, {-97, -16}}}, // Score 37
    {1, {{-88, -1}}}, // Score 38
    {1, {{-46, -57}}}, // Score 39
    {1, {{-74, -97}}}, // Score 40
    {2, {{-74, -97}, {-61, -86}}}, // Score 41
    {1, {{-103, -58}}}, // Score 42
    {2, {{-74, -97}, {-75, -12}}}, // Score 43
    {2, {{-74, -97}, {-40, -73}}}, // Score 44
    {1, {{-47, -31}}}, // Score 45
    {2, {{-45, -39}, {-103, -26}}}, // Score 46
    {2, {{-60, -4}, {-37, -62}}}, // Score 47
    {1, {{-96, -30}}}, // Score 48
    {2, {{-74, -97}, {-106, -72}}}, // Score 49
    {1, {{-75, -51}}}, // Score 50
    {1, {{-63, -21}}}, // Score 51
    {2, {{-74, -97}, {-97, -81}}}, // Score 52
    {2, {{-74, -97}, {-37, -62}}}, // Score 53
    {1, {{-55, -72}}}, // Score 54
    {2, {{-75, -51}, {-86, -87}}}, // Score 55
    {2, {{-75, -51}, {-39, -50}}}, // Score 56
    {1, {{-82, -20}}}, // Score 57
    {2, {{-75, -51}, {-112, -37}}}, // Score 58
    {2, {{-88, -1}, {-91, -25}}}, // Score 59
    {1, {{-75, -78}}}  // Score 60 
};

void showDartArrows(uint16_t score) {

    lv_obj_add_flag(ui_imgArrow, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_imgArrow2, LV_OBJ_FLAG_HIDDEN);
    
    // Return for invalid values, used for hiding arrows
    if (score > 60) {
        return;
    }
    
    const ScoreArrows& arrows = scoreArrowPositions[score];
    

    if (arrows.dartCount >= 1) {
        lv_obj_set_pos(ui_imgArrow, arrows.darts[0].x, arrows.darts[0].y);
        lv_obj_clear_flag(ui_imgArrow, LV_OBJ_FLAG_HIDDEN);
    }
    
    if (arrows.dartCount >= 2) {
        lv_obj_set_pos(ui_imgArrow2, arrows.darts[1].x, arrows.darts[1].y);
        lv_obj_clear_flag(ui_imgArrow2, LV_OBJ_FLAG_HIDDEN);
    }
}