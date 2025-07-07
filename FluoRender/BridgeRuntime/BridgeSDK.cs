using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.IO;
using System.Text.RegularExpressions;

namespace BridgeSDK
{
    #region Types
    [StructLayout(LayoutKind.Sequential)]
    public struct Window
    {
        private readonly UInt32 handle;

        public Window(UInt32 handle)
        {
            this.handle = handle;
        }

        public static implicit operator UInt32(Window window)
        {
            return window.handle;
        }

        public static implicit operator Window(UInt32 handle)
        {
            return new Window(handle);
        }
    }

    public enum UniqueHeadIndices : uint
    {
        FirstLookingGlassDevice = uint.MaxValue
    }

    public enum PixelFormats : uint
    {
        NoFormat = 0x0,
        RGB = 0x1907,
        RGBA = 0x1908,
        BGRA = 0x80E1,
        Red = 0x1903,
        RGB_DXT1 = 0x83F0,
        RGBA_DXT5 = 0x83F3,
        YCoCg_DXT5 = 0x01,
        A_RGTC1 = 0x8DBB,
        SRGB = 0x8C41,
        SRGB_A = 0x8C43,
        R32F = 0x822E,
        RGBA32F = 0x8814
    }

    public enum MTLPixelFormats : uint
    {
        RGBA8Unorm = 70,  // MTLPixelFormatRGBA8Unorm
        BGRA8Unorm = 80,  // MTLPixelFormatBGRA8Unorm
        RGBA16Float = 77,  // MTLPixelFormatRGBA16Float
        RGBA32Float = 124, // MTLPixelFormatRGBA32Float
        R32Float = 85,  // MTLPixelFormatR32Float
        RG32Float = 87,  // MTLPixelFormatRG32Float
        RGB10A2Unorm = 24,  // MTLPixelFormatRGB10A2Unorm
        RG16Float = 74,  // MTLPixelFormatRG16Float
        RG16Unorm = 62,  // MTLPixelFormatRG16Unorm
        Depth32Float = 252, // MTLPixelFormatDepth32Float
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct CalibrationSubpixelCell
    {
        public float ROffsetX;
        public float ROffsetY;
        public float GOffsetX;
        public float GOffsetY;
        public float BOffsetX;
        public float BOffsetY;
    }
    #endregion

    public partial class Controller
    {
        public const string BridgeVersion = "2.5.1";

        public static string SettingsPath()
        {
            string ret = "";

            if (GetPlatformID() == PlatformID.Win32NT)
            {
                IntPtr pPath;
                SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, IntPtr.Zero, out pPath);

                ret = Marshal.PtrToStringUni(pPath);
                Marshal.FreeCoTaskMem(pPath);

                ret = Path.Combine(ret, "Looking Glass", "Bridge");
            }
            else if (GetPlatformID() == PlatformID.MacOSX)
            {
                string home = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
                ret = Path.Combine(home, "Library", "Application Support", "Looking Glass", "Bridge");
            }
            else
            {
                string home = Environment.GetEnvironmentVariable("HOME");
                ret = Path.Combine(home, ".lgf", "Bridge");
            }

            ret = Path.Combine(ret, "settings.json");

            return ret;
        }

        public static string BridgeInstallLocation(string requestedVersion)
        {
            string ret = "";

            string settingsPath = SettingsPath();
            if (File.Exists(settingsPath))
            {
                string json = File.ReadAllText(settingsPath);

                // Extract "install_locations" array
                Match match = Regex.Match(json, "\"install_locations\":\\s*\\[(.*?)\\]", RegexOptions.Singleline);
                if (match.Success)
                {
                    string installLocations = match.Groups[1].Value;

                    // Parse each object within the array
                    var versionPaths = new Dictionary<string, string>();
                    foreach (Match entryMatch in Regex.Matches(installLocations, "\\{(.*?)\\}", RegexOptions.Singleline))
                    {
                        string entry = entryMatch.Value;

                        string version = Regex.Match(entry, "\"version\":\\s*\"(.*?)\"").Groups[1].Value;
                        string path = Regex.Match(entry, "\"path\":\\s*\"(.*?)\"").Groups[1].Value;

                        if (!string.IsNullOrWhiteSpace(version) && !string.IsNullOrWhiteSpace(path))
                        {
                            versionPaths[version] = Regex.Unescape(path);
                        }
                    }

                    // Check if the requested version exists
                    if (versionPaths.ContainsKey(requestedVersion))
                    {
                        return versionPaths[requestedVersion];
                    }

                    // Find the highest version with the same major version number
                    string majorVersionOfRequested = requestedVersion.Split('.')[0];
                    string highestVersion = null;
                    foreach (var vp in versionPaths)
                    {
                        string majorVersion = vp.Key.Split('.')[0];
                        if (majorVersion == majorVersionOfRequested)
                        {
                            if (highestVersion == null || string.CompareOrdinal(vp.Key, highestVersion) > 0)
                            {
                                highestVersion = vp.Key;
                            }
                        }
                    }

                    if (!string.IsNullOrEmpty(highestVersion))
                    {
                        return versionPaths[highestVersion];
                    }
                }
            }

            return ret;  // Return an empty string if no matching version found
        }

        private static PlatformID GetPlatformID()
        {
            if (!platformID.HasValue)
            {
                if (Environment.OSVersion.Platform == PlatformID.Win32NT)
                {
                    platformID = PlatformID.Win32NT;
                }
                else if (Environment.OSVersion.Platform == PlatformID.Unix)
                {
                    // Additional check to distinguish between Linux and macOS
                    if (IsRunningOnMac())
                    {
                        platformID = PlatformID.MacOSX;
                    }
                    else
                    {
                        platformID = PlatformID.Unix;  // Assuming Unix here means Linux
                    }
                }
                else
                {
                    platformID = Environment.OSVersion.Platform;
                }
            }

            return platformID.Value;
        }

        private static bool IsRunningOnMac()
        {
            // PInvoke to the system's uname function which returns system information
            IntPtr buf = IntPtr.Zero;
            try
            {
                buf = Marshal.AllocHGlobal(8192);
                // This is a hacktastic way of getting sysname from uname()
                if (uname(buf) == 0)
                {
                    string os = Marshal.PtrToStringAnsi(buf);
                    if (os == "Darwin")
                        return true;
                }
            }
            catch
            {
                // Handle exceptions if needed
            }
            finally
            {
                if (buf != IntPtr.Zero)
                    Marshal.FreeHGlobal(buf);
            }
            return false;
        }

        [DllImport("libc")]
        private static extern int uname(IntPtr buf);

        private static string libraryPath = "";
        private static PlatformID? platformID = null;

        private delegate bool InitializeInternalDelegateUTF16([MarshalAs(UnmanagedType.LPWStr)] string app_name);
        private delegate bool InitializeInternalDelegateUTF8([MarshalAs(UnmanagedType.LPUTF8Str)] string app_name);
        private delegate bool UninitializeDelegate();
        private delegate bool InstanceWindowGLDelegate(ref Window windowHandle, uint headIndex);
        private delegate bool GetWindowDimensionsDelegate(Window wnd, ref uint width, ref uint height);
        private delegate bool GetMaxTextureSizeDelegate(Window wnd, ref uint width);
        private delegate bool SetInteropQuiltTextureGLDelegate(Window wnd, ulong texture, PixelFormats format, uint width, uint height, uint vx, uint vy, float aspect, float zoom);
        private delegate bool DrawInteropQuiltTextureGLDelegate(Window wnd, ulong texture, PixelFormats format, uint width, uint height, uint vx, uint vy, float aspect, float zoom);
        private delegate bool SaveTextureToFileGLDelegateUTF16(Window wnd, [MarshalAs(UnmanagedType.LPWStr)] string filename, ulong texture, PixelFormats format, uint width, uint height);
        private delegate bool SaveTextureToFileGLDelegateUTF8(Window wnd, [MarshalAs(UnmanagedType.LPUTF8Str)] string filename, ulong texture, PixelFormats format, uint width, uint height);
        private delegate bool InstanceWindowMetalDelegate(IntPtr metal_device, ref Window windowHandle, uint headIndex);
        private delegate bool CreateMetalTextureWithIOSurfaceDelegate(Window wnd, IntPtr descriptorHandle, out IntPtr texture);
        private delegate bool ReleaseMetalTextureDelegate(Window wnd, IntPtr texture);
        private delegate bool SaveMetalTextureToFileDelegate(Window wnd, [MarshalAs(UnmanagedType.LPUTF8Str)] string filename, IntPtr texture, PixelFormats format, uint width, uint height);
        private delegate bool DrawInteropQuiltTextureMetalDelegate(Window wnd, IntPtr texture, uint vx, uint vy, float aspect, float zoom);
        private delegate bool CopyMetalTextureDelegate(Window wnd, IntPtr src, IntPtr dest);
        private delegate bool InstanceWindowDXDelegate(IntPtr dxDevice, ref Window windowHandle, uint headIndex);
        private delegate bool RegisterTextureDXDelegate(Window wnd, IntPtr texture);
        private delegate bool UnregisterTextureDXDelegate(Window wnd, IntPtr texture);
        private delegate bool SaveTextureToFileDXDelegate(Window wnd, [MarshalAs(UnmanagedType.LPWStr)] string filename, IntPtr texture);
        private delegate bool DrawInteropQuiltTextureDXDelegate(Window wnd, IntPtr texture, uint vx, uint vy, float aspect, float zoom);        
        private delegate bool GetCalibrationDelegate(Window wnd, ref float center, ref float pitch, ref float slope, ref int width, ref int height, ref float dpi, ref float flip_x, ref int invView, ref float viewcone, ref float fringe, ref int cell_pattern_mode, ref int number_of_cells, IntPtr cells);
        private delegate bool ShowWindowDelegate(Window wnd, bool flag);
        private delegate bool CreateTextureDXDelegate(Window wnd, uint width, uint height, out IntPtr dx_texture);
        private delegate bool ReleaseTextureDXDelegate(Window wnd, IntPtr dx_texture);
        private delegate bool CopyTextureDXDelegate(Window wnd, IntPtr src, IntPtr dest);
        private delegate bool SaveImageToFileDelegateUTF8(Window wnd, [MarshalAs(UnmanagedType.LPUTF8Str)]string filename, IntPtr image, PixelFormats format, ulong width, ulong height);
        private delegate bool SaveImageToFileDelegateUTF16(Window wnd, [MarshalAs(UnmanagedType.LPWStr)] string filename, IntPtr image, PixelFormats format, ulong width, ulong height);
        private delegate bool DeviceFromResourceDXDelegate(IntPtr dx_resource, out IntPtr dx_device);
        private delegate bool ReleaseDeviceDXDelegate(IntPtr dx_device);
        private delegate bool GetDeviceNameDelegate(Window wnd, ref int numberOfDeviceNameWchars, IntPtr deviceName);
        private delegate bool GetDeviceSerialDelegate(Window wnd, ref int numberOfSerialWchars, IntPtr serial);
        private delegate bool GetDefaultQuiltSettingsDelegate(Window wnd, ref float aspect, ref int quiltX, ref int quiltY, ref int tileX, ref int tileY);
        private delegate bool GetDisplayForWindowDelegate(Window wnd, ref ulong display_index);
        private delegate bool GetDeviceTypeDelegate(Window wnd, ref int hw_enum);
        private delegate bool GetViewConeDelegate(Window wnd, ref float viewcone);
        private delegate bool GetDisplaysDelegate(ref int number_of_indices, ulong[] indices);
        private delegate bool GetDeviceNameForDisplayDelegate(ulong display_index, ref int number_of_device_name_wchars, IntPtr device_name);
        private delegate bool GetDeviceSerialForDisplayDelegate(ulong display_index, ref int number_of_serial_wchars, IntPtr serial);
        private delegate bool GetDimensionsForDisplayDelegate(ulong display_index, ref ulong width, ref ulong height);
        private delegate bool GetDeviceTypeForDisplayDelegate(ulong display_index, ref int hw_enum);
        private delegate bool GetCalibrationForDisplayDelegate(ulong display_index, ref float center, ref float pitch, ref float slope, ref int width, ref int height, ref float dpi, ref float flip_x, ref int invView, ref float viewcone, ref float fringe, ref int cell_pattern_mode, ref int number_of_cells, IntPtr cells);
        private delegate bool GetInvViewForDisplayDelegate(ulong display_index, ref int invview);
        private delegate bool GetRiForDisplayDelegate(ulong display_index, ref int ri);
        private delegate bool GetBiForDisplayDelegate(ulong display_index, ref int bi);
        private delegate bool GetTiltForDisplayDelegate(ulong display_index, ref float tilt);
        private delegate bool GetDisplayAspectForDisplayDelegate(ulong display_index, ref float displayaspect);
        private delegate bool GetFringeForDisplayDelegate(ulong display_index, ref float fringe);
        private delegate bool GetSubpForDisplayDelegate(ulong display_index, ref float subp);
        private delegate bool GetViewConeForDisplayDelegate(ulong display_index, ref float viewcone);
        private delegate bool GetPitchForDisplayDelegate(ulong display_index, ref float pitch);
        private delegate bool GetCenterForDisplayDelegate(ulong display_index, ref float center);
        private delegate bool GetDefaultQuiltSettingsForDisplayDelegate(ulong display_index, ref float aspect, ref int quilt_width, ref int quilt_height, ref int quilt_columns, ref int quilt_rows);
        private delegate bool GetInvViewDelegate(Window wnd, ref int invview);
        private delegate bool GetRiDelegate(Window wnd, ref int ri);
        private delegate bool GetBiDelegate(Window wnd, ref int bi);
        private delegate bool GetTiltDelegate(Window wnd, ref float tilt);
        private delegate bool GetDisplayAspectDelegate(Window wnd, ref float displayaspect);
        private delegate bool GetFringeDelegate(Window wnd, ref float fringe);
        private delegate bool GetSubpDelegate(Window wnd, ref float subp);
        private delegate bool GetPitchDelegate(Window wnd, ref float pitch);
        private delegate bool GetCenterDelegate(Window wnd, ref float center);
        private delegate bool GetWindowPositionDelegate(Window wnd, ref long x, ref long y);
        private delegate bool GetPositionForDisplayDelegate(ulong display_index, ref long x, ref long y);
        private delegate bool InstanceOffscreenWindowGLDelegate(ref Window windowHandle, uint headIndex);
        private delegate bool GetOffscreenWindowTextureGLDelegate(Window wnd, out ulong texture, out PixelFormats format, out uint width, out uint height);
        private delegate bool GetOffscreenWindowTextureDXDelegate(Window wnd, out IntPtr texture);
        private delegate bool InstanceOffscreenWindowDXDelegate(IntPtr dxDevice, ref Window windowHandle, uint headIndex);
        private delegate bool InstanceOffscreenWindowMetalDelegate(IntPtr metalDevice, ref IntPtr wnd, ulong displayIndex);
        private delegate bool GetOffscreenWindowTextureMetalDelegate(IntPtr wnd, ref IntPtr texture);

        private static class DynamicLibraryLoader
        {
            [DllImport("kernel32", SetLastError = true)]
            private static extern IntPtr LoadLibrary(string path);

            [DllImport("kernel32", SetLastError = true)]
            private static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

            [DllImport("libdl")]
            private static extern IntPtr dlopen(string path, int mode);

            [DllImport("libdl")]
            private static extern IntPtr dlsym(IntPtr handle, string symbol);

            private static Dictionary<string, Delegate> functionCache = new Dictionary<string, Delegate>();

            public static T LoadFunction<T>(string path, string functionName) where T : Delegate
            {
                string key = path + functionName;
                if (functionCache.TryGetValue(key, out Delegate existingDelegate))
                {
                    return existingDelegate as T;
                }

                IntPtr handle = GetPlatformID() == PlatformID.Win32NT ? LoadLibrary(path) : dlopen(path, 1);
                if (handle == IntPtr.Zero) throw new Exception("Failed to load library.");

                IntPtr funcPtr = GetPlatformID() == PlatformID.Win32NT ? GetProcAddress(handle, functionName) : dlsym(handle, functionName);
                if (funcPtr == IntPtr.Zero) throw new Exception("Failed to get function pointer.");

                T delegateInstance = Marshal.GetDelegateForFunctionPointer<T>(funcPtr);
                functionCache[key] = delegateInstance;
                return delegateInstance;
            }
        }

        public static bool Initialize(string app_name, string desired_bridge_version = BridgeVersion)
        {
            string installPath = BridgeInstallLocation(desired_bridge_version);
            return InitializeWithPath(app_name, installPath);
        }

        public static bool InitializeWithPath(string app_name, string manual_install_location)
        {
            if (string.IsNullOrEmpty(manual_install_location))
            {
                return false;
            }

            if (GetPlatformID() == PlatformID.MacOSX)
            {
                libraryPath = Path.Combine(manual_install_location, "libbridge_inproc.dylib");

                try
                {
                    var func = DynamicLibraryLoader.LoadFunction<InitializeInternalDelegateUTF8>(libraryPath, "initialize_bridge");
                    return func(app_name);
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Error: " + ex.Message);
                    return false;
                }
            }
            else if (GetPlatformID() == PlatformID.Win32NT)
            {
                SetDllDirectory(manual_install_location);

                libraryPath = Path.Combine(manual_install_location, "bridge_inproc.dll");

                try
                {
                    var func = DynamicLibraryLoader.LoadFunction<InitializeInternalDelegateUTF16>(libraryPath, "initialize_bridge");
                    return func(app_name);
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Error: " + ex.Message);
                    return false;
                }
            }
            else
            {
                return false;
            }
        }

        public static bool Uninitialize()
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<UninitializeDelegate>(libraryPath, "uninitialize_bridge");
                return func();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool InstanceWindowGL(ref Window windowHandle, uint headIndex = (uint)UniqueHeadIndices.FirstLookingGlassDevice)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<InstanceWindowGLDelegate>(libraryPath, "instance_window_gl");
                return func(ref windowHandle, headIndex);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetWindowDimensions(Window wnd, ref uint width, ref uint height)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetWindowDimensionsDelegate>(libraryPath, "get_window_dimensions");
                return func(wnd, ref width, ref height);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetMaxTextureSize(Window wnd, ref uint width)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetMaxTextureSizeDelegate>(libraryPath, "get_max_texture_size");
                return func(wnd, ref width);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool SetInteropQuiltTextureGL(Window wnd, ulong texture, PixelFormats format, uint width, uint height, uint vx, uint vy, float aspect, float zoom)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<SetInteropQuiltTextureGLDelegate>(libraryPath, "set_interop_quilt_texture_gl");
                return func(wnd, texture, format, width, height, vx, vy, aspect, zoom);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool DrawInteropQuiltTextureGL(Window wnd, ulong texture, PixelFormats format, uint width, uint height, uint vx, uint vy, float aspect, float zoom)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<DrawInteropQuiltTextureGLDelegate>(libraryPath, "draw_interop_quilt_texture_gl");
                return func(wnd, texture, format, width, height, vx, vy, aspect, zoom);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool SaveTextureToFileGL(Window wnd, string filename, ulong texture, PixelFormats format, uint width, uint height)
        {
            try
            {
                if (GetPlatformID() == PlatformID.Win32NT)
                {
                    var func = DynamicLibraryLoader.LoadFunction<SaveTextureToFileGLDelegateUTF16>(libraryPath, "save_texture_to_file_gl");
                    return func(wnd, filename, texture, format, width, height);
                }
                else
                {
                    var func = DynamicLibraryLoader.LoadFunction<SaveTextureToFileGLDelegateUTF8>(libraryPath, "save_texture_to_file_gl");
                    return func(wnd, filename, texture, format, width, height);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool InstanceWindowMetal(IntPtr metal_device, ref Window windowHandle, uint headIndex = (uint)UniqueHeadIndices.FirstLookingGlassDevice)
        {
            try
            {
                if (GetPlatformID() == PlatformID.MacOSX)
                {
                    var func = DynamicLibraryLoader.LoadFunction<InstanceWindowMetalDelegate>(libraryPath, "instance_window_metal");
                    return func(metal_device, ref windowHandle, headIndex);
                }
                return false;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool InstanceOffscreenWindowMetal(IntPtr metalDevice, ref IntPtr wnd, ulong displayIndex = (ulong)UniqueHeadIndices.FirstLookingGlassDevice)
        {
            try
            {
                if (GetPlatformID() == PlatformID.MacOSX)
                {
                    var func = DynamicLibraryLoader.LoadFunction<InstanceOffscreenWindowMetalDelegate>(libraryPath, "instance_offscreen_window_metal");
                    return func(metalDevice, ref wnd, displayIndex);
                }
                return false;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetOffscreenWindowTextureMetal(IntPtr wnd, ref IntPtr texture)
        {
            try
            {
                if (GetPlatformID() == PlatformID.MacOSX)
                {
                    var func = DynamicLibraryLoader.LoadFunction<GetOffscreenWindowTextureMetalDelegate>(libraryPath, "get_offscreen_window_texture_metal");
                    return func(wnd, ref texture);
                }
                return false;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool CreateMetalTextureWithIOSurface(Window wnd, IntPtr descriptorHandle, out IntPtr texture)
        {
            try
            {
                if (GetPlatformID() == PlatformID.MacOSX)
                {
                    var func = DynamicLibraryLoader.LoadFunction<CreateMetalTextureWithIOSurfaceDelegate>(libraryPath, "create_metal_texture_with_iosurface");
                    return func(wnd, descriptorHandle, out texture);
                }
                texture = IntPtr.Zero;
                return false;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                texture = IntPtr.Zero;
                return false;
            }
        }

        public static bool ReleaseMetalTexture(Window wnd, IntPtr texture)
        {
            try
            {
                if (GetPlatformID() == PlatformID.MacOSX)
                {
                    var func = DynamicLibraryLoader.LoadFunction<ReleaseMetalTextureDelegate>(libraryPath, "release_metal_texture");
                    return func(wnd, texture);
                }
                return false;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool SaveMetalTextureToFile(Window wnd, string filename, IntPtr texture, PixelFormats format, uint width, uint height)
        {
            try
            {
                if (GetPlatformID() == PlatformID.MacOSX)
                {
                    var func = DynamicLibraryLoader.LoadFunction<SaveMetalTextureToFileDelegate>(libraryPath, "save_metal_texture_to_file");
                    return func(wnd, filename, texture, format, width, height);
                }
                return false;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool DrawInteropQuiltTextureMetal(Window wnd, IntPtr texture, uint vx, uint vy, float aspect, float zoom)
        {
            try
            {
                if (GetPlatformID() == PlatformID.MacOSX)
                {
                    var func = DynamicLibraryLoader.LoadFunction<DrawInteropQuiltTextureMetalDelegate>(libraryPath, "draw_interop_quilt_texture_metal");
                    return func(wnd, texture, vx, vy, aspect, zoom);
                }
                return false;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool CopyMetalTexture(Window wnd, IntPtr src, IntPtr dest)
        {
            try
            {
                if (Environment.OSVersion.Platform == PlatformID.MacOSX)
                {
                    var func = DynamicLibraryLoader.LoadFunction<CopyMetalTextureDelegate>(libraryPath, "copy_metal_texture");
                    return func(wnd, src, dest);
                }
                return false;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool InstanceWindowDX(IntPtr dxDevice, ref Window windowHandle, uint headIndex = (uint)UniqueHeadIndices.FirstLookingGlassDevice)
        {
            try
            {
                if (GetPlatformID() == PlatformID.Win32NT)
                {
                    var func = DynamicLibraryLoader.LoadFunction<InstanceWindowDXDelegate>(libraryPath, "instance_window_dx");
                    return func(dxDevice, ref windowHandle, headIndex);
                }
                return false;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool RegisterTextureDX(Window wnd, IntPtr texture)
        {
            try
            {
                if (GetPlatformID() == PlatformID.Win32NT)
                {
                    var func = DynamicLibraryLoader.LoadFunction<RegisterTextureDXDelegate>(libraryPath, "register_texture_dx");
                    return func(wnd, texture);
                }
                return false;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool UnregisterTextureDX(Window wnd, IntPtr texture)
        {
            try
            {
                if (GetPlatformID() == PlatformID.Win32NT)
                {
                    var func = DynamicLibraryLoader.LoadFunction<UnregisterTextureDXDelegate>(libraryPath, "unregister_texture_dx");
                    return func(wnd, texture);
                }
                return false;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool SaveTextureToFileDX(Window wnd, string filename, IntPtr texture)
        {
            try
            {
                if (GetPlatformID() == PlatformID.Win32NT)
                {
                    var func = DynamicLibraryLoader.LoadFunction<SaveTextureToFileDXDelegate>(libraryPath, "save_texture_to_file_dx");
                    return func(wnd, filename, texture);
                }
                return false;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool DrawInteropQuiltTextureDX(Window wnd, IntPtr texture, uint vx, uint vy, float aspect, float zoom)
        {
            try
            {
                if (GetPlatformID() == PlatformID.Win32NT)
                {
                    var func = DynamicLibraryLoader.LoadFunction<DrawInteropQuiltTextureDXDelegate>(libraryPath, "draw_interop_quilt_texture_dx");
                    return func(wnd, texture, vx, vy, aspect, zoom);
                }
                return false;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool ShowWindow(Window wnd, bool flag)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<ShowWindowDelegate>(libraryPath, "show_window");
                return func(wnd, flag);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool CreateTextureDX(Window wnd, uint width, uint height, out IntPtr dx_texture)
        {
            try
            {
                if (GetPlatformID() != PlatformID.Win32NT)
                {
                    dx_texture = IntPtr.Zero;
                    return false;
                }
                var func = DynamicLibraryLoader.LoadFunction<CreateTextureDXDelegate>(libraryPath, "create_texture_dx");
                return func(wnd, width, height, out dx_texture);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                dx_texture = IntPtr.Zero;
                return false;
            }
        }

        public static bool ReleaseTextureDX(Window wnd, IntPtr dx_texture)
        {
            try
            {
                if (GetPlatformID() != PlatformID.Win32NT)
                {
                    return false;
                }
                var func = DynamicLibraryLoader.LoadFunction<ReleaseTextureDXDelegate>(libraryPath, "release_texture_dx");
                return func(wnd, dx_texture);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool CopyTextureDX(Window wnd, IntPtr src, IntPtr dest)
        {
            try
            {
                if (GetPlatformID() != PlatformID.Win32NT)
                {
                    return false;
                }
                var func = DynamicLibraryLoader.LoadFunction<CopyTextureDXDelegate>(libraryPath, "copy_texture_dx");
                return func(wnd, src, dest);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetOffscreenWindowTextureDX(Window wnd, out IntPtr texture)
        {
            texture = IntPtr.Zero;

            try
            {
                if (GetPlatformID() == PlatformID.Win32NT)
                {
                    var func = DynamicLibraryLoader.LoadFunction<GetOffscreenWindowTextureDXDelegate>(libraryPath, "get_offscreen_window_texture_dx");
                    return func(wnd, out texture);
                }
                return false;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool SaveImageToFile(Window wnd, string filename, IntPtr image, PixelFormats format, ulong width, ulong height)
        {
            try
            {
                if (GetPlatformID() == PlatformID.Win32NT)
                {
                    var func = DynamicLibraryLoader.LoadFunction<SaveImageToFileDelegateUTF16>(libraryPath, "save_image_to_file");
                    return func(wnd, filename, image, format, width, height);
                }
                else
                {
                    var func = DynamicLibraryLoader.LoadFunction<SaveImageToFileDelegateUTF8>(libraryPath, "save_image_to_file");
                    return func(wnd, filename, image, format, width, height);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool DeviceFromResourceDX(IntPtr dx_resource, out IntPtr dx_device)
        {
            try
            {
                if (GetPlatformID() != PlatformID.Win32NT)
                {
                    dx_device = IntPtr.Zero;
                    return false; // Not supported on non-Windows platforms
                }

                var func = DynamicLibraryLoader.LoadFunction<DeviceFromResourceDXDelegate>(libraryPath, "device_from_resource_dx");
                return func(dx_resource, out dx_device);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                dx_device = IntPtr.Zero;
                return false;
            }
        }

        public static bool ReleaseDeviceDX(IntPtr dx_device)
        {
            try
            {
                if (GetPlatformID() != PlatformID.Win32NT)
                {
                    return false; // Not supported on non-Windows platforms
                }

                var func = DynamicLibraryLoader.LoadFunction<ReleaseDeviceDXDelegate>(libraryPath, "release_device_dx");
                return func(dx_device);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetDeviceName(Window wnd, ref int numberOfDeviceNameWchars, IntPtr deviceName)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetDeviceNameDelegate>(libraryPath, "get_device_name");
                return func(wnd, ref numberOfDeviceNameWchars, deviceName);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetDeviceSerial(Window wnd, ref int numberOfSerialWchars, IntPtr serial)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetDeviceSerialDelegate>(libraryPath, "get_device_serial");
                return func(wnd, ref numberOfSerialWchars, serial);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetDefaultQuiltSettings(Window wnd, ref float aspect, ref int quiltX, ref int quiltY, ref int tileX, ref int tileY)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetDefaultQuiltSettingsDelegate>(libraryPath, "get_default_quilt_settings");
                return func(wnd, ref aspect, ref quiltX, ref quiltY, ref tileX, ref tileY);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetDisplayForWindow(Window wnd, ref ulong display_index)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetDisplayForWindowDelegate>(libraryPath, "get_display_for_window");
                return func(wnd, ref display_index);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetDeviceType(Window wnd, ref int hw_enum)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetDeviceTypeDelegate>(libraryPath, "get_device_type");
                return func(wnd, ref hw_enum);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetViewCone(Window wnd, ref float viewcone)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetViewConeDelegate>(libraryPath, "get_viewcone");
                return func(wnd, ref viewcone);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetDisplays(ref int number_of_indices, ulong[] indices)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetDisplaysDelegate>(libraryPath, "get_displays");
                return func(ref number_of_indices, indices);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetDeviceNameForDisplay(ulong display_index, ref int number_of_device_name_wchars, IntPtr device_name)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetDeviceNameForDisplayDelegate>(libraryPath, "get_device_name_for_display");
                return func(display_index, ref number_of_device_name_wchars, device_name);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetDeviceSerialForDisplay(ulong display_index, ref int number_of_serial_wchars, IntPtr serial)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetDeviceSerialForDisplayDelegate>(libraryPath, "get_device_serial_for_display");
                return func(display_index, ref number_of_serial_wchars, serial);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetDimensionsForDisplay(ulong display_index, ref ulong width, ref ulong height)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetDimensionsForDisplayDelegate>(libraryPath, "get_dimensions_for_display");
                return func(display_index, ref width, ref height);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetDeviceTypeForDisplay(ulong display_index, ref int hw_enum)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetDeviceTypeForDisplayDelegate>(libraryPath, "get_device_type_for_display");
                return func(display_index, ref hw_enum);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetCalibrationForDisplay(ulong display_index, ref float center, ref float pitch, ref float slope, ref int width, ref int height, 
                                                    ref float dpi, ref float flip_x, ref int invView, ref float viewcone, ref float fringe, 
                                                    ref int cell_pattern_mode, ref int number_of_cells, IntPtr cells)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetCalibrationForDisplayDelegate>(libraryPath, "get_calibration_for_display");
                return func(display_index, ref center, ref pitch, ref slope, ref width, ref height, ref dpi, ref flip_x, ref invView, ref viewcone, ref fringe, ref cell_pattern_mode, ref number_of_cells, cells);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetInvViewForDisplay(ulong display_index, ref int invview)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetInvViewForDisplayDelegate>(libraryPath, "get_invview_for_display");
                return func(display_index, ref invview);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetRiForDisplay(ulong display_index, ref int ri)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetRiForDisplayDelegate>(libraryPath, "get_ri_for_display");
                return func(display_index, ref ri);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetBiForDisplay(ulong display_index, ref int bi)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetBiForDisplayDelegate>(libraryPath, "get_bi_for_display");
                return func(display_index, ref bi);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetTiltForDisplay(ulong display_index, ref float tilt)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetTiltForDisplayDelegate>(libraryPath, "get_tilt_for_display");
                return func(display_index, ref tilt);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetDisplayAspectForDisplay(ulong display_index, ref float displayaspect)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetDisplayAspectForDisplayDelegate>(libraryPath, "get_displayaspect_for_display");
                return func(display_index, ref displayaspect);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetFringeForDisplay(ulong display_index, ref float fringe)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetFringeForDisplayDelegate>(libraryPath, "get_fringe_for_display");
                return func(display_index, ref fringe);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetSubpForDisplay(ulong display_index, ref float subp)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetSubpForDisplayDelegate>(libraryPath, "get_subp_for_display");
                return func(display_index, ref subp);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetViewConeForDisplay(ulong display_index, ref float viewcone)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetViewConeForDisplayDelegate>(libraryPath, "get_viewcone_for_display");
                return func(display_index, ref viewcone);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetPitchForDisplay(ulong display_index, ref float pitch)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetPitchForDisplayDelegate>(libraryPath, "get_pitch_for_display");
                return func(display_index, ref pitch);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetCenterForDisplay(ulong display_index, ref float center)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetCenterForDisplayDelegate>(libraryPath, "get_center_for_display");
                return func(display_index, ref center);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetDefaultQuiltSettingsForDisplay(ulong display_index, ref float aspect, ref int quilt_width, ref int quilt_height, ref int quilt_columns, ref int quilt_rows)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetDefaultQuiltSettingsForDisplayDelegate>(libraryPath, "get_default_quilt_settings_for_display");
                return func(display_index, ref aspect, ref quilt_width, ref quilt_height, ref quilt_columns, ref quilt_rows);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetCalibration(Window wnd, ref float center, ref float pitch, ref float slope, ref int width, ref int height, ref float dpi, ref float flip_x,
                                          ref int invView, ref float viewcone, ref float fringe, ref int cell_pattern_mode, ref int number_of_cells, IntPtr cells)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetCalibrationDelegate>(libraryPath, "get_calibration");
                return func(wnd, ref center, ref pitch, ref slope, ref width, ref height, ref dpi, ref flip_x, ref invView, ref viewcone, ref fringe, ref cell_pattern_mode, ref number_of_cells, cells);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetInvView(Window wnd, ref int invview)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetInvViewDelegate>(libraryPath, "get_invview");
                return func(wnd, ref invview);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetRi(Window wnd, ref int ri)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetRiDelegate>(libraryPath, "get_ri");
                return func(wnd, ref ri);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetBi(Window wnd, ref int bi)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetBiDelegate>(libraryPath, "get_bi");
                return func(wnd, ref bi);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetTilt(Window wnd, ref float tilt)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetTiltDelegate>(libraryPath, "get_tilt");
                return func(wnd, ref tilt);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetDisplayAspect(Window wnd, ref float displayaspect)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetDisplayAspectDelegate>(libraryPath, "get_displayaspect");
                return func(wnd, ref displayaspect);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetFringe(Window wnd, ref float fringe)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetFringeDelegate>(libraryPath, "get_fringe");
                return func(wnd, ref fringe);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetSubp(Window wnd, ref float subp)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetSubpDelegate>(libraryPath, "get_subp");
                return func(wnd, ref subp);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetPitch(Window wnd, ref float pitch)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetPitchDelegate>(libraryPath, "get_pitch");
                return func(wnd, ref pitch);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetCenter(Window wnd, ref float center)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetCenterDelegate>(libraryPath, "get_center");
                return func(wnd, ref center);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetWindowPosition(Window wnd, ref long x, ref long y)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetWindowPositionDelegate>(libraryPath, "get_window_position");
                return func(wnd, ref x, ref y);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetWindowPositionForDisplay(ulong display_index, ref long x, ref long y)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetPositionForDisplayDelegate>(libraryPath, "get_window_position_for_display");
                return func(display_index, ref x, ref y);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool InstanceOffscreenWindowGL(ref Window windowHandle, uint headIndex = (uint)UniqueHeadIndices.FirstLookingGlassDevice)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<InstanceOffscreenWindowGLDelegate>(libraryPath, "instance_offscreen_window_gl");
                return func(ref windowHandle, headIndex);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        public static bool GetOffscreenWindowTextureGL(Window wnd, out ulong texture, out PixelFormats format, out uint width, out uint height)
        {
            try
            {
                var func = DynamicLibraryLoader.LoadFunction<GetOffscreenWindowTextureGLDelegate>(libraryPath, "get_offscreen_window_texture_gl");
                return func(wnd, out texture, out format, out width, out height);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                texture = 0;
                format = PixelFormats.NoFormat;
                width = 0;
                height = 0;
                return false;
            }
        }

        public static bool InstanceOffscreenWindowDX(IntPtr dxDevice, ref Window windowHandle, uint headIndex = (uint)UniqueHeadIndices.FirstLookingGlassDevice)
        {
            try
            {
                if (GetPlatformID() == PlatformID.Win32NT)
                {
                    var func = DynamicLibraryLoader.LoadFunction<InstanceOffscreenWindowDXDelegate>(libraryPath, "instance_offscreen_window_dx");
                    return func(dxDevice, ref windowHandle, headIndex);
                }
                return false;

            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
                return false;
            }
        }

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool SetDllDirectory(string lpPathName);

        private static readonly Guid FOLDERID_RoamingAppData = new Guid("3EB685DB-65F9-4CF6-A03A-E3EF65729F3D");

        [DllImport("shell32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        private static extern int SHGetKnownFolderPath([MarshalAs(UnmanagedType.LPStruct)] Guid rfid, uint dwFlags, IntPtr hToken, out IntPtr pszPath);
    }
}

