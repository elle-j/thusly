# Front Page Code Snippets for Thusly

Below are the copyable code snippets that are also shown with syntax highlighting on the repository's front page.

## Variables, Data Types, and Literals

```
const id: 100           // number
var valid: true         // boolean
var enabled: false      // boolean
var nickname: none      // none
nickname: "Alex"        // text
```

## Selection

```
var age: 30

if age in 25..35
    @out("You are young")
end
```

```
var timeLeft: 120
var cancelled: false

if timeLeft = 0 or cancelled
    @out("Done")
elseif timeLeft < 60
    @out("Almost done")
else
    @out("Keep going")
end
```

## Loop

```
foreach value in 0..10
    @out(value)
end
```

```
var a: 0
var b: 10
var c: 2

foreach value in a..b step c
    @out(value)
end
```

```
var i: 0

while i < 10 {i +: 1}
    @out(i)
end
```
