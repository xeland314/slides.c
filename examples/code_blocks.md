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

---

## Advanced Number Formats (C)

Testing alternative bases, scientific notation, and C type suffixes.

```c
void test_numbers() {
    // Hexadecimal, Binary and Octal with underscores
    int hex_val = 0x7F_A0;
    int bin_val = 0b1010_1100;
    int oct_val = 0755;

    // Floats, Doubles and Scientific Notation
    float pi = 3.14159f;
    double sci_num = 6.022e+23;
    double micro = 1.0e-6;

    // Integer suffixes (Unsigned Long Long)
    unsigned long long huge = 1_000_000_000ULL;
}

```

---

## Complex Numbers & Separators (Python)

Testing Python's unique literals and ensuring 'print' or 'in' don't break.

```python
def math_literals():
    # Python underscores as digit separators
    population = 8_300_000
    bitmask = 0b1100_0011
    
    # Complex/Imaginaries and floats
    z1 = 2.5j
    z2 = 3.0J
    avogadro = 6.022e23

    # Testing that 'in' inside 'print' or 'index' is SAFE now
    for items in [z1, z2]:
        print(f"Is z1 in list? {z1 in [z1]}")

```

---

## Operators & Logic Stress-Test

Verifying that arithmetic, bitwise, assignment, and comparison operators highlight correctly.

```c
void verify_operators() {
    int a = 10, b = 20, c = 0;

    // Arithmetic & Assignment
    c += (a * b) / 2;
    c %= 3;

    // Bitwise & Logic Comparisons
    if ((a != b) && (a <= b || b == 20)) {
        c = a ^ b | ~c;
    }
}

```
