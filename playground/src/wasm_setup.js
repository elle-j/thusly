const outputElement = document.getElementById("th-output");
const runElement = document.getElementById("th-run");

// const statusElement = document.getElementById("status");
// const progressElement = document.getElementById("progress");
// const spinnerElement = document.getElementById("spinner");

// Will be set `onRuntimeInitialized`.
function run(code) {}

function getPrintFn(/*isError = false*/) {
  outputElement.value = "";

  return function(text) {
    if (arguments.length > 1)
      text = Array.prototype.slice.call(arguments).join(" ");

    // These replacements are necessary if rendering to raw HTML.
    // text = text.replace(/&/g, "&amp;");
    // text = text.replace(/</g, "&lt;");
    // text = text.replace(/>/g, "&gt;");
    // text = text.replace("\n", "<br>", "g");

    // If also wanting to log to the JS console.
    // if (isError)
    //   console.error(text);
    // else
    //   console.log(text);

    outputElement.value += text + "\n";
    // Go to bottom
    // outputElement.scrollTop = outputElement.scrollHeight;
  };
};

// Need `var` or no declaration here due to JS glue code starting with:
// `var Module=typeof Module!="undefined"?Module:{}`
var Module = {
  preRun: [],

  postRun: [],

  print: getPrintFn(),

  printErr: getPrintFn(),

  // Invoked when the runtime is fully initialized; i.e., when compiled code is safe to run.
  onRuntimeInitialized: () => {
    run = function(code, debugCompilation, debugExecution) {
      if (!code)
        return;

      Module.ccall(
        "run_source",                             // C function name
        "int",                                    // Return type
        ["string", "boolean", "boolean"],         // Argument types
        [code, debugCompilation, debugExecution], // Arguments
      );
    };
    runElement.removeAttribute("disabled");
    runElement.style.cursor = "pointer";
  },

  setStatus: (text) => {
    // if (!Module.setStatus.last)
    //   Module.setStatus.last = { time: Date.now(), text: "" };

    // if (text === Module.setStatus.last.text)
    //   return;

    // const progressUpdate = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
    // const now = Date.now();
    // const updateTooSoon = now - Module.setStatus.last.time < 30;
    // if (progressUpdate && updateTooSoon)
    //   return;

    // Module.setStatus.last.time = now;
    // Module.setStatus.last.text = text;

    // if (progressUpdate) {
    //   text = progressUpdate[1];
    //   progressElement.value = parseInt(progressUpdate[2]) * 100;
    //   progressElement.max = parseInt(progressUpdate[4]) * 100;
    //   progressElement.hidden = false;
    //   spinnerElement.hidden = false;
    // }
    // else {
    //   progressElement.value = null;
    //   progressElement.max = null;
    //   progressElement.hidden = true;
    //   if (!text)
    //     spinnerElement.hidden = true;
    // }
    // statusElement.innerHTML = text;
  },

  totalDependencies: 0,

  monitorRunDependencies: (left) => {
    this.totalDependencies = Math.max(this.totalDependencies, left);
    Module.setStatus(left
      ? "Preparing... (" + (this.totalDependencies-left) + "/" + this.totalDependencies + ")"
      : "All downloads complete."
    );
  }
};

Module.setStatus("Downloading...");

window.onerror = () => {
  // Module.setStatus("Exception thrown, see JavaScript console");
  // spinnerElement.style.display = "none";
  Module.setStatus = (text) => {
    if (text)
      console.error("[post-exception status] " + text);
  };
};
