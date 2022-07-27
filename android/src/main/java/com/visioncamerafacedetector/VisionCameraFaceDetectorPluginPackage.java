package com.visioncamerafacedetector;

import androidx.annotation.NonNull;

import com.facebook.react.ReactPackage;
import com.facebook.react.bridge.NativeModule;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.uimanager.ViewManager;
import com.mrousavy.camera.frameprocessor.FrameProcessorPlugin;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class VisionCameraFaceDetectorPluginPackage implements ReactPackage {
  @NonNull
  @org.jetbrains.annotations.NotNull
  @Override
  public List<NativeModule> createNativeModules(@NonNull @org.jetbrains.annotations.NotNull ReactApplicationContext reactContext) {
    FrameProcessorPlugin.register(new VisionCameraFaceDetectorPlugin());
    List<NativeModule> modules = new ArrayList<>();
    modules.add(new VisionCameraFaceDetectorModule(reactContext));
    return modules;
  }

  @NonNull
  @org.jetbrains.annotations.NotNull
  @Override
  public List<ViewManager> createViewManagers(@NonNull @org.jetbrains.annotations.NotNull ReactApplicationContext reactContext) {
    return Collections.emptyList();
  }
}
