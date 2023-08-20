import { snippets } from "./snippets.js";

const languageId = "thusly";

const editorConfiguration = {
  autoClosingBrackets: true,
  autoIndent: true,
  automaticLayout: true,
  "bracketPairColorization.enabled": false,
  detectIndentation: true,
  fontFamily: "monospace",
  fontSize: 14,
  // fontWeight: "normal",
  language: languageId,
  letterSpacing: 0.5,
  minimap: { enabled: true },
  padding: { top: 30 },
  scrollbar: {
    horizontalScrollbarSize: 5,
    verticalScrollbarSize: 5,
  },
  tabSize: 4,
  theme: languageId,
  value: snippets.loops,
};

const languageConfiguration = {
  // wordPattern:
  //   /(-?\d*\.\d\w*)|([^\`\~\!\@\#\%\^\&\*\(\)\-\=\+\[\{\]\}\\\|\;\:\'\"\,\.\<\>\/\?\s]+)/g,

  comments: {
    lineComment: "//",
  },

  brackets: [
    ["{", "}"],
    ["(", ")"],
  ],

  // onEnterRules: [],

  autoClosingPairs: [
    { open: "{", close: "}" },
    { open: "(", close: ")" },
    { open: '"', close: '"', notIn: ["string"] },
    { open: "block", close: "end" },
    { open: "if", close: "end" },
    { open: "else", close: "end" },
    { open: "foreach", close: "end" },
    { open: "while", close: "end" },
  ],

  folding: {
    markers: {
      start: new RegExp("^\\s*//\\s*#?region\\b"),
      end: new RegExp("^\\s*//\\s*#?endregion\\b"),
    },
  },
};

const tokensProvider = {
  // Set defaultToken to `invalid` to see what is not yet tokenized.
  defaultToken: "invalid",
  tokenPostfix: ".thusly",

  keywords: [
    "and",
    "block",
    "else",
    "end",
    "false",
    "foreach",
    "if",
    "in",
    "mod",
    "none",
    "not",
    "or",
    "@out",
    "step",
    "true",
    "var",
    "while",
  ],

  operators: [
    ":",
    "..",
    "=",
    "!=",
    ">",
    ">=",
    "<",
    "<=",
    "-",
    "-:",
    "+",
    "+:",
    "/",
    "*",
    "*:",
  ],

  symbols: /[=><:+\-*\/]+/,

  digits: /\d+(_+\d+)*/,

  charsLowerCaseStart: /[a-z_@][\w$]*/,

  charsUpperCaseStart: /[A-Z][\w\$]*/,

  tokenizer: {
    root: [
      [/[{}]/, "delimiter.bracket"],
      { include: "common" },
    ],

    common: [
      // Identifiers and keywords.
      [
        /@charsLowerCaseStart/,
        {
          cases: {
            "@keywords": "keyword",
            "@default": "identifier",
          },
        },
      ],
      // [/[A-Z][\w\$]*/, "type.identifier"], // Shows class names nicely
      [/@charsUpperCaseStart/, "identifier"],

      // Whitespace.
      { include: "@whitespace" },

      // Delimiters and operators.
      [/[{}()]/, "@brackets"],
      // [/(?:=([^=]))/, "delimiter"],
      [
        /@symbols/,
        {
          cases: {
            "@operators": "delimiter",
            "@default": "",
          },
        },
      ],

      // Numbers.
      [/(@digits)\.(@digits)/, "number.float"],
      [/(@digits)/, "number"],

      // Delimiter: after number because of .\d floats.
      [/[,.]/, "delimiter"],

      // Strings.
      [/"([^"\\]|\\.)*$/, "string.invalid"], // Non-teminated string.
      [/"/, "string", "@stringDouble"],
    ],

    whitespace: [
      [/[ \t]+/, ""],
      [/\/\/.*$/, "comment"],
    ],

    comment: [
      // [/\/\/.*$/, "comment"],
    ],

    stringDouble: [
      [/[^\\"]+/, "string"],
      [/"/, "string", "@pop"],
    ],

    bracketCounting: [
      [/\{/, "delimiter.bracket", "@bracketCounting"],
      [/\}/, "delimiter.bracket", "@pop"],
      { include: "common" },
    ]
  }
};

const colors = {
  gold: "#deb26f",
  gray: "#a7a7a7",
  pink: "#c692e9",
  purple: "#81a8fd",
  purpleDark: "#1e1e3f",
  red: "#cf3b3b",
  white: "#ffffff",
};

const thuslyTheme = {
  base: "vs-dark",
  inherit: false,

  colors: {
    "editor.background": colors.purpleDark,
    // "editor.foreground": colors.white,
  },

  rules: [
    { token: "", foreground: colors.white, fontStyle: "bold" },
    { token: "keyword", foreground: colors.pink },
    { token: "punctuation", foreground: colors.white },
    { token: "variable", foreground: colors.white },
    { token: "variable.predefined", foreground: colors.white },
    { token: "variable.function", foreground: colors.pink },
    { token: "string", foreground: colors.gold },
    { token: "number", foreground: colors.gold },
    { token: "boolean", foreground: colors.gold },
    // { token: "constant", foreground: colors.gold },
    { token: "comment", foreground: colors.gray },
    // { token: "type", foreground: "" },
    { token: "delimiter", foreground: colors.white },
    { token: "entity.name.function", foreground: colors.pink }, // <--
    // { token: "key", foreground: colors.white },
    // { token: "emphasis", fontStyle: "italic" },
    // { token: "strong", fontStyle: "bold" },
    { token: "invalid", foreground: colors.red },
  ],
}

monaco.editor.defineTheme(languageId, thuslyTheme);
monaco.languages.register({ id: languageId });
monaco.languages.setLanguageConfiguration(languageId, languageConfiguration);
monaco.languages.setMonarchTokensProvider(languageId, tokensProvider);
const editor = monaco.editor.create(document.getElementById("th-editor"), editorConfiguration);

export { editor };
