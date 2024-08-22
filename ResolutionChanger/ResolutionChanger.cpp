#include <windows.h>
#include <iostream>

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cout << "Usage: ChangeResolution <width> <height>\n";
        return 1;
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);

    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);
    devMode.dmPelsWidth = width;
    devMode.dmPelsHeight = height;
    devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

    //LONG result = ChangeDisplaySettings(&devMode, 0);
    // Add CDS_UPDATEREGISTRY to save the settings permanently
    LONG result = ChangeDisplaySettings(&devMode, CDS_UPDATEREGISTRY);
	
    if (result == DISP_CHANGE_SUCCESSFUL) {
        std::cout << "Resolution changed to " << width << "x" << height << " successfully.\n";
    }
    else {
        std::cout << "Failed to change resolution. Error code: " << result << "\n";
    }

    return 0;
}