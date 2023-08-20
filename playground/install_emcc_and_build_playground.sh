#!/usr/bin/env bash

if ! which emcc;
then
  echo -e "\nDownloading Emscripten..."
  git clone https://github.com/emscripten-core/emsdk.git
  echo "Done!"

  echo -e "\nInstalling and activating Emscripten..."
  cd emsdk
  ./emsdk install 3.1.41
  ./emsdk activate 3.1.41
  source ./emsdk_env.sh
  cd ..
  echo "Done!"
fi

./build_playground.sh
