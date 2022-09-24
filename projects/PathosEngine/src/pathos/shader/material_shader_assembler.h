#pragma once

namespace pathos {

	class MaterialShader;
	struct MaterialTemplate;

	// Generates material shaders based on material templates.
	class MaterialShaderAssembler final {

	public:
		static MaterialShaderAssembler& get();

		// Load material template files.
		void loadMaterialTemplate();

		// Enumerate material files and call generateMaterialShader().
		void parseAllMaterialShaders();

		// Parse material shader file to generate corresponding shader program.
		MaterialShader* generateMaterialShader(const char* fullpath, const char* filename);

	private:
		MaterialShaderAssembler() = default;
		~MaterialShaderAssembler();

		MaterialTemplate* materialTemplate = nullptr;
		bool bTemplateLoaded = false;

		std::vector<MaterialShader*> materialShaders;

	};

}
