/*
File:   defines.h
Author: Taylor Robbins
Date:   02\02\2025
*/

#ifndef _DEFINES_H
#define _DEFINES_H

#define LOAD_FROM_RESOURCES_FOLDER 1
#define FP3D_SCENE_ENABLED 1
#define MOUSE_LERP_WINDOW_PADDING 32 //px

#define CLAY_DEF_PADDING   2 //px
#define CLAY_TOPBAR_HEIGHT 30 //px

#define CLAY_TOPBAR_TOGGLE_HOTKEY Key_F7
#define IMGUI_TOPBAR_TOGGLE_HOTKEY Key_F6

#define TEST_PHYS_GRAVITY       NewV3(0, -9.8f, 0)
#define TEST_PHYS_BOX_SIZE      NewV3(0.2f, 0.1f, 0.15f)
#define TEST_PHYS_BOX_DENSITY   1.0f
#define TEST_PHYS_SIM_STEP_SIZE 1 //ms

#endif //  _DEFINES_H
