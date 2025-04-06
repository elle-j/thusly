export const snippets = {
  loops: `// Go ahead and type some code here!

// Bounded loop

foreach value in 0..3
    @out value
end

// Unbounded loop

var i: 0

while i < 4 {i +: 1}
    @out i
end
`,

  selection: `// Go ahead and type some code here!

// Selection

var age: 35
if age < 30
    @out "You are young"
else
    @out "You were young"
end
`,

  standaloneBlock: `// Go ahead and type some code here!

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
