package com.visioncamerafacedetector;

import static java.lang.Math.ceil;

import android.annotation.SuppressLint;
import android.graphics.PointF;
import android.graphics.Rect;
import android.media.Image;

import androidx.camera.core.ImageProxy;

import com.facebook.react.bridge.Arguments;
import com.facebook.react.bridge.WritableMap;
import com.facebook.react.bridge.WritableNativeArray;
import com.facebook.react.bridge.WritableNativeMap;
import com.google.android.gms.tasks.Task;
import com.google.android.gms.tasks.Tasks;
import com.google.mlkit.vision.common.InputImage;
import com.google.mlkit.vision.face.Face;
import com.google.mlkit.vision.face.FaceContour;
import com.google.mlkit.vision.face.FaceDetection;
import com.google.mlkit.vision.face.FaceDetector;

import com.google.mlkit.vision.face.FaceDetectorOptions;
import com.google.mlkit.vision.face.FaceLandmark;
import com.mrousavy.camera.frameprocessor.FrameProcessorPlugin;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class VisionCameraFaceDetectorPlugin extends FrameProcessorPlugin {

  FaceDetectorOptions options =
    new FaceDetectorOptions.Builder()
      .setPerformanceMode(FaceDetectorOptions.PERFORMANCE_MODE_ACCURATE)
      .setContourMode(FaceDetectorOptions.CONTOUR_MODE_ALL)
      .setClassificationMode(FaceDetectorOptions.CLASSIFICATION_MODE_ALL)
      .setLandmarkMode(FaceDetectorOptions.LANDMARK_MODE_ALL)
      .setMinFaceSize(0.15f)
      .build();

  FaceDetector faceDetector = FaceDetection.getClient(options);

  private WritableMap processBoundingBox(Rect boundingBox) {
    WritableMap bounds = Arguments.createMap();

    // Calculate offset (we need to center the overlay on the target)
    Double offsetX = (boundingBox.exactCenterX() - ceil(boundingBox.width())) / 2.0f;
    Double offsetY = (boundingBox.exactCenterY() - ceil(boundingBox.height())) / 2.0f;

    Double x = boundingBox.right + offsetX;
    Double y = boundingBox.top + offsetY;


    bounds.putDouble("x", boundingBox.centerX() + (boundingBox.centerX() - x));
    bounds.putDouble("y", boundingBox.centerY() + (y - boundingBox.centerY()));
    bounds.putDouble("width", boundingBox.width());
    bounds.putDouble("height", boundingBox.height());


    bounds.putDouble("boundingCenterX", boundingBox.centerX());
    bounds.putDouble("boundingCenterY", boundingBox.centerY());
    bounds.putDouble("boundingExactCenterX", boundingBox.exactCenterX());
    bounds.putDouble("boundingExactCenterY", boundingBox.exactCenterY());

    return bounds;
  }

  private WritableMap processFaceContours(Face face) {
    // All faceContours
    int[] faceContoursTypes =
      new int[]{
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

    WritableMap faceContoursTypesMap = new WritableNativeMap();

    for (int i = 0; i < faceContoursTypesStrings.length; i++) {
      FaceContour contour = face.getContour(faceContoursTypes[i]);
      List<PointF> points = contour.getPoints();
      WritableNativeArray pointsArray = new WritableNativeArray();

      for (int j = 0; j < points.size(); j++) {
        WritableMap currentPointsMap = new WritableNativeMap();

        currentPointsMap.putDouble("x", points.get(j).x);
        currentPointsMap.putDouble("y", points.get(j).y);

        pointsArray.pushMap(currentPointsMap);
      }
      faceContoursTypesMap.putArray(faceContoursTypesStrings[contour.getFaceContourType() - 1], pointsArray);
    }

    return faceContoursTypesMap;
  }

  private WritableMap processFaceLandmarks(Face face) {

    Map<String, Integer> faceLandmarkTypesMap = new HashMap<>();
    faceLandmarkTypesMap.put("MOUTH_BOTTOM", FaceLandmark.MOUTH_BOTTOM);
    faceLandmarkTypesMap.put("MOUTH_RIGHT", FaceLandmark.MOUTH_RIGHT);
    faceLandmarkTypesMap.put("MOUTH_LEFT", FaceLandmark.MOUTH_LEFT);
    faceLandmarkTypesMap.put("RIGHT_EYE", FaceLandmark.RIGHT_EYE);
    faceLandmarkTypesMap.put("LEFT_EYE", FaceLandmark.LEFT_EYE);
    faceLandmarkTypesMap.put("RIGHT_EAR", FaceLandmark.RIGHT_EAR);
    faceLandmarkTypesMap.put("LEFT_EAR", FaceLandmark.LEFT_EAR);
    faceLandmarkTypesMap.put("RIGHT_CHEEK", FaceLandmark.RIGHT_CHEEK);
    faceLandmarkTypesMap.put("LEFT_CHEEK", FaceLandmark.LEFT_CHEEK);
    faceLandmarkTypesMap.put("NOSE_BASE", FaceLandmark.NOSE_BASE);

    WritableMap faceLandmarksMap = new WritableNativeMap();

    for (Map.Entry<String, Integer> entry : faceLandmarkTypesMap.entrySet()) {
      String faceLandmark = entry.getKey();
      int faceLandmarkTypeID = entry.getValue();

      FaceLandmark landmark = face.getLandmark(faceLandmarkTypeID);

      float positionX = landmark != null ? landmark.getPosition().x : -1f;
      float positionY = landmark != null ? landmark.getPosition().y : -1f;

      WritableMap currentPointsMap = new WritableNativeMap();
      currentPointsMap.putDouble("x", positionX);
      currentPointsMap.putDouble("y", positionY);

      faceLandmarksMap.putMap(faceLandmark, currentPointsMap);
    }

    return faceLandmarksMap;
  }

  @SuppressLint("NewApi")
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
          WritableMap map = new WritableNativeMap();

          map.putDouble("rollAngle", face.getHeadEulerAngleZ()); // Head is rotated to the left rotZ degrees
          map.putDouble("pitchAngle", face.getHeadEulerAngleX()); // Head is rotated to the right rotX degrees
          map.putDouble("yawAngle", face.getHeadEulerAngleY());  // Head is tilted sideways rotY degrees
          map.putDouble("leftEyeOpenProbability", face.getLeftEyeOpenProbability() == null ? -1f : face.getLeftEyeOpenProbability());
          map.putDouble("rightEyeOpenProbability", face.getRightEyeOpenProbability() == null ? -1f : face.getRightEyeOpenProbability());
          map.putDouble("smilingProbability", face.getSmilingProbability() == null ? -1f : face.getSmilingProbability());

          WritableMap contours = processFaceContours(face);
          WritableMap bounds = processBoundingBox(face.getBoundingBox());
          WritableMap landmarks = processFaceLandmarks(face);

          map.putMap("bounds", bounds);
          map.putMap("contours", contours);
          map.putMap("landmarks", landmarks);

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
