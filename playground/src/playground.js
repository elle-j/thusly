import { editor } from "./editor_setup.js";
import { snippets } from "./snippets.js";

const runButtonElement = document.getElementById("th-run");
const debugCompilationElement = document.getElementById("th-debug-compilation");
const debugExecutionElement = document.getElementById("th-debug-execution");
const outputElement = document.getElementById("th-output");

const loopsSnippetElement = document.getElementById("th-loops-snippet");
const selectionSnippetElement = document.getElementById("th-selection-snippet");
const blockSnippetElement = document.getElementById("th-block-snippet");

runButtonElement.addEventListener("click", () => {
  clearOutput();
  const code = editor.getValue();
  run(code, debugCompilationElement.checked, debugExecutionElement.checked);
});

loopsSnippetElement.addEventListener("click", () => {
  showSelectedSnippet(loopsSnippetElement.value);
});

selectionSnippetElement.addEventListener("click", () => {
  showSelectedSnippet(selectionSnippetElement.value);
});

blockSnippetElement.addEventListener("click", () => {
  showSelectedSnippet(blockSnippetElement.value);
});

function showSelectedSnippet(name) {
  editor.setValue(snippets[name]);
}

function clearOutput() {
  outputElement.value = "";
}
