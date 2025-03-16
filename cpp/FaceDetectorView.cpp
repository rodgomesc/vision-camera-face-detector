#include "FaceDetectorView.h"

#include "raylib.h"

#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <fbjni/fbjni.h>
#include <jni.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "JSurface.h"

#include <thread>
#include <chrono>
#include <mutex>

#define LOG_TAG "FaceDetectorView"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Forward declarations of platform-specific functions from rcore_android.c
extern "C" {
    bool InitWindowAndroid(ANativeWindow* nativeWindow);
    bool MakeContextCurrent(void);
    void ReleaseContext(void);
}

namespace FaceDetector {

using namespace facebook::jni;
using namespace std;

void FaceDetectorView::registerNatives() {
    registerHybrid({
        makeNativeMethod("initHybrid", FaceDetectorView::initHybrid),
        makeNativeMethod("nativeSurfaceCreated", FaceDetectorView::nativeSurfaceCreated),
        makeNativeMethod("nativeSurfaceChanged", FaceDetectorView::nativeSurfaceChanged),
        makeNativeMethod("nativeSurfaceDestroyed", FaceDetectorView::nativeSurfaceDestroyed)
    });
}

local_ref<FaceDetectorView::jhybriddata> FaceDetectorView::initHybrid(
    alias_ref<jhybridobject> jThis, jlong jsRuntimePointer,
    alias_ref<jobject> context) {
    return makeCxxInstance(jThis, jsRuntimePointer, context);
}

FaceDetectorView::FaceDetectorView(alias_ref<jhybridobject> jThis,
                                   jlong jsContext,
                                   alias_ref<jobject> javaContext) {
    context_ = make_global(javaContext);
    width_ = 0;
    height_ = 0;
    contextValid_ = false;
    isRendering_ = false;
    shouldStopRendering_ = false;
    LOGI("FaceDetectorView created");
}

FaceDetectorView::~FaceDetectorView() {
    StopRenderLoop();
    LOGI("FaceDetectorView destroyed");
}

void* FaceDetectorView::RenderThreadFunc(void* arg) {
    FaceDetectorView* view = static_cast<FaceDetectorView*>(arg);

    // Make the EGL context current in this thread
    if (!MakeContextCurrent()) {
        LOGE("Failed to make EGL context current in render thread");
        return nullptr;
    }

    while (!view->shouldStopRendering_) {
        if (view->contextValid_) {
            view->RenderFrame();
        }
       // Cap frame rate to ~60 FPS
       std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    // Release the EGL context from this thread
    ReleaseContext();

    return nullptr;
}

void FaceDetectorView::StartRenderLoop() {
    if (!isRendering_) {
        shouldStopRendering_ = false;
        isRendering_ = true;
        pthread_create(&renderThread_, nullptr, RenderThreadFunc, this);
        LOGI("Render loop started");
    }
}

void FaceDetectorView::StopRenderLoop() {
    if (isRendering_) {
        shouldStopRendering_ = true;
        pthread_join(renderThread_, nullptr);
        isRendering_ = false;
        LOGI("Render loop stopped");
    }
}

void FaceDetectorView::nativeSurfaceCreated(
    alias_ref<JSurface::javaobject> surface)
{
    JNIEnv* env = Environment::current();
    nativeWindow_ = ANativeWindow_fromSurface(env, surface.get());

    if (nativeWindow_ != nullptr)
    {
        width_ = ANativeWindow_getWidth(nativeWindow_);
        height_ = ANativeWindow_getHeight(nativeWindow_);
        LOGI("Surface created: %dx%d", width_, height_);

        InitWindow(width_, height_, "Test");
        if (InitWindowAndroid(nativeWindow_)) {
            contextValid_ = true;
            LOGI("Window initialized successfully");

            // Release the EGL context from the main thread before starting render thread
            ReleaseContext();

            StartRenderLoop();
        } else {
            LOGE("Failed to initialize window");
        }
    }
    else
    {
        LOGE("Failed to get native window from surface");
    }
}

void FaceDetectorView::nativeSurfaceChanged(
    alias_ref<JSurface::javaobject> surface, jint width, jint height) {
    width_ = width;
    height_ = height;
    LOGI("Surface changed: %dx%d", width_, height_);

    RenderFrame();

}

void FaceDetectorView::nativeSurfaceDestroyed(
    alias_ref<JSurface::javaobject> surface) {
    LOGI("Surface destroyed");

    StopRenderLoop();
    contextValid_ = false;

    // Make the context current in this thread to clean up
    if (MakeContextCurrent()) {
        if (nativeWindow_ != nullptr) {
            ANativeWindow_release(nativeWindow_);
            nativeWindow_ = nullptr;
        }

        // Release the context again
        ReleaseContext();
    }
}

void FaceDetectorView::RenderFrame() {
    if (!contextValid_) return;

    static float rotation = 0.0f;
    static float pulse = 0.0f;

    BeginDrawing();
        ClearBackground(Color{20, 30, 40, 255});


            rotation += 2.0f;
            pulse = sinf(GetTime() * 2) * 10.0f;


            DrawCircleGradient(width_ / 2, height_ / 2, 200,
                           Color{100, 200, 255, 100},
                           Color{50, 100, 200, 50});
            DrawPoly(Vector2{(float)width_ / 2, (float)height_ / 2}, 6, 80 + pulse, rotation, Color{255, 200, 0, 200});

            // Draw text on top of the circle
            const char* text = TextFormat("FPS: %d", GetFPS());
            int fontSize = 60;
            Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 2);
            DrawText(text,
                    width_ / 2 - textSize.x / 2,  // Center horizontally
                    height_ / 2 - 400 - textSize.y / 2,  // Center vertically
                    fontSize,
                    GREEN);

            for (int i = 0; i < 8; i++) {
                float angle = rotation * DEG2RAD + (i * PI / 4);
                float radius = 100.0f;
                float x = width_ / 2 + cosf(angle) * radius;
                float y = height_ / 2 + sinf(angle) * radius;
                DrawCircle(x, y, 20 + (pulse / 2), Color{(unsigned char)(255 - i * 30), (unsigned char)(i * 30), 255, 200});
            }

            DrawRing(Vector2{(float)width_ / 2, (float)height_ / 2}, 140 + pulse, 150 + pulse, 0, 360, 60, Color{255, 100, 100, 128});

    EndDrawing();
}

}  // namespace FaceDetector
