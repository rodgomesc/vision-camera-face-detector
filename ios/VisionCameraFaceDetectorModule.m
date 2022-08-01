//
//  VisionCameraFaceDetectorModule.m
//  vision-camera-face-detector
//
//  Created by Yudi Edri Alviska on 28/07/22.
//
#import <React/RCTBridgeModule.h>

@interface RCT_EXTERN_MODULE(VisionCameraFaceDetectorModule, NSObject)

RCT_EXTERN_METHOD(detectFromBase64:(NSString)imageString
                  withResolver:(RCTPromiseResolveBlock)resolve
                  withRejecter:(RCTPromiseRejectBlock)reject)


+ (BOOL)requiresMainQueueSetup
{
  return NO;
}

@end
