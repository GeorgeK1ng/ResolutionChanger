#include <windows.h>
#include <string>
#include <iostream>
#include <iomanip>  // For setting width and formatting

void ListMonitors() {
    DISPLAY_DEVICE displayDevice;
    ZeroMemory(&displayDevice, sizeof(displayDevice));
    displayDevice.cb = sizeof(displayDevice);

    // Print header with separators
    std::cout << std::left
        << std::setw(13) << "| Monitor" << " | "
        << std::setw(10) << "Resolution" << " | "
        << std::setw(11) << "Color Depth" << " | " << "\n";

    // Print separator line
    std::cout << " " << std::string(13, '-') << " " << std::string(12, '-') << " " << std::string(13, '-') << "\n";

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
                << std::setw(11) << (std::to_string(devMode.dmBitsPerPel) + " bits") << " | " << "\n";

            displayedMonitorIndex++;  // Increment the index for displayed monitors
        }

        monitorIndex++;
        ZeroMemory(&displayDevice, sizeof(displayDevice)); // Reset displayDevice for next iteration
        displayDevice.cb = sizeof(displayDevice);
    }
}

void SetResolution(int monitorIndex, int width, int height, int bitsPerPixel) {
    DISPLAY_DEVICE displayDevice;
    ZeroMemory(&displayDevice, sizeof(displayDevice));
    displayDevice.cb = sizeof(displayDevice);

    if (!EnumDisplayDevices(NULL, monitorIndex, &displayDevice, 0)) {
        std::cout << "Monitor " << monitorIndex + 1 << " not found.\n";
        return;
    }

    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);

    if (!EnumDisplaySettings(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &devMode)) {
        std::cout << "Failed to retrieve current settings for monitor " << monitorIndex + 1 << ".\n";
        return;
    }

    // Set the desired resolution and bits per pixel (color depth)
    devMode.dmPelsWidth = width;
    devMode.dmPelsHeight = height;
    devMode.dmBitsPerPel = bitsPerPixel;
    devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

    // Change the display settings for the specific monitor
    LONG result = ChangeDisplaySettingsEx(displayDevice.DeviceName, &devMode, NULL, CDS_UPDATEREGISTRY | CDS_GLOBAL, NULL);

    if (result == DISP_CHANGE_SUCCESSFUL) {
        std::cout << "Resolution changed to " << width << "x" << height
            << " with " << bitsPerPixel << " bits per pixel on monitor "
            << monitorIndex + 1 << " successfully.\n";
    }
    else {
        std::cout << "Failed to change resolution on monitor " << monitorIndex + 1 << ". Error code: " << result << "\n";
    }
}

int main(int argc, char* argv[])
{
    if (argc == 1) {
        // No arguments provided, list monitors
        ListMonitors();
        return 0;
    }

    if (std::string(argv[1]) == "/?" || std::string(argv[1]) == "/help") {
        std::cout << " Usage:\n";
        std::cout << "  To list monitors: " << "ResolutionChanger.exe" << "\n";
        std::cout << "  To set resolution: " << "ResolutionChanger.exe" << " /set <monitor_index> <width> <height> <bits_per_pixel>\n";
    }

    if (argc == 6 && std::string(argv[1]) == "/set") {
        int monitorIndex = atoi(argv[2]) - 1; // Convert to zero-based index
        int width = atoi(argv[3]);
        int height = atoi(argv[4]);
        int bitsPerPixel = atoi(argv[5]);
        SetResolution(monitorIndex, width, height, bitsPerPixel);
        return 0;
    }

    return 1;
}
