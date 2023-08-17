const statusElement = document.getElementById("status");
const progressElement = document.getElementById("progress");
const spinnerElement = document.getElementById("spinner");

// Need `var` or no declaration here.
var Module = {
  preRun: [],

  postRun: [],

  print: (function() {
    const element = document.getElementById("output");
    if (element)
      // Clear browser cache.
      element.value = "";

    return function(text) {
      if (arguments.length > 1)
        text = Array.prototype.slice.call(arguments).join(" ");

      // These replacements are necessary if you render to raw HTML.
      //text = text.replace(/&/g, "&amp;");
      //text = text.replace(/</g, "&lt;");
      //text = text.replace(/>/g, "&gt;");
      //text = text.replace("\n", "<br>", "g");

      console.log(text);

      if (element) {
        element.value += text + "\n";
        // Focus on bottom
        element.scrollTop = element.scrollHeight;
      }
    };
  })(),

  canvas: (() => {
    const canvas = document.getElementById("canvas");
    // As a default initial behavior, pop up an alert when webgl context is lost.
    canvas.addEventListener(
      "webglcontextlost",
      (e) => {
        // (Temporary use of `alert()`)
        alert("WebGL context lost. You will need to reload the page.");
        e.preventDefault();
      },
      false
    );

    return canvas;
  })(),

  setStatus: (text) => {
    if (!Module.setStatus.last)
      Module.setStatus.last = { time: Date.now(), text: "" };

    if (text === Module.setStatus.last.text)
      return;

    const progressUpdate = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
    const now = Date.now();
    const updateTooSoon = now - Module.setStatus.last.time < 30;
    if (progressUpdate && updateTooSoon)
      return;

    Module.setStatus.last.time = now;
    Module.setStatus.last.text = text;

    if (progressUpdate) {
      text = progressUpdate[1];
      progressElement.value = parseInt(progressUpdate[2]) * 100;
      progressElement.max = parseInt(progressUpdate[4]) * 100;
      progressElement.hidden = false;
      spinnerElement.hidden = false;
    }
    else {
      progressElement.value = null;
      progressElement.max = null;
      progressElement.hidden = true;
      if (!text)
        spinnerElement.hidden = true;
    }
    statusElement.innerHTML = text;
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
  Module.setStatus("Exception thrown, see JavaScript console");
  spinnerElement.style.display = "none";
  Module.setStatus = (text) => {
    if (text)
      console.error("[post-exception status] " + text);
  };
};
