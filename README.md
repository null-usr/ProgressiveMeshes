--- README.md ---
# Game Architecture Final Project: Progressive Meshes with OpenGL

This project implements **Progressive Meshes** using OpenGL, based on [Hoppe's PM project](http://hhoppe.com/proj/pm/).

---

## Requirements

- **GLFW3** and **GLM** via MSYS2 UCRT64:

```bash
pacman -S mingw-w64-ucrt-x86_64-glfw
pacman -S mingw-w64-ucrt-x86_64-glm
```

- **GLAD**: Download from [glad website](https://glad.dav1d.de/).  
  Generate the OpenGL loader, then place:
  - Headers → `./lib/glad/include`
  - Source → `./src`  
  Make sure to include these in your project build.

---

## Supported Meshes

- Only **Bunny 1k** and **Suzanne** are safe for use with this implementation.  
- Other meshes may not be compatible.

---

## Build Instructions

Using **g++** (manual build example):

```bash
g++ *.cpp -o main \
    -L<gl_header_locations> \
    -lglfw3 -pthread -lGLEW -lGLU -lGL -lrt \
    -lXrandr -lXxf86vm -lXi -lXinerama -lX11 -ldl -lXcursor
```

Or use **CMake** (recommended) with the provided `CMakeLists.txt`.
--- README.md ---
# Game Architecture Final Project: Progressive Meshes with OpenGL

This project implements **Progressive Meshes** using OpenGL, based on [Hoppe's PM project](http://hhoppe.com/proj/pm/).

---

## Requirements

- **GLFW3** and **GLM** via MSYS2 UCRT64:

```bash
pacman -S mingw-w64-ucrt-x86_64-glfw
pacman -S mingw-w64-ucrt-x86_64-glm
```

- **GLAD**: Download from [glad website](https://glad.dav1d.de/).  
  Generate the OpenGL loader, then place:
  - Headers → `./lib/glad/include`
  - Source → `./src`  
  Make sure to include these in your project build.

---

## Supported Meshes

- Only **Bunny 1k** and **Suzanne** are safe for use with this implementation.  
- Other meshes may not be compatible.

---

## Build Instructions

Using **g++** (manual build example):

```bash
g++ *.cpp -o main \
    -L/usr/local/lib \
    -lglfw3 -pthread -lGLEW -lGLU -lGL -lrt \
    -lXrandr -lXxf86vm -lXi -lXinerama -lX11 -ldl -lXcursor
```

Or use **CMake** (recommended) with the provided `CMakeLists.txt`.

---

## Run

```bash
./main
```

Ensure your GLFW and GLAD dependencies are correctly set up.

---

## References

- Progressive Meshes: [http://hhoppe.com/proj/pm/](http://hhoppe.com/proj/pm/)
- GLFW: [https://www.glfw.org/](https://www.glfw.org/)
- GLM: [https://github.com/g-truc/glm](https://github.com/g-truc/glm)
- GLAD: [https://glad.dav1d.de/](https://glad.dav1d.de/)
