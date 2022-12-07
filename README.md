
## Introduction

OpenGL rendering engine mainly to toy with real-time graphics programming.

NOTE: Here my main focus is on implementing rendering techniques,
so the engine architecture might bad and not that robust.
Also I give only little effort to non-rendering subsystems.

### Development Environment

It's highly probable the project won't compile nor launch in other configurations.

* GL version: OpenGL 4.6 Core Profile
* Language: C++17
* OS: Windows 10 or 11
* IDE: Visual Studio 2022

### How to Build

1. Clone this repo.
2. Run `Setup.ps1` to download media resources. (you need execution policy for PowerShell script)
3. Build all projects in `PathosEngine.sln`.
4. Execute one of test projects.

### Sample Images

![AreaLightTest](https://user-images.githubusercontent.com/11644393/191719865-a5db07fa-336b-4d3c-96cf-5b0da60812c5.jpg)
Area light sources (sphere light and rect light).

![McGuireFireplaceRoom](https://user-images.githubusercontent.com/11644393/191719903-67ba8cf6-887c-40a9-a386-8d32a91323a7.jpg)
Asynchronous Wavefront OBJ asset loading, local lights with omnidirectional shadow mapping, screen space light shaft, and screen space ambient occlusion.

![RenderChallenge1](https://user-images.githubusercontent.com/11644393/199586888-a70163b2-5b7e-4dbc-b050-c0da462ca27a.jpg)
Procedural geometries and textures, volumetric clouds, and post process bloom.

![GLTF-DamagedHelmet](https://user-images.githubusercontent.com/11644393/199479537-4e5abc6a-7f5a-4236-a84f-8a5ef1296f2f.jpg)
Asynchronous glTF asset loading, Cook-Torrance BRDF for local illumination, and basic sky IBL for global illumination.

![GLTF-Sponza](https://user-images.githubusercontent.com/11644393/199479551-c4d0a6f8-e705-4570-9b7d-c2a43f785b74.jpg)
Yet another Sponza scene.

## Summary of Rendering Features
* Rendering pipeline
  * Depth prepass
  * Local illumination
    * Area lights (sphere light, rect light)
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
  * Scene capture (rendering from separate views other than the main view)
* Engine subsystems
  * Render thread
  * Asynchronous asset loaders (Wavefront OBJ, glTF)
  * Runtime shader recompilation
  * Actor system (component-based development)
  * Material shader assembly system
  * RenderDoc integration
  * Console variables and console window

## Third-party Libraries
* OpenGL        : [glm](https://github.com/g-truc/glm), glLoadGen
* Window system : [freeglut](https://github.com/FreeGLUTProject/freeglut)
* Image         : [freeimage](https://freeimage.sourceforge.io/), [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h)
* Font          : [freetype](https://github.com/freetype/freetype)
* 3D model      : [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader), [tinygltf](https://github.com/syoyo/tinygltf), [assimp](https://github.com/assimp/assimp)
* JSON          : [nlohmann](https://github.com/nlohmann/json)
