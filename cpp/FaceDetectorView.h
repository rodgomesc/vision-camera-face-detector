#pragma once

#include <fbjni/fbjni.h>

#include "JSurface.h"

namespace FaceDetector {

using namespace facebook::jni;

class FaceDetectorView : public facebook::jni::HybridClass<FaceDetectorView> {
 public:
  static constexpr auto kJavaDescriptor =
      "Lcom/visioncamerafacedetector/FaceDetectorView;";

  static void registerNatives();

  typedef HybridClass<FaceDetectorView> HybridBase;
  typedef FaceDetectorView Self;

  static facebook::jni::local_ref<jhybriddata> initHybrid(
      alias_ref<jhybridobject> jThis, jlong jsContext,
      alias_ref<jobject> context);

  FaceDetectorView(alias_ref<jhybridobject> jThis, jlong jsContext,
                   alias_ref<jobject> javaContext);

  ~FaceDetectorView();

  void nativeSurfaceCreated(alias_ref<JSurface::javaobject> surface);
  void nativeSurfaceChanged(alias_ref<JSurface::javaobject> surface, jint width,
                            jint height);
  void nativeSurfaceDestroyed(alias_ref<JSurface::javaobject> surface);

 private:
  void RenderFrame();
  void StartRenderLoop();
  void StopRenderLoop();
  static void* RenderThreadFunc(void* arg);

  // Java context reference
  global_ref<jobject> context_;

  // Surface and window
  ANativeWindow* nativeWindow_ = nullptr;

  // Surface dimensions
  int width_;
  int height_;

  // EGL context state
  bool contextValid_ = false;

  bool isRendering_;
  pthread_t renderThread_;
  std::atomic<bool> shouldStopRendering_;
};

}  // namespace FaceDetector