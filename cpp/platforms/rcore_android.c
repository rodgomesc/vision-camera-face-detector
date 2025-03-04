/**********************************************************************************************
 *
 *   rcore_android - Android platform functions
 *
 *   Copyright (c) 2024 rodgomesc (@rodgomesc)
 *
 **********************************************************************************************/

#include <android/log.h>
#include <android/native_window.h>
#include <time.h>

#include "raylib.h"
#include "raymath.h"

// Platform specific functions declaration
Vector2 GetWindowScaleDPI(void) {
  // On Android, we return 1.0 since we handle scaling differently
  return (Vector2){1.0f, 1.0f};
}

double GetTime(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}

int InitPlatform(void) {
  // Android initialization is handled by the surface view
  return 0;
}

void ClosePlatform(void) {
  // Android cleanup is handled by the surface view
}

void SwapScreenBuffer(void) {
  // Screen buffer swapping is handled by eglSwapBuffers in RaylibSurface
}

void PollInputEvents(void) {
  // Input events are handled by Android input system
}

void MaximizeWindow(void) {
  // Not applicable on Android
}

void MinimizeWindow(void) {
  // Not applicable on Android
}

void SetWindowSize(int width, int height) {
  // Window size is managed by the Android system
}