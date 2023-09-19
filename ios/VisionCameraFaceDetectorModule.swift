//
//  VisionCameraFaceDetectorModule.swift
//  vision-camera-face-detector
//
//  Created by Yudi Edri Alviska on 28/07/22.
//

import Vision
import MLKitFaceDetection
import MLKitVision
import CoreML
import UIKit

@objc(VisionCameraFaceDetectorModule)
class VisionCameraFaceDetectorModule: NSObject {
    
    static var FaceDetectorOption: FaceDetectorOptions = {
        let option = FaceDetectorOptions()
        option.performanceMode = .accurate
        return option
    }()
    
    static var faceDetector = FaceDetector.faceDetector(options: FaceDetectorOption)
    
    @objc(detectFromBase64:withResolver:withRejecter:)
    func detectFromBase64(imageString: String, resolve: @escaping RCTPromiseResolveBlock, reject: @escaping RCTPromiseRejectBlock) -> Void {
        let stringData = Data(base64Encoded: imageString) ?? nil
        let uiImage = UIImage(data: stringData!)
        
        if (uiImage != nil) {
            let image = VisionImage(image: uiImage!)
            do {
                let faces: [Face] =  try VisionCameraFaceDetectorModule.faceDetector.results(in: image)
                if (!faces.isEmpty){
                    for face in faces {
                        let faceFrame = face.frame
                        let imageCrop = getImageFaceFromUIImage(from: uiImage!, rectImage: faceFrame)
                        resolve(convertImageToBase64(image:imageCrop!))
                        return
                    }
                } else {
                    resolve("")
                }
            } catch {
                reject("Error", error.localizedDescription, error)
            }
        }
    }
}
