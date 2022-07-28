//
//  VisionCameraFaceDetectorModule.swift
//  vision-camera-face-detector
//
//  Created by Yudi Edri Alviska on 28/07/22.
//

import Foundation

@objc(VisionCameraFaceDetectorModule)
class VisionCameraFaceDetectorModule: NSObject {
    
    @objc(detectFromBase64:withResolver:withRejecter:)
    func detectFromBase64(imageString: String, resolve:RCTPromiseResolveBlock,reject:RCTPromiseRejectBlock) -> Void {
        resolve("detectFromBase64")
    }
}
