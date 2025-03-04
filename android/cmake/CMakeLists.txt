cmake_minimum_required(VERSION 3.4.1)
project(VisionCameraFaceDetector)
set (PACKAGE_NAME "faceDetector")

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_STANDARD 20)


include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../externals/raylib/src/
)

find_package(ReactAndroid REQUIRED CONFIG)
find_package(fbjni REQUIRED CONFIG)

add_custom_target(apply_raylib_patch ALL
    COMMAND ${CMAKE_COMMAND} -E env bash ${CMAKE_CURRENT_SOURCE_DIR}/apply_patch.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/../../externals/raylib/src/rcore.c
    COMMENT "pathing raylib import for android in rcore.c"
)


add_library(${PACKAGE_NAME}
        SHARED

        ${CMAKE_CURRENT_SOURCE_DIR}/../../externals/raylib/src/rcore.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../externals/raylib/src/rmodels.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../externals/raylib/src/rshapes.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../externals/raylib/src/rtext.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../externals/raylib/src/rtextures.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../externals/raylib/src/utils.c

        # our custom rcore implementation
        ../../cpp/platforms/rcore_android.c
        ../../cpp/RaylibSurface.cpp
        ../../cpp/FaceDetectorView.cpp
        ../../cpp/JniOnLoad.cpp

)

target_include_directories(
        ${PACKAGE_NAME}
        PUBLIC
        "${NODE_MODULES_DIR}/react-native/ReactCommon"
        "${NODE_MODULES_DIR}/react-native/ReactCommon/callinvoker"
        "${NODE_MODULES_DIR}/react-native/ReactAndroid/src/main/jni/react/turbomodule"
)

target_link_libraries(
        ${PACKAGE_NAME}
        android
        log
        EGL
        GLESv2
        ReactAndroid::jsi
        ReactAndroid::reactnativejni
        fbjni::fbjni
)

add_dependencies(${PACKAGE_NAME} apply_raylib_patch)

# Define compiler macros for raylib
target_compile_definitions(${PACKAGE_NAME} PRIVATE
        GRAPHICS_API_OPENGL_ES2
        PLATFORM_ANDROID
        __ANDROID__
)
