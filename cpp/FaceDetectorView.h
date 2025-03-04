#pragma once

#include <fbjni/fbjni.h>

#include <condition_variable>
#include <mutex>
#include <thread>

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
      alias_ref<jhybridobject> jThis, jlong jsRuntimePointer,
      alias_ref<jobject> context);

  FaceDetectorView(alias_ref<jhybridobject> jThis, jlong jsContext,
                   alias_ref<jobject> javaContext);

  ~FaceDetectorView();

 private:
  void nativeSurfaceCreated(alias_ref<JSurface::javaobject> surface);
  void nativeSurfaceChanged(alias_ref<JSurface::javaobject> surface, jint width,
                            jint height);
  void nativeSurfaceDestroyed(alias_ref<JSurface::javaobject> surface);

  // Java context reference
  global_ref<jobject> context_;

  // Surface and window
  ANativeWindow* nativeWindow_ = nullptr;

  // Rendering thread
  std::thread renderingThread_;
  std::mutex renderingMutex_;
  std::condition_variable renderingCondition_;
  std::atomic<bool> isRunning_;

  // Surface dimensions
  int width_;
  int height_;

  void startRenderingThread();
  void stopRenderingThread();
  void renderLoop();
  void RenderFrame();
};

}  // namespace FaceDetector