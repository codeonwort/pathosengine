
## Introduction
OpenGL rendering library (for 3d graphics study)

* IDE: Visual Studio 2022, Visual Studio Code
* Language: C++17
* GL version: OpenGL 4.6 Core Profile

## Third-party Libraries
* OpenGL        : [glm](https://github.com/g-truc/glm), glLoadGen
* Window system : [freeglut](https://github.com/FreeGLUTProject/freeglut)
* Image         : [freeimage](https://freeimage.sourceforge.io/), [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h)
* Font          : [freetype](https://github.com/freetype/freetype)
* 3D model      : [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader), [tinygltf](https://github.com/syoyo/tinygltf), [assimp](https://github.com/assimp/assimp)
* JSON          : [nlohmann](https://github.com/nlohmann/json)

## Features
* Rendering pipeline
  * Depth prepass
  * Local illumination
    * Area lights (sphere, rect)
    * PBR materials
  * Global illumination
    * Cascaded shadow map
    * Screen-space ambient occlusion
    * Screen-space reflection (WIP)
    * Basic sky IBL
  * Volumetric clouds
  * Post processing
    * God ray (light shaft)
    * Bloom
    * Tone mapping
    * Anti-aliasing (FXAA, TAA)
    * Super resolution (AMD FSR1)
    * Depth of field
  * Scene capture
* System
  * Render thread
  * Asynchronous asset loader
  * Runtime shader recompilation
  * Actor system (component-based development)
  * Material shader assembly system

## Sample Images

![AreaLightTest](https://user-images.githubusercontent.com/11644393/191719865-a5db07fa-336b-4d3c-96cf-5b0da60812c5.jpg)

![McGuireFireplaceRoom](https://user-images.githubusercontent.com/11644393/191719903-67ba8cf6-887c-40a9-a386-8d32a91323a7.jpg)

![RC1](https://user-images.githubusercontent.com/11644393/191719967-fa640a08-7b4c-4e8e-9a94-36550ef605fe.jpg)
