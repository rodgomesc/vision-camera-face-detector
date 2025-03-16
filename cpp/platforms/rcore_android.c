/**********************************************************************************************
 *
 *   rcore_android - Functions to manage window, graphics device and inputs
 *
 *   PLATFORM: ANDROID
 *       - Android (ARM, ARM64)
 *
 *   LIMITATIONS:
 *       - Surface-based rendering using ANativeWindow
 *
 *   DEPENDENCIES:
 *       - Android NDK: Provides C API to access Android functionality
 *
 *   Copyright (c) 2024 rodgomesc (@rodgomesc)
 *
 **********************************************************************************************/

#include <android/log.h>
#include <android/native_window.h>
#include <time.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "raylib.h"
#include "rlgl.h"

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
   bool appEnabled;                    // Flag to detect if app is active ** = true
   bool contextRebindRequired;         // Used to know context rebind required

    EGLDisplay device;          // Native display device (physical screen connection)
    EGLSurface surface;         // Surface to draw on, framebuffers (connected to context)
    EGLContext context;         // Graphic context, mode in which drawing can be done
    EGLConfig config;           // Graphic config
    ANativeWindow* window;      // Native window
} PlatformData;

// Define required types that are used in CoreData but not exposed in raylib.h
typedef struct { int x; int y; } Point;
typedef struct { unsigned int width; unsigned int height; } Size;

// Core global state context data
typedef struct CoreData {
    struct {
        const char *title;                  // Window text title const pointer
        unsigned int flags;                 // Configuration flags (bit based), keeps window state
        bool ready;                         // Check if window has been initialized successfully
        bool fullscreen;                    // Check if fullscreen mode is enabled
        bool shouldClose;                   // Check if window set for closing
        bool resizedLastFrame;              // Check if window has been resized last frame
        bool eventWaiting;                  // Wait for events before ending frame
        bool usingFbo;                      // Using FBO (RenderTexture) for rendering instead of default framebuffer

        Point position;                     // Window position (required on fullscreen toggle)
        Point previousPosition;             // Window previous position (required on borderless windowed toggle)
        Size display;                       // Display width and height (monitor, device-screen, LCD, ...)
        Size screen;                        // Screen width and height (used render area)
        Size previousScreen;                // Screen previous width and height (required on borderless windowed toggle)
        Size currentFbo;                    // Current render width and height (depends on active fbo)
        Size render;                        // Framebuffer width and height (render area, including black bars if required)
        Point renderOffset;                 // Offset from render area (must be divided by 2)
        Size screenMin;                     // Screen minimum width and height (for resizable window)
        Size screenMax;                     // Screen maximum width and height (for resizable window)
        Matrix screenScale;                 // Matrix to scale screen (framebuffer rendering)

        char **dropFilepaths;               // Store dropped files paths pointers (provided by GLFW)
        unsigned int dropFileCount;         // Count dropped files strings

    } Window;
    struct {
        const char *basePath;               // Base path for data storage

    } Storage;
    struct {
        struct {
            int exitKey;                    // Default exit key
            char currentKeyState[512];      // Registers current frame key state
            char previousKeyState[512];     // Registers previous frame key state

            // NOTE: Since key press logic involves comparing prev vs cur key state, we need to handle key repeats specially
            char keyRepeatInFrame[512];     // Registers key repeats for current frame

            int keyPressedQueue[16];        // Input keys queue
            int keyPressedQueueCount;       // Input keys queue count

            int charPressedQueue[16];       // Input characters queue (unicode)
            int charPressedQueueCount;      // Input characters queue count

        } Keyboard;
        struct {
            Vector2 offset;                 // Mouse offset
            Vector2 scale;                  // Mouse scaling
            Vector2 currentPosition;        // Mouse position on screen
            Vector2 previousPosition;       // Previous mouse position

            int cursor;                     // Tracks current mouse cursor
            bool cursorHidden;              // Track if cursor is hidden
            bool cursorOnScreen;            // Tracks if cursor is inside client area

            char currentButtonState[3];     // Registers current mouse button state
            char previousButtonState[3];    // Registers previous mouse button state
            Vector2 currentWheelMove;       // Registers current mouse wheel variation
            Vector2 previousWheelMove;      // Registers previous mouse wheel variation

        } Mouse;
        struct {
            int pointCount;                 // Number of touch points active
            int pointId[10];                // Point identifiers
            Vector2 position[10];           // Touch position on screen
            char currentTouchState[10];     // Registers current touch state
            char previousTouchState[10];    // Registers previous touch state

        } Touch;
        struct {
            int lastButtonPressed;          // Register last gamepad button pressed
            int axisCount[4];               // Register number of available gamepad axis
            bool ready[4];                  // Flag to know if gamepad is ready
            char name[4][64];               // Gamepad name holder
            char currentButtonState[4][32]; // Current gamepad buttons state
            char previousButtonState[4][32];// Previous gamepad buttons state
            float axisState[4][8];          // Gamepad axis state

        } Gamepad;
    } Input;
    struct {
        double current;                     // Current time measure
        double previous;                    // Previous time measure
        double update;                      // Time measure for frame update
        double draw;                        // Time measure for frame draw
        double frame;                       // Time measure for one frame
        double target;                      // Desired time for one frame, if 0 not applied
        unsigned long long int base;        // Base time measure for hi-res timer (PLATFORM_ANDROID, PLATFORM_DRM)
        unsigned int frameCounter;          // Frame counter

    } Time;
} CoreData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
extern CoreData CORE;   // Global CORE state context
static PlatformData platform = { 0 };   // Platform specific data


// Add font loading function declaration from rtext.c to rcore_android.c
extern void LoadFontDefault(void);
extern void UnloadFontDefault(void);


//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------

static void SetupViewport(int width, int height);         // Setup viewport

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

// Initialize platform
int InitPlatform(void)
{

  // Initialize display basic configuration
  //----------------------------------------------------------------------------
  CORE.Window.currentFbo.width = CORE.Window.screen.width;
  CORE.Window.currentFbo.height = CORE.Window.screen.height;

   // Set some default window flags
  CORE.Window.flags &= ~FLAG_WINDOW_HIDDEN;       // false
  CORE.Window.flags &= ~FLAG_WINDOW_MINIMIZED;    // false
  CORE.Window.flags |= FLAG_WINDOW_MAXIMIZED;     // true
  CORE.Window.flags &= ~FLAG_WINDOW_UNFOCUSED;    // false

  return 0;
}

int SetupFramebuffer() {
    CORE.Window.renderOffset.x = 0;
    CORE.Window.renderOffset.y = CORE.Window.display.height;
}

// Initialize display device and framebuffer
// NOTE: width and height represent the screen (framebuffer) desired size, not actual display size
// If width or height are 0, default display size will be used for framebuffer size
// NOTE: returns false in case graphic device could not be created
static int InitGraphicsDevice(void)
{
    CORE.Window.fullscreen = true;
    CORE.Window.flags |= FLAG_FULLSCREEN_MODE;

    EGLint samples = 0;
    EGLint sampleBuffer = 0;
    if (CORE.Window.flags & FLAG_MSAA_4X_HINT)
    {
        samples = 4;
        sampleBuffer = 1;
        TRACELOG(LOG_INFO, "DISPLAY: Trying to enable MSAA x4");
    }

    const EGLint framebufferAttribs[] =
            {
                    EGL_RENDERABLE_TYPE, (rlGetVersion() == RL_OPENGL_ES_30)? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT,
                    EGL_RED_SIZE, 8,
                    EGL_GREEN_SIZE, 8,
                    EGL_BLUE_SIZE, 8,
                    EGL_ALPHA_SIZE, 8,
                    EGL_DEPTH_SIZE, 16,
                    EGL_STENCIL_SIZE, 8,
                    EGL_SAMPLE_BUFFERS, sampleBuffer,
                    EGL_SAMPLES, samples,
                    EGL_NONE
            };

    const EGLint contextAttribs[] =
            {
                    EGL_CONTEXT_CLIENT_VERSION, 2,
                    EGL_NONE
            };

    EGLint numConfigs = 0;

    platform.device = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (platform.device == EGL_NO_DISPLAY)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to initialize EGL device");
        return -1;
    }

    if (eglInitialize(platform.device, NULL, NULL) == EGL_FALSE)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to initialize EGL device");
        return -1;
    }

    eglChooseConfig(platform.device, framebufferAttribs, &platform.config, 1, &numConfigs);
    eglBindAPI(EGL_OPENGL_ES_API);

    platform.context = eglCreateContext(platform.device, platform.config, EGL_NO_CONTEXT, contextAttribs);
    if (platform.context == EGL_NO_CONTEXT)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to create EGL context");
        return -1;
    }

    EGLint displayFormat = 0;
    eglGetConfigAttrib(platform.device, platform.config, EGL_NATIVE_VISUAL_ID, &displayFormat);

    SetupFramebuffer();

    ANativeWindow_setBuffersGeometry(platform.window, CORE.Window.render.width, CORE.Window.render.height, displayFormat);

    platform.surface = eglCreateWindowSurface(platform.device, platform.config, platform.window, NULL);

    if (eglMakeCurrent(platform.device, platform.surface, platform.surface, platform.context) == EGL_FALSE)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to attach EGL rendering context to EGL surface");
        return -1;
    }

    CORE.Window.render.width = CORE.Window.screen.width;
    CORE.Window.render.height = CORE.Window.screen.height;
    CORE.Window.currentFbo.width = CORE.Window.render.width;
    CORE.Window.currentFbo.height = CORE.Window.render.height;

    // Initialize OpenGL context (states and resources)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);               // Enable color/alpha blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // Set blending mode
    glEnable(GL_DEPTH_TEST);          // Enable depth test
    glDepthFunc(GL_LEQUAL);          // Type of depth testing to apply
    glDisable(GL_CULL_FACE);         // Disable backface culling

    TRACELOG(LOG_INFO, "DISPLAY: Device initialized successfully");
    TRACELOG(LOG_INFO, "    > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
    TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
    TRACELOG(LOG_INFO, "    > Render size:  %i x %i", CORE.Window.render.width, CORE.Window.render.height);
    TRACELOG(LOG_INFO, "    > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);

    // Load OpenGL extensions
    rlLoadExtensions(eglGetProcAddress);

    CORE.Window.ready = true;

    return 0;
}

// Close platform
void ClosePlatform(void)
{
    // Release EGL surface and context
    if (platform.device != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(platform.device, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (platform.surface != EGL_NO_SURFACE)
        {
            eglDestroySurface(platform.device, platform.surface);
            platform.surface = EGL_NO_SURFACE;
        }

        if (platform.context != EGL_NO_CONTEXT)
        {
            eglDestroyContext(platform.device, platform.context);
            platform.context = EGL_NO_CONTEXT;
        }

        eglTerminate(platform.device);
        platform.device = EGL_NO_DISPLAY;
    }

    // Release native window reference - we don't actually destroy it because it was provided externally
    if (platform.window != NULL)
    {
        // NOTE: We don't call ANativeWindow_release here since the window was provided by FaceDetectorView
        // and will be managed by that component
        platform.window = NULL;
    }

    CORE.Window.ready = false;
    CORE.Window.fullscreen = false;
}

// Make the EGL context current in the calling thread
bool MakeContextCurrent(void)
{
    if (platform.device != EGL_NO_DISPLAY && platform.surface != EGL_NO_SURFACE && platform.context != EGL_NO_CONTEXT)
    {
        if (eglMakeCurrent(platform.device, platform.surface, platform.surface, platform.context))
        {
            return true;
        }
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to make EGL context current");
    }
    return false;
}

// Release the EGL context from the calling thread
void ReleaseContext(void)
{
    if (platform.device != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(platform.device, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
}

// Initialize window using ANativeWindow
bool InitWindowAndroid(ANativeWindow* nativeWindow)
{
    platform.window = nativeWindow;


    if (platform.contextRebindRequired)
    {
        // Reset screen scaling to full display size
        EGLint displayFormat = 0;
        eglGetConfigAttrib(platform.device, platform.config, EGL_NATIVE_VISUAL_ID, &displayFormat);

        ANativeWindow_setBuffersGeometry(nativeWindow,
            CORE.Window.render.width + CORE.Window.renderOffset.x,
            CORE.Window.render.height + CORE.Window.renderOffset.y,
            displayFormat);

        // Recreate display surface and re-attach OpenGL context
        platform.surface = eglCreateWindowSurface(platform.device, platform.config, nativeWindow, NULL);
        return MakeContextCurrent();
    }
    else
    {
        CORE.Window.display.width = ANativeWindow_getWidth(nativeWindow);
        CORE.Window.display.height = ANativeWindow_getHeight(nativeWindow);

        CORE.Window.screen.width = CORE.Window.display.width;
        CORE.Window.screen.height = CORE.Window.display.height;

        // Initialize graphics device (display device and OpenGL context)
        if (InitGraphicsDevice() != 0)
        {
            TRACELOG(LOG_WARNING, "DISPLAY: Failed to initialize graphics device");
            return false;
        }

        // Initialize OpenGL context (states and resources)
        rlglInit(CORE.Window.currentFbo.width, CORE.Window.currentFbo.height);

        // Setup default viewport
        SetupViewport(CORE.Window.currentFbo.width, CORE.Window.currentFbo.height);



        #if defined(SUPPORT_MODULE_RTEXT) && defined(SUPPORT_DEFAULT_FONT)
            // Load default font
            // WARNING: External function: Module required: rtext
            LoadFontDefault();
            #if defined(SUPPORT_MODULE_RSHAPES)
                // Set font white rectangle for shapes drawing, so shapes and text can be batched together
                // WARNING: rshapes module is required, if not available, default internal white rectangle is used
                Rectangle rec = GetFontDefault().recs[95];
                if (CORE.Window.flags & FLAG_MSAA_4X_HINT)
                {
                    // NOTE: We try to maxime rec padding to avoid pixel bleeding on MSAA filtering
                    SetShapesTexture(GetFontDefault().texture, (Rectangle){ rec.x + 2, rec.y + 2, 1, 1 });
                }
                else
                {
                    // NOTE: We set up a 1px padding on char rectangle to avoid pixel bleeding
                    SetShapesTexture(GetFontDefault().texture, (Rectangle){ rec.x + 1, rec.y + 1, rec.width - 2, rec.height - 2 });
                }
            #endif
         #else
            #if defined(SUPPORT_MODULE_RSHAPES)
            // Set default texture and rectangle to be used for shapes drawing
            // NOTE: rlgl default texture is a 1x1 pixel UNCOMPRESSED_R8G8B8A8
            Texture2D texture = { rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
            SetShapesTexture(texture, (Rectangle){ 0.0f, 0.0f, 1.0f, 1.0f });    // WARNING: Module required: rshapes
            #endif
        #endif

        return true;
    }
}

// Get current time in seconds
double GetTime(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec/1000000000.0;
}

// Swap back buffer with front buffer
void SwapScreenBuffer(void)
{
    if (platform.device != EGL_NO_DISPLAY && platform.surface != EGL_NO_SURFACE)
    {
        eglSwapBuffers(platform.device, platform.surface);
    }
}

// Poll input events
void PollInputEvents(void)
{
    // Input events are handled by Android input system
}

Vector2 GetWindowScaleDPI(void)
{
    return (Vector2){1.0f, 1.0f};
}

void MaximizeWindow(void)
{
    // Maximize window is handled by Android
}

void MinimizeWindow(void)
{
    // Minimize window is handled by Android
}

void SetWindowSize(int width, int height)
{
    // Set window size is handled by Android
}


// Show mouse cursor
void ShowCursor(void) { }

// Hides mouse cursor
void HideCursor(void) { }


// Enable cursor
void EnableCursor(void) { }

// Disable cursor
void DisableCursor(void) { }

// Setup viewport for specified size
static void SetupViewport(int width, int height)
{
    CORE.Window.screen.width = width;
    CORE.Window.screen.height = height;

    // Set viewport width and height
    glViewport(0, 0, width, height);

    // Set up projection matrix for 2D rendering
    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();
    rlOrtho(0, width, height, 0, 0.0f, 1.0f);

    // Set modelview matrix
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
}
