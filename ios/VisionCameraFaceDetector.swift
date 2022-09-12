import Vision
import MLKitFaceDetection
import MLKitVision
import CoreML

import UIKit
import AVFoundation

@objc(VisionCameraFaceDetector)
public class VisionCameraFaceDetector: NSObject, FrameProcessorPluginBase {
    static var FaceDetectorOption: FaceDetectorOptions = {
        let option = FaceDetectorOptions()
        option.contourMode = .all
        option.classificationMode = .all
        option.landmarkMode = .all
        option.performanceMode = .accurate // doesn't work in fast mode!, why?
        return option
    }()
    
    static var faceDetector = FaceDetector.faceDetector(options: FaceDetectorOption)
    
    private static func processContours(from face: Face) -> [String:[[String:CGFloat]]] {
      let faceContoursTypes = [
        FaceContourType.face,
        FaceContourType.leftEyebrowTop,
        FaceContourType.leftEyebrowBottom,
        FaceContourType.rightEyebrowTop,
        FaceContourType.rightEyebrowBottom,
        FaceContourType.leftEye,
        FaceContourType.rightEye,
        FaceContourType.upperLipTop,
        FaceContourType.upperLipBottom,
        FaceContourType.lowerLipTop,
        FaceContourType.lowerLipBottom,
        FaceContourType.noseBridge,
        FaceContourType.noseBottom,
        FaceContourType.leftCheek,
        FaceContourType.rightCheek,
      ]
      
      let faceContoursTypesStrings = [
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
        "RIGHT_CHEEK",
      ];
      
      var faceContoursTypesMap: [String:[[String:CGFloat]]] = [:]
      
      for i in 0..<faceContoursTypes.count {
        let contour = face.contour(ofType: faceContoursTypes[i]);
        
        var pointsArray: [[String:CGFloat]] = []
        
        if let points = contour?.points {
          for point in points {
            let currentPointsMap = [
                "x": point.x,
                "y": point.y,
            ]
            
            pointsArray.append(currentPointsMap)
          }
          
          faceContoursTypesMap[faceContoursTypesStrings[i]] = pointsArray
        }
      }
      
      return faceContoursTypesMap
    }
    
    private static func processFaceLandmarks(from face: Face) -> [String:CGPoint] {
        
        let faceLandmarkTypesMap = [
            "MOUTH_BOTTOM" : FaceLandmarkType.mouthBottom,
            "MOUTH_RIGHT" : FaceLandmarkType.mouthRight,
            "MOUTH_LEFT" : FaceLandmarkType.mouthLeft,
            "RIGHT_EYE" : FaceLandmarkType.rightEye,
            "LEFT_EYE" : FaceLandmarkType.leftEye,
            "RIGHT_EAR" : FaceLandmarkType.rightEar,
            "LEFT_EAR" : FaceLandmarkType.leftEar,
            "RIGHT_CHEEK" : FaceLandmarkType.rightCheek,
            "LEFT_CHEEK" : FaceLandmarkType.leftCheek,
            "NOSE_BASE" : FaceLandmarkType.noseBase,
        ]
        
      
//      let faceContoursTypesStrings = [
//        "FACE",
//        "LEFT_EYEBROW_TOP",
//        "LEFT_EYEBROW_BOTTOM",
//        "RIGHT_EYEBROW_TOP",
//        "RIGHT_EYEBROW_BOTTOM",
//        "LEFT_EYE",
//        "RIGHT_EYE",
//        "UPPER_LIP_TOP",
//        "UPPER_LIP_BOTTOM",
//        "LOWER_LIP_TOP",
//        "LOWER_LIP_BOTTOM",
//        "NOSE_BRIDGE",
//        "NOSE_BOTTOM",
//        "LEFT_CHEEK",
//        "RIGHT_CHEEK",
//      ];
        
        var faceLandmarkTypes: [String:CGPoint] = [:]
        
        for (key, value) in faceLandmarkTypesMap {
            
            let faceLandmarks = face.landmark(ofType: value);
//            var pointsArray: [[String:CGFloat]] = []
            
//            let currentPointsMap: [String:CGFloat?] = [
//                "x": faceLandmarks?.position.x,
//                "y": faceLandmarks?.position.y,
//            ]
            
            let currentPointsMap: CGPoint = CGPoint(
                x: faceLandmarks?.position.x != nil ? faceLandmarks!.position.x : -1,
                y: faceLandmarks?.position.y != nil ? faceLandmarks!.position.y : -1)
            
            faceLandmarkTypes[key] = currentPointsMap
            
        }
      
      return faceLandmarkTypes
    }
    
    private static func processBoundingBox(from face: Face) -> [String:Any] {
        let frameRect = face.frame

        let offsetX = (frameRect.midX - ceil(frameRect.width)) / 2.0
        let offsetY = (frameRect.midY - ceil(frameRect.height)) / 2.0

        let x = frameRect.maxX + offsetX
        let y = frameRect.minY + offsetY

        return [
          "x": frameRect.midX + (frameRect.midX - x),
          "y": frameRect.midY + (y - frameRect.midY),
          "width": frameRect.width,
          "height": frameRect.height,
          "boundingCenterX": frameRect.midX,
          "boundingCenterY": frameRect.midY
        ]
    }
    
    @objc
    public static func callback(_ frame: Frame!, withArgs _: [Any]!) -> Any! {
        let image = VisionImage(buffer: frame.buffer)
        image.orientation = .up
        
        var faceAttributes: [Any] = []
        
        do {
            let faces: [Face] =  try faceDetector.results(in: image)
            if (!faces.isEmpty){
                for face in faces {
                    var map: [String: Any] = [:]
                    
                    map["rollAngle"] = face.headEulerAngleZ  // Head is tilted sideways rotZ degrees
                    map["pitchAngle"] = face.headEulerAngleX  // Head is rotated to the uptoward rotX degrees
                    map["yawAngle"] = face.headEulerAngleY   // Head is rotated to the right rotY degrees
                    map["leftEyeOpenProbability"] = face.leftEyeOpenProbability
                    map["rightEyeOpenProbability"] = face.rightEyeOpenProbability
                    map["smilingProbability"] = face.smilingProbability
                    map["bounds"] = processBoundingBox(from: face)
                    map["contours"] = processContours(from: face)
                    map["landmarks"] = processFaceLandmarks(from: face)
                    
                    faceAttributes.append(map)
                }
            }
        } catch _ {
            return nil
        }
        return faceAttributes
    }
}
