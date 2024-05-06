/*
    HoloPlayCore 0.1.1

    Intended for use with HoloPlay Service 1.0.1 and above.

    File: include/HoloPlayCore.h

    Author: Evan Kahn
    Contact: evan@lookingglassfactory.com

     This header contains declarations for functions in the HoloPlayCore dynamic library,
    which handles communication with the HoloPlay Service runtime.
    
     A programmer looking to design a custom renderer for Looking Glass devices ought only to
    need the functions declared in this header file. They have simple arguments and return types,
    and are designed to be easily bound from any programming language and dropped into custom
    rendering pipelines.

     Nearly all of them map directly to combinations of functions declared in libHoloPlayCore.h;
    they are provided for developer convenience and simple cross-language binding of functions
    that might return structs or opaque object pointers. For insight on how these work under the 
    hood and more direct access to the data they provide, see that file.
	    
    Copyright 2020 Looking Glass Factory

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software
    and associated documentation files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or
    substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
    BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/


#ifndef IMPORT_DECL
#ifdef _WIN32
#ifdef HPC_STATIC
#define IMPORT_DECL 
#else
#define IMPORT_DECL __declspec(dllimport)
#endif
#else
#define IMPORT_DECL
#endif
#endif

#include "libHoloPlayCore.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /*
       Note on string return functions:

        Any function of the form "size_t hpc_DoThing(..., char* out_buf, size_t out_buf_sz)" copies a
       string into a preallocated buffer of size out_buf_sz. The buffer is allocated and owned by
       the caller.

        These functions return 0 if out_buf_sz was sufficient, or the actual size of string if
       out_buf_sz was too small.

        This makes it much easier to pass strings into managed memory (i.e. for .NET bindings).
       Most of the strings returned by these functions are quite small.
    */

    /*
        hpc_InitializeApp

         Registers a client app with HoloPlay Service, and updates the global state variable.
        If you intend to register an app under a specific name, this *MUST* be the first 
        function call you make.

         Future versions of HoloPlayService and HoloPlayCore will refuse to return calibration
        information unless hpc_InitializeApp has been called first.

         Args: name of application (will be registered by HoloPlay Service)
          Application commercial licensing type (see hpc_license_type)
         Returns: error code returned from HoloPlay Service request.
    */
    IMPORT_DECL hpc_client_error hpc_InitializeApp(const char *app_name, hpc_license_type app_type);

    /*
        hpc_RefreshState

         Send a new message to HoloPlayService requesting updated device information;
        overwrite the state variable storing that information. hpc_InitializeApp will
        initially populate this variable.
         Call this function again to update that variable.

         Args: none
         Returns: error code returned from HoloPlay Service request. (See libHoloPlayCore.h
        for error code definitions.)
    */

    IMPORT_DECL hpc_client_error hpc_RefreshState(void);

    /*
        hpc_CloseApp

         Closes connection to HoloPlay Service. This should *always* be called before your
        app exits.

         Args: none
         Returns: error code returned from HoloPlay Service request.
    */
    IMPORT_DECL int hpc_CloseApp(void);

    /*
        hpc_GetHoloPlayCoreVersion

         Args: (see note on string return functions)
         Returns: current version of HoloPlay Core, as a string
    */
    IMPORT_DECL size_t hpc_GetHoloPlayCoreVersion(char *out_buf, size_t out_buf_sz);

    /*
         The following are helper functions to query information from the state message,
        meaning that they will only return valid information following the invocation
        of hpc_InitializeApp(), and only return new information after hpc_RefreshState().
    */

    /*
        hpc_GetStateAsJson

         Args: (see note on string return functions)
         Returns: The current state message, serialized into a char buffer as JSON.

         Mostly useful for debugging - custom message queries can be constructed without
        an external JSON serializer, using the helper functions in libHoloPlayCore.h.
    */
    IMPORT_DECL size_t hpc_GetStateAsJSON(char *out_buf, size_t out_buf_sz);

    /*
        hpc_GetHoloPlayServiceVersion

         Args: (see note on string return functions)
         Returns: current version of HoloPlay Service, as a string
    */
    IMPORT_DECL size_t hpc_GetHoloPlayServiceVersion(char *out_buf, size_t out_buf_sz);

    /*
        hpc_GetNumDevices

         Args: none
         Returns: number of Looking Glass devices connnected with valid calibrations.
    */
    IMPORT_DECL int hpc_GetNumDevices();

    /*
        hpc_GetDeviceHDMIName

         Args: (see note on string return functions)
         Returns: Looking Glass device name retrieved from EDID.
    */
    IMPORT_DECL size_t hpc_GetDeviceHDMIName(int dev_index, char *out_buf, size_t out_buf_sz);

    /*
        hpc_GetDeviceSerial

         Args: (see note on string return functions)
         Returns: Serial number of device (same as the one printed on label)
    */
    IMPORT_DECL size_t hpc_GetDeviceSerial(int dev_index, char *out_buf, size_t out_buf_sz);

    /*
        hpc_GetDeviceType

         Args: (see note on string return functions)
         Returns: Device type as string (currently one of "standard", "large", "pro", "8k")
    */
    IMPORT_DECL size_t hpc_GetDeviceType(int dev_index, char *out_buf, size_t out_buf_sz);

    /*
        hpc_GetDeviceProperty functions

         The following functions query per-device configuration parameters.
        They will return 0 if the device or calibration isn't found.

         Args: device index
         Returns: varies

        A note on device indexing:

         HoloPlay Service assigns each connected device a persistent index,
        starting from 0. When a new device is connected, HoloPlay Service will
        assign it the lowest index available.

         In certain situations, there may be no device present at a certain index.
        For instance, if two devices are connected, they will receive indices 0, then 1,
        respectively. If the device indexed 0 is disconnected, there will be a device
        present at index 1 but not at index 0. The next device to be connected will be
        assigned index 0.
    */

    /*
        hpc_GetDevicePropertyWinX

         Returns: X position of monitor, in window coordinates, reported by OS.
    */
    IMPORT_DECL int hpc_GetDevicePropertyWinX(int dev_index);

    /*
        hpc_GetDevicePropertyWinY

         Returns: Y position of monitor, in window coordinates, reported by OS.
    */
    IMPORT_DECL int hpc_GetDevicePropertyWinY(int dev_index);

    /*
         The following functions return values retrieved from the lenticular
        calibration file associated with the device and transmitted over USB.

         They should be loaded into the lenticular shader as uniform parameters
        of the same name and type.
    */

    /*
        hpc_GetDevicePropertyScreenW

         Returns: screen width in pixels
    */
    IMPORT_DECL int hpc_GetDevicePropertyScreenW(int dev_index);

    /*
        hpc_GetDevicePropertyScreenH

         Returns: screen height in pixels
    */
    IMPORT_DECL int hpc_GetDevicePropertyScreenH(int dev_index);

    /*
        hpc_GetDevicePropertyInvView

         Returns: whether the lenticular shader should be inverted. (1 or 0)
    */
    IMPORT_DECL int hpc_GetDevicePropertyInvView(int dev_index);

    /*
        hpc_GetDevicePropertyRi

         Returns: 'red index' of each lenticular subpixel. (0 or 2)
    */
    IMPORT_DECL int hpc_GetDevicePropertyRi(int dev_index);

    /*
        hpc_GetDevicePropertyBi

         Returns: 'blue index' of each lenticular subpixel. (0 or 2)
    */
    IMPORT_DECL int hpc_GetDevicePropertyBi(int dev_index);

    /*
        hpc_GetDevicePropertyPitch

         Returns: lenticular lens pitch.
    */
    IMPORT_DECL float hpc_GetDevicePropertyPitch(int dev_index);

    /*
        hpc_GetDevicePropertyCenter

         Returns: lenticular center offset.
    */
    IMPORT_DECL float hpc_GetDevicePropertyCenter(int dev_index);

    /*
        hpc_GetDevicePropertyTilt

         Returns: lenticular tilt angle.
    */
    IMPORT_DECL float hpc_GetDevicePropertyTilt(int dev_index);

    /*
        hpc_GetDevicePropertyAspect

         Returns: display aspect ratio. (Equal to ScreenW/ScreenH)
    */
    IMPORT_DECL float hpc_GetDevicePropertyDisplayAspect(int dev_index);

    /*
        hpc_GetDevicePropertyFringe

         Returns: display fringe correction uniform. (Currently only
        applicable to Large/Pro units.)
    */
    IMPORT_DECL float hpc_GetDevicePropertyFringe(int dev_index);

    /*
        hpc_GetDevicePropertySubp

         Returns: display subpixel size.
    */
    IMPORT_DECL float hpc_GetDevicePropertySubp(int dev_index);

    /*
        hpc_GetDevicePropertyQuiltX

         Returns: recommended horizontal quilt texture resolution.
         Note: Only works with Holoplay Service 1.2.0 or later; Holoplay Core 0.1.2 or later
    */
    IMPORT_DECL int hpc_GetDevicePropertyQuiltX(int dev_index);

    /*
        hpc_GetDevicePropertyQuiltY

         Returns: recommended vertical quilt texture resolution.
         Note: Only works with Holoplay Service 1.2.0 or later; Holoplay Core 0.1.2 or later
    */
    IMPORT_DECL int hpc_GetDevicePropertyQuiltY(int dev_index);

    /*
        hpc_GetDevicePropertyTileX

         Returns: recommended horizontal quilt tiling dimension
         Note: Only works with Holoplay Service 1.2.0 or later; Holoplay Core 0.1.2 or later
    */
    IMPORT_DECL int hpc_GetDevicePropertyTileX(int dev_index);

    /*
        hpc_GetDevicePropertyTileY

         Returns: recommended vertical quilt tiling dimension
         Note: Only works with Holoplay Service 1.2.0 or later; Holoplay Core 0.1.2 or later
    */
    IMPORT_DECL int hpc_GetDevicePropertyTileY(int dev_index);

    /*
        hpc_GetDevicePropertyQuiltAspect

         Returns: recommended quilt aspect ratio.
         Note: Only works with Holoplay Service 1.2.0 or later; Holoplay Core 0.1.2 or later
    */
    IMPORT_DECL float hpc_GetDevicePropertyQuiltAspect(int dev_index);
#ifdef __cplusplus
}
#endif
