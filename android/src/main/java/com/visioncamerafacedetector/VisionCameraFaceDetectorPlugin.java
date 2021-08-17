package com.visioncamerafacedetector;


import androidx.camera.core.ImageProxy;

import com.facebook.react.bridge.WritableNativeMap;

import com.mrousavy.camera.frameprocessor.FrameProcessorPlugin;


public class VisionCameraFaceDetectorPlugin extends FrameProcessorPlugin {

  @Override
  public Object callback(ImageProxy frame, Object[] params) {

    WritableNativeMap map = new WritableNativeMap();
    map.putString("key","face detector test from java module");
    return map;
  }


  VisionCameraFaceDetectorPlugin() {
    super("scanFaces");
  }
}
