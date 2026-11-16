 # WinBGIm Graphics Library for C++

A practical setup guide and collection of resources for using the **WinBGIm Graphics Library** with C++ on Windows.

WinBGIm provides a simple graphics interface for C++ programs. It allows developers and students to create graphical applications using shapes, colors, text, and basic drawing functions.

## Features

- Draw circles, rectangles, lines, and other shapes.
- Display text and colors in a graphics window.
- Create simple 2D graphical applications.
- Use `graphics.h` with supported MinGW-based C++ compilers.
- Beginner-friendly examples and setup instructions.

## Repository Structure

```text
WinBGIm-Graphics-Library_cpp/
│
├── include/
│   ├── graphics.h
│   └── winbgim.h
│
├── lib/
│   └── libbgi.a
│
├── examples/
│   └── first_graphics.cpp
│
├── README.md
└── .gitignore
```

## Requirements

- Windows operating system
- Dev-C++ or another compatible MinGW-based C++ IDE
- A compatible WinBGIm library build
- C++ compiler

**Important:** The architecture of `libbgi.a` must match your compiler toolchain.

## Installation Guide

### 1. Download the Repository

Clone this repository:

```bash
git clone https://github.com/Bboysis/WinBGIm-Graphics-Library_cpp.git
```

### 2. Copy the Header Files

Copy these files:

- `graphics.h`
- `winbgim.h`

Into your compiler's `include` directory.

### 3. Copy the Graphics Library

Copy:

```text
libbgi.a
```

Into your compiler's `lib` directory.

### 4. Configure Linker Settings

In Dev-C++:

1. Open your C++ project.
2. Go to Project → Project Options.
3. Select Parameters.
4. Under Linker, add:

```text
-lbgi
-lgdi32
-lcomdlg32
-luuid
-loleaut32
-lole32
```

5. Save the settings.

## Example Program

Create a C++ file named `first_graphics.cpp`:

```cpp
#include <graphics.h>

int main()
{
    initwindow(800, 600, "My First Graphics");

    setcolor(RED);
    circle(400, 300, 100);

    setcolor(BLUE);
    rectangle(250, 150, 550, 450);

    getch();
    closegraph();

    return 0;
}
```

Compile and run the program using a compatible compiler and library.

## Common Errors

| Error | Possible Solution |
|---|---|
| `graphics.h: No such file` | Check the include directory. |
| `cannot find -lbgi` | Check the library directory. |
| `undefined reference to initwindow` | Check linker settings and library compatibility. |
| Architecture mismatch | Use a library build compatible with your compiler. |

## Contributions

Contributions, improvements, documentation updates, and additional examples are welcome.

If you find an issue, please open a GitHub Issue.

## Project Information

**Repository:** [WinBGIm-Graphics-Library_cpp](https://github.com/Bboysis/WinBGIm-Graphics-Library_cpp)

**Maintainer:** [Bboysis](https://github.com/Bboysis)

**Status:** Documentation and library setup in progress.

## Acknowledgements

WinBGIm is an existing graphics library. Original authors and copyright holders must receive appropriate credit. Please refer to the original source and its license before redistribution.

---

Made for C++ developers and students learning computer graphics.
