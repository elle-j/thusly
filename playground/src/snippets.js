export const snippets = {
  overview: `// Selection

var age: 30
if age < 30
  @out "You are young"
else
  @out "You were young"
end

// Foreach

foreach value in 0..10
  @out value
end

var a: 0
var b: 10
var c: 2

foreach value in a..b step c
  @out value
end

// While

var i: 0

while i < 10 {i +: 1}
  @out i
end

// Standalone block

var scopeTest: "global"
@out scopeTest

block
  scopeTest: "changed global"
  var scopeTest: "local"
  @out scopeTest
end

@out scopeTest
`
};
