import Vision
import MLKitFaceDetection
import MLKitVision


@objc(VisionCameraFaceDetector)
public class VisionCameraFaceDetector: NSObject, FrameProcessorPluginBase {


  var FaceDetectorOption: FaceDetectorOptions = {
    let option = FaceDetectorOptions()
    option.contourMode = .all
    option.performanceMode = .fast
    return option
  }()

  private lazy var faceDetector = FaceDetector.faceDetector(options: FaceDetectorOption)
  
    
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
    var faces: [Face]

     
    do {
       faces =  faceDetector.results(in: image)
        
        if (!faces.isEmpty){
            var processedFaces = processFace(from: faces)
            return processedFaces
        }
    } catch let error {
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
