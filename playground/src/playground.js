import { editor } from "./editor_setup.js";
import { snippets } from "./snippets.js";

const runButtonElement = document.getElementById("thusly-run");
const debugCompilationElement = document.getElementById("debug-compilation");
const debugExecutionElement = document.getElementById("debug-execution");
const outputElement = document.getElementById("output");

const loopsSnippetElement = document.getElementById("loops-snippet");
const selectionSnippetElement = document.getElementById("selection-snippet");
const blockSnippetElement = document.getElementById("block-snippet");

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
