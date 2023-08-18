import { editor } from "./editor_setup.js";

document.getElementById("run").addEventListener("click", () => {
  clearOutput();
  const code = editor.getValue();
  run(code);
});

function clearOutput() {
  document.getElementById("output").value = "";
}
