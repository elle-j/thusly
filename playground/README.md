# Interactive Browser Playground for Thusly

An interactive playground in the browser for the Thusly programming language.

**---> THIS README IS A WORK IN PROGRESS <---**

## Getting Started

### Prerequisites

* [Emscripten](https://emscripten.org/docs/getting_started/downloads.html#download-and-install)
  * Download the recommended emsdk.
  * Don't forget to activate PATH and other environment variables in your terminal after installing it.
    ```sh
    source path/to/emsdk_env.sh
    ```
* [Node.js](https://nodejs.org/en/download)

### Building the Project

Run the below command to compile the C source code to WebAssembly and generate browser distribution files to `playground/dist`.

```sh
./build_playground.sh
```

> **If permission is denied**, first add executable permission to the build script by running:
> `chmod +x build_playground.sh`.

### Starting the Playground

Once you have [built](#building-the-project) the project you can open `playground/dist/index.html` in a browser via an HTTP server.

> **HTTP Server:**
>
> You cannot open the HTML file directly from your local hard drive (`file://`). You need to run it through an HTTP server (`http://`). If you are using VS Code, you can use the extension [Live Server](https://marketplace.visualstudio.com/items?itemName=ritwickdey.LiveServer).
