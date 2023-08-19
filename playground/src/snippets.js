export const snippets = {
  loops: `// Bounded loop

foreach value in 0..10
  @out value
end

// Unbounded loop

var i: 0

while i < 10 {i +: 1}
  @out i
end
`,

  selection: `// Selection

var age: 30
if age < 30
  @out "You are young"
else
  @out "You were young"
end
`,

  standaloneBlock: `// Standalone block

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
