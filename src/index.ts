import type { Frame } from 'react-native-vision-camera';

/**
 * Scans QR codes.
 */

export interface Face {
  title: string;
  url: string;
}

export function scanFaces(frame: Frame): Face[] {
  'worklet';
  return __scanFaces(frame);
}
