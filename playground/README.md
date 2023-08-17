# Interactive Browser Playground for Thusly

An interactive playground in the browser for the Thusly programming language.

**---> THIS README IS A WORK IN PROGRESS <---**

## Getting Started

### Prerequisites

* [Emscripten](https://emscripten.org/docs/getting_started/downloads.html#download-and-install)

### Building the Project

Run the below commands to compile the C source code to Wasm and a corresponding HTML and JS file.

```sh
cd playground
mkdir build
emcc -o build/index.html src/main_wasm.c -O3 --shell-file src/playground.html
cp src/*.js build
```

### Starting the Playground

Once you have [built](#building-the-project) the project you can open `playground/build/index.html` in a browser via an HTTP server.

> **HTTP Server:**
>
> You cannot open the HTML file directly from your local hard drive (`file://`). You need to run it through an HTTP server (`http://`). If you are using VS Code, you can use the extension [Live Server](https://marketplace.visualstudio.com/items?itemName=ritwickdey.LiveServer).