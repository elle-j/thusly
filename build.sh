#!/usr/bin/env bash

# Check dependencies
if ! which cmake;
then
  echo "Configuration error: CMake was not found. Please install CMake v3.20 or higher and rerun the script."
fi

echo -e "\nCreating directory 'bin' for the build files and executable binary..."
rm -rf bin
mkdir bin
cd bin
echo "Done!"

echo -e "\nConfiguring project and generating native build system..."
cmake ../
echo "Done!"

echo -e "\nBuilding and linking the project..."
cmake --build .
echo "Done!"

# Add execute permission to the binary.
chmod +x cthusly

# Navigate back to the root.
cd ..
