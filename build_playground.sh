#!/usr/bin/env bash

# Check dependencies
if ! which emcc;
then
  echo -e "\nConfiguration error: Emscripten was not found. Please install the recommended emsdk and rerun the script."
  echo "If Emscripten is installed, make sure to enable its command by running \`source path/to/emsdk_env.sh\` in your terminal."
  exit 1
fi

output_dir="playground/dist"
source_dir="playground/src"
interpreter_dir=src

echo -e "\nCreating directory \`$output_dir\` for the distribution files..."
rm -rf $output_dir
mkdir $output_dir
echo "Done!"

echo -e "\nCompiling to Wasm and generating distribution files..."
# TODO: Don't import `$interpreter_dir/main.c`
# files_to_compile="$source_dir/main_wasm.c $interpreter_dir/compiler.c $interpreter_dir/debug.c $interpreter_dir/gc_object.c $interpreter_dir/memory.c $interpreter_dir/program.c $interpreter_dir/table.c $interpreter_dir/thusly_value.c $interpreter_dir/tokenizer.c $interpreter_dir/vm.c"
files_to_compile="$source_dir/main_wasm.c $interpreter_dir/*.c"
# Flags: (see https://emscripten.org/docs/tools_reference/emcc.html)
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
# --no-entry:
#   Will not enter `main()` function.
#   (Adding this option until `$interpreter_dir/main.c` can be removed from `$files_to_compile`.)
emcc $files_to_compile -o $output_dir/index.html -O3 --shell-file $source_dir/playground.html -s NO_EXIT_RUNTIME=1 -s "EXPORTED_RUNTIME_METHODS=['ccall']" --no-entry
cp $source_dir/*.js $output_dir
echo "Done!"

echo -e "\nCongratulations! You can start the playground by opening \`$output_dir/index.html\` in the browser via an \`http://\` server (not \`file://\`)."
