package com.visioncamerafacedetector;

import static com.visioncamerafacedetector.Constant.TF_OD_API_INPUT_SIZE;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.RectF;
import android.util.Base64;

import androidx.annotation.NonNull;

import com.facebook.react.bridge.Promise;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactMethod;
import com.facebook.react.module.annotations.ReactModule;
import com.google.android.gms.tasks.Task;
import com.google.android.gms.tasks.Tasks;
import com.google.mlkit.vision.common.InputImage;
import com.google.mlkit.vision.face.Face;
import com.google.mlkit.vision.face.FaceDetection;
import com.google.mlkit.vision.face.FaceDetector;
import com.google.mlkit.vision.face.FaceDetectorOptions;

import java.util.List;

@ReactModule(name = VisionCameraFaceDetectorModule.NAME)
public class VisionCameraFaceDetectorModule extends ReactContextBaseJavaModule {
  public static final String NAME = "VisionCameraFaceDetectorModule";

  FaceDetectorOptions options =
    new FaceDetectorOptions.Builder()
      .setPerformanceMode(FaceDetectorOptions.PERFORMANCE_MODE_ACCURATE)
      .setContourMode(FaceDetectorOptions.CONTOUR_MODE_ALL)
      .setClassificationMode(FaceDetectorOptions.CLASSIFICATION_MODE_ALL)
      .setMinFaceSize(0.15f)
      .build();

  FaceDetector faceDetector = FaceDetection.getClient(options);

  public VisionCameraFaceDetectorModule(ReactApplicationContext reactContext) {
    super(reactContext);
  }

  @Override
  @NonNull
  public String getName() {
    return NAME;
  }

  @ReactMethod
  public void detectFromBase64(String imageString, Promise promise) {
    try {
      byte[] decodedString = Base64.decode(imageString, Base64.DEFAULT);
      Bitmap bmpStorageResult = BitmapFactory.decodeByteArray(decodedString, 0, decodedString.length);
      InputImage image = InputImage.fromBitmap(bmpStorageResult, 0);
      Task<List<Face>> task = faceDetector.process(image);
      List<Face> faces = Tasks.await(task);
      if (faces.size() > 0) {
        for (Face face : faces) {
          Bitmap bmpFaceStorage = Bitmap.createBitmap(TF_OD_API_INPUT_SIZE, TF_OD_API_INPUT_SIZE, Bitmap.Config.ARGB_8888);
          final RectF faceBB = new RectF(face.getBoundingBox());
          final Canvas cvFace = new Canvas(bmpFaceStorage);
          float sx = ((float) TF_OD_API_INPUT_SIZE) / faceBB.width();
          float sy = ((float) TF_OD_API_INPUT_SIZE) / faceBB.height();
          Matrix matrix = new Matrix();
          matrix.postTranslate(-faceBB.left, -faceBB.top);
          matrix.postScale(sx, sy);
          cvFace.drawBitmap(bmpStorageResult, matrix, null);
          promise.resolve(new Convert().getBase64Image(bmpFaceStorage));
        }
      } else {
        promise.resolve("");
      }
    } catch (Exception e) {
      e.printStackTrace();
      promise.reject(new Throwable(e));
    }
  }
}
