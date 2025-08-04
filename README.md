# WinOpenGL - OpenGL Program using the Win32 API
![TriangleGL](RepoIcon.png)
This is a pretty simple OpenGL 3.3 core program that draws a triangle to the screen
without using any external libraries. This program is only native to Windows,
so anyone using Linux/Mac will not be able to build.

## BUILDING

To build the project, you just need to open the solution file, and build the variant of the program
that you want to do (either x86, or x64).

## THIRD-PARTY LICENSING

This project uses the OpenGL Registry (https://registry.khronos.org/OpenGL/index_gl.php) to define all
the available function pointers used within this program. I've attached the license for these headers defined
below:

```txt
Copyright (c) 2008-2018 The Khronos Group Inc.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and/or associated documentation files (the
"Materials"), to deal in the Materials without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Materials, and to
permit persons to whom the Materials are furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Materials.

THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
```