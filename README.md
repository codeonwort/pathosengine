
## Introduction

OpenGL rendering engine mainly to toy with real-time graphics programming.

It's architecture follows game engine patterns but it's not meant to be a game engine.

### Development Environment

The project will only compile and run on the following environment:

* GL version: OpenGL 4.6 Core Profile
* Language: C++17
* OS: Windows 11
* IDE: Visual Studio 2022

### How to Build

1. Clone this repo.
2. Run `Setup.ps1` to download media resources. (you need execution policy for PowerShell script)
3. Build all projects in `PathosEngine.sln`.
4. Execute one of test projects.

## Sample Images

<details open>
  <summary>Collapse</summary>

![RenderChallenge1](https://user-images.githubusercontent.com/11644393/199586888-a70163b2-5b7e-4dbc-b050-c0da462ca27a.jpg)
RenderingChallenge1 world to showcase procedural geometries and textures, volumetric clouds, and bloom post process.

![RenderingChallenge2](https://user-images.githubusercontent.com/11644393/238012342-56108b01-5fc5-4f8a-8263-a1a538d95417.jpg)
RenderingChallenge2 world to showcase procedural geometries and sky.

<img src="https://github.com/user-attachments/assets/3d381473-fa85-4f39-8f94-93a2cd34e9cb" width="32%" />
<img src="https://github.com/user-attachments/assets/802e0af8-a47d-479f-ab0a-320eee934c95" width="32%" />
<img src="https://github.com/user-attachments/assets/81f5cf26-1136-4d79-8f61-e6b7cce8f733" width="32%" />
LightRoom world to showcase omnidirectional shadows and emissive lights.

![WIP-Landscape](https://github.com/user-attachments/assets/1b2f57bf-6a8e-487d-9884-6819baa0c089)
WIP RacingGame world to showcase large-scale rendering.

![GLTF-Sponza](https://user-images.githubusercontent.com/11644393/199479551-c4d0a6f8-e705-4570-9b7d-c2a43f785b74.jpg)
A toy rendering project always need yet another Sponza scene :)

</details>

## Summary of Rendering Features
* Rendering pipeline
  * Depth prepass
  * Local illumination
    * Area lights (sphere light, rect light)
    * PBR materials
  * Global illumination
    * Cascaded shadow map
    * Screen-space ambient occlusion
    * Screen-space reflection
	* Real-time illumination from sky light sources (skybox, panorama, or atmosphere)
    * Real-time indirect illumination from irradiance/radiance probes (WIP)
  * Volumetric clouds
  * Large-scale landscape (WIP)
  * Post processing
    * God ray (light shaft)
    * Auto exposure
    * Bloom
    * Tone mapping
    * Anti-aliasing (FXAA, TAA)
    * Super resolution (AMD FSR1)
    * Depth of field
* Engine subsystems
  * Render thread
  * Asynchronous asset loaders (Wavefront OBJ, glTF)
  * Scene capture (rendering from separate views other than the main view)
  * Runtime shader recompilation
  * Actor system (component-based development)
  * Material shader assembly system (generate material shaders from templates so that they can run in frame rendering pipeline)
  * RenderDoc integration
  * Console variables and console window

## Third-party Libraries
* OpenGL        : [glm](https://github.com/g-truc/glm), glLoadGen
* Window system : [freeglut](https://github.com/FreeGLUTProject/freeglut)
* Image         : [freeimage](https://freeimage.sourceforge.io/), [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h)
* Font          : [freetype](https://github.com/freetype/freetype)
* 3D model      : [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader), [tinygltf](https://github.com/syoyo/tinygltf), [assimp](https://github.com/assimp/assimp)
* JSON          : [nlohmann](https://github.com/nlohmann/json)
