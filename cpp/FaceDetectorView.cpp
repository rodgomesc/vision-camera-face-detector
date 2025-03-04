#include "FaceDetectorView.h"

#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <fbjni/fbjni.h>
#include <jni.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "JSurface.h"
#include "RaylibSurface.h"

// Include Raylib headers
#include "raylib.h"
#include "rlgl.h"  // Required for low-level OpenGL access

#define LOG_TAG "FaceDetectorView"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace FaceDetector {

using namespace facebook::jni;
using namespace std;

void FaceDetectorView::registerNatives() {
  registerHybrid({makeNativeMethod("initHybrid", FaceDetectorView::initHybrid),
                  makeNativeMethod("nativeSurfaceCreated",
                                   FaceDetectorView::nativeSurfaceCreated),
                  makeNativeMethod("nativeSurfaceChanged",
                                   FaceDetectorView::nativeSurfaceChanged),
                  makeNativeMethod("nativeSurfaceDestroyed",
                                   FaceDetectorView::nativeSurfaceDestroyed)});
}

local_ref<FaceDetectorView::jhybriddata> FaceDetectorView::initHybrid(
    alias_ref<jhybridobject> jThis, jlong jsRuntimePointer,
    alias_ref<jobject> context) {
  return makeCxxInstance(jThis, jsRuntimePointer, context);
}

FaceDetectorView::FaceDetectorView(alias_ref<jhybridobject> jThis,
                                   jlong jsContext,
                                   alias_ref<jobject> javaContext) {
  // Store the context for later use
  context_ = make_global(javaContext);

  // Initialize rendering thread state
  isRunning_ = false;
  width_ = 0;
  height_ = 0;

  LOGI("FaceDetectorView created");
}

FaceDetectorView::~FaceDetectorView() {
  stopRenderingThread();
  LOGI("FaceDetectorView destroyed");
}

void FaceDetectorView::nativeSurfaceCreated(
    alias_ref<JSurface::javaobject> surface) {
  // Get the native window from the surface
  JNIEnv* env = Environment::current();
  nativeWindow_ = ANativeWindow_fromSurface(env, surface.get());

  if (nativeWindow_ != nullptr) {
    // Get the window dimensions
    width_ = ANativeWindow_getWidth(nativeWindow_);
    height_ = ANativeWindow_getHeight(nativeWindow_);

    LOGI("Surface created: %dx%d", width_, height_);

    // Start the rendering thread
    startRenderingThread();
  } else {
    LOGE("Failed to get native window from surface");
  }
}

void FaceDetectorView::nativeSurfaceChanged(
    alias_ref<JSurface::javaobject> surface, jint width, jint height) {
  // Update dimensions
  width_ = width;
  height_ = height;

  LOGI("Surface changed: %dx%d", width_, height_);

  // Update the surface size in the rendering thread
  if (RaylibSurface::IsInitialized()) {
    RaylibSurface::UpdateSize(width_, height_);
  }
}

void FaceDetectorView::nativeSurfaceDestroyed(
    alias_ref<JSurface::javaobject> surface) {
  LOGI("Surface destroyed");

  // Stop the rendering thread
  stopRenderingThread();

  // Release the native window
  if (nativeWindow_ != nullptr) {
    ANativeWindow_release(nativeWindow_);
    nativeWindow_ = nullptr;
  }
}

void FaceDetectorView::startRenderingThread() {
  // Ensure any previous thread is stopped
  stopRenderingThread();

  // Start a new rendering thread
  isRunning_ = true;
  renderingThread_ = std::thread(&FaceDetectorView::renderLoop, this);

  LOGI("Rendering thread started");
}

void FaceDetectorView::stopRenderingThread() {
  // Signal the thread to stop
  isRunning_ = false;

  // Notify the condition variable to wake up the thread
  renderingCondition_.notify_one();

  // Wait for the thread to join if it's joinable
  if (renderingThread_.joinable()) {
    renderingThread_.join();
    LOGI("Rendering thread stopped");
  }
}

void FaceDetectorView::renderLoop() {
  // Initialize Raylib for this surface
  if (!RaylibSurface::InitWithNativeWindow(nativeWindow_, width_, height_)) {
    LOGE("Failed to initialize Raylib for surface");
    return;
  }

  // Main rendering loop
  while (isRunning_) {
    // Render frame
    RenderFrame();

    // Small sleep to avoid consuming too much CPU
    std::this_thread::sleep_for(std::chrono::milliseconds(16));  // ~60 FPS
  }

  // Clean up Raylib
  RaylibSurface::Cleanup();
}

void FaceDetectorView::RenderFrame() {
  static float rotation = 0.0f;  // For animation
  static float pulse = 0.0f;     // For size pulsing

  // Clear the background
  rlClearScreenBuffers();
  rlClearColor(20, 30, 40, 255);  // Dark blue background

  // Set up the drawing state
  rlMatrixMode(RL_PROJECTION);
  rlLoadIdentity();
  rlOrtho(0, width_, height_, 0, 0.0f, 1.0f);
  rlMatrixMode(RL_MODELVIEW);
  rlLoadIdentity();

  // Update animations
  rotation += 2.0f;                     // Rotate 2 degrees per frame
  pulse = sinf(GetTime() * 2) * 10.0f;  // Smooth pulsing effect

  // Draw a gradient-filled circle in the background
  DrawCircleGradient(width_ / 2, height_ / 2, 200,
                     Color{100, 200, 255, 100},  // Light blue center
                     Color{50, 100, 200, 50});   // Darker blue outer

  // Draw a rotating polygon
  DrawPoly(Vector2{(float)width_ / 2, (float)height_ / 2},
           6,                                   // Hexagon
           80 + pulse,                          // Pulsing size
           rotation, Color{255, 200, 0, 200});  // Golden yellow

  // Draw multiple circles with different colors
  for (int i = 0; i < 8; i++) {
    float angle = rotation * DEG2RAD + (i * PI / 4);
    float radius = 100.0f;
    float x = width_ / 2 + cosf(angle) * radius;
    float y = height_ / 2 + sinf(angle) * radius;
    DrawCircle(x, y, 20 + (pulse / 2),
               Color{(unsigned char)(255 - i * 30), (unsigned char)(i * 30),
                     255, 200});
  }

  // Draw a ring
  DrawRing(Vector2{(float)width_ / 2, (float)height_ / 2}, 140 + pulse,
           150 + pulse, 0, 360,
           60,                          // segments
           Color{255, 100, 100, 128});  // Semi-transparent red

  // End drawing and swap buffers
  rlDrawRenderBatchActive();
  eglSwapBuffers(RaylibSurface::display_, RaylibSurface::surface_);
}

}  // namespace FaceDetector