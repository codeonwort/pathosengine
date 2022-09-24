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

		// Parse material shader files and generate corresponding shader programs.
		void parseAllMaterialShaders();

		MaterialShader* generateMaterialShader(const char* fullpath, const char* filename);

	private:
		MaterialShaderAssembler() = default;
		~MaterialShaderAssembler();

		MaterialTemplate* materialTemplate = nullptr;
		bool bTemplateLoaded = false;

		std::vector<MaterialShader*> materials;

	};

}
