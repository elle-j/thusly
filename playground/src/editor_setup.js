import { snippets } from "./snippets.js";

const languageId = "thusly";

const editorConfiguration = {
  // name: "overview.thusly",
  language: languageId,
  // extraEditorClassName: "",
  // theme: "TODO",
  autoClosingBrackets: true,
  automaticLayout: true,
  detectIndentation: true,
  tabSize: 2,
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

// monaco.editor.defineTheme("TODO", );
monaco.languages.register({ id: languageId });
monaco.languages.setLanguageConfiguration(languageId, languageConfiguration);
monaco.languages.setMonarchTokensProvider(languageId, tokensProvider);
const editor = monaco.editor.create(document.getElementById("th-editor"), editorConfiguration);

export { editor };
