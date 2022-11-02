
## Introduction
OpenGL rendering library (for 3d graphics study)

* IDE: Visual Studio 2022, Visual Studio Code
* Language: C++17
* GL version: OpenGL 4.6 Core Profile

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
![RenderChallenge1](https://user-images.githubusercontent.com/11644393/199479187-dd45c3cc-5202-4e1a-8f4a-68a81c938c23.jpg)
![GLTF-DamagedHelmet](https://user-images.githubusercontent.com/11644393/199479537-4e5abc6a-7f5a-4236-a84f-8a5ef1296f2f.jpg)
![GLTF-Sponza](https://user-images.githubusercontent.com/11644393/199479551-c4d0a6f8-e705-4570-9b7d-c2a43f785b74.jpg)

## Third-party Libraries
* OpenGL        : [glm](https://github.com/g-truc/glm), glLoadGen
* Window system : [freeglut](https://github.com/FreeGLUTProject/freeglut)
* Image         : [freeimage](https://freeimage.sourceforge.io/), [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h)
* Font          : [freetype](https://github.com/freetype/freetype)
* 3D model      : [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader), [tinygltf](https://github.com/syoyo/tinygltf), [assimp](https://github.com/assimp/assimp)
* JSON          : [nlohmann](https://github.com/nlohmann/json)
