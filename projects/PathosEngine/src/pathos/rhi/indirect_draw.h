#pragma once

// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glMultiDrawElementsIndirect.xhtml
// MDI parameters to fill in an indirect draw buffer.
//
// Example usage:
//
//     std::vector<DrawElementsIndirectCommand> commands(drawCount);
//     for (size_t i = 0; i < drawCount; ++i) commands[i] = ...;
//     Buffer* buffer = ...;
//     buffer->writeToGPU(0, sizeof(DrawElementsIndirectCommand) * drawCount, commands.data());
//     cmdList.bindBuffer(GL_DRAW_INDIRECT_BUFFER, buffer->internal_getGLName());
//     cmdList.multiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, drawCount, 0);
//
struct DrawElementsIndirectCommand {
	uint32  count;
	uint32  instanceCount;
	uint32  firstIndex;
	int32   baseVertex;
	uint32  baseInstance;
};
