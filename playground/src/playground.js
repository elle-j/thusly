import { editor } from "./editor_setup.js";
import { snippets } from "./snippets.js";

const runButtonElement = document.getElementById("th-run");
const debugCompilationElement = document.getElementById("th-debug-compilation");
const debugExecutionElement = document.getElementById("th-debug-execution");

runButtonElement.addEventListener("click", () => {
  clearOutput();
  const code = editor.getValue();
  run(code, debugCompilationElement.checked, debugExecutionElement.checked);
});

const exampleContainerElements = document.getElementsByClassName("th-tab-example");
for (const container of exampleContainerElements) {
  container.addEventListener("click", () => {
    const inputElement = container.getElementsByTagName("input")[0];
    inputElement.checked = true;
    showSelectedSnippet(inputElement.value);
  });
}

function showSelectedSnippet(name) {
  editor.setValue(snippets[name]);
}

const outputElement = document.getElementById("th-output");
function clearOutput() {
  outputElement.value = "";
}
