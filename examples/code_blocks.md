# Code Blocks Support

We now support code blocks with general syntax highlighting.

```c
#include <stdio.h>

int main() {
    // This is a comment
    printf("Hello, World!\n");
    if (1) {
        return 0;
    }
}
```

---

## Multiple Languages

Even if it's general, it works for many languages.

```python
def hello():
    # Python comment
    message = "Hello from Python"
    for i in range(10):
        print(message)
```

---

## Bracket Highlighting

The highlighter also colors brackets and symbols.

```javascript
const obj = {
    arr: [1, 2, 3],
    fn: (x) => {
        return x * 2;
    }
};
```
