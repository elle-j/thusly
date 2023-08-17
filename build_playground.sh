#!/usr/bin/env bash

# Check dependencies
if ! which emcc;
then
  echo -e "\nConfiguration error: Emscripten was not found. Please install the recommended emsdk and rerun the script."
  echo "If Emscripten is installed, make sure to enable its command by running 'source path/to/emsdk_env.sh' in your terminal."
  exit 1
fi

echo -e "\nCreating directory 'playground/dist' for the distribution files..."
cd playground
rm -rf dist
mkdir dist
echo "Done!"

echo -e "\nCompiling to Wasm and generating distribution files..."
emcc -o dist/index.html src/main_wasm.c -O3 --shell-file src/playground.html
cp src/*.js dist
echo "Done!"

echo -e "\nCongratulations! You can start the playground by opening 'playground/dist/index.html' in the browser via an 'http://' server (not 'file://')."

# Navigate back to the root.
cd ..
