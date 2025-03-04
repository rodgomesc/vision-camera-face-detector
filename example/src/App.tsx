import * as React from 'react';

import { StyleSheet, Text, View } from 'react-native';
import { FaceDetectorView } from 'vision-camera-face-detector';

export default function App() {
  return (
    <View style={styles.container}>
      <Text>AEEEEEEEEEEE</Text>
      <FaceDetectorView
        style={{
          width: '100%',
          height: '100%',
          borderWidth: 4,
          borderStyle: 'solid',
          borderColor: 'yellow',
        }}
      />
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
  },
  box: {
    width: 60,
    height: 60,
    marginVertical: 20,
  },
});
