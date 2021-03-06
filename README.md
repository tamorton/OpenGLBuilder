## OpenGL for C++ Builder

Creates an OpenGL window using Embarcadero C++ Builder. This is written to use OpenGL in a Windows applications. C++ Builder from Embarcadero uses the FireMonkey or VCL framework for a GUI. FireMonkey includes a 3D window and control but, for Windows, it is implemented in DirectX. On other platforms it uses OpenGL.

The header files included with Windows only support OpenGL version 1. Headers for the more recent versions were created using GLAD (https://glad.dav1d.de/).

The open source library GLFW (https://www.glfw.org/) is used to create windows, contexts and surfaces, and receive input and events. GLFW was compiled for C++ Builder and the libraries for 32 and 64 bit applications are included.

Vector and matrix math is implemented using the GLM library (https://github.com/g-truc/glm) which is included.

The bitmap font used was generated with Codehead's Bitmap Font Generator (https://github.com/CodeheadUK/CBFG).

Wavefront obj files can be loaded using the Tiny Object Loader library (https://github.com/tinyobjloader/tinyobjloader) which is included. Currently only a limited part of the data is loaded (triangles with the diffuse texture).

In this project, the OpenGL window is set to use version 3.3 of OpenGL. There is a demo window which creates the OpenGL window and adds objects to it. There are functions to add textured triangles, colored triangles, 2D text, 3D text with 3D points and obj files.

