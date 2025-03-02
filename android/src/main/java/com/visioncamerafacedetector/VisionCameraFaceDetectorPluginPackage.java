package com.visioncamerafacedetector;

import androidx.annotation.NonNull;

import com.facebook.react.ReactPackage;
import com.facebook.react.bridge.NativeModule;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.uimanager.ViewManager;

import com.mrousavy.camera.frameprocessors.FrameProcessorPlugin;
import com.mrousavy.camera.frameprocessors.FrameProcessorPluginRegistry;
import java.util.Collections;
import java.util.List;

public class VisionCameraFaceDetectorPluginPackage implements ReactPackage {
  static {
    FrameProcessorPluginRegistry.addFrameProcessorPlugin("scanFaces", VisionCameraFaceDetectorPlugin::new);
  }

  @NonNull
  @org.jetbrains.annotations.NotNull
  @Override
  public List<NativeModule> createNativeModules(@NonNull @org.jetbrains.annotations.NotNull ReactApplicationContext reactContext) {
    return Collections.emptyList();
  }

  @NonNull
  @org.jetbrains.annotations.NotNull
  @Override
  public List<ViewManager> createViewManagers(@NonNull @org.jetbrains.annotations.NotNull ReactApplicationContext reactContext) {
    return Collections.emptyList();
  }
}
