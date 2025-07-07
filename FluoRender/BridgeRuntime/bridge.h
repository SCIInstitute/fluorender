#pragma once

#ifdef _WIN32
#define INTEROP_EXPORT __declspec(dllexport)
#pragma warning(disable : 4244) 
#else
#define INTEROP_EXPORT __attribute__((visibility("default")))
#endif

#include <PixelFormats.h>
#include "callbacks.h"
#include <string>
#include <memory>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <map>
#include <stdexcept>
#include <filesystem>
#include <cstdint>
#ifdef _WIN32
#include <Winsock2.h>
#include <Windows.h>
#include <Unknwn.h>
#include <shlobj_core.h>
#include <Knownfolders.h>
#else
#include <dlfcn.h>
#include <pwd.h>
#include <unistd.h>
struct IUnknown;
#endif

#ifdef _WIN32
    const static wchar_t BridgeVersion[] = L"2.5.1";
#else
    const static char BridgeVersion[] = "2.5.1";
#endif

#ifdef _WIN32
	const static wchar_t MinBridgeVersion[] = L"2.5.0";
#else
	const static char MinBridgeVersion[] = "2.5.0";
#endif

const uint32_t FIRST_LOOKING_GLASS_DEVICE = -1;
const uint32_t OFFSCREEN_WINDOW           = -2;

typedef unsigned long WINDOW_HANDLE;

struct CalibrationSubpixelCell
{
	float ROffsetX;
	float ROffsetY;
	float GOffsetX;
	float GOffsetY;
	float BOffsetX;
	float BOffsetY;
};

struct LKGCalibration
{
    float center = 0;
    float pitch = 0;
    float slope = 0;
    int width = 0;
    int height = 0;
    float dpi = 0;
    float flip_x = 0;
    int invView = 0;
    float viewcone = 0.0f;
    float fringe = 0.0f;
    int cell_pattern_mode = 0;
    std::vector<CalibrationSubpixelCell> cells;
};


/** 
 * @brief function initializes the Bridge thread and allocates required resources
 * 
 * @param app_name the C-string name to associate with this application instance
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @note This function must be called prior to any other from the Bridge SDK. 
 * 
 * @see Controller::Initialize
 */
#ifdef _WIN32
extern "C" INTEROP_EXPORT bool initialize_bridge(const wchar_t *app_name);
#else
extern "C" INTEROP_EXPORT bool initialize_bridge(const char *app_name);
#endif

/** 
 * This function uninitializes the Bridge thread and deallocates required resources
 * 
 * @return always returns TRUE
 * 
 * @see Controller::Uninitialize
 */
extern "C" INTEROP_EXPORT bool uninitialize_bridge();

/** 
 * @brief This function queries the Bridge SDK to get the version number
 * 
 * The caller is responsible for allocating the postfix C-string buffer. To get 
 * the buffer size, this function should be called twice. When called with the 
 * postfix parameter set to nullptr, it will return the size of the postfix string. 
 * When called with the postfix parameter set to a buffer pointer, the build 
 * hash will be copied into the supplied buffer.
 * 
 * @return the semantic version of Bridge with build commit hash
 * 
 */
extern "C" INTEROP_EXPORT bool get_bridge_version(unsigned long* major, unsigned long *minor, unsigned long* build, int* number_of_postfix_wchars, wchar_t* postfix);

/**
 * @brief creates a new window using display parameters
 * 
 * If the display_index parameter is not supplied, the Bridge SDK will search 
 * for the first display that matches a Looking Glass product and use that. 
 * The window position and size will match the selected display.
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param display_index the optional index of the display to configure the window for
 * 
 * @return TRUE when a window was created, FALSE otherwise 
 * 
 * @note The window created is meant for use with OpenGL texture sharing
 * 
 * @see instance_offscreen_window
 * @see Controller::InstanceWindowGL
 */
extern "C" INTEROP_EXPORT bool instance_window_gl(WINDOW_HANDLE *wnd, unsigned long display_index = FIRST_LOOKING_GLASS_DEVICE);

/**
 * @brief creates a new window using the input parameters
 * 
 * The window width and height will match in the input parameters.
 * The calibration parameters will be loaded from the input calibration file path.
 * This function is meant for automated rendering and does not require a display.
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param width the desired width of the new window
 * @param height the desired height of the new window
 * @param calibration_path the filesystem path for a Looking Glass calibration file
 * 
 * @return TRUE when a window was created, FALSE otherwise 
 * 
 * @see instance_window_gl
 * @see Controller::InstanceOffscreenWindow
 */
extern "C" INTEROP_EXPORT bool instance_offscreen_window(WINDOW_HANDLE *wnd, unsigned long width, unsigned long height, const wchar_t* calibration_path);

extern "C" INTEROP_EXPORT bool instance_offscreen_window_gl(WINDOW_HANDLE *wnd, unsigned long display_index = FIRST_LOOKING_GLASS_DEVICE);
extern "C" INTEROP_EXPORT bool get_offscreen_window_texture_gl(WINDOW_HANDLE wnd, unsigned long long* texture, PixelFormats* format, unsigned long* width, unsigned long* height);

/**
 * @brief converts an RGBD image into a quilt image using input parameters
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param columns the desired output quilt columns
 * @param rows the desired output quilt rows
 * @param views the desired total number of views
 * @param aspect the aspect ratio of the resulting quilt
 * @param zoom the zoom applied to the RGBD input
 * @param cam_dist the camera distance applied during RGBD rendering
 * @param fov the field of view applied during RGBD rendering
 * @param crop_pos_x the x coordinate offset applied during RGBD rendering
 * @param crop_pos_y the y coordinate offset applied during RGBD rendering
 * @param depth_inversion a toggle to invert the depth map: 0=FALSE, 1=TRUE
 * @param chroma_depth a toggle to interpret the depth map using a full color spectrum: 0=FALSE, 1=TRUE
 * @param depth_loc the location of the depth map within the image: 0=top, 1=bottom, 2=right, 3=left
 * @param depthiness the amount of z-scaling applied to the RGBD input
 * @param depth_cutoff a z-depth threshold value applied to the RGBD input
 * @param focus the z-depth value representing the focal plane of the resulting quilt
 * @param input_path the local filesystem path to the input RGBD image
 * @param output_path the local filesystem path for the desired output quilt image
 * 
 * @return TRUE when successful, FALSE otherwise 
 * 
 * @see Controller::QuiltifyRGBD
 */
extern "C" INTEROP_EXPORT bool quiltify_rgbd(WINDOW_HANDLE wnd, unsigned long columns, unsigned long rows, unsigned long views, float aspect, float zoom, float cam_dist, float fov, float crop_pos_x, float crop_pos_y, unsigned long depth_inversion, unsigned long chroma_depth, unsigned long depth_loc, float depthiness, float depth_cutoff, float focus, const wchar_t* input_path, const wchar_t* output_path);

/**
 * @brief returns with size dimensions of the input window
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param width the pointer to the returned width value
 * @param height the pointer to the returned height value
 * 
 * @return width and height for the input window
 * 
 * @see get_window_position
 * @see Controller::GetWindowDimensions
 */
extern "C" INTEROP_EXPORT bool get_window_dimensions(WINDOW_HANDLE wnd, unsigned long* width, unsigned long* height);

/**
 * @brief returns with position of the input window
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param x the pointer to the returned x coordinate value
 * @param y the pointer to the returned y coordinate value
 * 
 * @return the x and y coordinates for the input window
 * 
 * @see get_window_dimensions
 * @see Controller::GetWindowPosition
 */
extern "C" INTEROP_EXPORT bool get_window_position(WINDOW_HANDLE wnd, long* x, long* y);

/**
 * @brief returns the maximum texture size for rendering to the window
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param size the pointer to the returned texture size
 * 
 * @return the maximum texture size dimension to be used for rendering to the window
 * 
 * @see set_interop_quilt_texture_gl
 * @see draw_interop_quilt_texture_gl
 * @see Controller::GetMaxTextureSize
 */
extern "C" INTEROP_EXPORT bool get_max_texture_size(WINDOW_HANDLE wnd, unsigned long* size);

/**
 * @brief assigns a shared render texture for applying the Looking Glass optical transformation
 * 
 * For OpenGL applications, a GL texture must be assigned as the shared render target
 * for use with draw_interop_quilt_texture_gl. This function designates the input texture
 * as the source of quilt views for which the optical transformation will be applied
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param texture the OpenGL texture handle to be shared between rendering contexts
 * @param format the pixel format of the supplied texture
 * @param width the width of the supplied texture
 * @param height the height of the supplied texture
 * @param vx the horizontal count of quilt views (columns) rendered to the texture
 * @param vy the vertical count of quilt views (rows) rendered to the texture
 * @param aspect the aspect ratio of the views
 * @param zoom the optional zoom to be applied
 * 
 * @return FALSE for invalid configurations, TRUE otherwise
 * 
 * @see draw_interop_quilt_texture_gl
 * @see Controller::SetInteropQuiltTextureGL
 */
extern "C" INTEROP_EXPORT bool set_interop_quilt_texture_gl(WINDOW_HANDLE wnd, unsigned long long texture, PixelFormats format, unsigned long width, unsigned long height, unsigned long vx, unsigned long vy, float aspect, float zoom);

/**
 * @brief triggers the Looking Glass optical transformation as a rendering post-processing step
 * 
 * For OpenGL applications, a GL texture must be assigned as the shared render target
 * for use with draw_interop_quilt_texture_gl. This function designates the input texture
 * as the source of quilt views for which the optical transformation will be applied
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param texture the texture handle to be shared between rendering contexts
 * @param format the pixel format of the supplied texture
 * @param width the width of the supplied texture
 * @param height the height of the supplied texture
 * @param vx the horizontal count of quilt views (columns) rendered to the texture
 * @param vy the vertical count of quilt views (rows) rendered to the texture
 * @param aspect the aspect ratio of the views
 * @param zoom the optional zoom to be applied
 * 
 * @return FALSE for invalid configurations, TRUE otherwise
 * 
 * @see set_interop_quilt_texture_gl
 * @see Controller::DrawInteropQuiltTextureGL
 */
extern "C" INTEROP_EXPORT bool draw_interop_quilt_texture_gl(WINDOW_HANDLE wnd, unsigned long long texture, PixelFormats format, unsigned long width, unsigned long height, unsigned long vx, unsigned long vy, float aspect, float zoom);

/**
 * @brief shows or hides the rending window based upon the input flag
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param flag the toggle to show or hide the window: FALSE=hide, TRUE=show
 * 
 * @return FALSE for an invalid window, TRUE otherwise
 * 
 */
extern "C" INTEROP_EXPORT bool show_window(WINDOW_HANDLE wnd, bool flag);

/**
 * @brief saves the input OpenGL texture to a file
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param filename the filesystem path for the saved image
 * @param texture the texture handle to be saved to disk
 * @param format the pixel format of the supplied texture
 * @param width the width of the supplied texture
 * @param height the height of the supplied texture
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @see save_image_to_file
 * @see Controller::SaveTextureToFileGL
 */
#ifdef WIN32
extern "C" INTEROP_EXPORT bool save_texture_to_file_gl(WINDOW_HANDLE wnd, wchar_t* filename, unsigned long long texture, PixelFormats format, unsigned long width, unsigned long height);
#else
extern "C" INTEROP_EXPORT bool save_texture_to_file_gl(WINDOW_HANDLE wnd, char* filename, unsigned long long texture, PixelFormats format, unsigned long width, unsigned long height);
#endif

/**
 * @brief saves the raw image buffer to a file
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param filename the filesystem path for the saved image
 * @param image the raw image buffer to be saved
 * @param format the pixel format of the supplied image
 * @param width the width of the supplied image
 * @param height the height of the supplied image
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @see save_texture_to_file_gl
 * @see Controller::SaveImageToFile
 */
#ifdef WIN32
extern "C" INTEROP_EXPORT bool save_image_to_file(WINDOW_HANDLE wnd, wchar_t* filename, void* image, PixelFormats format, unsigned long width, unsigned long height);
#else
extern "C" INTEROP_EXPORT bool save_image_to_file(WINDOW_HANDLE wnd, char* filename, void* image, PixelFormats format, unsigned long width, unsigned long height);
#endif

#ifdef WIN32
extern "C" INTEROP_EXPORT bool device_from_resource_dx(IUnknown * dx_resource, IUnknown **dx_device);
extern "C" INTEROP_EXPORT bool release_device_dx(IUnknown * dx_device);
extern "C" INTEROP_EXPORT bool instance_window_dx(IUnknown *dx_device, WINDOW_HANDLE *wnd, unsigned long display_index = FIRST_LOOKING_GLASS_DEVICE);
extern "C" INTEROP_EXPORT bool instance_offscreen_window_dx(IUnknown *dx_device, WINDOW_HANDLE *wnd, unsigned long display_index = FIRST_LOOKING_GLASS_DEVICE);
extern "C" INTEROP_EXPORT bool register_texture_dx(WINDOW_HANDLE wnd, IUnknown *dx_texture);
extern "C" INTEROP_EXPORT bool unregister_texture_dx(WINDOW_HANDLE wnd, IUnknown *dx_texture);
extern "C" INTEROP_EXPORT bool save_texture_to_file_dx(WINDOW_HANDLE wnd, wchar_t* filename, IUnknown * dx_texture);
extern "C" INTEROP_EXPORT bool draw_interop_quilt_texture_dx(WINDOW_HANDLE wnd, IUnknown * dx_texture, unsigned long vx, unsigned long vy, float aspect, float zoom);
extern "C" INTEROP_EXPORT bool create_texture_dx(WINDOW_HANDLE wnd, unsigned long width, unsigned long height, IUnknown **dx_texture);
extern "C" INTEROP_EXPORT bool release_texture_dx(WINDOW_HANDLE wnd, IUnknown *dx_texture);
extern "C" INTEROP_EXPORT bool copy_texture_dx(WINDOW_HANDLE wnd, IUnknown *src, IUnknown *dest);
extern "C" INTEROP_EXPORT bool get_offscreen_window_texture_dx(WINDOW_HANDLE wnd, IUnknown** dx_texture);
#endif

#ifdef __APPLE__
extern "C" INTEROP_EXPORT bool instance_window_metal(void *metal_device, WINDOW_HANDLE *wnd, unsigned long display_index = FIRST_LOOKING_GLASS_DEVICE);
extern "C" INTEROP_EXPORT bool create_metal_texture_with_iosurface(WINDOW_HANDLE wnd, void* descriptor, void** texture);
extern "C" INTEROP_EXPORT bool copy_metal_texture(WINDOW_HANDLE wnd, void* src, void* dest);
extern "C" INTEROP_EXPORT bool release_metal_texture(WINDOW_HANDLE wnd, void* texture);
extern "C" INTEROP_EXPORT bool save_metal_texture_to_file(WINDOW_HANDLE wnd, char* filename, void* texture, PixelFormats format, unsigned long width, unsigned long height);
extern "C" INTEROP_EXPORT bool draw_interop_quilt_texture_metal(WINDOW_HANDLE wnd, void* texture, unsigned long vx, unsigned long vy, float aspect, float zoom);
extern "C" INTEROP_EXPORT bool instance_offscreen_window_metal(void *metal_device, WINDOW_HANDLE *wnd, unsigned long display_index = FIRST_LOOKING_GLASS_DEVICE);
extern "C" INTEROP_EXPORT bool get_offscreen_window_texture_metal(WINDOW_HANDLE wnd, void** texture);
#endif

/** 
 * @brief returns the calibration values for the given window
 * 
 * The calibration parameters assocaited with the input window will be returned 
 * as parameter values. If the cells pointer is null, it will be ignored. Any 
 * other parameters with null values will be considered invalid.
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param center the pointer to the returned center value
 * @param pitch the pointer to the returned pitch value
 * @param slope the pointer to the returned slope value
 * @param width the pointer to the returned width value
 * @param height the pointer to the returned height value
 * @param dpi the pointer to the returned DPI value
 * @param flip_x the pointer to the returned horizontal flip toggle. 0=FALSE, 1=TRUE
 * @param invView the pointer to the returned inverted views toggle. 0=FALSE, 1=TRUE
 * @param viewcone the pointer to the returned viewcone value
 * @param fringe the pointer to the returned fringe value
 * @param cell_pattern_mode the pointer to the returned cell pattern mode
 * @param number_of_cells the pointer to the returned cell count
 * @param cells the pointer to the returned cell pattern struct
 * 
 * @return FALSE for invalid input parameters, TRUE otherwise
 * 
 * @see Controller::GetCalibration
 * @see get_calibration_for_display
 */
extern "C" INTEROP_EXPORT bool get_calibration(WINDOW_HANDLE wnd, 
	                                           float* center, 
	                                           float *pitch, 
	                                           float *slope, 
	                                           int* width, 
	                                           int* height, 
	                                           float* dpi, 
	                                           float* flip_x,
                                               int *invView,
									           float *viewcone,
									           float *fringe,
                                               int* cell_pattern_mode,
											   int* number_of_cells,
	                                           CalibrationSubpixelCell* cells);

/**
 * @brief returns the device name for the given window 
 * 
 * The caller is responsible for allocating the device_name C-string buffer. 
 * To get the buffer size, this function should be called twice. When called
 * with the device_name parameter set to nullptr, it will return the size of 
 * the device_name string. When called with the device_name parameter set to 
 * a buffer pointer, the device name will be copied into the supplied buffer
 * 
 * The size of the buffer is returned as a count of wide string characters.
 * In practice, wchar is 32 bits on Linux and MacOS but 16 bits on Windows. 
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param number_of_device_name_wchars the pointer to the returned size of the C-string buffer
 * @param device_name the pointer to the returned size device name C-string buffer
 * 
 * @return FALSE if number_of_device_name_wchars is null, TRUE otherwise
 * 
 * @see Controller::GetDeviceName
 * @see get_device_name_for_display
 */
extern "C" INTEROP_EXPORT bool get_device_name(WINDOW_HANDLE wnd, int* number_of_device_name_wchars, wchar_t* device_name);

/**
 * @brief returns the device serial number for the given window 
 * 
 * The caller is responsible for allocating the serial number C-string buffer. 
 * To get the buffer size, this function should be called twice. When called
 * with the serial parameter set to nullptr, it will return the size of 
 * the serial string. When called with the serial parameter set to 
 * a buffer pointer, the device serial will be copied into the supplied buffer
 * 
 * The size of the buffer is returned as a count of wide string characters.
 * In practice, wchar is 32 bits on Linux and MacOS but 16 bits on Windows. 
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param number_of_serial_wchars the pointer to the returned size of the C-string buffer
 * @param serial the pointer to the returned size device serial C-string buffer
 * 
 * @return FALSE if number_of_serial_wchars is null, TRUE otherwise
 * 
 * @see Controller::GetDeviceSerial
 * @see get_device_serial_for_display
 */
extern "C" INTEROP_EXPORT bool get_device_serial(WINDOW_HANDLE wnd, int* number_of_serial_wchars, wchar_t* serial);

/**
 * @brief returns the ideal quilt settings for the given window
 * 
 * The ideal quilt settings are aset of heuristics defined by Looking Glass 
 * for each display product.
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param aspect the pointer to the returned aspect value
 * @param quilt_width the pointer to the returned width value
 * @param quilt_height the pointer to the returned height value
 * @param quilt_columns the pointer to the returned quilt columns value
 * @param quilt_rows the pointer to the returned quilt rows value
 * 
 * @return FALSE if number_of_serial_wchars is null, TRUE otherwise
 * 
 * @see Controller::GetDefaultQuiltSettings
 */
extern "C" INTEROP_EXPORT bool get_default_quilt_settings(WINDOW_HANDLE wnd, float* aspect, int* quilt_width, int* quilt_height, int* quilt_columns, int* quilt_rows);

/**
 * @brief returns the display index used to render the window
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param display_index the zero-based index of the display
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @see Controller::GetDisplayForWindow
 */
extern "C" INTEROP_EXPORT bool get_display_for_window(WINDOW_HANDLE wnd, unsigned long* display_index);

/**
 * @brief returns the type of display product to render the window 
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param hw_enum the display enum value
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @see get_device_type_for_display
 * @see Controller::GetDeviceType
 */
extern "C" INTEROP_EXPORT bool get_device_type(WINDOW_HANDLE wnd, int* hw_enum);

/**
 * @brief returns the viewcone value for the display product that renders the window 
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param viewcone the pointer to the returned viewcone value
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @see get_viewcone_for_display
 * @see Controller::GetViewCone
 */
extern "C" INTEROP_EXPORT bool get_viewcone(WINDOW_HANDLE wnd, float* viewcone);

/**
 * @brief returns the display indices attached to the host
 * 
 * The caller is responsible for allocating the array of index values
 * To get the array size, this function should be called twice. When called
 * with the indices parameter set to nullptr, it will return the size of 
 * the indices array. When called with the indices parameter set to 
 * an array pointer, the display indices will be copied into the supplied array
 * 
 * @param number_of_indices the pointer to the returned size of the index array
 * @param indices the pointer to the returned array of display indices
 * 
 * @return FALSE if number_of_indices is null, TRUE otherwise
 * 
 * @see Controller::GetDisplays
 */
extern "C" INTEROP_EXPORT bool get_displays(int* number_of_indices, unsigned long* indices);

/**
 * @brief returns the inverted views toggle for the given window
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param invview the pointer to the returned inverted views toggle. 0=FALSE, 1=TRUE
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @see get_calibration
 * @see Controller::GetInvView
 */
extern "C" INTEROP_EXPORT bool get_invview(WINDOW_HANDLE wnd, int* invview);

/**
 * @brief returns the red index for the display used with the given window
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param ri the pointer to the returned red index value (0 or 2)
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @note this function is meant only to provide compatibility with HoloPlay Core SDK
 * 
 * @see get_calibration
 * @see Controller::GetRi
 */
extern "C" INTEROP_EXPORT bool get_ri(WINDOW_HANDLE wnd, int* ri);

/**
 * @brief returns the blue index for the display used with the given window
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param bi the pointer to the returned blue index value (0 or 2)
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @note this function is meant only to provide compatibility with HoloPlay Core SDK
 * 
 * @see get_calibration
 * @see Controller::GetBi
 */
extern "C" INTEROP_EXPORT bool get_bi(WINDOW_HANDLE wnd, int *bi);

/**
 * @brief returns the tilt for the display used with the given window
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param tilt the pointer to the returned tilt value
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @note this function is meant only to provide compatibility with HoloPlay Core SDK
 * 
 * @see get_calibration
 * @see Controller::GetTilt
 */
extern "C" INTEROP_EXPORT bool get_tilt(WINDOW_HANDLE wnd, float* tilt);

/**
 * @brief returns the aspect ratio of the display used with the given window
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param displayaspect the pointer to the returned aspect ratio
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @note this function is meant only to provide compatibility with HoloPlay Core SDK
 * 
 * @see get_calibration
 * @see Controller::GetDisplayAspect
 */
extern "C" INTEROP_EXPORT bool get_displayaspect(WINDOW_HANDLE wnd, float* displayaspect);

/**
 * @brief returns the fringe value of the display used with the given window
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param fringe the pointer to the returned fringe value
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @note this function is meant only to provide compatibility with HoloPlay Core SDK
 * 
 * @see get_calibration
 * @see Controller::GetFringe
 */
extern "C" INTEROP_EXPORT bool get_fringe(WINDOW_HANDLE wnd, float* fringe);

/**
 * @brief returns the subpixel size of the display used with the given window
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param subp the pointer to the returned sub pixel size value
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @see Controller::GetSubp
 */
extern "C" INTEROP_EXPORT bool get_subp(WINDOW_HANDLE wnd, float* subp);

/**
 * @brief returns the pitch of the display used with the given window
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param pitch the pointer to the returned pitch value
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @note this function is meant only to provide compatibility with HoloPlay Core SDK
 * 
 * @see get_calibration
 * @see Controller::GetPitch
 */
extern "C" INTEROP_EXPORT bool get_pitch(WINDOW_HANDLE wnd, float* pitch);

/**
 * @brief returns the center of the display used with the given window
 * 
 * @param wnd the pointer used to store the platform-specific window handle
 * @param center the pointer to the returned center value
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @note this function is meant only to provide compatibility with HoloPlay Core SDK
 * 
 * @see get_calibration
 * @see Controller::GetCenter
 */
extern "C" INTEROP_EXPORT bool get_center(WINDOW_HANDLE wnd, float* center);

/**
 * @brief returns the device name for the given display index
 * 
 * The caller is responsible for allocating the device_name C-string buffer. 
 * To get the buffer size, this function should be called twice. When called
 * with the device_name parameter set to nullptr, it will return the size of 
 * the device_name string. When called with the device_name parameter set to 
 * a buffer pointer, the device name will be copied into the supplied buffer
 * 
 * The size of the buffer is returned as a count of wide string characters.
 * In practice, wchar is 32 bits on Linux and MacOS but 16 bits on Windows. 
 * 
 * @param display_index the index for the target Looking Glass display
 * @param number_of_device_name_wchars the pointer to the returned size of the C-string buffer
 * @param device_name the pointer to the returned size device name C-string buffer
 * 
 * @return FALSE if number_of_device_name_wchars is null, TRUE otherwise
 * 
 * @see Controller::GetDeviceNameForDisplay
 * @see get_device_name
 * @see get_displays
 */
extern "C" INTEROP_EXPORT bool get_device_name_for_display(unsigned long display_index, int* number_of_device_name_wchars, wchar_t* device_name);

/**
 * @brief returns the device serial number for the given display index
 * 
 * The caller is responsible for allocating the serial number C-string buffer. 
 * To get the buffer size, this function should be called twice. When called
 * with the serial parameter set to nullptr, it will return the size of 
 * the serial string. When called with the serial parameter set to 
 * a buffer pointer, the device serial will be copied into the supplied buffer
 * 
 * The size of the buffer is returned as a count of wide string characters.
 * In practice, wchar is 32 bits on Linux and MacOS but 16 bits on Windows. 
 * 
 * @param display_index the index for the target Looking Glass display
 * @param number_of_serial_wchars the pointer to the returned size of the C-string buffer
 * @param serial the pointer to the returned size device serial C-string buffer
 * 
 * @return FALSE if number_of_serial_wchars is null, TRUE otherwise
 * 
 * @see Controller::GetDeviceSerialForDisplay
 * @see get_device_serial
 * @see get_displays
 */
extern "C" INTEROP_EXPORT bool get_device_serial_for_display(unsigned long display_index, int* number_of_serial_wchars, wchar_t* serial);

/**
 * @brief returns with size dimensions of the given display index
 * 
 * @param display_index the index for the target Looking Glass display
 * @param width the pointer to the returned width value
 * @param height the pointer to the returned height value
 * 
 * @return width and height for the input window
 * 
 * @see Controller::GetDimensionsForDisplay
 * @see get_window_dimensions
 * @see get_displays
 */
extern "C" INTEROP_EXPORT bool get_dimensions_for_display(unsigned long display_index, unsigned long* width, unsigned long* height);

/**
 * @brief returns with position of the given display index
 * 
 * @param display_index the index for the target Looking Glass display
 * @param x the pointer to the returned x coordinate value
 * @param y the pointer to the returned y coordinate value
 * 
 * @return the x and y coordinates for the input window
 * 
 * @see Controller::GetWindowPositionForDisplay
 * @see get_window_position
 * @see get_displays
 */
extern "C" INTEROP_EXPORT bool get_window_position_for_display(unsigned long display_index, long* x, long* y);

/**
 * @brief returns the type of display product for the given display index
 * 
 * @param display_index the index for the target Looking Glass display
 * @param hw_enum the display enum value
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @see Controller::GetDeviceTypeForDisplay
 * @see get_device_type
 */
extern "C" INTEROP_EXPORT bool get_device_type_for_display(unsigned long display_index, int* hw_enum);

/** 
 * @brief returns the calibration values for the given display index
 * 
 * The calibration parameters assocaited with the input display will be returned 
 * as parameter values. If the cells pointer is null, it will be ignored. Any 
 * other parameters with null values will be considered invalid.
 * 
 * @param display_index the index for the target Looking Glass display
 * @param center the pointer to the returned center value
 * @param pitch the pointer to the returned pitch value
 * @param slope the pointer to the returned slope value
 * @param width the pointer to the returned width value
 * @param height the pointer to the returned height value
 * @param dpi the pointer to the returned DPI value
 * @param flip_x the pointer to the returned horizontal flip toggle. 0=FALSE, 1=TRUE
 * @param invView the pointer to the returned inverted views toggle. 0=FALSE, 1=TRUE
 * @param viewcone the pointer to the returned viewcone value
 * @param fringe the pointer to the returned fringe value
 * @param cell_pattern_mode the pointer to the returned cell pattern mode
 * @param number_of_cells the pointer to the returned cell count
 * @param cells the pointer to the returned cell pattern struct
 * 
 * @return FALSE for invalid input parameters, TRUE otherwise
 * 
 * @see Controller::GetCalibrationForDisplay
 * @see get_calibration
 * @see get_displays
 */
extern "C" INTEROP_EXPORT bool get_calibration_for_display(unsigned long display_index, 
	                                           float* center, 
	                                           float *pitch, 
	                                           float *slope, 
	                                           int* width, 
	                                           int* height, 
	                                           float* dpi, 
	                                           float* flip_x,
                                               int* invView,
									           float* viewcone,
									           float* fringe,
                                               int* cell_pattern_mode,
											   int* number_of_cells,
	                                           CalibrationSubpixelCell* cells);

/**
 * @brief returns the inverted views toggle for the given display index
 * 
 * @param display_index the index for the target Looking Glass display
 * @param invview the pointer to the returned inverted views toggle. 0=FALSE, 1=TRUE
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @see Controller::GetInvViewForDisplay
 * @see get_invview
 * @see get_calibration_for_display
 * @see get_displays
 */
extern "C" INTEROP_EXPORT bool get_invview_for_display(unsigned long display_index, int* invview);

/**
 * @brief returns the red index for the display used with the given display index
 * 
 * @param display_index the index for the target Looking Glass display
 * @param ri the pointer to the returned red index value (0 or 2)
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @note this function is meant only to provide compatibility with HoloPlay Core SDK
 * 
 * @see Controller::GetRiForDisplay
 * @see get_ri
 * @see get_calibration_for_display
 * @see get_displays
 */
extern "C" INTEROP_EXPORT bool get_ri_for_display(unsigned long display_index, int* ri);

/**
 * @brief returns the blue index for the display used with the given display index
 * 
 * @param display_index the index for the target Looking Glass display
 * @param bi the pointer to the returned blue index value (0 or 2)
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @note this function is meant only to provide compatibility with HoloPlay Core SDK
 * 
 * @see Controller::GetBiForDisplay
 * @see get_bi
 * @see get_calibration_for_display
 * @see get_displays
 */
extern "C" INTEROP_EXPORT bool get_bi_for_display(unsigned long display_index, int *bi);

/**
 * @brief returns the tilt for the display used with the given display index
 * 
 * @param display_index the index for the target Looking Glass display
 * @param tilt the pointer to the returned tilt value
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @note this function is meant only to provide compatibility with HoloPlay Core SDK
 * 
 * @see Controller::GetTiltForDisplay
 * @see get_tilt
 * @see get_calibration_for_display
 * @see get_displays
 */
extern "C" INTEROP_EXPORT bool get_tilt_for_display(unsigned long display_index, float* tilt);

/**
 * @brief returns the aspect ratio of the display used with the given display index
 * 
 * @param display_index the index for the target Looking Glass display
 * @param displayaspect the pointer to the returned aspect ratio
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @note this function is meant only to provide compatibility with HoloPlay Core SDK
 * 
 * @see Controller::GetDisplayAspectForDisplay
 * @see get_displayaspect
 * @see get_calibration_for_display
 * @see get_displays
 */
extern "C" INTEROP_EXPORT bool get_displayaspect_for_display(unsigned long display_index, float* displayaspect);

/**
 * @brief returns the fringe value of the display used with the given display index
 * 
 * @param display_index the index for the target Looking Glass display
 * @param fringe the pointer to the returned fringe value
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @note this function is meant only to provide compatibility with HoloPlay Core SDK
 * 
 * @see Controller::GetFringeForDisplay
 * @see get_fringe
 * @see get_calibration_for_display
 * @see get_displays
 */
extern "C" INTEROP_EXPORT bool get_fringe_for_display(unsigned long display_index, float* fringe);

/**
 * @brief returns the subpixel size of the display used with the given display index
 * 
 * @param display_index the index for the target Looking Glass display
 * @param subp the pointer to the returned sub pixel size value
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @see Controller::GetSubpForDisplay
 * @see get_subp
 */
extern "C" INTEROP_EXPORT bool get_subp_for_display(unsigned long display_index, float* subp);

/**
 * @brief returns the viewcone value for the display product with the given display index
 * 
 * @param display_index the index for the target Looking Glass display
 * @param viewcone the pointer to the returned viewcone value
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @see Controller::GetViewConeForDisplay
 * @see get_viewcone
 */
extern "C" INTEROP_EXPORT bool get_viewcone_for_display(unsigned long display_index, float* viewcone);

/**
 * @brief returns the pitch of the display used with the given display index
 * 
 * @param display_index the index for the target Looking Glass display
 * @param pitch the pointer to the returned pitch value
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @note this function is meant only to provide compatibility with HoloPlay Core SDK
 * 
 * @see Controller::GetPitchForDisplay
 * @see get_pitch
 * @see get_calibration_for_display
 * @see get_displays
 */
extern "C" INTEROP_EXPORT bool get_pitch_for_display(unsigned long display_index, float* pitch);

/**
 * @brief returns the center of the display used with the given display index
 * 
 * @param display_index the index for the target Looking Glass display
 * @param center the pointer to the returned center value
 * 
 * @return TRUE if successful, FALSE otherwise
 * 
 * @note this function is meant only to provide compatibility with HoloPlay Core SDK
 * 
 * @see Controller::GetCenterForDisplay
 * @see get_center
 * @see get_calibration_for_display
 * @see get_displays
 */
extern "C" INTEROP_EXPORT bool get_center_for_display(unsigned long display_index, float* center);

/**
 * @brief returns the ideal quilt settings for the given display index
 * 
 * The ideal quilt settings are aset of heuristics defined by Looking Glass 
 * for each display product.
 * 
 * @param display_index the index for the target Looking Glass display
 * @param aspect the pointer to the returned aspect value
 * @param quilt_width the pointer to the returned width value
 * @param quilt_height the pointer to the returned height value
 * @param quilt_columns the pointer to the returned quilt columns value
 * @param quilt_rows the pointer to the returned quilt rows value
 * 
 * @return FALSE if number_of_serial_wchars is null, TRUE otherwise
 * 
 * @see Controller::GetDefaultQuiltSettingsForDisplay
 * @see get_default_quilt_settings
 */
extern "C" INTEROP_EXPORT bool get_default_quilt_settings_for_display(unsigned long display_index, float* aspect, int* quilt_width, int* quilt_height, int* quilt_columns, int* quilt_rows);

#ifdef _WIN32
#pragma warning(default : 4244)
#endif
