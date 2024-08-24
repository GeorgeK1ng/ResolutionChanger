#include <windows.h>
#include <string>
#include <iostream>
#include <sstream> 
#include <iomanip>  // For setting width and formatting
#include <set>  // For storing unique display modes
#include <vector> // For sorting display modes
#include <algorithm> // For sort function

// Define a structure to hold unique display mode attributes
struct DisplayMode {
    DWORD width; 
    DWORD height; 
    DWORD colorDepth;
    DWORD refreshRate; 

    bool operator<(const DisplayMode& other) const {
        if (width != other.width) return width < other.width;
        if (height != other.height) return height < other.height;
        if (colorDepth != other.colorDepth) return colorDepth < other.colorDepth;
        return refreshRate < other.refreshRate;
    }
};

// Function to get descriptive error messages
std::string GetDisplayChangeError(LONG result) {
    switch (result) {
    case DISP_CHANGE_SUCCESSFUL:
        return "The display settings were successfully changed.";
    case DISP_CHANGE_RESTART:
        return "The computer must be restarted for the changes to take effect.";
    case DISP_CHANGE_BADFLAGS:
        return "Invalid flags were passed.";
    case DISP_CHANGE_BADPARAM:
        return "Invalid parameter was passed.";
    case DISP_CHANGE_FAILED:
        return "The display driver failed the specified graphics mode.";
    case DISP_CHANGE_BADMODE:
        return "The graphics mode is not supported.";
    case DISP_CHANGE_NOTUPDATED:
        return "The settings change was not applied. A reboot might be necessary.";
    default: {
        std::ostringstream oss;
        oss << "An unknown error occurred. Error code: " << result;
        return oss.str();
        }
    }
}

// Function to list all available display modes for a monitor
void ListAvailableModes(int monitorIndex) {
    DISPLAY_DEVICE displayDevice;
    ZeroMemory(&displayDevice, sizeof(displayDevice));
    displayDevice.cb = sizeof(displayDevice);

    if (!EnumDisplayDevices(NULL, monitorIndex, &displayDevice, 0)) {
        std::cout << "Error: Monitor " << monitorIndex + 1 << " not found.\n";
        return;
    }

    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);
    int modeIndex = 0;

    std::set<DisplayMode> uniqueModes;  // Set to store unique display modes

    // Print header with separators
    std::cout << std::left
        << std::setw(7) << "| Width" << " | "
        << std::setw(6) << "Height" << " | "
        << std::setw(11) << "Color Depth" << " | "
        << std::setw(12) << "Refresh Rate" << " |" << "\n";

    // Print separator line
    std::cout << " " << std::string(7, '-') << " " << std::string(8, '-') << " " << std::string(13, '-') << " " << std::string(14, '-') << " " << "\n";

    while (EnumDisplaySettings(displayDevice.DeviceName, modeIndex, &devMode)) {
        DisplayMode currentMode = { devMode.dmPelsWidth, devMode.dmPelsHeight, devMode.dmBitsPerPel, devMode.dmDisplayFrequency };

        // Check if this mode has already been listed
        if (uniqueModes.find(currentMode) == uniqueModes.end()) {
            // If not found, add to the set
            uniqueModes.insert(currentMode);
        }

        modeIndex++;
    }

    // Copy set to vector for sorting
    std::vector<DisplayMode> sortedModes(uniqueModes.begin(), uniqueModes.end());
    std::sort(sortedModes.begin(), sortedModes.end());  // Sort using the custom comparison operator

    // Print sorted modes
    for (const auto& mode : sortedModes) {
        std::cout << std::left
            << std::setw(7) << ("| " + std::to_string(mode.width)) << " | "
            << std::setw(6) << mode.height << " | "
            << std::setw(11) << (std::to_string(mode.colorDepth) + " bits") << " | "
            << std::setw(12) << (std::to_string(mode.refreshRate) + " Hz") << " | " << "\n";
    }
}

// Function to list monitors and their settings
void ListMonitors() {
    DISPLAY_DEVICE displayDevice;
    ZeroMemory(&displayDevice, sizeof(displayDevice));
    displayDevice.cb = sizeof(displayDevice);

    // Print header with separators
    std::cout << std::left
        << std::setw(13) << "| Monitor" << " | "
        << std::setw(10) << "Resolution" << " | "
        << std::setw(11) << "Color Depth" << " | "
        << std::setw(12) << "Refresh Rate" << " | " << "\n";

    // Print separator line
    std::cout << " " << std::string(13, '-') << " " << std::string(12, '-') << " " << std::string(13, '-') << " " << std::string(14, '-') << "\n";

    int monitorIndex = 0;
    int displayedMonitorIndex = 1;
    while (EnumDisplayDevices(NULL, monitorIndex, &displayDevice, 0)) {
        DEVMODE devMode;
        ZeroMemory(&devMode, sizeof(devMode));
        devMode.dmSize = sizeof(devMode);

        // Check if the monitor is active and connected
        if (EnumDisplaySettings(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &devMode)) {
            std::string monitorLabel = "Monitor " + std::to_string(displayedMonitorIndex);
            if (displayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
                monitorLabel += " *";
            }

            std::cout << std::left << std::setw(13) << ("| " + monitorLabel) << " | "
                << std::setw(10) << (std::to_string(devMode.dmPelsWidth) + "x" + std::to_string(devMode.dmPelsHeight)) << " | "
                << std::setw(11) << (std::to_string(devMode.dmBitsPerPel) + " bits") << " | "
                << std::setw(12) << (std::to_string(devMode.dmDisplayFrequency) + " Hz") << " | " << "\n";

            displayedMonitorIndex++;  // Increment the index for displayed monitors
        }

        monitorIndex++;
        ZeroMemory(&displayDevice, sizeof(displayDevice)); // Reset displayDevice for next iteration
        displayDevice.cb = sizeof(displayDevice);
    }
}

// Function to check if new settings match the current settings
bool AreSettingsSame(const DEVMODE& currentMode, int width, int height, int bitsPerPixel = -1, int refreshRate = -1) {
    bool sameResolution = (currentMode.dmPelsWidth == width && currentMode.dmPelsHeight == height);
    bool sameColorDepth = (bitsPerPixel == -1 || currentMode.dmBitsPerPel == bitsPerPixel);
    bool sameRefreshRate = (refreshRate == -1 || currentMode.dmDisplayFrequency == refreshRate);

    return sameResolution && sameColorDepth && sameRefreshRate;
}

// Function to set resolution for a monitor (width and height only)
void SetResolution1(int monitorIndex, int width, int height) {
    DISPLAY_DEVICE displayDevice;
    ZeroMemory(&displayDevice, sizeof(displayDevice));
    displayDevice.cb = sizeof(displayDevice);

    if (!EnumDisplayDevices(NULL, monitorIndex, &displayDevice, 0)) {
        std::cout << "Error: Monitor " << monitorIndex + 1 << " not found.\n";
        return;
    }

    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);

    if (!EnumDisplaySettings(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &devMode)) {
        std::cout << "Error: Failed to retrieve current settings for monitor " << monitorIndex + 1 << ".\n";
        return;
    }

    if (AreSettingsSame(devMode, width, height)) {
        std::cout << "The resolution is already set to " << width << "x" << height << " on monitor " << monitorIndex + 1 << ".\n";
        return;
    }

    // Set the desired resolution
    devMode.dmPelsWidth = width;
    devMode.dmPelsHeight = height;
    devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

    // Change the display settings for the specific monitor
    LONG result = ChangeDisplaySettingsEx(displayDevice.DeviceName, &devMode, NULL, CDS_UPDATEREGISTRY | CDS_GLOBAL, NULL);

    std::cout << GetDisplayChangeError(result) << "\n";
}

// Function to set resolution and color depth for a monitor
void SetResolution2(int monitorIndex, int width, int height, int bitsPerPixel) {
    DISPLAY_DEVICE displayDevice;
    ZeroMemory(&displayDevice, sizeof(displayDevice));
    displayDevice.cb = sizeof(displayDevice);

    if (!EnumDisplayDevices(NULL, monitorIndex, &displayDevice, 0)) {
        std::cout << "Error: Monitor " << monitorIndex + 1 << " not found.\n";
        return;
    }

    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);

    if (!EnumDisplaySettings(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &devMode)) {
        std::cout << "Error: Failed to retrieve current settings for monitor " << monitorIndex + 1 << ".\n";
        return;
    }

    if (AreSettingsSame(devMode, width, height, bitsPerPixel)) {
        std::cout << "The resolution and color depth are already set to " << width << "x" << height << " and " << bitsPerPixel << " bits on monitor " << monitorIndex + 1 << ".\n";
        return;
    }

    // Set the desired resolution and bits per pixel (color depth)
    devMode.dmPelsWidth = width;
    devMode.dmPelsHeight = height;
    devMode.dmBitsPerPel = bitsPerPixel;
    devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

    // Change the display settings for the specific monitor
    LONG result = ChangeDisplaySettingsEx(displayDevice.DeviceName, &devMode, NULL, CDS_UPDATEREGISTRY | CDS_GLOBAL, NULL);

    std::cout << GetDisplayChangeError(result) << "\n";
}

// Function to set resolution, color depth, and refresh rate for a monitor
void SetResolution3(int monitorIndex, int width, int height, int bitsPerPixel, int refreshRate) {
    DISPLAY_DEVICE displayDevice;
    ZeroMemory(&displayDevice, sizeof(displayDevice));
    displayDevice.cb = sizeof(displayDevice);

    if (!EnumDisplayDevices(NULL, monitorIndex, &displayDevice, 0)) {
        std::cout << "Error: Monitor " << monitorIndex + 1 << " not found.\n";
        return;
    }

    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);

    if (!EnumDisplaySettings(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &devMode)) {
        std::cout << "Error: Failed to retrieve current settings for monitor " << monitorIndex + 1 << ".\n";
        return;
    }

    if (AreSettingsSame(devMode, width, height, bitsPerPixel, refreshRate)) {
        std::cout << "The resolution, color depth, and refresh rate are already set to " << width << "x" << height << ", " << bitsPerPixel << " bits, and " << refreshRate << " Hz on monitor " << monitorIndex + 1 << ".\n";
        return;
    }

    // Set the desired resolution, bits per pixel (color depth), and refresh rate
    devMode.dmPelsWidth = width;
    devMode.dmPelsHeight = height;
    devMode.dmBitsPerPel = bitsPerPixel;
    devMode.dmDisplayFrequency = refreshRate;
    devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

    // Change the display settings for the specific monitor
    LONG result = ChangeDisplaySettingsEx(displayDevice.DeviceName, &devMode, NULL, CDS_UPDATEREGISTRY | CDS_GLOBAL, NULL);

    std::cout << GetDisplayChangeError(result) << "\n";
}

void ShowHelp() {
    std::cout << "\n ResolutionChanger\n\n";
    std::cout << " This tool allows you to list and modify display settings for connected monitors,\n";
    std::cout << " including setting screen resolution, color depth, and refresh rate.\n\n";
    std::cout << " Available Commands:\n";
    std::cout << "   /list                 - Lists all connected monitors and their current settings.\n";
    std::cout << "   /modes                - Lists all available display modes for a monitor.\n";
    std::cout << "   /set                  - Sets the resolution, color depth, and refresh rate for a monitor.\n";
    std::cout << "\n Usage Examples:\n";
    std::cout << "   /list                 - Displays the current settings for all monitors.\n";
    std::cout << "   /modes 1              - Lists all available display modes for Monitor 1.\n";
    std::cout << "   /set 1 1920 1080      - Sets Monitor 1 to 1920x1080 resolution.\n";
    std::cout << "   /set 2 1280 720 32    - Sets Monitor 2 to 1280x720 resolution with 32-bit color depth.\n";
    std::cout << "   /set 2 1280 720 32 60 - Sets Monitor 2 to 1280x720 resolution, 32-bit color depth, and 60 Hz refresh rate.\n\n";
    std::cout << " Notes: \n";
    std::cout << "     1) Ensure that you specify the correct monitor index when applying settings.\n";
    std::cout << "     2) The /set command requires at least 3 parameters: monitor_index, width, height.\n";
    std::cout << "     3) Additional parameters (bits per pixel and refresh rate) are optional.\n\n";
}

int main(int argc, char* argv[]) {
    if (argc == 1 || std::string(argv[1]) == "/list") {
        // No arguments provided, list monitors
        ListMonitors();
        return 0;
    }

    if (std::string(argv[1]) == "/?" || std::string(argv[1]) == "/help") {
        ShowHelp();
        return 0;
    }

    if (std::string(argv[1]) == "/set") {
        if (argc == 5) {
            int monitorIndex = atoi(argv[2]) - 1; // Convert to zero-based index
            int width = atoi(argv[3]);
            int height = atoi(argv[4]);

            SetResolution1(monitorIndex, width, height);
        }
        else if (argc == 6) {
            int monitorIndex = atoi(argv[2]) - 1;
            int width = atoi(argv[3]);
            int height = atoi(argv[4]);
            int bitsPerPixel = atoi(argv[5]);

            SetResolution2(monitorIndex, width, height, bitsPerPixel);
        }
        else if (argc == 7) {
            int monitorIndex = atoi(argv[2]) - 1;
            int width = atoi(argv[3]);
            int height = atoi(argv[4]);
            int bitsPerPixel = atoi(argv[5]);
            int refreshRate = atoi(argv[6]);

            SetResolution3(monitorIndex, width, height, bitsPerPixel, refreshRate);
        }
        else {
            std::cout << "Error: Invalid number of arguments for /set command. Use '/?' or '/help' for usage information.\n";
        }
        return 0;
    }

    if (std::string(argv[1]) == "/modes" && argc == 3) {
        int monitorIndex = atoi(argv[2]) - 1;
        ListAvailableModes(monitorIndex);
        return 0;
    }

    std::cout << "Error: Invalid command. Use '/?' or '/help' for usage information.\n";
    return 1;
}
