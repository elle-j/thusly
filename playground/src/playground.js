import { editor } from "./editor_setup.js";

const debugCompilationElement = document.getElementById("debug-compilation");
const debugExecutionElement = document.getElementById("debug-execution");

document.getElementById("run").addEventListener("click", () => {
  clearOutput();
  const code = editor.getValue();
  run(code, debugCompilationElement.checked, debugExecutionElement.checked);
});

function clearOutput() {
  document.getElementById("output").value = "";
}
