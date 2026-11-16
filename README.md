# 🎨 WinBGIm Graphics Library (C++)

A simple repository for **WinBGIm graphics programming in C++**, featuring library files, beginner-friendly examples, and small graphics projects.

Learn computer graphics by drawing shapes, creating illustrations, and experimenting with animation.

<p align="center">
  **Draw • Create • Animate • Learn**
</p>


 ## 📌 About the Project

**WinBGIm Graphics Library** is a collection of resources and practical C++ examples for working with the WinBGIm graphics library on Windows.

This repository is designed for students, beginners, and anyone interested in learning computer graphics using C++.

The main goal is to provide simple, understandable examples that demonstrate how graphics programming works.

## ✨ Features

- 🎨 Drawing shapes and colorful objects
- 🏠 Creating simple illustrations
- 🚦 Drawing everyday objects
- 🚗 Basic object animation
- ⚽ Moving and bouncing objects
- 💻 Practical C++ graphics examples
- 📚 Beginner-friendly source code

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
The repository includes practical graphics programs.

| Example | Description |
|---|---|
| `car_picture.cpp` | Draws a simple car |
| `house.cpp` | Creates a house illustration |
| `traffic_light.cpp` | Draws a traffic light |
| `flower_drawing.cpp` | Creates a colorful flower |
| `moving_car.cpp` | Animates a moving car |
| `bouncing_ball.cpp` | Demonstrates a bouncing ball animation |

More examples and mini projects can be added as the repository grows.

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

## 🎯 Learning Objectives

Through these examples, you can practice:

- Using `initwindow()` to create a graphics window
- Drawing with `line()`, `rectangle()`, and `circle()`
- Creating filled shapes with `fillellipse()` and `bar()`
- Applying colors with `setcolor()` and `setfillstyle()`
- Animating objects using loops and position updates
- Managing graphics windows with `closegraph()`
## 🤝 Contributing_with_your_first_cpp_code

Contributions and suggestions are welcome!

If you have a useful graphics example or a small C++ project, you can contribute:

1. Fork this repository.
2. Create a new branch.
3. Add your example inside the `examples/` folder.
4. Commit your changes.
5. Submit a Pull Request.

Please keep examples simple, readable, and easy to understand.

## Project Information

**Repository:** [WinBGIm-Graphics-Library_cpp](https://github.com/Bboysis/WinBGIm-Graphics-Library_cpp)

**Maintainer:** [Bboysis](https://github.com/Bboysis)

**Status:** Documentation and library setup in progress.

## 👨‍💻 Author

**Sisay**

## Acknowledgements

WinBGIm is an existing graphics library

---
<p align="center">
  ⭐ If you find this repository useful, consider giving it a star!
</p>
<p align="center">
  Made for C++ developers and students learning computer graphics.
</p>

<p align="center">
  **Happy Coding! if you have problem contact in ig,or telegram i will suuport you 💻**
</p>