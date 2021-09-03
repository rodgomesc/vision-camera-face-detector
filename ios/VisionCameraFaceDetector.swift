import Vision
import MLKitFaceDetection
import MLKitVision
import CoreML



@objc(VisionCameraFaceDetector)
public class VisionCameraFaceDetector: NSObject, FrameProcessorPluginBase {


  static var FaceDetectorOption: FaceDetectorOptions = {
    let option = FaceDetectorOptions()
    option.contourMode = .all
    option.performanceMode = .fast
    return option
  }()

  static var faceDetector = FaceDetector.faceDetector(options: FaceDetectorOption)
  
    
  private static func processFace(from faces: [Face]?) -> Any  {
    guard let faces = faces else {
        return []
    }
    var faceMap: [Any] = []
    
    for face in faces {
        var faceCharacteristics: [String: CGFloat] = [:]
        
        if face.hasHeadEulerAngleX {
           faceCharacteristics["rotX"] = face.headEulerAngleX  // Head is rotated to the uptoward rotX degrees
         }
         if face.hasHeadEulerAngleY {
            faceCharacteristics["rotY"] = face.headEulerAngleY // Head is rotated to the right rotY degrees
         }
         if face.hasHeadEulerAngleZ {
            faceCharacteristics["rotZ"] = face.headEulerAngleZ  // Head is tilted sideways rotZ degrees
         }
        
        faceMap.append(faceCharacteristics)
    }
    
    return faceMap
  }
    
  @objc
  public static func callback(_ frame: Frame!, withArgs _: [Any]!) -> Any! {
    let image = VisionImage.init(buffer: frame.buffer)
    image.orientation = frame.orientation
    var faces:  [Face]
            
    do {
        faces =  try faceDetector.results(in: image)
           if (!faces.isEmpty){
               let processedFaces = processFace(from: faces)
               return processedFaces
           }
       } catch _ {
           return []
       }

    

    // guard let imageBuffer = CMSampleBufferGetImageBuffer(frame.buffer) else {
    //   return nil
    // }

    NSLog("ExamplePlugin!!!")
    
    
    // let orientation = frame.orientation
    // code goes here
     return []
  }
}
