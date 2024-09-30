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

const loopsSnippetElement = document.getElementById("th-loops-snippet");
loopsSnippetElement.parentElement.addEventListener("click", () => {
  showSelectedSnippet(loopsSnippetElement.value);
});

const selectionSnippetElement = document.getElementById("th-selection-snippet");
selectionSnippetElement.parentElement.addEventListener("click", () => {
  showSelectedSnippet(selectionSnippetElement.value);
});

const blockSnippetElement = document.getElementById("th-block-snippet");
blockSnippetElement.parentElement.addEventListener("click", () => {
  showSelectedSnippet(blockSnippetElement.value);
});

function showSelectedSnippet(name) {
  editor.setValue(snippets[name]);
}

const outputElement = document.getElementById("th-output");
function clearOutput() {
  outputElement.value = "";
}
