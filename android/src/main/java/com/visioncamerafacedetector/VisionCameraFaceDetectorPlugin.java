package com.visioncamerafacedetector;


import android.annotation.SuppressLint;
import android.graphics.Rect;
import android.media.Image;

import androidx.camera.core.ImageProxy;

import com.facebook.react.bridge.WritableNativeArray;
import com.google.android.gms.tasks.Task;
import com.google.android.gms.tasks.Tasks;
import com.google.mlkit.vision.common.InputImage;
import com.google.mlkit.vision.face.Face;
import com.google.mlkit.vision.face.FaceDetection;
import com.google.mlkit.vision.face.FaceDetector;
import com.facebook.react.bridge.WritableNativeMap;

import com.google.mlkit.vision.face.FaceDetectorOptions;
import com.mrousavy.camera.frameprocessor.FrameProcessorPlugin;

import java.util.List;


public class VisionCameraFaceDetectorPlugin extends FrameProcessorPlugin {

  FaceDetectorOptions options =
    new FaceDetectorOptions.Builder()
      .setPerformanceMode(FaceDetectorOptions.PERFORMANCE_MODE_ACCURATE)
      .setLandmarkMode(FaceDetectorOptions.LANDMARK_MODE_ALL)
      .setClassificationMode(FaceDetectorOptions.CLASSIFICATION_MODE_ALL)
      .setMinFaceSize(0.15f)
      .enableTracking()
      .build();

  FaceDetector faceDetector = FaceDetection.getClient(options);


  @Override
  public Object callback(ImageProxy frame, Object[] params) {


    @SuppressLint("UnsafeOptInUsageError")
    Image mediaImage = frame.getImage();

    if (mediaImage != null) {
      InputImage image = InputImage.fromMediaImage(mediaImage, frame.getImageInfo().getRotationDegrees());
      Task<List<Face>> task = faceDetector.process(image);
      WritableNativeArray array = new WritableNativeArray();
      try {
        List<Face> faces = Tasks.await(task);

        for (Face face : faces) {
          WritableNativeMap map = new WritableNativeMap();

          Rect bounds = face.getBoundingBox();
          map.putDouble("eulerAngleX", face.getHeadEulerAngleX()); // Head is rotated to the left rotY degrees
          map.putDouble("eulerAngleY", face.getHeadEulerAngleY()); // Head is rotated to the right rotY degrees
          map.putDouble("eulerAngleZ", face.getHeadEulerAngleZ());  // Head is tilted sideways rotZ degrees
          map.putString("boundingBox", face.getBoundingBox().flattenToString());
          map.putDouble("leftEyeOpenProbability", face.getLeftEyeOpenProbability());
          map.putDouble("rightEyeOpenProbability", face.getRightEyeOpenProbability());

          array.pushMap(map);
        }
        return array;
      } catch (Exception e) {
        e.printStackTrace();
      }
    }
    return null;
  }


  VisionCameraFaceDetectorPlugin() {
    super("scanFaces");
  }
}
