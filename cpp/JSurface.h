//
// Created by rodrigo gomes on 16/05/24.
//

#pragma once

#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <fbjni/fbjni.h>

namespace FaceDetector {
using namespace facebook::jni;

class JSurface : public JavaClass<JSurface> {
 public:
  static constexpr auto kJavaDescriptor = "Landroid/view/Surface;";
};
}  // namespace FaceDetector
