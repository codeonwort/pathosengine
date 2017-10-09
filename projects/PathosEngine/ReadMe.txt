========================================================================
    정적 라이브러리 : PathosEngine 프로젝트 개요
========================================================================

### Directory Configuration

src/							root
	glsl/						shader files. currently not used (shaders are hard-coded into classes)
	pathos/						main source code
		camera/					camera class
		light/					light and shadow
		loader/					various loaders (image, 3d object, and so on)
		material/				material class
		mesh/					mesh and geometry (TODO: move material into mesh or separate geom from mesh)
		overlay/				2d display over 3d scene
		render/					renderer & render passes (shader programs)
		text/					literally, text
		wrapper/				simple wrappers
		core.h/.cpp				fundamental objects
		engine.h/.cpp			manages engine itself
	tinyobjloader/				tinyobjloader integration (https://github.com/syoyo/tinyobjloader)

PathosEngine.vcxproj
    응용 프로그램 마법사를 사용하여 생성한 VC++ 프로젝트의 기본 프로젝트 파일입니다. 파일을 생성한 Visual C++ 버전에 대한 정보와 응용 프로그램 마법사를 사용하여 선택한 플랫폼, 구성 및 프로젝트 기능에 대한 정보가 포함되어 있습니다.

PathosEngine.vcxproj.filters
    응용 프로그램 마법사를 사용하여 생성된 VC++ 프로젝트의 필터 파일입니다. 이 파일에는 프로젝트의 파일과 필터 간의 연결 정보가 들어 있습니다. 이러한 연결은 특정 노드에서 유사한 확장명으로 그룹화된 파일을 표시하기 위해 IDE에서 사용됩니다. 예를 들어 ".cpp" 파일은 "소스 파일" 필터와 연결되어 있습니다.

/////////////////////////////////////////////////////////////////////////////
기타 참고:

응용 프로그램 마법사에서 사용하는 "TODO:" 주석은 사용자가 추가하거나 사용자 지정해야 하는 소스 코드 부분을 나타냅니다.

/////////////////////////////////////////////////////////////////////////////
