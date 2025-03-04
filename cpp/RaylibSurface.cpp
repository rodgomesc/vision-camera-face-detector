#include "RaylibSurface.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/log.h>

#include "raylib.h"
#include "rlgl.h"  // Required for rlglInit, rlglClose, rlViewport

#define LOG_TAG "RaylibSurface"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace FaceDetector {

// Initialize static members
EGLDisplay RaylibSurface::display_ = EGL_NO_DISPLAY;
EGLSurface RaylibSurface::surface_ = EGL_NO_SURFACE;
EGLContext RaylibSurface::context_ = EGL_NO_CONTEXT;
EGLConfig RaylibSurface::config_ = nullptr;
ANativeWindow* RaylibSurface::window_ = nullptr;
bool RaylibSurface::initialized_ = false;

bool RaylibSurface::InitWithNativeWindow(ANativeWindow* window, int width,
                                         int height) {
  if (initialized_) {
    LOGI("Raylib already initialized");
    return true;
  }

  if (!window) {
    LOGE("Invalid native window");
    return false;
  }

  window_ = window;

  // Initialize EGL
  if (!InitEGL(window)) {
    LOGE("Failed to initialize EGL");
    return false;
  }

  // Set up Raylib configuration
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);

  // Initialize Raylib core
  // We're not calling InitWindow() because we're using our own EGL context
  // Instead, we need to initialize the necessary Raylib components manually

  // Set up the initial screen size
  // This is similar to what happens in InitWindow() but without creating a new
  // window
  LOGI("Setting up Raylib with surface %dx%d", width, height);

  // Initialize Raylib globals
  // These are normally set in InitWindow()
  rlglInit(width, height);

  // Clear the background to white
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Swap buffers to show the cleared background
  eglSwapBuffers(display_, surface_);

  initialized_ = true;
  LOGI("Raylib initialized with surface %dx%d", width, height);
  return true;
}

void RaylibSurface::Cleanup() {
  if (!initialized_) {
    return;
  }

  // Clean up Raylib resources
  rlglClose();

  // Terminate EGL
  TerminateEGL();

  window_ = nullptr;
  initialized_ = false;
  LOGI("Raylib cleaned up");
}

bool RaylibSurface::IsInitialized() { return initialized_; }

void RaylibSurface::UpdateSize(int width, int height) {
  if (!initialized_) {
    LOGE("Raylib not initialized");
    return;
  }

  // Update the viewport
  glViewport(0, 0, width, height);

  // Update Raylib's internal state
  rlViewport(0, 0, width, height);

  LOGI("Surface size updated to %dx%d", width, height);
}

bool RaylibSurface::InitEGL(ANativeWindow* window) {
  // Initialize EGL
  display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (display_ == EGL_NO_DISPLAY) {
    LOGE("eglGetDisplay() returned error %d", eglGetError());
    return false;
  }

  if (!eglInitialize(display_, 0, 0)) {
    LOGE("eglInitialize() returned error %d", eglGetError());
    return false;
  }

  // Configure EGL
  const EGLint attribs[] = {EGL_SURFACE_TYPE,
                            EGL_WINDOW_BIT,
                            EGL_RENDERABLE_TYPE,
                            EGL_OPENGL_ES2_BIT,
                            EGL_BLUE_SIZE,
                            8,
                            EGL_GREEN_SIZE,
                            8,
                            EGL_RED_SIZE,
                            8,
                            EGL_ALPHA_SIZE,
                            8,
                            EGL_DEPTH_SIZE,
                            16,
                            EGL_STENCIL_SIZE,
                            8,
                            EGL_NONE};

  EGLint numConfigs;
  if (!eglChooseConfig(display_, attribs, &config_, 1, &numConfigs)) {
    LOGE("eglChooseConfig() returned error %d", eglGetError());
    return false;
  }

  // Get the native visual ID to configure the ANativeWindow
  EGLint format;
  eglGetConfigAttrib(display_, config_, EGL_NATIVE_VISUAL_ID, &format);
  ANativeWindow_setBuffersGeometry(window, 0, 0, format);

  // Create a surface
  surface_ = eglCreateWindowSurface(display_, config_, window, nullptr);
  if (surface_ == EGL_NO_SURFACE) {
    LOGE("eglCreateWindowSurface() returned error %d", eglGetError());
    return false;
  }

  // Create a context
  const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

  context_ =
      eglCreateContext(display_, config_, EGL_NO_CONTEXT, contextAttribs);
  if (context_ == EGL_NO_CONTEXT) {
    LOGE("eglCreateContext() returned error %d", eglGetError());
    return false;
  }

  // Make the context current
  if (!eglMakeCurrent(display_, surface_, surface_, context_)) {
    LOGE("eglMakeCurrent() returned error %d", eglGetError());
    return false;
  }

  LOGI("EGL initialized");
  return true;
}

void RaylibSurface::TerminateEGL() {
  if (display_ != EGL_NO_DISPLAY) {
    eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    if (context_ != EGL_NO_CONTEXT) {
      eglDestroyContext(display_, context_);
      context_ = EGL_NO_CONTEXT;
    }

    if (surface_ != EGL_NO_SURFACE) {
      eglDestroySurface(display_, surface_);
      surface_ = EGL_NO_SURFACE;
    }

    eglTerminate(display_);
    display_ = EGL_NO_DISPLAY;
  }

  LOGI("EGL terminated");
}

}  // namespace FaceDetector