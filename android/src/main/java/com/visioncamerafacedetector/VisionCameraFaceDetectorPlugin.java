package com.visioncamerafacedetector;


import static java.lang.Math.ceil;
import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;

import com.google.gson.Gson;

import android.annotation.SuppressLint;
import android.graphics.PointF;
import android.graphics.Rect;
import android.media.Image;
import android.util.Log;


import androidx.camera.core.ImageProxy;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.facebook.react.bridge.Arguments;
import com.facebook.react.bridge.WritableMap;
import com.facebook.react.bridge.WritableNativeArray;
import com.facebook.react.bridge.WritableNativeMap;
import com.facebook.react.bridge.ReactApplicationContext;
import com.google.android.gms.tasks.Task;
import com.google.android.gms.tasks.Tasks;
import com.google.mlkit.vision.common.InputImage;
import com.google.mlkit.vision.face.Face;
import com.google.mlkit.vision.face.FaceContour;
import com.google.mlkit.vision.face.FaceDetection;
import com.google.mlkit.vision.face.FaceDetector;


import com.google.mlkit.vision.face.FaceDetectorOptions;
import com.mrousavy.camera.frameprocessors.FrameProcessorPlugin;
import com.mrousavy.camera.frameprocessors.Frame;
import com.mrousavy.camera.frameprocessors.VisionCameraProxy;
import com.mrousavy.camera.core.FrameInvalidError;

public class VisionCameraFaceDetectorPlugin extends FrameProcessorPlugin {

  FaceDetectorOptions options =
    new FaceDetectorOptions.Builder()
      .setPerformanceMode(FaceDetectorOptions.PERFORMANCE_MODE_ACCURATE)
      .setContourMode(FaceDetectorOptions.CONTOUR_MODE_ALL)
      .setClassificationMode(FaceDetectorOptions.CLASSIFICATION_MODE_ALL)
      .setMinFaceSize(0.15f)
      .build();

  FaceDetector faceDetector = FaceDetection.getClient(options);

  private  HashMap<String, Object> processBoundingBox(Rect boundingBox) {
     HashMap<String, Object> bounds =  new HashMap<String, Object>();

    // Calculate offset (we need to center the overlay on the target)
    Double offsetX =  (boundingBox.exactCenterX() - ceil(boundingBox.width())) / 2.0f;
    Double offsetY =  (boundingBox.exactCenterY() - ceil(boundingBox.height())) / 2.0f;

    Double x = boundingBox.right + offsetX;
    Double y = boundingBox.top + offsetY;

    // bounds.put("x", boundingBox.centerX() + (boundingBox.centerX() - x));
    // bounds.put("y", boundingBox.centerY() + (y - boundingBox.centerY()));

    bounds.put("x", boundingBox.left);
    bounds.put("y", boundingBox.top);
    bounds.put("width", boundingBox.width());
    bounds.put("height", boundingBox.height());


    bounds.put("boundingCenterX", boundingBox.centerX());
    bounds.put("boundingCenterY", boundingBox.centerY());
    bounds.put("boundingExactCenterX", boundingBox.exactCenterX());
    bounds.put("boundingExactCenterY", boundingBox.exactCenterY());

    return bounds;
  }

  private  HashMap<String, Object> processFaceContours(Face face) {
    // All faceContours
    int[] faceContoursTypes =
      new int[] {
        FaceContour.FACE,
        FaceContour.LEFT_EYEBROW_TOP,
        FaceContour.LEFT_EYEBROW_BOTTOM,
        FaceContour.RIGHT_EYEBROW_TOP,
        FaceContour.RIGHT_EYEBROW_BOTTOM,
        FaceContour.LEFT_EYE,
        FaceContour.RIGHT_EYE,
        FaceContour.UPPER_LIP_TOP,
        FaceContour.UPPER_LIP_BOTTOM,
        FaceContour.LOWER_LIP_TOP,
        FaceContour.LOWER_LIP_BOTTOM,
        FaceContour.NOSE_BRIDGE,
        FaceContour.NOSE_BOTTOM,
        FaceContour.LEFT_CHEEK,
        FaceContour.RIGHT_CHEEK
      };

    String[] faceContoursTypesStrings = {
        "FACE",
        "LEFT_EYEBROW_TOP",
        "LEFT_EYEBROW_BOTTOM",
        "RIGHT_EYEBROW_TOP",
        "RIGHT_EYEBROW_BOTTOM",
        "LEFT_EYE",
        "RIGHT_EYE",
        "UPPER_LIP_TOP",
        "UPPER_LIP_BOTTOM",
        "LOWER_LIP_TOP",
        "LOWER_LIP_BOTTOM",
        "NOSE_BRIDGE",
        "NOSE_BOTTOM",
        "LEFT_CHEEK",
        "RIGHT_CHEEK"
      };

    HashMap<String, Object> faceContoursTypesMap = new HashMap<String, Object>();

      for (int i = 0; i < faceContoursTypesStrings.length; i++) {
        FaceContour contour = face.getContour(faceContoursTypes[i]);
        List<PointF> points = contour.getPoints();
        List<HashMap<String, Object>> pointsArray = new ArrayList<HashMap<String, Object>>();

          for (int j = 0; j < points.size(); j++) {
             HashMap<String, Object> currentPointsMap = new  HashMap<String, Object>();

            currentPointsMap.put("x", points.get(j).x);
            currentPointsMap.put("y", points.get(j).y);

            pointsArray.add(currentPointsMap);
          }
          faceContoursTypesMap.put(faceContoursTypesStrings[contour.getFaceContourType() - 1], pointsArray);
      }

    return faceContoursTypesMap;
  }

  @SuppressLint("NewApi")
  @Override
  public Object callback( Frame frame, Map<String,Object> params) {
    try {
      Image mediaImage = frame.getImage();
      ImageProxy imageProxy = frame.getImageProxy();

      if (mediaImage != null) {
        InputImage image = InputImage.fromMediaImage(mediaImage, imageProxy.getImageInfo().getRotationDegrees());
        Task<List<Face>> task = faceDetector.process(image);
        List<HashMap<String, Object>> array = new ArrayList<HashMap<String, Object>>();
        try {
          List<Face> faces = Tasks.await(task);
          for (Face face : faces) {
            HashMap<String, Object> map = new HashMap<String, Object>();

            map.put("rollAngle", face.getHeadEulerAngleZ()); // Head is rotated to the left rotZ degrees
            map.put("pitchAngle", face.getHeadEulerAngleX()); // Head is rotated to the right rotX degrees
            map.put("yawAngle", face.getHeadEulerAngleY());  // Head is tilted sideways rotY degrees
            map.put("leftEyeOpenProbability", face.getLeftEyeOpenProbability());
            map.put("rightEyeOpenProbability", face.getRightEyeOpenProbability());
            map.put("smilingProbability", face.getSmilingProbability());
            

            HashMap<String, Object> contours = processFaceContours(face);
            HashMap<String, Object> bounds = processBoundingBox(face.getBoundingBox());

            map.put("bounds", bounds);
            map.put("contours", contours);

            array.add(map);
          }

          Gson gson = new Gson();

          return gson.toJson(array);
        } catch (Exception e) {
          e.printStackTrace();
        }
      }

      return null;
    } catch (FrameInvalidError e) {
      return null;
    }
  }

  VisionCameraFaceDetectorPlugin(@NonNull VisionCameraProxy proxy, @Nullable Map<String, Object> options) {super();}

  

}
