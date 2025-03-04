#!/bin/sh

echo "patching raylib to use react_native_rcore.c"

TARGET_FILE=$1
OLD_STRING='#include "platforms/rcore_android.c"'
NEW_STRING='//#include "platforms/rcore_android.c"'

sed -i '' "s|${OLD_STRING}|${NEW_STRING}|g" "$TARGET_FILE"

if [ $? -ne 0 ]; then
    echo "Failed to replace string"
    exit 1
else
    if grep -q "${NEW_STRING}" "$TARGET_FILE"; then
        echo "String replaced successfully"
    else
        echo "Pattern not found, but continuing"
    fi
fi

# Copy our custom react_native_rcore.h to the raylib source directory
echo "Copying react_native_rcore.h to raylib source directory"
cp "${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/react_native_rcore.h" "${CMAKE_CURRENT_SOURCE_DIR}/../../externals/raylib/src/"

echo "Patch applied successfully"
