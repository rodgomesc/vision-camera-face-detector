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
  
    private static func processContours(from faces: [Face]?) -> [String:CGFloat] {
    // TODO: implement face contour calculations
    return [:]
  }
    
    private static func processBoundingBox(from faces: [Face]?) -> [String:CGFloat] {
    // TODO: implement bounding box calculations
    return [:]
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
               
                map["rollAngle"] = face.headEulerAngleX  // Head is rotated to the uptoward rotX degrees
                map["pitchAngle"] = face.headEulerAngleY // Head is rotated to the right rotY degrees
                map["yawAngle"] = face.headEulerAngleZ  // Head is tilted sideways rotZ degrees
                map["leftEyeOpenProbability"] = face.leftEyeOpenProbability
                map["rightEyeOpenProbability"] = face.rightEyeOpenProbability
                map["smilingProbability"] = face.smilingProbability
                map["bounds"] = processBoundingBox(from: faces)
                map["cotours"] = processContours(from: faces)

                faceAttributes.append(map)
            }
       }
       } catch _ {
           return nil
       }
    return faceAttributes
  }
}
