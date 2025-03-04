//
// Created by rodrigo gomes on 16/05/24.
//

#pragma once

#include <EGL/egl.h>
#include <android/native_window.h>

class EGLContextHandler {
 public:
  EGLContextHandler();

  ~EGLContextHandler();

  bool initialize(ANativeWindow *nativeWindow);

  void terminate();

  void swapBuffers();

  int getWidth() const;

  int getHeight() const;

  bool isValid() const { return _eglDisplay != EGL_NO_DISPLAY; }

  int _width;
  int _height;

 private:
  EGLDisplay _eglDisplay;
  EGLConfig _eglConfig;
  EGLSurface _eglSurface;
  EGLContext _eglContext;
};
