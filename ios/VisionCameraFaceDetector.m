/*#import <Foundation/Foundation.h>
#import <VisionCamera/FrameProcessorPlugin.h>

@interface VISION_EXPORT_SWIFT_FRAME_PROCESSOR(scanFaces, VisionCameraFaceDetector)
@end

*/

#import <Foundation/Foundation.h>
#import <VisionCamera/FrameProcessorPlugin.h>
#import <VisionCamera/FrameProcessorPluginRegistry.h>
#import <VisionCamera/Frame.h>
#import "VisionCameraFaceDetector-Swift.h"


@interface VisionCameraFaceDetector (FrameProcessorPluginLoader)
@end

@implementation VisionCameraFaceDetector (FrameProcessorPluginLoader)

+ (void)load
{
    [FrameProcessorPluginRegistry addFrameProcessorPlugin:@"scanFaces"
                                        withInitializer:^FrameProcessorPlugin* (VisionCameraProxyHolder* proxy, NSDictionary* options) {
        return [[VisionCameraFaceDetector alloc] initWithProxy:proxy options:options];
    }];
}

@end

