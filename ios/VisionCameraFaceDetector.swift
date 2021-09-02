import Vision
import MLKit


@objc(VisionCameraFaceDetector)
public class VisionCameraFaceDetector: NSObject, FrameProcessorPluginBase {
  let options = FaceDetectorOptions()
   
  @objc
  public static func callback(_ frame: Frame!, withArgs _: [Any]!) -> Any! {
    let image = VisionImage(frame: UIImage)
    visionImage.orientation = image.imageOrientation
    
    let faceDetector = FaceDetector.faceDetector(options: options)
    var faces: [Face]
    
    do {
         faces = try faceDetector.process(visionImage)
       } catch let error {
            return nil
       }

    
    

    // guard let imageBuffer = CMSampleBufferGetImageBuffer(frame.buffer) else {
    //   return nil
    // }

    // NSLog("ExamplePlugin!!!")
    
    
    // let orientation = frame.orientation
    // code goes here
     return faces
  }
}
