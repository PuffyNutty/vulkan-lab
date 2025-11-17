# VulkanLab – Experimental Project

I made this project to learn the Vulkan graphics API, so don't expect much of high quality. If you for some reason want to build it, here's how:


## How to build:

1) Install CMake, either from the website or using a package manager like homebrew. You need at least version 3.10.0
2) Install SDL3, preferably using a package manager, or some other way to ensure it gets embedded into CMake's search paths.
3) Install the VulkanSDK from LunarG. This is platform-specific, but you'll likely need to run install_vulkan.py at the very end to complete the process.
4) Cd into this project's directory (the one containing cmakelists.txt), and run 'cmake -S . -B build'. If no errors get thrown, this will create a makefile in the 'build' folder.
5) Build the executable using 'make -C build'. This will create an executable in ${PROJECT_DIR}/build/bin.
6) Run the executable using './build/bin/VulkanLab'.