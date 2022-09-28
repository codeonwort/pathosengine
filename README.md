
## Introduction
OpenGL rendering library (for 3d graphics study)

* IDE: Visual Studio 2022, Visual Studio Code
* Language: C++17
* GL version: OpenGL 4.6 Core Profile

## Third-party Libraries
* OpenGL        : glm, glLoadGen
* Window system : freeglut
* Image         : freeimage, stb_image
* Font          : freetype
* 3D model      : tinyobjloader, assimp

## Features
* Rendering
  * Area lights (sphere, rect)
  * Depth prepass
  * Global illumination
    * Cascaded shadow map
    * Screen-space ambient occlusion
    * Screen-space reflection
    * Basic IBL (sky irradiance sampling on diffuse/glossy/specular surfaces, without sky occlusion)
  * Volumetric clouds
  * Post processing
    * God ray
    * Bloom
	* Tone mapping
	* FXAA
	* Depth of field
  * PBR materials
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
