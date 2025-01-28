//? #version 460 core
// indirect_draw.glsl

// SSBO containing this struct should have std430 layout as it's not 16-byte aligned.
struct DrawElementsIndirectCommand {
	uint count;
	uint instanceCount;
	uint firstIndex;
	int  baseVertex;
	uint baseInstance;
};
