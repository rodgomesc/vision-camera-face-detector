#include "FaceDetectorView.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android/log.h>

#include <chrono>
#include <thread>

#include "EGLContextHandler.h"
#include "Global.h"
#include "raylib.h"
#include "react_native_rcore.h"

#define LOG_TAG "FaceDetectorView"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace FaceDetector {
using namespace facebook::jni;

using TSelf = local_ref<HybridClass<FaceDetectorView>::jhybriddata>;

FaceDetectorView::FaceDetectorView(
    const alias_ref<FaceDetectorView::jhybridobject> &javaThis,
    Runtime *runtime, alias_ref<JObject> context) {
  _javaPart = make_global(javaThis);
  _runtime = runtime;
  if (globalEglHandler == nullptr) {
    globalEglHandler = std::make_unique<EGLContextHandler>();
  }
}

FaceDetectorView::~FaceDetectorView() {
  if (_nativeWindow) {
    ANativeWindow_release(_nativeWindow);
  }
}

void FaceDetectorView::registerNatives() {
  registerHybrid({makeNativeMethod("initHybrid", FaceDetectorView::initHybrid),
                  makeNativeMethod("nativeSurfaceCreated",
                                   FaceDetectorView::onSurfaceCreated),
                  makeNativeMethod("nativeSurfaceChanged",
                                   FaceDetectorView::onSurfaceChanged),
                  makeNativeMethod("nativeSurfaceDestroyed",
                                   FaceDetectorView::onSurfaceDestroyed)});
}

Runtime *FaceDetectorView::getJSRuntime() { return _runtime; }

ANativeWindow *FaceDetectorView::getNativeWindow() { return _nativeWindow; }

TSelf FaceDetectorView::initHybrid(alias_ref<jhybridobject> jThis,
                                   jlong jsRuntimePointer,
                                   alias_ref<JObject> context) {
  auto jsRuntime = reinterpret_cast<Runtime *>(jsRuntimePointer);
  return makeCxxInstance(jThis, jsRuntime, context);
}

void FaceDetectorView::onSurfaceCreated(alias_ref<JSurface> surface) {
  _nativeWindow = ANativeWindow_fromSurface(
      facebook::jni::Environment::current(), surface.get());
  if (_nativeWindow == nullptr) {
    LOGI("Failed to get native window from surface");
    return;
  }

  globalEglHandler->_width = ANativeWindow_getWidth(_nativeWindow);
  globalEglHandler->_height = ANativeWindow_getHeight(_nativeWindow);

  CORE.Window.screen.width = globalEglHandler->_width;
  CORE.Window.screen.height = globalEglHandler->_height;

  InitWindow(globalEglHandler->_width, globalEglHandler->_height,
             "Raylib on ANativeWindow");
  SetTargetFPS(60);

  _renderThread = std::thread(&FaceDetectorView::renderLoop, this);
  //     _renderThread = std::thread(&FaceDetectorView::debugDraw, this);
}

void FaceDetectorView::onSurfaceChanged(alias_ref<JSurface> surface, int width,
                                        int height) {
  LOGI("onSurfaceChanged called: width=%d, height=%d", width, height);
  SetWindowSize(width, height);
}

void FaceDetectorView::onSurfaceDestroyed(alias_ref<JSurface> surface) {
  LOGI("onSurfaceDestroyed called");
  if (_nativeWindow) {
    ANativeWindow_release(_nativeWindow);
    _nativeWindow = nullptr;
  }
  ClosePlatform();
  LOGI("Native window destroyed");
}

void FaceDetectorView::renderLoop() {
  if (!_nativeWindow) {
    LOGI("Native window is null, exiting render loop");
    return;
  }

  // Initialize OpenGL context
  if (!globalEglHandler->initialize(_nativeWindow)) {
    LOGI("Failed to initialize EGL context, exiting render loop");
    return;
  }

  while (_nativeWindow) {
    if (!globalEglHandler || !globalEglHandler->isValid()) {
      LOGI("EGL context lost, exiting render loop");
      break;
    }

    BeginDrawing();
    ClearBackground(RED);
    DrawRectangle(0, 0, 100, 100, RED);
    DrawText("Hello, Raylib!", 0, 0, 20, GREEN);
    EndDrawing();

    // Add a small delay to prevent excessive CPU usage
    std::this_thread::sleep_for(std::chrono::milliseconds(16));  // ~60 FPS
  }

  // Cleanup
  if (globalEglHandler) {
    globalEglHandler->terminate();
  }
}

void FaceDetectorView::debugDraw() {
  globalEglHandler->initialize(_nativeWindow);

  while (_nativeWindow) {
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);  // Clear the screen with black color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Define the vertices for a rectangle
    GLfloat vertices[] = {
        -0.5f, -0.5f, 0.0f,  // Bottom left
        0.5f,  -0.5f, 0.0f,  // Bottom right
        0.5f,  0.5f,  0.0f,  // Top right
        -0.5f, 0.5f,  0.0f   // Top left
    };

    // Define the indices for the rectangle (two triangles)
    GLushort indices[] = {
        0, 1, 2,  // First triangle
        2, 3, 0   // Second triangle
    };

    // Enable the vertex array
    glEnableVertexAttribArray(0);

    // Set the vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);

    // Draw the rectangle
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    // Disable the vertex array
    glDisableVertexAttribArray(0);

    globalEglHandler->swapBuffers();
  }
}
}  // namespace FaceDetector
