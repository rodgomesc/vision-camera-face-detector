/* eslint-disable react-native/no-inline-styles */
import * as React from 'react';
import { runOnJS } from 'react-native-reanimated';

import { StyleSheet, View, Text } from 'react-native';
import {
  useCameraDevices,
  useFrameProcessor,
} from 'react-native-vision-camera';

import { Camera } from 'react-native-vision-camera';
import { scanFaces, Face } from 'vision-camera-face-detector';

export default function App() {
  const [hasPermission, setHasPermission] = React.useState(false);
  const [faces, setFaces] = React.useState<Face[]>([]);
  const devices = useCameraDevices();
  const device = devices.back;

  React.useEffect(() => {
    (async () => {
      const status = await Camera.requestCameraPermission();
      setHasPermission(status === 'authorized');
    })();
  }, []);

  React.useEffect(() => {
    if (faces.length > 0) {
      console.log(JSON.stringify(faces[0]));
    }
  }, [faces]);

  const frameProcessor = useFrameProcessor((frame) => {
    'worklet';
    const scannedFaces = scanFaces(frame);

    runOnJS(setFaces)(scannedFaces);
  }, []);

  return (
    device != null &&
    hasPermission && (
      <>
        <Camera
          style={StyleSheet.absoluteFill}
          device={device}
          isActive={true}
          frameProcessor={frameProcessor}
          frameProcessorFps={30}
        />
        {faces.length ? (
          <>
            <View
              style={[
                styles.faceFrame,
                {
                  top: faces[0].bounds.y,
                  left: faces[0].bounds.x,
                  width: faces[0].bounds.width,
                  height: faces[0].bounds.height,
                },
              ]}
            />
            <View
              style={[
                styles.faceFrame,

                {
                  top: faces[0].bounds.boundingCenterY,
                  left: faces[0].bounds.boundingCenterX,
                  width: 2,
                  height: 2,
                },
              ]}
            />
            <View
              style={[
                styles.faceFrame,

                {
                  top: faces[0].bounds.boundingExactCenterX,
                  left: faces[0].bounds.boundingExactCenterY,
                  width: 2,
                  height: 2,
                  backgroundColor: 'blue',
                  borderColor: 'red',
                },
              ]}
            />
            {/* {faces[0].contours.FACE.map((point, i) => (
              <View
                key={i}
                style={[
                  styles.faceFrame,

                  {
                    top: point.y,
                    left: point.x - 40,
                    width: 2,
                    height: 2,
                  },
                ]}
              />
            ))} */}
            {/* {faces[0].contours.RIGHT_EYE.map((point, i) => (
              <View
                key={i}
                style={[
                  styles.faceFrame,
                  {
                    top: point.y,
                    left: point.x,
                    width: 2,
                    height: 2,
                  },
                ]}
              />
            ))}
            {faces[0].contours.LEFT_EYE.map((point, i) => (
              <View
                key={i}
                style={[
                  styles.faceFrame,
                  {
                    top: point.y,
                    left: point.x,
                    width: 2,
                    height: 2,
                  },
                ]}
              />
            ))}
            {faces[0].contours.UPPER_LIP_BOTTOM.map((point, i) => (
              <View
                key={i}
                style={[
                  styles.faceFrame,
                  {
                    top: point.y,
                    left: point.x,
                    width: 2,
                    height: 2,
                  },
                ]}
              />
            ))}
            {faces[0].contours.UPPER_LIP_TOP.map((point, i) => (
              <View
                key={i}
                style={[
                  styles.faceFrame,
                  {
                    top: point.y,
                    left: point.x,
                    width: 2,
                    height: 2,
                  },
                ]}
              />
            ))}
            {faces[0].contours.LOWER_LIP_BOTTOM.map((point, i) => (
              <View
                key={i}
                style={[
                  styles.faceFrame,
                  {
                    top: point.y,
                    left: point.x,
                    width: 2,
                    height: 2,
                  },
                ]}
              />
            ))}
            {faces[0].contours.UPPER_LIP_BOTTOM.map((point, i) => (
              <View
                key={i}
                style={[
                  styles.faceFrame,
                  {
                    top: point.y,
                    left: point.x,
                    width: 2,
                    height: 2,
                  },
                ]}
              />
            ))}
            {faces[0].contours.NOSE_BRIDGE.map((point, i) => (
              <View
                key={i}
                style={[
                  styles.faceFrame,
                  {
                    top: point.y,
                    left: point.x,
                    width: 2,
                    height: 2,
                  },
                ]}
              />
            ))} */}
            <Text style={styles.text}>
              Left eye Open: {faces[0].leftEyeOpenProbability}
            </Text>
            <Text style={[styles.text, { bottom: 30 }]}>
              Right eye Open: {faces[0].rightEyeOpenProbability}
            </Text>
          </>
        ) : null}
      </>
    )
  );
}

const styles = StyleSheet.create({
  faceFrame: {
    borderWidth: 1,

    borderStyle: 'solid',
    position: 'absolute',
  },
  text: {
    position: 'absolute',
    bottom: 10,
    color: 'yellow',
  },
});
