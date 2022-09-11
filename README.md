
## Introduction
OpenGL rendering library (for 3d graphics study)

* IDE: Visual Studio 2022, Visual Studio Code
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

## Sample Images

![McGuireFireplaceRoom](https://user-images.githubusercontent.com/11644393/103409265-53a7e380-4ba9-11eb-8683-0122d3ef0b04.jpg)

![OmnidirectionalShadowMapping](https://cloud.githubusercontent.com/assets/11644393/15381530/30adbcc2-1dbb-11e6-9286-13c0f82e6f92.jpg)

![LightScattering](https://user-images.githubusercontent.com/11644393/71560025-ecdc7f80-2aa7-11ea-900f-f7a76fda0843.jpg)

![RC1_wip2](https://user-images.githubusercontent.com/11644393/98628358-c212b700-2359-11eb-9465-71a1791e15c8.jpg)
