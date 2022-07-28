//
//  Convert.swift
//  vision-camera-face-detector
//
//  Created by Yudi Edri Alviska on 28/07/22.
//

import Foundation
import UIKit
import MLImage

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
