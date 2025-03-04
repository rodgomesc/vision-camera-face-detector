#pragma once

#include <EGL/egl.h>
#include <android/native_window.h>

namespace FaceDetector {

// This class handles the integration between Raylib and our Android surface
class RaylibSurface {
 public:
  // Initialize Raylib with an Android native window
  static bool InitWithNativeWindow(ANativeWindow* window, int width,
                                   int height);

  // Clean up Raylib resources
  static void Cleanup();

  // Check if Raylib is initialized
  static bool IsInitialized();

  // Update the surface size
  static void UpdateSize(int width, int height);

  // EGL display and surface (public for direct access in rendering)
  static EGLDisplay display_;
  static EGLSurface surface_;

 private:
  // EGL context and config
  static EGLContext context_;
  static EGLConfig config_;

  // Native window
  static ANativeWindow* window_;

  // Initialization state
  static bool initialized_;

  // Initialize EGL
  static bool InitEGL(ANativeWindow* window);

  // Terminate EGL
  static void TerminateEGL();
};

}  // namespace FaceDetector