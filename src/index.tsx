import { NativeModules } from 'react-native';

type VisionCameraFaceDetectorType = {
  multiply(a: number, b: number): Promise<number>;
};

const { VisionCameraFaceDetector } = NativeModules;

export default VisionCameraFaceDetector as VisionCameraFaceDetectorType;
