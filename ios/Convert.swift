//
//  Convert.swift
//  vision-camera-face-detector
//
//  Created by Yudi Edri Alviska on 28/07/22.
//

import Foundation
import UIKit
import MLImage

public func getImageFaceFromBuffer(from sampleBuffer: CMSampleBuffer?, rectImage: CGRect) -> UIImage? {
    guard let sampleBuffer = sampleBuffer else {
        print("Sample buffer is NULL.")
        return nil
    }
    guard let imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer) else {
        print("Invalid sample buffer.")
        return nil
    }
    let ciimage = CIImage(cvPixelBuffer: imageBuffer)
    let context = CIContext(options: nil)
    let cgImage = context.createCGImage(ciimage, from: ciimage.extent)!
    
    if (!rectImage.isNull) {
        let imageRef: CGImage = cgImage.cropping(to: rectImage)!
        let imageCrop: UIImage = UIImage(cgImage: imageRef, scale: 0.5, orientation: .right)
        return imageCrop
    } else {
        return nil
    }
}

public func getImageFaceFromUIImage(from image: UIImage, rectImage: CGRect) -> UIImage? {
    let imageRef: CGImage = (image.cgImage?.cropping(to: rectImage)!)!
    let imageCrop: UIImage = UIImage(cgImage: imageRef, scale: 0.5, orientation: image.imageOrientation)
    return imageCrop
}

public func convertImageToBase64(image: UIImage) -> String {
    let imageData = image.pngData()!
    return imageData.base64EncodedString()
}

public func convertBase64ToImage(strBase64: String) -> UIImage? {
    let dataDecoded : Data = Data(base64Encoded: strBase64, options: .ignoreUnknownCharacters)!
    return UIImage(data: dataDecoded)
}
