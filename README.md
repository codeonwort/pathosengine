
## Introduction
OpenGL rendering library (for 3d graphics study)

* IDE: Visual Studio 2019, Visual Studio Code
* Language: C++
* GL version: OpenGL 4.6 Core Profile

## Third-party Libraries
* OpenGL        : glm, glLoadGen
* Window system : freeglut
* Image         : freeimage, stb_image
* Font          : freetype
* 3D model      : tinyobjloader, assimp

note: Requires 'Visual C++ 2013 Redistributable Package' because I used pre-built binaries. This limitation may be removed in future.

## Features
* Rendering
  * PBR materials
  * Cascaded shadow map
  * Bloom
  * Depth of field
  * Tone mapping
  * FXAA
  * Screen-space ambient occlusion
  * Scene capture
* System
  * Render command list
  * Asynchronous asset loader
  * Runtime shader recompilation
  * Actor system (component-based development)

## Sample Images

![McGuireFireplaceRoom](https://user-images.githubusercontent.com/11644393/73363954-02360b00-42ed-11ea-8f2b-3184ff567848.jpg)

![OmnidirectionalShadowMapping](https://cloud.githubusercontent.com/assets/11644393/15381530/30adbcc2-1dbb-11e6-9286-13c0f82e6f92.jpg)

![LightScattering](https://user-images.githubusercontent.com/11644393/71560025-ecdc7f80-2aa7-11ea-900f-f7a76fda0843.jpg)

![RC1_wip](https://user-images.githubusercontent.com/11644393/97787481-d89c6e00-1bf5-11eb-9a36-c8e3f8328dbb.jpg)