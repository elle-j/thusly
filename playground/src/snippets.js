export const snippets = {
  loops: `// Type some code here!

// Bounded loop

foreach value in 0..5
    @out value
end

// Unbounded loop

var i: 0

while i < 5 {i +: 1}
    @out i
end
`,

  selection: `// Type some code here!

// Selection

var age: 30
if age < 30
    @out "You are young"
else
    @out "You were young"
end
`,

  standaloneBlock: `// Type some code here!

// Standalone block

var scopeTest: "global"
@out scopeTest

block
    scopeTest: "changed global"
    var scopeTest: "local"
    @out scopeTest
end

@out scopeTest
`,
};
