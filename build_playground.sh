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
# Flags:
# -o:
#   To where the compiler should output the JavaScript glue code and HTML.
# -03:
#   Enables the highest optimization.
#   (Levels: -O0 (no optimization), -O1, -O2, -Os, -Oz, -Og, and -O3)
# --shell-file:
#   The path to the HTML template to use to create the distributed HTML.
# -s NO_EXIT_RUNTIME=1:
#   Prevents the runtime from shutting down when `main()` exits,
#   which would invalidate calls to compiled code.
emcc src/main_wasm.c -o dist/index.html -O3 --shell-file src/playground.html -s NO_EXIT_RUNTIME=1 -s "EXPORTED_RUNTIME_METHODS=['ccall']"
cp src/*.js dist
echo "Done!"

echo -e "\nCongratulations! You can start the playground by opening 'playground/dist/index.html' in the browser via an 'http://' server (not 'file://')."

# Navigate back to the root.
cd ..
