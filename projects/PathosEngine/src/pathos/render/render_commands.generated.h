struct RenderCommand_cullFace : public RenderCommandBase {
	GLenum mode;

	static void APIENTRY execute(const RenderCommand_cullFace* __restrict params) {
		glCullFace(
			params->mode
		);
	}
};
struct RenderCommand_frontFace : public RenderCommandBase {
	GLenum mode;

	static void APIENTRY execute(const RenderCommand_frontFace* __restrict params) {
		glFrontFace(
			params->mode
		);
	}
};
struct RenderCommand_hint : public RenderCommandBase {
	GLenum target;
	GLenum mode;

	static void APIENTRY execute(const RenderCommand_hint* __restrict params) {
		glHint(
			params->target,
			params->mode
		);
	}
};
struct RenderCommand_lineWidth : public RenderCommandBase {
	GLfloat width;

	static void APIENTRY execute(const RenderCommand_lineWidth* __restrict params) {
		glLineWidth(
			params->width
		);
	}
};
struct RenderCommand_pointSize : public RenderCommandBase {
	GLfloat size;

	static void APIENTRY execute(const RenderCommand_pointSize* __restrict params) {
		glPointSize(
			params->size
		);
	}
};
struct RenderCommand_polygonMode : public RenderCommandBase {
	GLenum face;
	GLenum mode;

	static void APIENTRY execute(const RenderCommand_polygonMode* __restrict params) {
		glPolygonMode(
			params->face,
			params->mode
		);
	}
};
struct RenderCommand_scissor : public RenderCommandBase {
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_scissor* __restrict params) {
		glScissor(
			params->x,
			params->y,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_texParameterf : public RenderCommandBase {
	GLenum target;
	GLenum pname;
	GLfloat param;

	static void APIENTRY execute(const RenderCommand_texParameterf* __restrict params) {
		glTexParameterf(
			params->target,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_texParameterfv : public RenderCommandBase {
	GLenum target;
	GLenum pname;
	const GLfloat *params;

	static void APIENTRY execute(const RenderCommand_texParameterfv* __restrict params) {
		glTexParameterfv(
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_texParameteri : public RenderCommandBase {
	GLenum target;
	GLenum pname;
	GLint param;

	static void APIENTRY execute(const RenderCommand_texParameteri* __restrict params) {
		glTexParameteri(
			params->target,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_texParameteriv : public RenderCommandBase {
	GLenum target;
	GLenum pname;
	const GLint *params;

	static void APIENTRY execute(const RenderCommand_texParameteriv* __restrict params) {
		glTexParameteriv(
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_texImage1D : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLint internalformat;
	GLsizei width;
	GLint border;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_texImage1D* __restrict params) {
		glTexImage1D(
			params->target,
			params->level,
			params->internalformat,
			params->width,
			params->border,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_texImage2D : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLint internalformat;
	GLsizei width;
	GLsizei height;
	GLint border;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_texImage2D* __restrict params) {
		glTexImage2D(
			params->target,
			params->level,
			params->internalformat,
			params->width,
			params->height,
			params->border,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_drawBuffer : public RenderCommandBase {
	GLenum buf;

	static void APIENTRY execute(const RenderCommand_drawBuffer* __restrict params) {
		glDrawBuffer(
			params->buf
		);
	}
};
struct RenderCommand_clear : public RenderCommandBase {
	GLbitfield mask;

	static void APIENTRY execute(const RenderCommand_clear* __restrict params) {
		glClear(
			params->mask
		);
	}
};
struct RenderCommand_clearColor : public RenderCommandBase {
	GLfloat red;
	GLfloat green;
	GLfloat blue;
	GLfloat alpha;

	static void APIENTRY execute(const RenderCommand_clearColor* __restrict params) {
		glClearColor(
			params->red,
			params->green,
			params->blue,
			params->alpha
		);
	}
};
struct RenderCommand_clearStencil : public RenderCommandBase {
	GLint s;

	static void APIENTRY execute(const RenderCommand_clearStencil* __restrict params) {
		glClearStencil(
			params->s
		);
	}
};
struct RenderCommand_clearDepth : public RenderCommandBase {
	GLdouble depth;

	static void APIENTRY execute(const RenderCommand_clearDepth* __restrict params) {
		glClearDepth(
			params->depth
		);
	}
};
struct RenderCommand_stencilMask : public RenderCommandBase {
	GLuint mask;

	static void APIENTRY execute(const RenderCommand_stencilMask* __restrict params) {
		glStencilMask(
			params->mask
		);
	}
};
struct RenderCommand_colorMask : public RenderCommandBase {
	GLboolean red;
	GLboolean green;
	GLboolean blue;
	GLboolean alpha;

	static void APIENTRY execute(const RenderCommand_colorMask* __restrict params) {
		glColorMask(
			params->red,
			params->green,
			params->blue,
			params->alpha
		);
	}
};
struct RenderCommand_depthMask : public RenderCommandBase {
	GLboolean flag;

	static void APIENTRY execute(const RenderCommand_depthMask* __restrict params) {
		glDepthMask(
			params->flag
		);
	}
};
struct RenderCommand_disable : public RenderCommandBase {
	GLenum cap;

	static void APIENTRY execute(const RenderCommand_disable* __restrict params) {
		glDisable(
			params->cap
		);
	}
};
struct RenderCommand_enable : public RenderCommandBase {
	GLenum cap;

	static void APIENTRY execute(const RenderCommand_enable* __restrict params) {
		glEnable(
			params->cap
		);
	}
};
struct RenderCommand_finish : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_finish* __restrict params) {
		glFinish(
		);
	}
};
struct RenderCommand_flush : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_flush* __restrict params) {
		glFlush(
		);
	}
};
struct RenderCommand_blendFunc : public RenderCommandBase {
	GLenum sfactor;
	GLenum dfactor;

	static void APIENTRY execute(const RenderCommand_blendFunc* __restrict params) {
		glBlendFunc(
			params->sfactor,
			params->dfactor
		);
	}
};
struct RenderCommand_logicOp : public RenderCommandBase {
	GLenum opcode;

	static void APIENTRY execute(const RenderCommand_logicOp* __restrict params) {
		glLogicOp(
			params->opcode
		);
	}
};
struct RenderCommand_stencilFunc : public RenderCommandBase {
	GLenum func;
	GLint ref;
	GLuint mask;

	static void APIENTRY execute(const RenderCommand_stencilFunc* __restrict params) {
		glStencilFunc(
			params->func,
			params->ref,
			params->mask
		);
	}
};
struct RenderCommand_stencilOp : public RenderCommandBase {
	GLenum fail;
	GLenum zfail;
	GLenum zpass;

	static void APIENTRY execute(const RenderCommand_stencilOp* __restrict params) {
		glStencilOp(
			params->fail,
			params->zfail,
			params->zpass
		);
	}
};
struct RenderCommand_depthFunc : public RenderCommandBase {
	GLenum func;

	static void APIENTRY execute(const RenderCommand_depthFunc* __restrict params) {
		glDepthFunc(
			params->func
		);
	}
};
struct RenderCommand_pixelStoref : public RenderCommandBase {
	GLenum pname;
	GLfloat param;

	static void APIENTRY execute(const RenderCommand_pixelStoref* __restrict params) {
		glPixelStoref(
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_pixelStorei : public RenderCommandBase {
	GLenum pname;
	GLint param;

	static void APIENTRY execute(const RenderCommand_pixelStorei* __restrict params) {
		glPixelStorei(
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_readBuffer : public RenderCommandBase {
	GLenum src;

	static void APIENTRY execute(const RenderCommand_readBuffer* __restrict params) {
		glReadBuffer(
			params->src
		);
	}
};
struct RenderCommand_readPixels : public RenderCommandBase {
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;
	GLenum format;
	GLenum type;
	void *pixels;

	static void APIENTRY execute(const RenderCommand_readPixels* __restrict params) {
		glReadPixels(
			params->x,
			params->y,
			params->width,
			params->height,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_getBooleanv : public RenderCommandBase {
	GLenum pname;
	GLboolean *data;

	static void APIENTRY execute(const RenderCommand_getBooleanv* __restrict params) {
		glGetBooleanv(
			params->pname,
			params->data
		);
	}
};
struct RenderCommand_getDoublev : public RenderCommandBase {
	GLenum pname;
	GLdouble *data;

	static void APIENTRY execute(const RenderCommand_getDoublev* __restrict params) {
		glGetDoublev(
			params->pname,
			params->data
		);
	}
};
struct RenderCommand_getError : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_getError* __restrict params) {
		glGetError(
		);
	}
};
struct RenderCommand_getFloatv : public RenderCommandBase {
	GLenum pname;
	GLfloat *data;

	static void APIENTRY execute(const RenderCommand_getFloatv* __restrict params) {
		glGetFloatv(
			params->pname,
			params->data
		);
	}
};
struct RenderCommand_getIntegerv : public RenderCommandBase {
	GLenum pname;
	GLint *data;

	static void APIENTRY execute(const RenderCommand_getIntegerv* __restrict params) {
		glGetIntegerv(
			params->pname,
			params->data
		);
	}
};
struct RenderCommand_getString : public RenderCommandBase {
	GLenum name;

	static void APIENTRY execute(const RenderCommand_getString* __restrict params) {
		glGetString(
			params->name
		);
	}
};
struct RenderCommand_getTexImage : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLenum format;
	GLenum type;
	void *pixels;

	static void APIENTRY execute(const RenderCommand_getTexImage* __restrict params) {
		glGetTexImage(
			params->target,
			params->level,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_getTexParameterfv : public RenderCommandBase {
	GLenum target;
	GLenum pname;
	GLfloat *params;

	static void APIENTRY execute(const RenderCommand_getTexParameterfv* __restrict params) {
		glGetTexParameterfv(
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getTexParameteriv : public RenderCommandBase {
	GLenum target;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getTexParameteriv* __restrict params) {
		glGetTexParameteriv(
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getTexLevelParameterfv : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLenum pname;
	GLfloat *params;

	static void APIENTRY execute(const RenderCommand_getTexLevelParameterfv* __restrict params) {
		glGetTexLevelParameterfv(
			params->target,
			params->level,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getTexLevelParameteriv : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getTexLevelParameteriv* __restrict params) {
		glGetTexLevelParameteriv(
			params->target,
			params->level,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_isEnabled : public RenderCommandBase {
	GLenum cap;

	static void APIENTRY execute(const RenderCommand_isEnabled* __restrict params) {
		glIsEnabled(
			params->cap
		);
	}
};
struct RenderCommand_depthRange : public RenderCommandBase {
	GLdouble n;
	GLdouble f;

	static void APIENTRY execute(const RenderCommand_depthRange* __restrict params) {
		glDepthRange(
			params->n,
			params->f
		);
	}
};
struct RenderCommand_viewport : public RenderCommandBase {
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_viewport* __restrict params) {
		glViewport(
			params->x,
			params->y,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_drawArrays : public RenderCommandBase {
	GLenum mode;
	GLint first;
	GLsizei count;

	static void APIENTRY execute(const RenderCommand_drawArrays* __restrict params) {
		glDrawArrays(
			params->mode,
			params->first,
			params->count
		);
	}
};
struct RenderCommand_drawElements : public RenderCommandBase {
	GLenum mode;
	GLsizei count;
	GLenum type;
	const void *indices;

	static void APIENTRY execute(const RenderCommand_drawElements* __restrict params) {
		glDrawElements(
			params->mode,
			params->count,
			params->type,
			params->indices
		);
	}
};
struct RenderCommand_getPointerv : public RenderCommandBase {
	GLenum pname;
	void **params;

	static void APIENTRY execute(const RenderCommand_getPointerv* __restrict params) {
		glGetPointerv(
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_polygonOffset : public RenderCommandBase {
	GLfloat factor;
	GLfloat units;

	static void APIENTRY execute(const RenderCommand_polygonOffset* __restrict params) {
		glPolygonOffset(
			params->factor,
			params->units
		);
	}
};
struct RenderCommand_copyTexImage1D : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLenum internalformat;
	GLint x;
	GLint y;
	GLsizei width;
	GLint border;

	static void APIENTRY execute(const RenderCommand_copyTexImage1D* __restrict params) {
		glCopyTexImage1D(
			params->target,
			params->level,
			params->internalformat,
			params->x,
			params->y,
			params->width,
			params->border
		);
	}
};
struct RenderCommand_copyTexImage2D : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLenum internalformat;
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;
	GLint border;

	static void APIENTRY execute(const RenderCommand_copyTexImage2D* __restrict params) {
		glCopyTexImage2D(
			params->target,
			params->level,
			params->internalformat,
			params->x,
			params->y,
			params->width,
			params->height,
			params->border
		);
	}
};
struct RenderCommand_copyTexSubImage1D : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint x;
	GLint y;
	GLsizei width;

	static void APIENTRY execute(const RenderCommand_copyTexSubImage1D* __restrict params) {
		glCopyTexSubImage1D(
			params->target,
			params->level,
			params->xoffset,
			params->x,
			params->y,
			params->width
		);
	}
};
struct RenderCommand_copyTexSubImage2D : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_copyTexSubImage2D* __restrict params) {
		glCopyTexSubImage2D(
			params->target,
			params->level,
			params->xoffset,
			params->yoffset,
			params->x,
			params->y,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_texSubImage1D : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLint xoffset;
	GLsizei width;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_texSubImage1D* __restrict params) {
		glTexSubImage1D(
			params->target,
			params->level,
			params->xoffset,
			params->width,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_texSubImage2D : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLsizei width;
	GLsizei height;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_texSubImage2D* __restrict params) {
		glTexSubImage2D(
			params->target,
			params->level,
			params->xoffset,
			params->yoffset,
			params->width,
			params->height,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_bindTexture : public RenderCommandBase {
	GLenum target;
	GLuint texture;

	static void APIENTRY execute(const RenderCommand_bindTexture* __restrict params) {
		glBindTexture(
			params->target,
			params->texture
		);
	}
};
struct RenderCommand_deleteTextures : public RenderCommandBase {
	GLsizei n;
	const GLuint *textures;

	static void APIENTRY execute(const RenderCommand_deleteTextures* __restrict params) {
		glDeleteTextures(
			params->n,
			params->textures
		);
	}
};
struct RenderCommand_genTextures : public RenderCommandBase {
	GLsizei n;
	GLuint *textures;

	static void APIENTRY execute(const RenderCommand_genTextures* __restrict params) {
		glGenTextures(
			params->n,
			params->textures
		);
	}
};
struct RenderCommand_isTexture : public RenderCommandBase {
	GLuint texture;

	static void APIENTRY execute(const RenderCommand_isTexture* __restrict params) {
		glIsTexture(
			params->texture
		);
	}
};
struct RenderCommand_drawRangeElements : public RenderCommandBase {
	GLenum mode;
	GLuint start;
	GLuint end;
	GLsizei count;
	GLenum type;
	const void *indices;

	static void APIENTRY execute(const RenderCommand_drawRangeElements* __restrict params) {
		glDrawRangeElements(
			params->mode,
			params->start,
			params->end,
			params->count,
			params->type,
			params->indices
		);
	}
};
struct RenderCommand_texImage3D : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLint internalformat;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLint border;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_texImage3D* __restrict params) {
		glTexImage3D(
			params->target,
			params->level,
			params->internalformat,
			params->width,
			params->height,
			params->depth,
			params->border,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_texSubImage3D : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint zoffset;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_texSubImage3D* __restrict params) {
		glTexSubImage3D(
			params->target,
			params->level,
			params->xoffset,
			params->yoffset,
			params->zoffset,
			params->width,
			params->height,
			params->depth,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_copyTexSubImage3D : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint zoffset;
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_copyTexSubImage3D* __restrict params) {
		glCopyTexSubImage3D(
			params->target,
			params->level,
			params->xoffset,
			params->yoffset,
			params->zoffset,
			params->x,
			params->y,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_activeTexture : public RenderCommandBase {
	GLenum texture;

	static void APIENTRY execute(const RenderCommand_activeTexture* __restrict params) {
		glActiveTexture(
			params->texture
		);
	}
};
struct RenderCommand_sampleCoverage : public RenderCommandBase {
	GLfloat value;
	GLboolean invert;

	static void APIENTRY execute(const RenderCommand_sampleCoverage* __restrict params) {
		glSampleCoverage(
			params->value,
			params->invert
		);
	}
};
struct RenderCommand_compressedTexImage3D : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLint border;
	GLsizei imageSize;
	const void *data;

	static void APIENTRY execute(const RenderCommand_compressedTexImage3D* __restrict params) {
		glCompressedTexImage3D(
			params->target,
			params->level,
			params->internalformat,
			params->width,
			params->height,
			params->depth,
			params->border,
			params->imageSize,
			params->data
		);
	}
};
struct RenderCommand_compressedTexImage2D : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;
	GLint border;
	GLsizei imageSize;
	const void *data;

	static void APIENTRY execute(const RenderCommand_compressedTexImage2D* __restrict params) {
		glCompressedTexImage2D(
			params->target,
			params->level,
			params->internalformat,
			params->width,
			params->height,
			params->border,
			params->imageSize,
			params->data
		);
	}
};
struct RenderCommand_compressedTexImage1D : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLenum internalformat;
	GLsizei width;
	GLint border;
	GLsizei imageSize;
	const void *data;

	static void APIENTRY execute(const RenderCommand_compressedTexImage1D* __restrict params) {
		glCompressedTexImage1D(
			params->target,
			params->level,
			params->internalformat,
			params->width,
			params->border,
			params->imageSize,
			params->data
		);
	}
};
struct RenderCommand_compressedTexSubImage3D : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint zoffset;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLenum format;
	GLsizei imageSize;
	const void *data;

	static void APIENTRY execute(const RenderCommand_compressedTexSubImage3D* __restrict params) {
		glCompressedTexSubImage3D(
			params->target,
			params->level,
			params->xoffset,
			params->yoffset,
			params->zoffset,
			params->width,
			params->height,
			params->depth,
			params->format,
			params->imageSize,
			params->data
		);
	}
};
struct RenderCommand_compressedTexSubImage2D : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLsizei width;
	GLsizei height;
	GLenum format;
	GLsizei imageSize;
	const void *data;

	static void APIENTRY execute(const RenderCommand_compressedTexSubImage2D* __restrict params) {
		glCompressedTexSubImage2D(
			params->target,
			params->level,
			params->xoffset,
			params->yoffset,
			params->width,
			params->height,
			params->format,
			params->imageSize,
			params->data
		);
	}
};
struct RenderCommand_compressedTexSubImage1D : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLint xoffset;
	GLsizei width;
	GLenum format;
	GLsizei imageSize;
	const void *data;

	static void APIENTRY execute(const RenderCommand_compressedTexSubImage1D* __restrict params) {
		glCompressedTexSubImage1D(
			params->target,
			params->level,
			params->xoffset,
			params->width,
			params->format,
			params->imageSize,
			params->data
		);
	}
};
struct RenderCommand_getCompressedTexImage : public RenderCommandBase {
	GLenum target;
	GLint level;
	void *img;

	static void APIENTRY execute(const RenderCommand_getCompressedTexImage* __restrict params) {
		glGetCompressedTexImage(
			params->target,
			params->level,
			params->img
		);
	}
};
struct RenderCommand_blendFuncSeparate : public RenderCommandBase {
	GLenum sfactorRGB;
	GLenum dfactorRGB;
	GLenum sfactorAlpha;
	GLenum dfactorAlpha;

	static void APIENTRY execute(const RenderCommand_blendFuncSeparate* __restrict params) {
		glBlendFuncSeparate(
			params->sfactorRGB,
			params->dfactorRGB,
			params->sfactorAlpha,
			params->dfactorAlpha
		);
	}
};
struct RenderCommand_multiDrawArrays : public RenderCommandBase {
	GLenum mode;
	const GLint *first;
	const GLsizei *count;
	GLsizei drawcount;

	static void APIENTRY execute(const RenderCommand_multiDrawArrays* __restrict params) {
		glMultiDrawArrays(
			params->mode,
			params->first,
			params->count,
			params->drawcount
		);
	}
};
struct RenderCommand_multiDrawElements : public RenderCommandBase {
	GLenum mode;
	const GLsizei *count;
	GLenum type;
	const void *const*indices;
	GLsizei drawcount;

	static void APIENTRY execute(const RenderCommand_multiDrawElements* __restrict params) {
		glMultiDrawElements(
			params->mode,
			params->count,
			params->type,
			params->indices,
			params->drawcount
		);
	}
};
struct RenderCommand_pointParameterf : public RenderCommandBase {
	GLenum pname;
	GLfloat param;

	static void APIENTRY execute(const RenderCommand_pointParameterf* __restrict params) {
		glPointParameterf(
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_pointParameterfv : public RenderCommandBase {
	GLenum pname;
	const GLfloat *params;

	static void APIENTRY execute(const RenderCommand_pointParameterfv* __restrict params) {
		glPointParameterfv(
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_pointParameteri : public RenderCommandBase {
	GLenum pname;
	GLint param;

	static void APIENTRY execute(const RenderCommand_pointParameteri* __restrict params) {
		glPointParameteri(
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_pointParameteriv : public RenderCommandBase {
	GLenum pname;
	const GLint *params;

	static void APIENTRY execute(const RenderCommand_pointParameteriv* __restrict params) {
		glPointParameteriv(
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_blendColor : public RenderCommandBase {
	GLfloat red;
	GLfloat green;
	GLfloat blue;
	GLfloat alpha;

	static void APIENTRY execute(const RenderCommand_blendColor* __restrict params) {
		glBlendColor(
			params->red,
			params->green,
			params->blue,
			params->alpha
		);
	}
};
struct RenderCommand_blendEquation : public RenderCommandBase {
	GLenum mode;

	static void APIENTRY execute(const RenderCommand_blendEquation* __restrict params) {
		glBlendEquation(
			params->mode
		);
	}
};
struct RenderCommand_genQueries : public RenderCommandBase {
	GLsizei n;
	GLuint *ids;

	static void APIENTRY execute(const RenderCommand_genQueries* __restrict params) {
		glGenQueries(
			params->n,
			params->ids
		);
	}
};
struct RenderCommand_deleteQueries : public RenderCommandBase {
	GLsizei n;
	const GLuint *ids;

	static void APIENTRY execute(const RenderCommand_deleteQueries* __restrict params) {
		glDeleteQueries(
			params->n,
			params->ids
		);
	}
};
struct RenderCommand_isQuery : public RenderCommandBase {
	GLuint id;

	static void APIENTRY execute(const RenderCommand_isQuery* __restrict params) {
		glIsQuery(
			params->id
		);
	}
};
struct RenderCommand_beginQuery : public RenderCommandBase {
	GLenum target;
	GLuint id;

	static void APIENTRY execute(const RenderCommand_beginQuery* __restrict params) {
		glBeginQuery(
			params->target,
			params->id
		);
	}
};
struct RenderCommand_endQuery : public RenderCommandBase {
	GLenum target;

	static void APIENTRY execute(const RenderCommand_endQuery* __restrict params) {
		glEndQuery(
			params->target
		);
	}
};
struct RenderCommand_getQueryiv : public RenderCommandBase {
	GLenum target;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getQueryiv* __restrict params) {
		glGetQueryiv(
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getQueryObjectiv : public RenderCommandBase {
	GLuint id;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getQueryObjectiv* __restrict params) {
		glGetQueryObjectiv(
			params->id,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getQueryObjectuiv : public RenderCommandBase {
	GLuint id;
	GLenum pname;
	GLuint *params;

	static void APIENTRY execute(const RenderCommand_getQueryObjectuiv* __restrict params) {
		glGetQueryObjectuiv(
			params->id,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_bindBuffer : public RenderCommandBase {
	GLenum target;
	GLuint buffer;

	static void APIENTRY execute(const RenderCommand_bindBuffer* __restrict params) {
		glBindBuffer(
			params->target,
			params->buffer
		);
	}
};
struct RenderCommand_deleteBuffers : public RenderCommandBase {
	GLsizei n;
	const GLuint *buffers;

	static void APIENTRY execute(const RenderCommand_deleteBuffers* __restrict params) {
		glDeleteBuffers(
			params->n,
			params->buffers
		);
	}
};
struct RenderCommand_genBuffers : public RenderCommandBase {
	GLsizei n;
	GLuint *buffers;

	static void APIENTRY execute(const RenderCommand_genBuffers* __restrict params) {
		glGenBuffers(
			params->n,
			params->buffers
		);
	}
};
struct RenderCommand_isBuffer : public RenderCommandBase {
	GLuint buffer;

	static void APIENTRY execute(const RenderCommand_isBuffer* __restrict params) {
		glIsBuffer(
			params->buffer
		);
	}
};
struct RenderCommand_bufferData : public RenderCommandBase {
	GLenum target;
	GLsizeiptr size;
	const void *data;
	GLenum usage;

	static void APIENTRY execute(const RenderCommand_bufferData* __restrict params) {
		glBufferData(
			params->target,
			params->size,
			params->data,
			params->usage
		);
	}
};
struct RenderCommand_bufferSubData : public RenderCommandBase {
	GLenum target;
	GLintptr offset;
	GLsizeiptr size;
	const void *data;

	static void APIENTRY execute(const RenderCommand_bufferSubData* __restrict params) {
		glBufferSubData(
			params->target,
			params->offset,
			params->size,
			params->data
		);
	}
};
struct RenderCommand_getBufferSubData : public RenderCommandBase {
	GLenum target;
	GLintptr offset;
	GLsizeiptr size;
	void *data;

	static void APIENTRY execute(const RenderCommand_getBufferSubData* __restrict params) {
		glGetBufferSubData(
			params->target,
			params->offset,
			params->size,
			params->data
		);
	}
};
struct RenderCommand_mapBuffer : public RenderCommandBase {
	GLenum target;
	GLenum access;

	static void APIENTRY execute(const RenderCommand_mapBuffer* __restrict params) {
		glMapBuffer(
			params->target,
			params->access
		);
	}
};
struct RenderCommand_unmapBuffer : public RenderCommandBase {
	GLenum target;

	static void APIENTRY execute(const RenderCommand_unmapBuffer* __restrict params) {
		glUnmapBuffer(
			params->target
		);
	}
};
struct RenderCommand_getBufferParameteriv : public RenderCommandBase {
	GLenum target;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getBufferParameteriv* __restrict params) {
		glGetBufferParameteriv(
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getBufferPointerv : public RenderCommandBase {
	GLenum target;
	GLenum pname;
	void **params;

	static void APIENTRY execute(const RenderCommand_getBufferPointerv* __restrict params) {
		glGetBufferPointerv(
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_blendEquationSeparate : public RenderCommandBase {
	GLenum modeRGB;
	GLenum modeAlpha;

	static void APIENTRY execute(const RenderCommand_blendEquationSeparate* __restrict params) {
		glBlendEquationSeparate(
			params->modeRGB,
			params->modeAlpha
		);
	}
};
struct RenderCommand_drawBuffers : public RenderCommandBase {
	GLsizei n;
	const GLenum *bufs;

	static void APIENTRY execute(const RenderCommand_drawBuffers* __restrict params) {
		glDrawBuffers(
			params->n,
			params->bufs
		);
	}
};
struct RenderCommand_stencilOpSeparate : public RenderCommandBase {
	GLenum face;
	GLenum sfail;
	GLenum dpfail;
	GLenum dppass;

	static void APIENTRY execute(const RenderCommand_stencilOpSeparate* __restrict params) {
		glStencilOpSeparate(
			params->face,
			params->sfail,
			params->dpfail,
			params->dppass
		);
	}
};
struct RenderCommand_stencilFuncSeparate : public RenderCommandBase {
	GLenum face;
	GLenum func;
	GLint ref;
	GLuint mask;

	static void APIENTRY execute(const RenderCommand_stencilFuncSeparate* __restrict params) {
		glStencilFuncSeparate(
			params->face,
			params->func,
			params->ref,
			params->mask
		);
	}
};
struct RenderCommand_stencilMaskSeparate : public RenderCommandBase {
	GLenum face;
	GLuint mask;

	static void APIENTRY execute(const RenderCommand_stencilMaskSeparate* __restrict params) {
		glStencilMaskSeparate(
			params->face,
			params->mask
		);
	}
};
struct RenderCommand_attachShader : public RenderCommandBase {
	GLuint program;
	GLuint shader;

	static void APIENTRY execute(const RenderCommand_attachShader* __restrict params) {
		glAttachShader(
			params->program,
			params->shader
		);
	}
};
struct RenderCommand_bindAttribLocation : public RenderCommandBase {
	GLuint program;
	GLuint index;
	const GLchar *name;

	static void APIENTRY execute(const RenderCommand_bindAttribLocation* __restrict params) {
		glBindAttribLocation(
			params->program,
			params->index,
			params->name
		);
	}
};
struct RenderCommand_compileShader : public RenderCommandBase {
	GLuint shader;

	static void APIENTRY execute(const RenderCommand_compileShader* __restrict params) {
		glCompileShader(
			params->shader
		);
	}
};
struct RenderCommand_createProgram : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_createProgram* __restrict params) {
		glCreateProgram(
		);
	}
};
struct RenderCommand_createShader : public RenderCommandBase {
	GLenum type;

	static void APIENTRY execute(const RenderCommand_createShader* __restrict params) {
		glCreateShader(
			params->type
		);
	}
};
struct RenderCommand_deleteProgram : public RenderCommandBase {
	GLuint program;

	static void APIENTRY execute(const RenderCommand_deleteProgram* __restrict params) {
		glDeleteProgram(
			params->program
		);
	}
};
struct RenderCommand_deleteShader : public RenderCommandBase {
	GLuint shader;

	static void APIENTRY execute(const RenderCommand_deleteShader* __restrict params) {
		glDeleteShader(
			params->shader
		);
	}
};
struct RenderCommand_detachShader : public RenderCommandBase {
	GLuint program;
	GLuint shader;

	static void APIENTRY execute(const RenderCommand_detachShader* __restrict params) {
		glDetachShader(
			params->program,
			params->shader
		);
	}
};
struct RenderCommand_disableVertexAttribArray : public RenderCommandBase {
	GLuint index;

	static void APIENTRY execute(const RenderCommand_disableVertexAttribArray* __restrict params) {
		glDisableVertexAttribArray(
			params->index
		);
	}
};
struct RenderCommand_enableVertexAttribArray : public RenderCommandBase {
	GLuint index;

	static void APIENTRY execute(const RenderCommand_enableVertexAttribArray* __restrict params) {
		glEnableVertexAttribArray(
			params->index
		);
	}
};
struct RenderCommand_getActiveAttrib : public RenderCommandBase {
	GLuint program;
	GLuint index;
	GLsizei bufSize;
	GLsizei *length;
	GLint *size;
	GLenum *type;
	GLchar *name;

	static void APIENTRY execute(const RenderCommand_getActiveAttrib* __restrict params) {
		glGetActiveAttrib(
			params->program,
			params->index,
			params->bufSize,
			params->length,
			params->size,
			params->type,
			params->name
		);
	}
};
struct RenderCommand_getActiveUniform : public RenderCommandBase {
	GLuint program;
	GLuint index;
	GLsizei bufSize;
	GLsizei *length;
	GLint *size;
	GLenum *type;
	GLchar *name;

	static void APIENTRY execute(const RenderCommand_getActiveUniform* __restrict params) {
		glGetActiveUniform(
			params->program,
			params->index,
			params->bufSize,
			params->length,
			params->size,
			params->type,
			params->name
		);
	}
};
struct RenderCommand_getAttachedShaders : public RenderCommandBase {
	GLuint program;
	GLsizei maxCount;
	GLsizei *count;
	GLuint *shaders;

	static void APIENTRY execute(const RenderCommand_getAttachedShaders* __restrict params) {
		glGetAttachedShaders(
			params->program,
			params->maxCount,
			params->count,
			params->shaders
		);
	}
};
struct RenderCommand_getAttribLocation : public RenderCommandBase {
	GLuint program;
	const GLchar *name;

	static void APIENTRY execute(const RenderCommand_getAttribLocation* __restrict params) {
		glGetAttribLocation(
			params->program,
			params->name
		);
	}
};
struct RenderCommand_getProgramiv : public RenderCommandBase {
	GLuint program;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getProgramiv* __restrict params) {
		glGetProgramiv(
			params->program,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getProgramInfoLog : public RenderCommandBase {
	GLuint program;
	GLsizei bufSize;
	GLsizei *length;
	GLchar *infoLog;

	static void APIENTRY execute(const RenderCommand_getProgramInfoLog* __restrict params) {
		glGetProgramInfoLog(
			params->program,
			params->bufSize,
			params->length,
			params->infoLog
		);
	}
};
struct RenderCommand_getShaderiv : public RenderCommandBase {
	GLuint shader;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getShaderiv* __restrict params) {
		glGetShaderiv(
			params->shader,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getShaderInfoLog : public RenderCommandBase {
	GLuint shader;
	GLsizei bufSize;
	GLsizei *length;
	GLchar *infoLog;

	static void APIENTRY execute(const RenderCommand_getShaderInfoLog* __restrict params) {
		glGetShaderInfoLog(
			params->shader,
			params->bufSize,
			params->length,
			params->infoLog
		);
	}
};
struct RenderCommand_getShaderSource : public RenderCommandBase {
	GLuint shader;
	GLsizei bufSize;
	GLsizei *length;
	GLchar *source;

	static void APIENTRY execute(const RenderCommand_getShaderSource* __restrict params) {
		glGetShaderSource(
			params->shader,
			params->bufSize,
			params->length,
			params->source
		);
	}
};
struct RenderCommand_getUniformLocation : public RenderCommandBase {
	GLuint program;
	const GLchar *name;

	static void APIENTRY execute(const RenderCommand_getUniformLocation* __restrict params) {
		glGetUniformLocation(
			params->program,
			params->name
		);
	}
};
struct RenderCommand_getUniformfv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLfloat *params;

	static void APIENTRY execute(const RenderCommand_getUniformfv* __restrict params) {
		glGetUniformfv(
			params->program,
			params->location,
			params->params
		);
	}
};
struct RenderCommand_getUniformiv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getUniformiv* __restrict params) {
		glGetUniformiv(
			params->program,
			params->location,
			params->params
		);
	}
};
struct RenderCommand_getVertexAttribdv : public RenderCommandBase {
	GLuint index;
	GLenum pname;
	GLdouble *params;

	static void APIENTRY execute(const RenderCommand_getVertexAttribdv* __restrict params) {
		glGetVertexAttribdv(
			params->index,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getVertexAttribfv : public RenderCommandBase {
	GLuint index;
	GLenum pname;
	GLfloat *params;

	static void APIENTRY execute(const RenderCommand_getVertexAttribfv* __restrict params) {
		glGetVertexAttribfv(
			params->index,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getVertexAttribiv : public RenderCommandBase {
	GLuint index;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getVertexAttribiv* __restrict params) {
		glGetVertexAttribiv(
			params->index,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getVertexAttribPointerv : public RenderCommandBase {
	GLuint index;
	GLenum pname;
	void **pointer;

	static void APIENTRY execute(const RenderCommand_getVertexAttribPointerv* __restrict params) {
		glGetVertexAttribPointerv(
			params->index,
			params->pname,
			params->pointer
		);
	}
};
struct RenderCommand_isProgram : public RenderCommandBase {
	GLuint program;

	static void APIENTRY execute(const RenderCommand_isProgram* __restrict params) {
		glIsProgram(
			params->program
		);
	}
};
struct RenderCommand_isShader : public RenderCommandBase {
	GLuint shader;

	static void APIENTRY execute(const RenderCommand_isShader* __restrict params) {
		glIsShader(
			params->shader
		);
	}
};
struct RenderCommand_linkProgram : public RenderCommandBase {
	GLuint program;

	static void APIENTRY execute(const RenderCommand_linkProgram* __restrict params) {
		glLinkProgram(
			params->program
		);
	}
};
struct RenderCommand_shaderSource : public RenderCommandBase {
	GLuint shader;
	GLsizei count;
	const GLchar *const*string;
	const GLint *length;

	static void APIENTRY execute(const RenderCommand_shaderSource* __restrict params) {
		glShaderSource(
			params->shader,
			params->count,
			params->string,
			params->length
		);
	}
};
struct RenderCommand_useProgram : public RenderCommandBase {
	GLuint program;

	static void APIENTRY execute(const RenderCommand_useProgram* __restrict params) {
		glUseProgram(
			params->program
		);
	}
};
struct RenderCommand_uniform1f : public RenderCommandBase {
	GLint location;
	GLfloat v0;

	static void APIENTRY execute(const RenderCommand_uniform1f* __restrict params) {
		glUniform1f(
			params->location,
			params->v0
		);
	}
};
struct RenderCommand_uniform2f : public RenderCommandBase {
	GLint location;
	GLfloat v0;
	GLfloat v1;

	static void APIENTRY execute(const RenderCommand_uniform2f* __restrict params) {
		glUniform2f(
			params->location,
			params->v0,
			params->v1
		);
	}
};
struct RenderCommand_uniform3f : public RenderCommandBase {
	GLint location;
	GLfloat v0;
	GLfloat v1;
	GLfloat v2;

	static void APIENTRY execute(const RenderCommand_uniform3f* __restrict params) {
		glUniform3f(
			params->location,
			params->v0,
			params->v1,
			params->v2
		);
	}
};
struct RenderCommand_uniform4f : public RenderCommandBase {
	GLint location;
	GLfloat v0;
	GLfloat v1;
	GLfloat v2;
	GLfloat v3;

	static void APIENTRY execute(const RenderCommand_uniform4f* __restrict params) {
		glUniform4f(
			params->location,
			params->v0,
			params->v1,
			params->v2,
			params->v3
		);
	}
};
struct RenderCommand_uniform1i : public RenderCommandBase {
	GLint location;
	GLint v0;

	static void APIENTRY execute(const RenderCommand_uniform1i* __restrict params) {
		glUniform1i(
			params->location,
			params->v0
		);
	}
};
struct RenderCommand_uniform2i : public RenderCommandBase {
	GLint location;
	GLint v0;
	GLint v1;

	static void APIENTRY execute(const RenderCommand_uniform2i* __restrict params) {
		glUniform2i(
			params->location,
			params->v0,
			params->v1
		);
	}
};
struct RenderCommand_uniform3i : public RenderCommandBase {
	GLint location;
	GLint v0;
	GLint v1;
	GLint v2;

	static void APIENTRY execute(const RenderCommand_uniform3i* __restrict params) {
		glUniform3i(
			params->location,
			params->v0,
			params->v1,
			params->v2
		);
	}
};
struct RenderCommand_uniform4i : public RenderCommandBase {
	GLint location;
	GLint v0;
	GLint v1;
	GLint v2;
	GLint v3;

	static void APIENTRY execute(const RenderCommand_uniform4i* __restrict params) {
		glUniform4i(
			params->location,
			params->v0,
			params->v1,
			params->v2,
			params->v3
		);
	}
};
struct RenderCommand_uniform1fv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_uniform1fv* __restrict params) {
		glUniform1fv(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform2fv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_uniform2fv* __restrict params) {
		glUniform2fv(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform3fv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_uniform3fv* __restrict params) {
		glUniform3fv(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform4fv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_uniform4fv* __restrict params) {
		glUniform4fv(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform1iv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLint *value;

	static void APIENTRY execute(const RenderCommand_uniform1iv* __restrict params) {
		glUniform1iv(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform2iv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLint *value;

	static void APIENTRY execute(const RenderCommand_uniform2iv* __restrict params) {
		glUniform2iv(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform3iv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLint *value;

	static void APIENTRY execute(const RenderCommand_uniform3iv* __restrict params) {
		glUniform3iv(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform4iv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLint *value;

	static void APIENTRY execute(const RenderCommand_uniform4iv* __restrict params) {
		glUniform4iv(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniformMatrix2fv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_uniformMatrix2fv* __restrict params) {
		glUniformMatrix2fv(
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_uniformMatrix3fv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_uniformMatrix3fv* __restrict params) {
		glUniformMatrix3fv(
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_uniformMatrix4fv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_uniformMatrix4fv* __restrict params) {
		glUniformMatrix4fv(
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_validateProgram : public RenderCommandBase {
	GLuint program;

	static void APIENTRY execute(const RenderCommand_validateProgram* __restrict params) {
		glValidateProgram(
			params->program
		);
	}
};
struct RenderCommand_vertexAttrib1d : public RenderCommandBase {
	GLuint index;
	GLdouble x;

	static void APIENTRY execute(const RenderCommand_vertexAttrib1d* __restrict params) {
		glVertexAttrib1d(
			params->index,
			params->x
		);
	}
};
struct RenderCommand_vertexAttrib1dv : public RenderCommandBase {
	GLuint index;
	const GLdouble *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib1dv* __restrict params) {
		glVertexAttrib1dv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib1f : public RenderCommandBase {
	GLuint index;
	GLfloat x;

	static void APIENTRY execute(const RenderCommand_vertexAttrib1f* __restrict params) {
		glVertexAttrib1f(
			params->index,
			params->x
		);
	}
};
struct RenderCommand_vertexAttrib1fv : public RenderCommandBase {
	GLuint index;
	const GLfloat *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib1fv* __restrict params) {
		glVertexAttrib1fv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib1s : public RenderCommandBase {
	GLuint index;
	GLshort x;

	static void APIENTRY execute(const RenderCommand_vertexAttrib1s* __restrict params) {
		glVertexAttrib1s(
			params->index,
			params->x
		);
	}
};
struct RenderCommand_vertexAttrib1sv : public RenderCommandBase {
	GLuint index;
	const GLshort *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib1sv* __restrict params) {
		glVertexAttrib1sv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib2d : public RenderCommandBase {
	GLuint index;
	GLdouble x;
	GLdouble y;

	static void APIENTRY execute(const RenderCommand_vertexAttrib2d* __restrict params) {
		glVertexAttrib2d(
			params->index,
			params->x,
			params->y
		);
	}
};
struct RenderCommand_vertexAttrib2dv : public RenderCommandBase {
	GLuint index;
	const GLdouble *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib2dv* __restrict params) {
		glVertexAttrib2dv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib2f : public RenderCommandBase {
	GLuint index;
	GLfloat x;
	GLfloat y;

	static void APIENTRY execute(const RenderCommand_vertexAttrib2f* __restrict params) {
		glVertexAttrib2f(
			params->index,
			params->x,
			params->y
		);
	}
};
struct RenderCommand_vertexAttrib2fv : public RenderCommandBase {
	GLuint index;
	const GLfloat *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib2fv* __restrict params) {
		glVertexAttrib2fv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib2s : public RenderCommandBase {
	GLuint index;
	GLshort x;
	GLshort y;

	static void APIENTRY execute(const RenderCommand_vertexAttrib2s* __restrict params) {
		glVertexAttrib2s(
			params->index,
			params->x,
			params->y
		);
	}
};
struct RenderCommand_vertexAttrib2sv : public RenderCommandBase {
	GLuint index;
	const GLshort *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib2sv* __restrict params) {
		glVertexAttrib2sv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib3d : public RenderCommandBase {
	GLuint index;
	GLdouble x;
	GLdouble y;
	GLdouble z;

	static void APIENTRY execute(const RenderCommand_vertexAttrib3d* __restrict params) {
		glVertexAttrib3d(
			params->index,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_vertexAttrib3dv : public RenderCommandBase {
	GLuint index;
	const GLdouble *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib3dv* __restrict params) {
		glVertexAttrib3dv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib3f : public RenderCommandBase {
	GLuint index;
	GLfloat x;
	GLfloat y;
	GLfloat z;

	static void APIENTRY execute(const RenderCommand_vertexAttrib3f* __restrict params) {
		glVertexAttrib3f(
			params->index,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_vertexAttrib3fv : public RenderCommandBase {
	GLuint index;
	const GLfloat *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib3fv* __restrict params) {
		glVertexAttrib3fv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib3s : public RenderCommandBase {
	GLuint index;
	GLshort x;
	GLshort y;
	GLshort z;

	static void APIENTRY execute(const RenderCommand_vertexAttrib3s* __restrict params) {
		glVertexAttrib3s(
			params->index,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_vertexAttrib3sv : public RenderCommandBase {
	GLuint index;
	const GLshort *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib3sv* __restrict params) {
		glVertexAttrib3sv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib4Nbv : public RenderCommandBase {
	GLuint index;
	const GLbyte *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib4Nbv* __restrict params) {
		glVertexAttrib4Nbv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib4Niv : public RenderCommandBase {
	GLuint index;
	const GLint *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib4Niv* __restrict params) {
		glVertexAttrib4Niv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib4Nsv : public RenderCommandBase {
	GLuint index;
	const GLshort *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib4Nsv* __restrict params) {
		glVertexAttrib4Nsv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib4Nub : public RenderCommandBase {
	GLuint index;
	GLubyte x;
	GLubyte y;
	GLubyte z;
	GLubyte w;

	static void APIENTRY execute(const RenderCommand_vertexAttrib4Nub* __restrict params) {
		glVertexAttrib4Nub(
			params->index,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_vertexAttrib4Nubv : public RenderCommandBase {
	GLuint index;
	const GLubyte *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib4Nubv* __restrict params) {
		glVertexAttrib4Nubv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib4Nuiv : public RenderCommandBase {
	GLuint index;
	const GLuint *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib4Nuiv* __restrict params) {
		glVertexAttrib4Nuiv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib4Nusv : public RenderCommandBase {
	GLuint index;
	const GLushort *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib4Nusv* __restrict params) {
		glVertexAttrib4Nusv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib4bv : public RenderCommandBase {
	GLuint index;
	const GLbyte *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib4bv* __restrict params) {
		glVertexAttrib4bv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib4d : public RenderCommandBase {
	GLuint index;
	GLdouble x;
	GLdouble y;
	GLdouble z;
	GLdouble w;

	static void APIENTRY execute(const RenderCommand_vertexAttrib4d* __restrict params) {
		glVertexAttrib4d(
			params->index,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_vertexAttrib4dv : public RenderCommandBase {
	GLuint index;
	const GLdouble *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib4dv* __restrict params) {
		glVertexAttrib4dv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib4f : public RenderCommandBase {
	GLuint index;
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat w;

	static void APIENTRY execute(const RenderCommand_vertexAttrib4f* __restrict params) {
		glVertexAttrib4f(
			params->index,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_vertexAttrib4fv : public RenderCommandBase {
	GLuint index;
	const GLfloat *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib4fv* __restrict params) {
		glVertexAttrib4fv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib4iv : public RenderCommandBase {
	GLuint index;
	const GLint *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib4iv* __restrict params) {
		glVertexAttrib4iv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib4s : public RenderCommandBase {
	GLuint index;
	GLshort x;
	GLshort y;
	GLshort z;
	GLshort w;

	static void APIENTRY execute(const RenderCommand_vertexAttrib4s* __restrict params) {
		glVertexAttrib4s(
			params->index,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_vertexAttrib4sv : public RenderCommandBase {
	GLuint index;
	const GLshort *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib4sv* __restrict params) {
		glVertexAttrib4sv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib4ubv : public RenderCommandBase {
	GLuint index;
	const GLubyte *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib4ubv* __restrict params) {
		glVertexAttrib4ubv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib4uiv : public RenderCommandBase {
	GLuint index;
	const GLuint *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib4uiv* __restrict params) {
		glVertexAttrib4uiv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttrib4usv : public RenderCommandBase {
	GLuint index;
	const GLushort *v;

	static void APIENTRY execute(const RenderCommand_vertexAttrib4usv* __restrict params) {
		glVertexAttrib4usv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribPointer : public RenderCommandBase {
	GLuint index;
	GLint size;
	GLenum type;
	GLboolean normalized;
	GLsizei stride;
	const void *pointer;

	static void APIENTRY execute(const RenderCommand_vertexAttribPointer* __restrict params) {
		glVertexAttribPointer(
			params->index,
			params->size,
			params->type,
			params->normalized,
			params->stride,
			params->pointer
		);
	}
};
struct RenderCommand_uniformMatrix2x3fv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_uniformMatrix2x3fv* __restrict params) {
		glUniformMatrix2x3fv(
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_uniformMatrix3x2fv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_uniformMatrix3x2fv* __restrict params) {
		glUniformMatrix3x2fv(
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_uniformMatrix2x4fv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_uniformMatrix2x4fv* __restrict params) {
		glUniformMatrix2x4fv(
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_uniformMatrix4x2fv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_uniformMatrix4x2fv* __restrict params) {
		glUniformMatrix4x2fv(
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_uniformMatrix3x4fv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_uniformMatrix3x4fv* __restrict params) {
		glUniformMatrix3x4fv(
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_uniformMatrix4x3fv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_uniformMatrix4x3fv* __restrict params) {
		glUniformMatrix4x3fv(
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_colorMaski : public RenderCommandBase {
	GLuint index;
	GLboolean r;
	GLboolean g;
	GLboolean b;
	GLboolean a;

	static void APIENTRY execute(const RenderCommand_colorMaski* __restrict params) {
		glColorMaski(
			params->index,
			params->r,
			params->g,
			params->b,
			params->a
		);
	}
};
struct RenderCommand_getBooleani_v : public RenderCommandBase {
	GLenum target;
	GLuint index;
	GLboolean *data;

	static void APIENTRY execute(const RenderCommand_getBooleani_v* __restrict params) {
		glGetBooleani_v(
			params->target,
			params->index,
			params->data
		);
	}
};
struct RenderCommand_getIntegeri_v : public RenderCommandBase {
	GLenum target;
	GLuint index;
	GLint *data;

	static void APIENTRY execute(const RenderCommand_getIntegeri_v* __restrict params) {
		glGetIntegeri_v(
			params->target,
			params->index,
			params->data
		);
	}
};
struct RenderCommand_enablei : public RenderCommandBase {
	GLenum target;
	GLuint index;

	static void APIENTRY execute(const RenderCommand_enablei* __restrict params) {
		glEnablei(
			params->target,
			params->index
		);
	}
};
struct RenderCommand_disablei : public RenderCommandBase {
	GLenum target;
	GLuint index;

	static void APIENTRY execute(const RenderCommand_disablei* __restrict params) {
		glDisablei(
			params->target,
			params->index
		);
	}
};
struct RenderCommand_isEnabledi : public RenderCommandBase {
	GLenum target;
	GLuint index;

	static void APIENTRY execute(const RenderCommand_isEnabledi* __restrict params) {
		glIsEnabledi(
			params->target,
			params->index
		);
	}
};
struct RenderCommand_beginTransformFeedback : public RenderCommandBase {
	GLenum primitiveMode;

	static void APIENTRY execute(const RenderCommand_beginTransformFeedback* __restrict params) {
		glBeginTransformFeedback(
			params->primitiveMode
		);
	}
};
struct RenderCommand_endTransformFeedback : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_endTransformFeedback* __restrict params) {
		glEndTransformFeedback(
		);
	}
};
struct RenderCommand_bindBufferRange : public RenderCommandBase {
	GLenum target;
	GLuint index;
	GLuint buffer;
	GLintptr offset;
	GLsizeiptr size;

	static void APIENTRY execute(const RenderCommand_bindBufferRange* __restrict params) {
		glBindBufferRange(
			params->target,
			params->index,
			params->buffer,
			params->offset,
			params->size
		);
	}
};
struct RenderCommand_bindBufferBase : public RenderCommandBase {
	GLenum target;
	GLuint index;
	GLuint buffer;

	static void APIENTRY execute(const RenderCommand_bindBufferBase* __restrict params) {
		glBindBufferBase(
			params->target,
			params->index,
			params->buffer
		);
	}
};
struct RenderCommand_transformFeedbackVaryings : public RenderCommandBase {
	GLuint program;
	GLsizei count;
	const GLchar *const*varyings;
	GLenum bufferMode;

	static void APIENTRY execute(const RenderCommand_transformFeedbackVaryings* __restrict params) {
		glTransformFeedbackVaryings(
			params->program,
			params->count,
			params->varyings,
			params->bufferMode
		);
	}
};
struct RenderCommand_getTransformFeedbackVarying : public RenderCommandBase {
	GLuint program;
	GLuint index;
	GLsizei bufSize;
	GLsizei *length;
	GLsizei *size;
	GLenum *type;
	GLchar *name;

	static void APIENTRY execute(const RenderCommand_getTransformFeedbackVarying* __restrict params) {
		glGetTransformFeedbackVarying(
			params->program,
			params->index,
			params->bufSize,
			params->length,
			params->size,
			params->type,
			params->name
		);
	}
};
struct RenderCommand_clampColor : public RenderCommandBase {
	GLenum target;
	GLenum clamp;

	static void APIENTRY execute(const RenderCommand_clampColor* __restrict params) {
		glClampColor(
			params->target,
			params->clamp
		);
	}
};
struct RenderCommand_beginConditionalRender : public RenderCommandBase {
	GLuint id;
	GLenum mode;

	static void APIENTRY execute(const RenderCommand_beginConditionalRender* __restrict params) {
		glBeginConditionalRender(
			params->id,
			params->mode
		);
	}
};
struct RenderCommand_endConditionalRender : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_endConditionalRender* __restrict params) {
		glEndConditionalRender(
		);
	}
};
struct RenderCommand_vertexAttribIPointer : public RenderCommandBase {
	GLuint index;
	GLint size;
	GLenum type;
	GLsizei stride;
	const void *pointer;

	static void APIENTRY execute(const RenderCommand_vertexAttribIPointer* __restrict params) {
		glVertexAttribIPointer(
			params->index,
			params->size,
			params->type,
			params->stride,
			params->pointer
		);
	}
};
struct RenderCommand_getVertexAttribIiv : public RenderCommandBase {
	GLuint index;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getVertexAttribIiv* __restrict params) {
		glGetVertexAttribIiv(
			params->index,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getVertexAttribIuiv : public RenderCommandBase {
	GLuint index;
	GLenum pname;
	GLuint *params;

	static void APIENTRY execute(const RenderCommand_getVertexAttribIuiv* __restrict params) {
		glGetVertexAttribIuiv(
			params->index,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_vertexAttribI1i : public RenderCommandBase {
	GLuint index;
	GLint x;

	static void APIENTRY execute(const RenderCommand_vertexAttribI1i* __restrict params) {
		glVertexAttribI1i(
			params->index,
			params->x
		);
	}
};
struct RenderCommand_vertexAttribI2i : public RenderCommandBase {
	GLuint index;
	GLint x;
	GLint y;

	static void APIENTRY execute(const RenderCommand_vertexAttribI2i* __restrict params) {
		glVertexAttribI2i(
			params->index,
			params->x,
			params->y
		);
	}
};
struct RenderCommand_vertexAttribI3i : public RenderCommandBase {
	GLuint index;
	GLint x;
	GLint y;
	GLint z;

	static void APIENTRY execute(const RenderCommand_vertexAttribI3i* __restrict params) {
		glVertexAttribI3i(
			params->index,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_vertexAttribI4i : public RenderCommandBase {
	GLuint index;
	GLint x;
	GLint y;
	GLint z;
	GLint w;

	static void APIENTRY execute(const RenderCommand_vertexAttribI4i* __restrict params) {
		glVertexAttribI4i(
			params->index,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_vertexAttribI1ui : public RenderCommandBase {
	GLuint index;
	GLuint x;

	static void APIENTRY execute(const RenderCommand_vertexAttribI1ui* __restrict params) {
		glVertexAttribI1ui(
			params->index,
			params->x
		);
	}
};
struct RenderCommand_vertexAttribI2ui : public RenderCommandBase {
	GLuint index;
	GLuint x;
	GLuint y;

	static void APIENTRY execute(const RenderCommand_vertexAttribI2ui* __restrict params) {
		glVertexAttribI2ui(
			params->index,
			params->x,
			params->y
		);
	}
};
struct RenderCommand_vertexAttribI3ui : public RenderCommandBase {
	GLuint index;
	GLuint x;
	GLuint y;
	GLuint z;

	static void APIENTRY execute(const RenderCommand_vertexAttribI3ui* __restrict params) {
		glVertexAttribI3ui(
			params->index,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_vertexAttribI4ui : public RenderCommandBase {
	GLuint index;
	GLuint x;
	GLuint y;
	GLuint z;
	GLuint w;

	static void APIENTRY execute(const RenderCommand_vertexAttribI4ui* __restrict params) {
		glVertexAttribI4ui(
			params->index,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_vertexAttribI1iv : public RenderCommandBase {
	GLuint index;
	const GLint *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribI1iv* __restrict params) {
		glVertexAttribI1iv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribI2iv : public RenderCommandBase {
	GLuint index;
	const GLint *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribI2iv* __restrict params) {
		glVertexAttribI2iv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribI3iv : public RenderCommandBase {
	GLuint index;
	const GLint *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribI3iv* __restrict params) {
		glVertexAttribI3iv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribI4iv : public RenderCommandBase {
	GLuint index;
	const GLint *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribI4iv* __restrict params) {
		glVertexAttribI4iv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribI1uiv : public RenderCommandBase {
	GLuint index;
	const GLuint *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribI1uiv* __restrict params) {
		glVertexAttribI1uiv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribI2uiv : public RenderCommandBase {
	GLuint index;
	const GLuint *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribI2uiv* __restrict params) {
		glVertexAttribI2uiv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribI3uiv : public RenderCommandBase {
	GLuint index;
	const GLuint *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribI3uiv* __restrict params) {
		glVertexAttribI3uiv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribI4uiv : public RenderCommandBase {
	GLuint index;
	const GLuint *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribI4uiv* __restrict params) {
		glVertexAttribI4uiv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribI4bv : public RenderCommandBase {
	GLuint index;
	const GLbyte *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribI4bv* __restrict params) {
		glVertexAttribI4bv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribI4sv : public RenderCommandBase {
	GLuint index;
	const GLshort *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribI4sv* __restrict params) {
		glVertexAttribI4sv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribI4ubv : public RenderCommandBase {
	GLuint index;
	const GLubyte *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribI4ubv* __restrict params) {
		glVertexAttribI4ubv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribI4usv : public RenderCommandBase {
	GLuint index;
	const GLushort *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribI4usv* __restrict params) {
		glVertexAttribI4usv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_getUniformuiv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint *params;

	static void APIENTRY execute(const RenderCommand_getUniformuiv* __restrict params) {
		glGetUniformuiv(
			params->program,
			params->location,
			params->params
		);
	}
};
struct RenderCommand_bindFragDataLocation : public RenderCommandBase {
	GLuint program;
	GLuint color;
	const GLchar *name;

	static void APIENTRY execute(const RenderCommand_bindFragDataLocation* __restrict params) {
		glBindFragDataLocation(
			params->program,
			params->color,
			params->name
		);
	}
};
struct RenderCommand_getFragDataLocation : public RenderCommandBase {
	GLuint program;
	const GLchar *name;

	static void APIENTRY execute(const RenderCommand_getFragDataLocation* __restrict params) {
		glGetFragDataLocation(
			params->program,
			params->name
		);
	}
};
struct RenderCommand_uniform1ui : public RenderCommandBase {
	GLint location;
	GLuint v0;

	static void APIENTRY execute(const RenderCommand_uniform1ui* __restrict params) {
		glUniform1ui(
			params->location,
			params->v0
		);
	}
};
struct RenderCommand_uniform2ui : public RenderCommandBase {
	GLint location;
	GLuint v0;
	GLuint v1;

	static void APIENTRY execute(const RenderCommand_uniform2ui* __restrict params) {
		glUniform2ui(
			params->location,
			params->v0,
			params->v1
		);
	}
};
struct RenderCommand_uniform3ui : public RenderCommandBase {
	GLint location;
	GLuint v0;
	GLuint v1;
	GLuint v2;

	static void APIENTRY execute(const RenderCommand_uniform3ui* __restrict params) {
		glUniform3ui(
			params->location,
			params->v0,
			params->v1,
			params->v2
		);
	}
};
struct RenderCommand_uniform4ui : public RenderCommandBase {
	GLint location;
	GLuint v0;
	GLuint v1;
	GLuint v2;
	GLuint v3;

	static void APIENTRY execute(const RenderCommand_uniform4ui* __restrict params) {
		glUniform4ui(
			params->location,
			params->v0,
			params->v1,
			params->v2,
			params->v3
		);
	}
};
struct RenderCommand_uniform1uiv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLuint *value;

	static void APIENTRY execute(const RenderCommand_uniform1uiv* __restrict params) {
		glUniform1uiv(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform2uiv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLuint *value;

	static void APIENTRY execute(const RenderCommand_uniform2uiv* __restrict params) {
		glUniform2uiv(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform3uiv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLuint *value;

	static void APIENTRY execute(const RenderCommand_uniform3uiv* __restrict params) {
		glUniform3uiv(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform4uiv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLuint *value;

	static void APIENTRY execute(const RenderCommand_uniform4uiv* __restrict params) {
		glUniform4uiv(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_texParameterIiv : public RenderCommandBase {
	GLenum target;
	GLenum pname;
	const GLint *params;

	static void APIENTRY execute(const RenderCommand_texParameterIiv* __restrict params) {
		glTexParameterIiv(
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_texParameterIuiv : public RenderCommandBase {
	GLenum target;
	GLenum pname;
	const GLuint *params;

	static void APIENTRY execute(const RenderCommand_texParameterIuiv* __restrict params) {
		glTexParameterIuiv(
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getTexParameterIiv : public RenderCommandBase {
	GLenum target;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getTexParameterIiv* __restrict params) {
		glGetTexParameterIiv(
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getTexParameterIuiv : public RenderCommandBase {
	GLenum target;
	GLenum pname;
	GLuint *params;

	static void APIENTRY execute(const RenderCommand_getTexParameterIuiv* __restrict params) {
		glGetTexParameterIuiv(
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_clearBufferiv : public RenderCommandBase {
	GLenum buffer;
	GLint drawbuffer;
	const GLint *value;

	static void APIENTRY execute(const RenderCommand_clearBufferiv* __restrict params) {
		glClearBufferiv(
			params->buffer,
			params->drawbuffer,
			params->value
		);
	}
};
struct RenderCommand_clearBufferuiv : public RenderCommandBase {
	GLenum buffer;
	GLint drawbuffer;
	const GLuint *value;

	static void APIENTRY execute(const RenderCommand_clearBufferuiv* __restrict params) {
		glClearBufferuiv(
			params->buffer,
			params->drawbuffer,
			params->value
		);
	}
};
struct RenderCommand_clearBufferfv : public RenderCommandBase {
	GLenum buffer;
	GLint drawbuffer;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_clearBufferfv* __restrict params) {
		glClearBufferfv(
			params->buffer,
			params->drawbuffer,
			params->value
		);
	}
};
struct RenderCommand_clearBufferfi : public RenderCommandBase {
	GLenum buffer;
	GLint drawbuffer;
	GLfloat depth;
	GLint stencil;

	static void APIENTRY execute(const RenderCommand_clearBufferfi* __restrict params) {
		glClearBufferfi(
			params->buffer,
			params->drawbuffer,
			params->depth,
			params->stencil
		);
	}
};
struct RenderCommand_getStringi : public RenderCommandBase {
	GLenum name;
	GLuint index;

	static void APIENTRY execute(const RenderCommand_getStringi* __restrict params) {
		glGetStringi(
			params->name,
			params->index
		);
	}
};
struct RenderCommand_isRenderbuffer : public RenderCommandBase {
	GLuint renderbuffer;

	static void APIENTRY execute(const RenderCommand_isRenderbuffer* __restrict params) {
		glIsRenderbuffer(
			params->renderbuffer
		);
	}
};
struct RenderCommand_bindRenderbuffer : public RenderCommandBase {
	GLenum target;
	GLuint renderbuffer;

	static void APIENTRY execute(const RenderCommand_bindRenderbuffer* __restrict params) {
		glBindRenderbuffer(
			params->target,
			params->renderbuffer
		);
	}
};
struct RenderCommand_deleteRenderbuffers : public RenderCommandBase {
	GLsizei n;
	const GLuint *renderbuffers;

	static void APIENTRY execute(const RenderCommand_deleteRenderbuffers* __restrict params) {
		glDeleteRenderbuffers(
			params->n,
			params->renderbuffers
		);
	}
};
struct RenderCommand_genRenderbuffers : public RenderCommandBase {
	GLsizei n;
	GLuint *renderbuffers;

	static void APIENTRY execute(const RenderCommand_genRenderbuffers* __restrict params) {
		glGenRenderbuffers(
			params->n,
			params->renderbuffers
		);
	}
};
struct RenderCommand_renderbufferStorage : public RenderCommandBase {
	GLenum target;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_renderbufferStorage* __restrict params) {
		glRenderbufferStorage(
			params->target,
			params->internalformat,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_getRenderbufferParameteriv : public RenderCommandBase {
	GLenum target;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getRenderbufferParameteriv* __restrict params) {
		glGetRenderbufferParameteriv(
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_isFramebuffer : public RenderCommandBase {
	GLuint framebuffer;

	static void APIENTRY execute(const RenderCommand_isFramebuffer* __restrict params) {
		glIsFramebuffer(
			params->framebuffer
		);
	}
};
struct RenderCommand_bindFramebuffer : public RenderCommandBase {
	GLenum target;
	GLuint framebuffer;

	static void APIENTRY execute(const RenderCommand_bindFramebuffer* __restrict params) {
		glBindFramebuffer(
			params->target,
			params->framebuffer
		);
	}
};
struct RenderCommand_deleteFramebuffers : public RenderCommandBase {
	GLsizei n;
	const GLuint *framebuffers;

	static void APIENTRY execute(const RenderCommand_deleteFramebuffers* __restrict params) {
		glDeleteFramebuffers(
			params->n,
			params->framebuffers
		);
	}
};
struct RenderCommand_genFramebuffers : public RenderCommandBase {
	GLsizei n;
	GLuint *framebuffers;

	static void APIENTRY execute(const RenderCommand_genFramebuffers* __restrict params) {
		glGenFramebuffers(
			params->n,
			params->framebuffers
		);
	}
};
struct RenderCommand_checkFramebufferStatus : public RenderCommandBase {
	GLenum target;

	static void APIENTRY execute(const RenderCommand_checkFramebufferStatus* __restrict params) {
		glCheckFramebufferStatus(
			params->target
		);
	}
};
struct RenderCommand_framebufferTexture1D : public RenderCommandBase {
	GLenum target;
	GLenum attachment;
	GLenum textarget;
	GLuint texture;
	GLint level;

	static void APIENTRY execute(const RenderCommand_framebufferTexture1D* __restrict params) {
		glFramebufferTexture1D(
			params->target,
			params->attachment,
			params->textarget,
			params->texture,
			params->level
		);
	}
};
struct RenderCommand_framebufferTexture2D : public RenderCommandBase {
	GLenum target;
	GLenum attachment;
	GLenum textarget;
	GLuint texture;
	GLint level;

	static void APIENTRY execute(const RenderCommand_framebufferTexture2D* __restrict params) {
		glFramebufferTexture2D(
			params->target,
			params->attachment,
			params->textarget,
			params->texture,
			params->level
		);
	}
};
struct RenderCommand_framebufferTexture3D : public RenderCommandBase {
	GLenum target;
	GLenum attachment;
	GLenum textarget;
	GLuint texture;
	GLint level;
	GLint zoffset;

	static void APIENTRY execute(const RenderCommand_framebufferTexture3D* __restrict params) {
		glFramebufferTexture3D(
			params->target,
			params->attachment,
			params->textarget,
			params->texture,
			params->level,
			params->zoffset
		);
	}
};
struct RenderCommand_framebufferRenderbuffer : public RenderCommandBase {
	GLenum target;
	GLenum attachment;
	GLenum renderbuffertarget;
	GLuint renderbuffer;

	static void APIENTRY execute(const RenderCommand_framebufferRenderbuffer* __restrict params) {
		glFramebufferRenderbuffer(
			params->target,
			params->attachment,
			params->renderbuffertarget,
			params->renderbuffer
		);
	}
};
struct RenderCommand_getFramebufferAttachmentParameteriv : public RenderCommandBase {
	GLenum target;
	GLenum attachment;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getFramebufferAttachmentParameteriv* __restrict params) {
		glGetFramebufferAttachmentParameteriv(
			params->target,
			params->attachment,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_generateMipmap : public RenderCommandBase {
	GLenum target;

	static void APIENTRY execute(const RenderCommand_generateMipmap* __restrict params) {
		glGenerateMipmap(
			params->target
		);
	}
};
struct RenderCommand_blitFramebuffer : public RenderCommandBase {
	GLint srcX0;
	GLint srcY0;
	GLint srcX1;
	GLint srcY1;
	GLint dstX0;
	GLint dstY0;
	GLint dstX1;
	GLint dstY1;
	GLbitfield mask;
	GLenum filter;

	static void APIENTRY execute(const RenderCommand_blitFramebuffer* __restrict params) {
		glBlitFramebuffer(
			params->srcX0,
			params->srcY0,
			params->srcX1,
			params->srcY1,
			params->dstX0,
			params->dstY0,
			params->dstX1,
			params->dstY1,
			params->mask,
			params->filter
		);
	}
};
struct RenderCommand_renderbufferStorageMultisample : public RenderCommandBase {
	GLenum target;
	GLsizei samples;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_renderbufferStorageMultisample* __restrict params) {
		glRenderbufferStorageMultisample(
			params->target,
			params->samples,
			params->internalformat,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_framebufferTextureLayer : public RenderCommandBase {
	GLenum target;
	GLenum attachment;
	GLuint texture;
	GLint level;
	GLint layer;

	static void APIENTRY execute(const RenderCommand_framebufferTextureLayer* __restrict params) {
		glFramebufferTextureLayer(
			params->target,
			params->attachment,
			params->texture,
			params->level,
			params->layer
		);
	}
};
struct RenderCommand_mapBufferRange : public RenderCommandBase {
	GLenum target;
	GLintptr offset;
	GLsizeiptr length;
	GLbitfield access;

	static void APIENTRY execute(const RenderCommand_mapBufferRange* __restrict params) {
		glMapBufferRange(
			params->target,
			params->offset,
			params->length,
			params->access
		);
	}
};
struct RenderCommand_flushMappedBufferRange : public RenderCommandBase {
	GLenum target;
	GLintptr offset;
	GLsizeiptr length;

	static void APIENTRY execute(const RenderCommand_flushMappedBufferRange* __restrict params) {
		glFlushMappedBufferRange(
			params->target,
			params->offset,
			params->length
		);
	}
};
struct RenderCommand_bindVertexArray : public RenderCommandBase {
	GLuint array;

	static void APIENTRY execute(const RenderCommand_bindVertexArray* __restrict params) {
		glBindVertexArray(
			params->array
		);
	}
};
struct RenderCommand_deleteVertexArrays : public RenderCommandBase {
	GLsizei n;
	const GLuint *arrays;

	static void APIENTRY execute(const RenderCommand_deleteVertexArrays* __restrict params) {
		glDeleteVertexArrays(
			params->n,
			params->arrays
		);
	}
};
struct RenderCommand_genVertexArrays : public RenderCommandBase {
	GLsizei n;
	GLuint *arrays;

	static void APIENTRY execute(const RenderCommand_genVertexArrays* __restrict params) {
		glGenVertexArrays(
			params->n,
			params->arrays
		);
	}
};
struct RenderCommand_isVertexArray : public RenderCommandBase {
	GLuint array;

	static void APIENTRY execute(const RenderCommand_isVertexArray* __restrict params) {
		glIsVertexArray(
			params->array
		);
	}
};
struct RenderCommand_drawArraysInstanced : public RenderCommandBase {
	GLenum mode;
	GLint first;
	GLsizei count;
	GLsizei instancecount;

	static void APIENTRY execute(const RenderCommand_drawArraysInstanced* __restrict params) {
		glDrawArraysInstanced(
			params->mode,
			params->first,
			params->count,
			params->instancecount
		);
	}
};
struct RenderCommand_drawElementsInstanced : public RenderCommandBase {
	GLenum mode;
	GLsizei count;
	GLenum type;
	const void *indices;
	GLsizei instancecount;

	static void APIENTRY execute(const RenderCommand_drawElementsInstanced* __restrict params) {
		glDrawElementsInstanced(
			params->mode,
			params->count,
			params->type,
			params->indices,
			params->instancecount
		);
	}
};
struct RenderCommand_texBuffer : public RenderCommandBase {
	GLenum target;
	GLenum internalformat;
	GLuint buffer;

	static void APIENTRY execute(const RenderCommand_texBuffer* __restrict params) {
		glTexBuffer(
			params->target,
			params->internalformat,
			params->buffer
		);
	}
};
struct RenderCommand_primitiveRestartIndex : public RenderCommandBase {
	GLuint index;

	static void APIENTRY execute(const RenderCommand_primitiveRestartIndex* __restrict params) {
		glPrimitiveRestartIndex(
			params->index
		);
	}
};
struct RenderCommand_copyBufferSubData : public RenderCommandBase {
	GLenum readTarget;
	GLenum writeTarget;
	GLintptr readOffset;
	GLintptr writeOffset;
	GLsizeiptr size;

	static void APIENTRY execute(const RenderCommand_copyBufferSubData* __restrict params) {
		glCopyBufferSubData(
			params->readTarget,
			params->writeTarget,
			params->readOffset,
			params->writeOffset,
			params->size
		);
	}
};
struct RenderCommand_getUniformIndices : public RenderCommandBase {
	GLuint program;
	GLsizei uniformCount;
	const GLchar *const*uniformNames;
	GLuint *uniformIndices;

	static void APIENTRY execute(const RenderCommand_getUniformIndices* __restrict params) {
		glGetUniformIndices(
			params->program,
			params->uniformCount,
			params->uniformNames,
			params->uniformIndices
		);
	}
};
struct RenderCommand_getActiveUniformsiv : public RenderCommandBase {
	GLuint program;
	GLsizei uniformCount;
	const GLuint *uniformIndices;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getActiveUniformsiv* __restrict params) {
		glGetActiveUniformsiv(
			params->program,
			params->uniformCount,
			params->uniformIndices,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getActiveUniformName : public RenderCommandBase {
	GLuint program;
	GLuint uniformIndex;
	GLsizei bufSize;
	GLsizei *length;
	GLchar *uniformName;

	static void APIENTRY execute(const RenderCommand_getActiveUniformName* __restrict params) {
		glGetActiveUniformName(
			params->program,
			params->uniformIndex,
			params->bufSize,
			params->length,
			params->uniformName
		);
	}
};
struct RenderCommand_getUniformBlockIndex : public RenderCommandBase {
	GLuint program;
	const GLchar *uniformBlockName;

	static void APIENTRY execute(const RenderCommand_getUniformBlockIndex* __restrict params) {
		glGetUniformBlockIndex(
			params->program,
			params->uniformBlockName
		);
	}
};
struct RenderCommand_getActiveUniformBlockiv : public RenderCommandBase {
	GLuint program;
	GLuint uniformBlockIndex;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getActiveUniformBlockiv* __restrict params) {
		glGetActiveUniformBlockiv(
			params->program,
			params->uniformBlockIndex,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getActiveUniformBlockName : public RenderCommandBase {
	GLuint program;
	GLuint uniformBlockIndex;
	GLsizei bufSize;
	GLsizei *length;
	GLchar *uniformBlockName;

	static void APIENTRY execute(const RenderCommand_getActiveUniformBlockName* __restrict params) {
		glGetActiveUniformBlockName(
			params->program,
			params->uniformBlockIndex,
			params->bufSize,
			params->length,
			params->uniformBlockName
		);
	}
};
struct RenderCommand_uniformBlockBinding : public RenderCommandBase {
	GLuint program;
	GLuint uniformBlockIndex;
	GLuint uniformBlockBinding;

	static void APIENTRY execute(const RenderCommand_uniformBlockBinding* __restrict params) {
		glUniformBlockBinding(
			params->program,
			params->uniformBlockIndex,
			params->uniformBlockBinding
		);
	}
};
struct RenderCommand_drawElementsBaseVertex : public RenderCommandBase {
	GLenum mode;
	GLsizei count;
	GLenum type;
	const void *indices;
	GLint basevertex;

	static void APIENTRY execute(const RenderCommand_drawElementsBaseVertex* __restrict params) {
		glDrawElementsBaseVertex(
			params->mode,
			params->count,
			params->type,
			params->indices,
			params->basevertex
		);
	}
};
struct RenderCommand_drawRangeElementsBaseVertex : public RenderCommandBase {
	GLenum mode;
	GLuint start;
	GLuint end;
	GLsizei count;
	GLenum type;
	const void *indices;
	GLint basevertex;

	static void APIENTRY execute(const RenderCommand_drawRangeElementsBaseVertex* __restrict params) {
		glDrawRangeElementsBaseVertex(
			params->mode,
			params->start,
			params->end,
			params->count,
			params->type,
			params->indices,
			params->basevertex
		);
	}
};
struct RenderCommand_drawElementsInstancedBaseVertex : public RenderCommandBase {
	GLenum mode;
	GLsizei count;
	GLenum type;
	const void *indices;
	GLsizei instancecount;
	GLint basevertex;

	static void APIENTRY execute(const RenderCommand_drawElementsInstancedBaseVertex* __restrict params) {
		glDrawElementsInstancedBaseVertex(
			params->mode,
			params->count,
			params->type,
			params->indices,
			params->instancecount,
			params->basevertex
		);
	}
};
struct RenderCommand_multiDrawElementsBaseVertex : public RenderCommandBase {
	GLenum mode;
	const GLsizei *count;
	GLenum type;
	const void *const*indices;
	GLsizei drawcount;
	const GLint *basevertex;

	static void APIENTRY execute(const RenderCommand_multiDrawElementsBaseVertex* __restrict params) {
		glMultiDrawElementsBaseVertex(
			params->mode,
			params->count,
			params->type,
			params->indices,
			params->drawcount,
			params->basevertex
		);
	}
};
struct RenderCommand_provokingVertex : public RenderCommandBase {
	GLenum mode;

	static void APIENTRY execute(const RenderCommand_provokingVertex* __restrict params) {
		glProvokingVertex(
			params->mode
		);
	}
};
struct RenderCommand_fenceSync : public RenderCommandBase {
	GLenum condition;
	GLbitfield flags;

	static void APIENTRY execute(const RenderCommand_fenceSync* __restrict params) {
		glFenceSync(
			params->condition,
			params->flags
		);
	}
};
struct RenderCommand_isSync : public RenderCommandBase {
	GLsync sync;

	static void APIENTRY execute(const RenderCommand_isSync* __restrict params) {
		glIsSync(
			params->sync
		);
	}
};
struct RenderCommand_deleteSync : public RenderCommandBase {
	GLsync sync;

	static void APIENTRY execute(const RenderCommand_deleteSync* __restrict params) {
		glDeleteSync(
			params->sync
		);
	}
};
struct RenderCommand_clientWaitSync : public RenderCommandBase {
	GLsync sync;
	GLbitfield flags;
	GLuint64 timeout;

	static void APIENTRY execute(const RenderCommand_clientWaitSync* __restrict params) {
		glClientWaitSync(
			params->sync,
			params->flags,
			params->timeout
		);
	}
};
struct RenderCommand_waitSync : public RenderCommandBase {
	GLsync sync;
	GLbitfield flags;
	GLuint64 timeout;

	static void APIENTRY execute(const RenderCommand_waitSync* __restrict params) {
		glWaitSync(
			params->sync,
			params->flags,
			params->timeout
		);
	}
};
struct RenderCommand_getInteger64v : public RenderCommandBase {
	GLenum pname;
	GLint64 *data;

	static void APIENTRY execute(const RenderCommand_getInteger64v* __restrict params) {
		glGetInteger64v(
			params->pname,
			params->data
		);
	}
};
struct RenderCommand_getSynciv : public RenderCommandBase {
	GLsync sync;
	GLenum pname;
	GLsizei bufSize;
	GLsizei *length;
	GLint *values;

	static void APIENTRY execute(const RenderCommand_getSynciv* __restrict params) {
		glGetSynciv(
			params->sync,
			params->pname,
			params->bufSize,
			params->length,
			params->values
		);
	}
};
struct RenderCommand_getInteger64i_v : public RenderCommandBase {
	GLenum target;
	GLuint index;
	GLint64 *data;

	static void APIENTRY execute(const RenderCommand_getInteger64i_v* __restrict params) {
		glGetInteger64i_v(
			params->target,
			params->index,
			params->data
		);
	}
};
struct RenderCommand_getBufferParameteri64v : public RenderCommandBase {
	GLenum target;
	GLenum pname;
	GLint64 *params;

	static void APIENTRY execute(const RenderCommand_getBufferParameteri64v* __restrict params) {
		glGetBufferParameteri64v(
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_framebufferTexture : public RenderCommandBase {
	GLenum target;
	GLenum attachment;
	GLuint texture;
	GLint level;

	static void APIENTRY execute(const RenderCommand_framebufferTexture* __restrict params) {
		glFramebufferTexture(
			params->target,
			params->attachment,
			params->texture,
			params->level
		);
	}
};
struct RenderCommand_texImage2DMultisample : public RenderCommandBase {
	GLenum target;
	GLsizei samples;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;
	GLboolean fixedsamplelocations;

	static void APIENTRY execute(const RenderCommand_texImage2DMultisample* __restrict params) {
		glTexImage2DMultisample(
			params->target,
			params->samples,
			params->internalformat,
			params->width,
			params->height,
			params->fixedsamplelocations
		);
	}
};
struct RenderCommand_texImage3DMultisample : public RenderCommandBase {
	GLenum target;
	GLsizei samples;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLboolean fixedsamplelocations;

	static void APIENTRY execute(const RenderCommand_texImage3DMultisample* __restrict params) {
		glTexImage3DMultisample(
			params->target,
			params->samples,
			params->internalformat,
			params->width,
			params->height,
			params->depth,
			params->fixedsamplelocations
		);
	}
};
struct RenderCommand_getMultisamplefv : public RenderCommandBase {
	GLenum pname;
	GLuint index;
	GLfloat *val;

	static void APIENTRY execute(const RenderCommand_getMultisamplefv* __restrict params) {
		glGetMultisamplefv(
			params->pname,
			params->index,
			params->val
		);
	}
};
struct RenderCommand_sampleMaski : public RenderCommandBase {
	GLuint maskNumber;
	GLbitfield mask;

	static void APIENTRY execute(const RenderCommand_sampleMaski* __restrict params) {
		glSampleMaski(
			params->maskNumber,
			params->mask
		);
	}
};
struct RenderCommand_bindFragDataLocationIndexed : public RenderCommandBase {
	GLuint program;
	GLuint colorNumber;
	GLuint index;
	const GLchar *name;

	static void APIENTRY execute(const RenderCommand_bindFragDataLocationIndexed* __restrict params) {
		glBindFragDataLocationIndexed(
			params->program,
			params->colorNumber,
			params->index,
			params->name
		);
	}
};
struct RenderCommand_getFragDataIndex : public RenderCommandBase {
	GLuint program;
	const GLchar *name;

	static void APIENTRY execute(const RenderCommand_getFragDataIndex* __restrict params) {
		glGetFragDataIndex(
			params->program,
			params->name
		);
	}
};
struct RenderCommand_genSamplers : public RenderCommandBase {
	GLsizei count;
	GLuint *samplers;

	static void APIENTRY execute(const RenderCommand_genSamplers* __restrict params) {
		glGenSamplers(
			params->count,
			params->samplers
		);
	}
};
struct RenderCommand_deleteSamplers : public RenderCommandBase {
	GLsizei count;
	const GLuint *samplers;

	static void APIENTRY execute(const RenderCommand_deleteSamplers* __restrict params) {
		glDeleteSamplers(
			params->count,
			params->samplers
		);
	}
};
struct RenderCommand_isSampler : public RenderCommandBase {
	GLuint sampler;

	static void APIENTRY execute(const RenderCommand_isSampler* __restrict params) {
		glIsSampler(
			params->sampler
		);
	}
};
struct RenderCommand_bindSampler : public RenderCommandBase {
	GLuint unit;
	GLuint sampler;

	static void APIENTRY execute(const RenderCommand_bindSampler* __restrict params) {
		glBindSampler(
			params->unit,
			params->sampler
		);
	}
};
struct RenderCommand_samplerParameteri : public RenderCommandBase {
	GLuint sampler;
	GLenum pname;
	GLint param;

	static void APIENTRY execute(const RenderCommand_samplerParameteri* __restrict params) {
		glSamplerParameteri(
			params->sampler,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_samplerParameteriv : public RenderCommandBase {
	GLuint sampler;
	GLenum pname;
	const GLint *param;

	static void APIENTRY execute(const RenderCommand_samplerParameteriv* __restrict params) {
		glSamplerParameteriv(
			params->sampler,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_samplerParameterf : public RenderCommandBase {
	GLuint sampler;
	GLenum pname;
	GLfloat param;

	static void APIENTRY execute(const RenderCommand_samplerParameterf* __restrict params) {
		glSamplerParameterf(
			params->sampler,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_samplerParameterfv : public RenderCommandBase {
	GLuint sampler;
	GLenum pname;
	const GLfloat *param;

	static void APIENTRY execute(const RenderCommand_samplerParameterfv* __restrict params) {
		glSamplerParameterfv(
			params->sampler,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_samplerParameterIiv : public RenderCommandBase {
	GLuint sampler;
	GLenum pname;
	const GLint *param;

	static void APIENTRY execute(const RenderCommand_samplerParameterIiv* __restrict params) {
		glSamplerParameterIiv(
			params->sampler,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_samplerParameterIuiv : public RenderCommandBase {
	GLuint sampler;
	GLenum pname;
	const GLuint *param;

	static void APIENTRY execute(const RenderCommand_samplerParameterIuiv* __restrict params) {
		glSamplerParameterIuiv(
			params->sampler,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_getSamplerParameteriv : public RenderCommandBase {
	GLuint sampler;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getSamplerParameteriv* __restrict params) {
		glGetSamplerParameteriv(
			params->sampler,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getSamplerParameterIiv : public RenderCommandBase {
	GLuint sampler;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getSamplerParameterIiv* __restrict params) {
		glGetSamplerParameterIiv(
			params->sampler,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getSamplerParameterfv : public RenderCommandBase {
	GLuint sampler;
	GLenum pname;
	GLfloat *params;

	static void APIENTRY execute(const RenderCommand_getSamplerParameterfv* __restrict params) {
		glGetSamplerParameterfv(
			params->sampler,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getSamplerParameterIuiv : public RenderCommandBase {
	GLuint sampler;
	GLenum pname;
	GLuint *params;

	static void APIENTRY execute(const RenderCommand_getSamplerParameterIuiv* __restrict params) {
		glGetSamplerParameterIuiv(
			params->sampler,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_queryCounter : public RenderCommandBase {
	GLuint id;
	GLenum target;

	static void APIENTRY execute(const RenderCommand_queryCounter* __restrict params) {
		glQueryCounter(
			params->id,
			params->target
		);
	}
};
struct RenderCommand_getQueryObjecti64v : public RenderCommandBase {
	GLuint id;
	GLenum pname;
	GLint64 *params;

	static void APIENTRY execute(const RenderCommand_getQueryObjecti64v* __restrict params) {
		glGetQueryObjecti64v(
			params->id,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getQueryObjectui64v : public RenderCommandBase {
	GLuint id;
	GLenum pname;
	GLuint64 *params;

	static void APIENTRY execute(const RenderCommand_getQueryObjectui64v* __restrict params) {
		glGetQueryObjectui64v(
			params->id,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_vertexAttribDivisor : public RenderCommandBase {
	GLuint index;
	GLuint divisor;

	static void APIENTRY execute(const RenderCommand_vertexAttribDivisor* __restrict params) {
		glVertexAttribDivisor(
			params->index,
			params->divisor
		);
	}
};
struct RenderCommand_vertexAttribP1ui : public RenderCommandBase {
	GLuint index;
	GLenum type;
	GLboolean normalized;
	GLuint value;

	static void APIENTRY execute(const RenderCommand_vertexAttribP1ui* __restrict params) {
		glVertexAttribP1ui(
			params->index,
			params->type,
			params->normalized,
			params->value
		);
	}
};
struct RenderCommand_vertexAttribP1uiv : public RenderCommandBase {
	GLuint index;
	GLenum type;
	GLboolean normalized;
	const GLuint *value;

	static void APIENTRY execute(const RenderCommand_vertexAttribP1uiv* __restrict params) {
		glVertexAttribP1uiv(
			params->index,
			params->type,
			params->normalized,
			params->value
		);
	}
};
struct RenderCommand_vertexAttribP2ui : public RenderCommandBase {
	GLuint index;
	GLenum type;
	GLboolean normalized;
	GLuint value;

	static void APIENTRY execute(const RenderCommand_vertexAttribP2ui* __restrict params) {
		glVertexAttribP2ui(
			params->index,
			params->type,
			params->normalized,
			params->value
		);
	}
};
struct RenderCommand_vertexAttribP2uiv : public RenderCommandBase {
	GLuint index;
	GLenum type;
	GLboolean normalized;
	const GLuint *value;

	static void APIENTRY execute(const RenderCommand_vertexAttribP2uiv* __restrict params) {
		glVertexAttribP2uiv(
			params->index,
			params->type,
			params->normalized,
			params->value
		);
	}
};
struct RenderCommand_vertexAttribP3ui : public RenderCommandBase {
	GLuint index;
	GLenum type;
	GLboolean normalized;
	GLuint value;

	static void APIENTRY execute(const RenderCommand_vertexAttribP3ui* __restrict params) {
		glVertexAttribP3ui(
			params->index,
			params->type,
			params->normalized,
			params->value
		);
	}
};
struct RenderCommand_vertexAttribP3uiv : public RenderCommandBase {
	GLuint index;
	GLenum type;
	GLboolean normalized;
	const GLuint *value;

	static void APIENTRY execute(const RenderCommand_vertexAttribP3uiv* __restrict params) {
		glVertexAttribP3uiv(
			params->index,
			params->type,
			params->normalized,
			params->value
		);
	}
};
struct RenderCommand_vertexAttribP4ui : public RenderCommandBase {
	GLuint index;
	GLenum type;
	GLboolean normalized;
	GLuint value;

	static void APIENTRY execute(const RenderCommand_vertexAttribP4ui* __restrict params) {
		glVertexAttribP4ui(
			params->index,
			params->type,
			params->normalized,
			params->value
		);
	}
};
struct RenderCommand_vertexAttribP4uiv : public RenderCommandBase {
	GLuint index;
	GLenum type;
	GLboolean normalized;
	const GLuint *value;

	static void APIENTRY execute(const RenderCommand_vertexAttribP4uiv* __restrict params) {
		glVertexAttribP4uiv(
			params->index,
			params->type,
			params->normalized,
			params->value
		);
	}
};
struct RenderCommand_minSampleShading : public RenderCommandBase {
	GLfloat value;

	static void APIENTRY execute(const RenderCommand_minSampleShading* __restrict params) {
		glMinSampleShading(
			params->value
		);
	}
};
struct RenderCommand_blendEquationi : public RenderCommandBase {
	GLuint buf;
	GLenum mode;

	static void APIENTRY execute(const RenderCommand_blendEquationi* __restrict params) {
		glBlendEquationi(
			params->buf,
			params->mode
		);
	}
};
struct RenderCommand_blendEquationSeparatei : public RenderCommandBase {
	GLuint buf;
	GLenum modeRGB;
	GLenum modeAlpha;

	static void APIENTRY execute(const RenderCommand_blendEquationSeparatei* __restrict params) {
		glBlendEquationSeparatei(
			params->buf,
			params->modeRGB,
			params->modeAlpha
		);
	}
};
struct RenderCommand_blendFunci : public RenderCommandBase {
	GLuint buf;
	GLenum src;
	GLenum dst;

	static void APIENTRY execute(const RenderCommand_blendFunci* __restrict params) {
		glBlendFunci(
			params->buf,
			params->src,
			params->dst
		);
	}
};
struct RenderCommand_blendFuncSeparatei : public RenderCommandBase {
	GLuint buf;
	GLenum srcRGB;
	GLenum dstRGB;
	GLenum srcAlpha;
	GLenum dstAlpha;

	static void APIENTRY execute(const RenderCommand_blendFuncSeparatei* __restrict params) {
		glBlendFuncSeparatei(
			params->buf,
			params->srcRGB,
			params->dstRGB,
			params->srcAlpha,
			params->dstAlpha
		);
	}
};
struct RenderCommand_drawArraysIndirect : public RenderCommandBase {
	GLenum mode;
	const void *indirect;

	static void APIENTRY execute(const RenderCommand_drawArraysIndirect* __restrict params) {
		glDrawArraysIndirect(
			params->mode,
			params->indirect
		);
	}
};
struct RenderCommand_drawElementsIndirect : public RenderCommandBase {
	GLenum mode;
	GLenum type;
	const void *indirect;

	static void APIENTRY execute(const RenderCommand_drawElementsIndirect* __restrict params) {
		glDrawElementsIndirect(
			params->mode,
			params->type,
			params->indirect
		);
	}
};
struct RenderCommand_uniform1d : public RenderCommandBase {
	GLint location;
	GLdouble x;

	static void APIENTRY execute(const RenderCommand_uniform1d* __restrict params) {
		glUniform1d(
			params->location,
			params->x
		);
	}
};
struct RenderCommand_uniform2d : public RenderCommandBase {
	GLint location;
	GLdouble x;
	GLdouble y;

	static void APIENTRY execute(const RenderCommand_uniform2d* __restrict params) {
		glUniform2d(
			params->location,
			params->x,
			params->y
		);
	}
};
struct RenderCommand_uniform3d : public RenderCommandBase {
	GLint location;
	GLdouble x;
	GLdouble y;
	GLdouble z;

	static void APIENTRY execute(const RenderCommand_uniform3d* __restrict params) {
		glUniform3d(
			params->location,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_uniform4d : public RenderCommandBase {
	GLint location;
	GLdouble x;
	GLdouble y;
	GLdouble z;
	GLdouble w;

	static void APIENTRY execute(const RenderCommand_uniform4d* __restrict params) {
		glUniform4d(
			params->location,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_uniform1dv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_uniform1dv* __restrict params) {
		glUniform1dv(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform2dv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_uniform2dv* __restrict params) {
		glUniform2dv(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform3dv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_uniform3dv* __restrict params) {
		glUniform3dv(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform4dv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_uniform4dv* __restrict params) {
		glUniform4dv(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniformMatrix2dv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_uniformMatrix2dv* __restrict params) {
		glUniformMatrix2dv(
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_uniformMatrix3dv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_uniformMatrix3dv* __restrict params) {
		glUniformMatrix3dv(
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_uniformMatrix4dv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_uniformMatrix4dv* __restrict params) {
		glUniformMatrix4dv(
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_uniformMatrix2x3dv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_uniformMatrix2x3dv* __restrict params) {
		glUniformMatrix2x3dv(
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_uniformMatrix2x4dv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_uniformMatrix2x4dv* __restrict params) {
		glUniformMatrix2x4dv(
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_uniformMatrix3x2dv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_uniformMatrix3x2dv* __restrict params) {
		glUniformMatrix3x2dv(
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_uniformMatrix3x4dv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_uniformMatrix3x4dv* __restrict params) {
		glUniformMatrix3x4dv(
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_uniformMatrix4x2dv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_uniformMatrix4x2dv* __restrict params) {
		glUniformMatrix4x2dv(
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_uniformMatrix4x3dv : public RenderCommandBase {
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_uniformMatrix4x3dv* __restrict params) {
		glUniformMatrix4x3dv(
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_getUniformdv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLdouble *params;

	static void APIENTRY execute(const RenderCommand_getUniformdv* __restrict params) {
		glGetUniformdv(
			params->program,
			params->location,
			params->params
		);
	}
};
struct RenderCommand_getSubroutineUniformLocation : public RenderCommandBase {
	GLuint program;
	GLenum shadertype;
	const GLchar *name;

	static void APIENTRY execute(const RenderCommand_getSubroutineUniformLocation* __restrict params) {
		glGetSubroutineUniformLocation(
			params->program,
			params->shadertype,
			params->name
		);
	}
};
struct RenderCommand_getSubroutineIndex : public RenderCommandBase {
	GLuint program;
	GLenum shadertype;
	const GLchar *name;

	static void APIENTRY execute(const RenderCommand_getSubroutineIndex* __restrict params) {
		glGetSubroutineIndex(
			params->program,
			params->shadertype,
			params->name
		);
	}
};
struct RenderCommand_getActiveSubroutineUniformiv : public RenderCommandBase {
	GLuint program;
	GLenum shadertype;
	GLuint index;
	GLenum pname;
	GLint *values;

	static void APIENTRY execute(const RenderCommand_getActiveSubroutineUniformiv* __restrict params) {
		glGetActiveSubroutineUniformiv(
			params->program,
			params->shadertype,
			params->index,
			params->pname,
			params->values
		);
	}
};
struct RenderCommand_getActiveSubroutineUniformName : public RenderCommandBase {
	GLuint program;
	GLenum shadertype;
	GLuint index;
	GLsizei bufsize;
	GLsizei *length;
	GLchar *name;

	static void APIENTRY execute(const RenderCommand_getActiveSubroutineUniformName* __restrict params) {
		glGetActiveSubroutineUniformName(
			params->program,
			params->shadertype,
			params->index,
			params->bufsize,
			params->length,
			params->name
		);
	}
};
struct RenderCommand_getActiveSubroutineName : public RenderCommandBase {
	GLuint program;
	GLenum shadertype;
	GLuint index;
	GLsizei bufsize;
	GLsizei *length;
	GLchar *name;

	static void APIENTRY execute(const RenderCommand_getActiveSubroutineName* __restrict params) {
		glGetActiveSubroutineName(
			params->program,
			params->shadertype,
			params->index,
			params->bufsize,
			params->length,
			params->name
		);
	}
};
struct RenderCommand_uniformSubroutinesuiv : public RenderCommandBase {
	GLenum shadertype;
	GLsizei count;
	const GLuint *indices;

	static void APIENTRY execute(const RenderCommand_uniformSubroutinesuiv* __restrict params) {
		glUniformSubroutinesuiv(
			params->shadertype,
			params->count,
			params->indices
		);
	}
};
struct RenderCommand_getUniformSubroutineuiv : public RenderCommandBase {
	GLenum shadertype;
	GLint location;
	GLuint *params;

	static void APIENTRY execute(const RenderCommand_getUniformSubroutineuiv* __restrict params) {
		glGetUniformSubroutineuiv(
			params->shadertype,
			params->location,
			params->params
		);
	}
};
struct RenderCommand_getProgramStageiv : public RenderCommandBase {
	GLuint program;
	GLenum shadertype;
	GLenum pname;
	GLint *values;

	static void APIENTRY execute(const RenderCommand_getProgramStageiv* __restrict params) {
		glGetProgramStageiv(
			params->program,
			params->shadertype,
			params->pname,
			params->values
		);
	}
};
struct RenderCommand_patchParameteri : public RenderCommandBase {
	GLenum pname;
	GLint value;

	static void APIENTRY execute(const RenderCommand_patchParameteri* __restrict params) {
		glPatchParameteri(
			params->pname,
			params->value
		);
	}
};
struct RenderCommand_patchParameterfv : public RenderCommandBase {
	GLenum pname;
	const GLfloat *values;

	static void APIENTRY execute(const RenderCommand_patchParameterfv* __restrict params) {
		glPatchParameterfv(
			params->pname,
			params->values
		);
	}
};
struct RenderCommand_bindTransformFeedback : public RenderCommandBase {
	GLenum target;
	GLuint id;

	static void APIENTRY execute(const RenderCommand_bindTransformFeedback* __restrict params) {
		glBindTransformFeedback(
			params->target,
			params->id
		);
	}
};
struct RenderCommand_deleteTransformFeedbacks : public RenderCommandBase {
	GLsizei n;
	const GLuint *ids;

	static void APIENTRY execute(const RenderCommand_deleteTransformFeedbacks* __restrict params) {
		glDeleteTransformFeedbacks(
			params->n,
			params->ids
		);
	}
};
struct RenderCommand_genTransformFeedbacks : public RenderCommandBase {
	GLsizei n;
	GLuint *ids;

	static void APIENTRY execute(const RenderCommand_genTransformFeedbacks* __restrict params) {
		glGenTransformFeedbacks(
			params->n,
			params->ids
		);
	}
};
struct RenderCommand_isTransformFeedback : public RenderCommandBase {
	GLuint id;

	static void APIENTRY execute(const RenderCommand_isTransformFeedback* __restrict params) {
		glIsTransformFeedback(
			params->id
		);
	}
};
struct RenderCommand_pauseTransformFeedback : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_pauseTransformFeedback* __restrict params) {
		glPauseTransformFeedback(
		);
	}
};
struct RenderCommand_resumeTransformFeedback : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_resumeTransformFeedback* __restrict params) {
		glResumeTransformFeedback(
		);
	}
};
struct RenderCommand_drawTransformFeedback : public RenderCommandBase {
	GLenum mode;
	GLuint id;

	static void APIENTRY execute(const RenderCommand_drawTransformFeedback* __restrict params) {
		glDrawTransformFeedback(
			params->mode,
			params->id
		);
	}
};
struct RenderCommand_drawTransformFeedbackStream : public RenderCommandBase {
	GLenum mode;
	GLuint id;
	GLuint stream;

	static void APIENTRY execute(const RenderCommand_drawTransformFeedbackStream* __restrict params) {
		glDrawTransformFeedbackStream(
			params->mode,
			params->id,
			params->stream
		);
	}
};
struct RenderCommand_beginQueryIndexed : public RenderCommandBase {
	GLenum target;
	GLuint index;
	GLuint id;

	static void APIENTRY execute(const RenderCommand_beginQueryIndexed* __restrict params) {
		glBeginQueryIndexed(
			params->target,
			params->index,
			params->id
		);
	}
};
struct RenderCommand_endQueryIndexed : public RenderCommandBase {
	GLenum target;
	GLuint index;

	static void APIENTRY execute(const RenderCommand_endQueryIndexed* __restrict params) {
		glEndQueryIndexed(
			params->target,
			params->index
		);
	}
};
struct RenderCommand_getQueryIndexediv : public RenderCommandBase {
	GLenum target;
	GLuint index;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getQueryIndexediv* __restrict params) {
		glGetQueryIndexediv(
			params->target,
			params->index,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_releaseShaderCompiler : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_releaseShaderCompiler* __restrict params) {
		glReleaseShaderCompiler(
		);
	}
};
struct RenderCommand_shaderBinary : public RenderCommandBase {
	GLsizei count;
	const GLuint *shaders;
	GLenum binaryformat;
	const void *binary;
	GLsizei length;

	static void APIENTRY execute(const RenderCommand_shaderBinary* __restrict params) {
		glShaderBinary(
			params->count,
			params->shaders,
			params->binaryformat,
			params->binary,
			params->length
		);
	}
};
struct RenderCommand_getShaderPrecisionFormat : public RenderCommandBase {
	GLenum shadertype;
	GLenum precisiontype;
	GLint *range;
	GLint *precision;

	static void APIENTRY execute(const RenderCommand_getShaderPrecisionFormat* __restrict params) {
		glGetShaderPrecisionFormat(
			params->shadertype,
			params->precisiontype,
			params->range,
			params->precision
		);
	}
};
struct RenderCommand_depthRangef : public RenderCommandBase {
	GLfloat n;
	GLfloat f;

	static void APIENTRY execute(const RenderCommand_depthRangef* __restrict params) {
		glDepthRangef(
			params->n,
			params->f
		);
	}
};
struct RenderCommand_clearDepthf : public RenderCommandBase {
	GLfloat d;

	static void APIENTRY execute(const RenderCommand_clearDepthf* __restrict params) {
		glClearDepthf(
			params->d
		);
	}
};
struct RenderCommand_getProgramBinary : public RenderCommandBase {
	GLuint program;
	GLsizei bufSize;
	GLsizei *length;
	GLenum *binaryFormat;
	void *binary;

	static void APIENTRY execute(const RenderCommand_getProgramBinary* __restrict params) {
		glGetProgramBinary(
			params->program,
			params->bufSize,
			params->length,
			params->binaryFormat,
			params->binary
		);
	}
};
struct RenderCommand_programBinary : public RenderCommandBase {
	GLuint program;
	GLenum binaryFormat;
	const void *binary;
	GLsizei length;

	static void APIENTRY execute(const RenderCommand_programBinary* __restrict params) {
		glProgramBinary(
			params->program,
			params->binaryFormat,
			params->binary,
			params->length
		);
	}
};
struct RenderCommand_programParameteri : public RenderCommandBase {
	GLuint program;
	GLenum pname;
	GLint value;

	static void APIENTRY execute(const RenderCommand_programParameteri* __restrict params) {
		glProgramParameteri(
			params->program,
			params->pname,
			params->value
		);
	}
};
struct RenderCommand_useProgramStages : public RenderCommandBase {
	GLuint pipeline;
	GLbitfield stages;
	GLuint program;

	static void APIENTRY execute(const RenderCommand_useProgramStages* __restrict params) {
		glUseProgramStages(
			params->pipeline,
			params->stages,
			params->program
		);
	}
};
struct RenderCommand_activeShaderProgram : public RenderCommandBase {
	GLuint pipeline;
	GLuint program;

	static void APIENTRY execute(const RenderCommand_activeShaderProgram* __restrict params) {
		glActiveShaderProgram(
			params->pipeline,
			params->program
		);
	}
};
struct RenderCommand_createShaderProgramv : public RenderCommandBase {
	GLenum type;
	GLsizei count;
	const GLchar *const*strings;

	static void APIENTRY execute(const RenderCommand_createShaderProgramv* __restrict params) {
		glCreateShaderProgramv(
			params->type,
			params->count,
			params->strings
		);
	}
};
struct RenderCommand_bindProgramPipeline : public RenderCommandBase {
	GLuint pipeline;

	static void APIENTRY execute(const RenderCommand_bindProgramPipeline* __restrict params) {
		glBindProgramPipeline(
			params->pipeline
		);
	}
};
struct RenderCommand_deleteProgramPipelines : public RenderCommandBase {
	GLsizei n;
	const GLuint *pipelines;

	static void APIENTRY execute(const RenderCommand_deleteProgramPipelines* __restrict params) {
		glDeleteProgramPipelines(
			params->n,
			params->pipelines
		);
	}
};
struct RenderCommand_genProgramPipelines : public RenderCommandBase {
	GLsizei n;
	GLuint *pipelines;

	static void APIENTRY execute(const RenderCommand_genProgramPipelines* __restrict params) {
		glGenProgramPipelines(
			params->n,
			params->pipelines
		);
	}
};
struct RenderCommand_isProgramPipeline : public RenderCommandBase {
	GLuint pipeline;

	static void APIENTRY execute(const RenderCommand_isProgramPipeline* __restrict params) {
		glIsProgramPipeline(
			params->pipeline
		);
	}
};
struct RenderCommand_getProgramPipelineiv : public RenderCommandBase {
	GLuint pipeline;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getProgramPipelineiv* __restrict params) {
		glGetProgramPipelineiv(
			params->pipeline,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_programUniform1i : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLint v0;

	static void APIENTRY execute(const RenderCommand_programUniform1i* __restrict params) {
		glProgramUniform1i(
			params->program,
			params->location,
			params->v0
		);
	}
};
struct RenderCommand_programUniform1iv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLint *value;

	static void APIENTRY execute(const RenderCommand_programUniform1iv* __restrict params) {
		glProgramUniform1iv(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform1f : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLfloat v0;

	static void APIENTRY execute(const RenderCommand_programUniform1f* __restrict params) {
		glProgramUniform1f(
			params->program,
			params->location,
			params->v0
		);
	}
};
struct RenderCommand_programUniform1fv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniform1fv* __restrict params) {
		glProgramUniform1fv(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform1d : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLdouble v0;

	static void APIENTRY execute(const RenderCommand_programUniform1d* __restrict params) {
		glProgramUniform1d(
			params->program,
			params->location,
			params->v0
		);
	}
};
struct RenderCommand_programUniform1dv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniform1dv* __restrict params) {
		glProgramUniform1dv(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform1ui : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint v0;

	static void APIENTRY execute(const RenderCommand_programUniform1ui* __restrict params) {
		glProgramUniform1ui(
			params->program,
			params->location,
			params->v0
		);
	}
};
struct RenderCommand_programUniform1uiv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLuint *value;

	static void APIENTRY execute(const RenderCommand_programUniform1uiv* __restrict params) {
		glProgramUniform1uiv(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform2i : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLint v0;
	GLint v1;

	static void APIENTRY execute(const RenderCommand_programUniform2i* __restrict params) {
		glProgramUniform2i(
			params->program,
			params->location,
			params->v0,
			params->v1
		);
	}
};
struct RenderCommand_programUniform2iv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLint *value;

	static void APIENTRY execute(const RenderCommand_programUniform2iv* __restrict params) {
		glProgramUniform2iv(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform2f : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLfloat v0;
	GLfloat v1;

	static void APIENTRY execute(const RenderCommand_programUniform2f* __restrict params) {
		glProgramUniform2f(
			params->program,
			params->location,
			params->v0,
			params->v1
		);
	}
};
struct RenderCommand_programUniform2fv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniform2fv* __restrict params) {
		glProgramUniform2fv(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform2d : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLdouble v0;
	GLdouble v1;

	static void APIENTRY execute(const RenderCommand_programUniform2d* __restrict params) {
		glProgramUniform2d(
			params->program,
			params->location,
			params->v0,
			params->v1
		);
	}
};
struct RenderCommand_programUniform2dv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniform2dv* __restrict params) {
		glProgramUniform2dv(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform2ui : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint v0;
	GLuint v1;

	static void APIENTRY execute(const RenderCommand_programUniform2ui* __restrict params) {
		glProgramUniform2ui(
			params->program,
			params->location,
			params->v0,
			params->v1
		);
	}
};
struct RenderCommand_programUniform2uiv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLuint *value;

	static void APIENTRY execute(const RenderCommand_programUniform2uiv* __restrict params) {
		glProgramUniform2uiv(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform3i : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLint v0;
	GLint v1;
	GLint v2;

	static void APIENTRY execute(const RenderCommand_programUniform3i* __restrict params) {
		glProgramUniform3i(
			params->program,
			params->location,
			params->v0,
			params->v1,
			params->v2
		);
	}
};
struct RenderCommand_programUniform3iv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLint *value;

	static void APIENTRY execute(const RenderCommand_programUniform3iv* __restrict params) {
		glProgramUniform3iv(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform3f : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLfloat v0;
	GLfloat v1;
	GLfloat v2;

	static void APIENTRY execute(const RenderCommand_programUniform3f* __restrict params) {
		glProgramUniform3f(
			params->program,
			params->location,
			params->v0,
			params->v1,
			params->v2
		);
	}
};
struct RenderCommand_programUniform3fv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniform3fv* __restrict params) {
		glProgramUniform3fv(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform3d : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLdouble v0;
	GLdouble v1;
	GLdouble v2;

	static void APIENTRY execute(const RenderCommand_programUniform3d* __restrict params) {
		glProgramUniform3d(
			params->program,
			params->location,
			params->v0,
			params->v1,
			params->v2
		);
	}
};
struct RenderCommand_programUniform3dv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniform3dv* __restrict params) {
		glProgramUniform3dv(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform3ui : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint v0;
	GLuint v1;
	GLuint v2;

	static void APIENTRY execute(const RenderCommand_programUniform3ui* __restrict params) {
		glProgramUniform3ui(
			params->program,
			params->location,
			params->v0,
			params->v1,
			params->v2
		);
	}
};
struct RenderCommand_programUniform3uiv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLuint *value;

	static void APIENTRY execute(const RenderCommand_programUniform3uiv* __restrict params) {
		glProgramUniform3uiv(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform4i : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLint v0;
	GLint v1;
	GLint v2;
	GLint v3;

	static void APIENTRY execute(const RenderCommand_programUniform4i* __restrict params) {
		glProgramUniform4i(
			params->program,
			params->location,
			params->v0,
			params->v1,
			params->v2,
			params->v3
		);
	}
};
struct RenderCommand_programUniform4iv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLint *value;

	static void APIENTRY execute(const RenderCommand_programUniform4iv* __restrict params) {
		glProgramUniform4iv(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform4f : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLfloat v0;
	GLfloat v1;
	GLfloat v2;
	GLfloat v3;

	static void APIENTRY execute(const RenderCommand_programUniform4f* __restrict params) {
		glProgramUniform4f(
			params->program,
			params->location,
			params->v0,
			params->v1,
			params->v2,
			params->v3
		);
	}
};
struct RenderCommand_programUniform4fv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniform4fv* __restrict params) {
		glProgramUniform4fv(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform4d : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLdouble v0;
	GLdouble v1;
	GLdouble v2;
	GLdouble v3;

	static void APIENTRY execute(const RenderCommand_programUniform4d* __restrict params) {
		glProgramUniform4d(
			params->program,
			params->location,
			params->v0,
			params->v1,
			params->v2,
			params->v3
		);
	}
};
struct RenderCommand_programUniform4dv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniform4dv* __restrict params) {
		glProgramUniform4dv(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform4ui : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint v0;
	GLuint v1;
	GLuint v2;
	GLuint v3;

	static void APIENTRY execute(const RenderCommand_programUniform4ui* __restrict params) {
		glProgramUniform4ui(
			params->program,
			params->location,
			params->v0,
			params->v1,
			params->v2,
			params->v3
		);
	}
};
struct RenderCommand_programUniform4uiv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLuint *value;

	static void APIENTRY execute(const RenderCommand_programUniform4uiv* __restrict params) {
		glProgramUniform4uiv(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix2fv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix2fv* __restrict params) {
		glProgramUniformMatrix2fv(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix3fv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix3fv* __restrict params) {
		glProgramUniformMatrix3fv(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix4fv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix4fv* __restrict params) {
		glProgramUniformMatrix4fv(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix2dv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix2dv* __restrict params) {
		glProgramUniformMatrix2dv(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix3dv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix3dv* __restrict params) {
		glProgramUniformMatrix3dv(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix4dv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix4dv* __restrict params) {
		glProgramUniformMatrix4dv(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix2x3fv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix2x3fv* __restrict params) {
		glProgramUniformMatrix2x3fv(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix3x2fv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix3x2fv* __restrict params) {
		glProgramUniformMatrix3x2fv(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix2x4fv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix2x4fv* __restrict params) {
		glProgramUniformMatrix2x4fv(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix4x2fv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix4x2fv* __restrict params) {
		glProgramUniformMatrix4x2fv(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix3x4fv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix3x4fv* __restrict params) {
		glProgramUniformMatrix3x4fv(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix4x3fv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix4x3fv* __restrict params) {
		glProgramUniformMatrix4x3fv(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix2x3dv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix2x3dv* __restrict params) {
		glProgramUniformMatrix2x3dv(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix3x2dv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix3x2dv* __restrict params) {
		glProgramUniformMatrix3x2dv(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix2x4dv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix2x4dv* __restrict params) {
		glProgramUniformMatrix2x4dv(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix4x2dv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix4x2dv* __restrict params) {
		glProgramUniformMatrix4x2dv(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix3x4dv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix3x4dv* __restrict params) {
		glProgramUniformMatrix3x4dv(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix4x3dv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix4x3dv* __restrict params) {
		glProgramUniformMatrix4x3dv(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_validateProgramPipeline : public RenderCommandBase {
	GLuint pipeline;

	static void APIENTRY execute(const RenderCommand_validateProgramPipeline* __restrict params) {
		glValidateProgramPipeline(
			params->pipeline
		);
	}
};
struct RenderCommand_getProgramPipelineInfoLog : public RenderCommandBase {
	GLuint pipeline;
	GLsizei bufSize;
	GLsizei *length;
	GLchar *infoLog;

	static void APIENTRY execute(const RenderCommand_getProgramPipelineInfoLog* __restrict params) {
		glGetProgramPipelineInfoLog(
			params->pipeline,
			params->bufSize,
			params->length,
			params->infoLog
		);
	}
};
struct RenderCommand_vertexAttribL1d : public RenderCommandBase {
	GLuint index;
	GLdouble x;

	static void APIENTRY execute(const RenderCommand_vertexAttribL1d* __restrict params) {
		glVertexAttribL1d(
			params->index,
			params->x
		);
	}
};
struct RenderCommand_vertexAttribL2d : public RenderCommandBase {
	GLuint index;
	GLdouble x;
	GLdouble y;

	static void APIENTRY execute(const RenderCommand_vertexAttribL2d* __restrict params) {
		glVertexAttribL2d(
			params->index,
			params->x,
			params->y
		);
	}
};
struct RenderCommand_vertexAttribL3d : public RenderCommandBase {
	GLuint index;
	GLdouble x;
	GLdouble y;
	GLdouble z;

	static void APIENTRY execute(const RenderCommand_vertexAttribL3d* __restrict params) {
		glVertexAttribL3d(
			params->index,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_vertexAttribL4d : public RenderCommandBase {
	GLuint index;
	GLdouble x;
	GLdouble y;
	GLdouble z;
	GLdouble w;

	static void APIENTRY execute(const RenderCommand_vertexAttribL4d* __restrict params) {
		glVertexAttribL4d(
			params->index,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_vertexAttribL1dv : public RenderCommandBase {
	GLuint index;
	const GLdouble *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribL1dv* __restrict params) {
		glVertexAttribL1dv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribL2dv : public RenderCommandBase {
	GLuint index;
	const GLdouble *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribL2dv* __restrict params) {
		glVertexAttribL2dv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribL3dv : public RenderCommandBase {
	GLuint index;
	const GLdouble *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribL3dv* __restrict params) {
		glVertexAttribL3dv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribL4dv : public RenderCommandBase {
	GLuint index;
	const GLdouble *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribL4dv* __restrict params) {
		glVertexAttribL4dv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribLPointer : public RenderCommandBase {
	GLuint index;
	GLint size;
	GLenum type;
	GLsizei stride;
	const void *pointer;

	static void APIENTRY execute(const RenderCommand_vertexAttribLPointer* __restrict params) {
		glVertexAttribLPointer(
			params->index,
			params->size,
			params->type,
			params->stride,
			params->pointer
		);
	}
};
struct RenderCommand_getVertexAttribLdv : public RenderCommandBase {
	GLuint index;
	GLenum pname;
	GLdouble *params;

	static void APIENTRY execute(const RenderCommand_getVertexAttribLdv* __restrict params) {
		glGetVertexAttribLdv(
			params->index,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_viewportArrayv : public RenderCommandBase {
	GLuint first;
	GLsizei count;
	const GLfloat *v;

	static void APIENTRY execute(const RenderCommand_viewportArrayv* __restrict params) {
		glViewportArrayv(
			params->first,
			params->count,
			params->v
		);
	}
};
struct RenderCommand_viewportIndexedf : public RenderCommandBase {
	GLuint index;
	GLfloat x;
	GLfloat y;
	GLfloat w;
	GLfloat h;

	static void APIENTRY execute(const RenderCommand_viewportIndexedf* __restrict params) {
		glViewportIndexedf(
			params->index,
			params->x,
			params->y,
			params->w,
			params->h
		);
	}
};
struct RenderCommand_viewportIndexedfv : public RenderCommandBase {
	GLuint index;
	const GLfloat *v;

	static void APIENTRY execute(const RenderCommand_viewportIndexedfv* __restrict params) {
		glViewportIndexedfv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_scissorArrayv : public RenderCommandBase {
	GLuint first;
	GLsizei count;
	const GLint *v;

	static void APIENTRY execute(const RenderCommand_scissorArrayv* __restrict params) {
		glScissorArrayv(
			params->first,
			params->count,
			params->v
		);
	}
};
struct RenderCommand_scissorIndexed : public RenderCommandBase {
	GLuint index;
	GLint left;
	GLint bottom;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_scissorIndexed* __restrict params) {
		glScissorIndexed(
			params->index,
			params->left,
			params->bottom,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_scissorIndexedv : public RenderCommandBase {
	GLuint index;
	const GLint *v;

	static void APIENTRY execute(const RenderCommand_scissorIndexedv* __restrict params) {
		glScissorIndexedv(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_depthRangeArrayv : public RenderCommandBase {
	GLuint first;
	GLsizei count;
	const GLdouble *v;

	static void APIENTRY execute(const RenderCommand_depthRangeArrayv* __restrict params) {
		glDepthRangeArrayv(
			params->first,
			params->count,
			params->v
		);
	}
};
struct RenderCommand_depthRangeIndexed : public RenderCommandBase {
	GLuint index;
	GLdouble n;
	GLdouble f;

	static void APIENTRY execute(const RenderCommand_depthRangeIndexed* __restrict params) {
		glDepthRangeIndexed(
			params->index,
			params->n,
			params->f
		);
	}
};
struct RenderCommand_getFloati_v : public RenderCommandBase {
	GLenum target;
	GLuint index;
	GLfloat *data;

	static void APIENTRY execute(const RenderCommand_getFloati_v* __restrict params) {
		glGetFloati_v(
			params->target,
			params->index,
			params->data
		);
	}
};
struct RenderCommand_getDoublei_v : public RenderCommandBase {
	GLenum target;
	GLuint index;
	GLdouble *data;

	static void APIENTRY execute(const RenderCommand_getDoublei_v* __restrict params) {
		glGetDoublei_v(
			params->target,
			params->index,
			params->data
		);
	}
};
struct RenderCommand_drawArraysInstancedBaseInstance : public RenderCommandBase {
	GLenum mode;
	GLint first;
	GLsizei count;
	GLsizei instancecount;
	GLuint baseinstance;

	static void APIENTRY execute(const RenderCommand_drawArraysInstancedBaseInstance* __restrict params) {
		glDrawArraysInstancedBaseInstance(
			params->mode,
			params->first,
			params->count,
			params->instancecount,
			params->baseinstance
		);
	}
};
struct RenderCommand_drawElementsInstancedBaseInstance : public RenderCommandBase {
	GLenum mode;
	GLsizei count;
	GLenum type;
	const void *indices;
	GLsizei instancecount;
	GLuint baseinstance;

	static void APIENTRY execute(const RenderCommand_drawElementsInstancedBaseInstance* __restrict params) {
		glDrawElementsInstancedBaseInstance(
			params->mode,
			params->count,
			params->type,
			params->indices,
			params->instancecount,
			params->baseinstance
		);
	}
};
struct RenderCommand_drawElementsInstancedBaseVertexBaseInstance : public RenderCommandBase {
	GLenum mode;
	GLsizei count;
	GLenum type;
	const void *indices;
	GLsizei instancecount;
	GLint basevertex;
	GLuint baseinstance;

	static void APIENTRY execute(const RenderCommand_drawElementsInstancedBaseVertexBaseInstance* __restrict params) {
		glDrawElementsInstancedBaseVertexBaseInstance(
			params->mode,
			params->count,
			params->type,
			params->indices,
			params->instancecount,
			params->basevertex,
			params->baseinstance
		);
	}
};
struct RenderCommand_getInternalformativ : public RenderCommandBase {
	GLenum target;
	GLenum internalformat;
	GLenum pname;
	GLsizei bufSize;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getInternalformativ* __restrict params) {
		glGetInternalformativ(
			params->target,
			params->internalformat,
			params->pname,
			params->bufSize,
			params->params
		);
	}
};
struct RenderCommand_getActiveAtomicCounterBufferiv : public RenderCommandBase {
	GLuint program;
	GLuint bufferIndex;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getActiveAtomicCounterBufferiv* __restrict params) {
		glGetActiveAtomicCounterBufferiv(
			params->program,
			params->bufferIndex,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_bindImageTexture : public RenderCommandBase {
	GLuint unit;
	GLuint texture;
	GLint level;
	GLboolean layered;
	GLint layer;
	GLenum access;
	GLenum format;

	static void APIENTRY execute(const RenderCommand_bindImageTexture* __restrict params) {
		glBindImageTexture(
			params->unit,
			params->texture,
			params->level,
			params->layered,
			params->layer,
			params->access,
			params->format
		);
	}
};
struct RenderCommand_memoryBarrier : public RenderCommandBase {
	GLbitfield barriers;

	static void APIENTRY execute(const RenderCommand_memoryBarrier* __restrict params) {
		glMemoryBarrier(
			params->barriers
		);
	}
};
struct RenderCommand_texStorage1D : public RenderCommandBase {
	GLenum target;
	GLsizei levels;
	GLenum internalformat;
	GLsizei width;

	static void APIENTRY execute(const RenderCommand_texStorage1D* __restrict params) {
		glTexStorage1D(
			params->target,
			params->levels,
			params->internalformat,
			params->width
		);
	}
};
struct RenderCommand_texStorage2D : public RenderCommandBase {
	GLenum target;
	GLsizei levels;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_texStorage2D* __restrict params) {
		glTexStorage2D(
			params->target,
			params->levels,
			params->internalformat,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_texStorage3D : public RenderCommandBase {
	GLenum target;
	GLsizei levels;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;
	GLsizei depth;

	static void APIENTRY execute(const RenderCommand_texStorage3D* __restrict params) {
		glTexStorage3D(
			params->target,
			params->levels,
			params->internalformat,
			params->width,
			params->height,
			params->depth
		);
	}
};
struct RenderCommand_drawTransformFeedbackInstanced : public RenderCommandBase {
	GLenum mode;
	GLuint id;
	GLsizei instancecount;

	static void APIENTRY execute(const RenderCommand_drawTransformFeedbackInstanced* __restrict params) {
		glDrawTransformFeedbackInstanced(
			params->mode,
			params->id,
			params->instancecount
		);
	}
};
struct RenderCommand_drawTransformFeedbackStreamInstanced : public RenderCommandBase {
	GLenum mode;
	GLuint id;
	GLuint stream;
	GLsizei instancecount;

	static void APIENTRY execute(const RenderCommand_drawTransformFeedbackStreamInstanced* __restrict params) {
		glDrawTransformFeedbackStreamInstanced(
			params->mode,
			params->id,
			params->stream,
			params->instancecount
		);
	}
};
struct RenderCommand_clearBufferData : public RenderCommandBase {
	GLenum target;
	GLenum internalformat;
	GLenum format;
	GLenum type;
	const void *data;

	static void APIENTRY execute(const RenderCommand_clearBufferData* __restrict params) {
		glClearBufferData(
			params->target,
			params->internalformat,
			params->format,
			params->type,
			params->data
		);
	}
};
struct RenderCommand_clearBufferSubData : public RenderCommandBase {
	GLenum target;
	GLenum internalformat;
	GLintptr offset;
	GLsizeiptr size;
	GLenum format;
	GLenum type;
	const void *data;

	static void APIENTRY execute(const RenderCommand_clearBufferSubData* __restrict params) {
		glClearBufferSubData(
			params->target,
			params->internalformat,
			params->offset,
			params->size,
			params->format,
			params->type,
			params->data
		);
	}
};
struct RenderCommand_dispatchCompute : public RenderCommandBase {
	GLuint num_groups_x;
	GLuint num_groups_y;
	GLuint num_groups_z;

	static void APIENTRY execute(const RenderCommand_dispatchCompute* __restrict params) {
		glDispatchCompute(
			params->num_groups_x,
			params->num_groups_y,
			params->num_groups_z
		);
	}
};
struct RenderCommand_dispatchComputeIndirect : public RenderCommandBase {
	GLintptr indirect;

	static void APIENTRY execute(const RenderCommand_dispatchComputeIndirect* __restrict params) {
		glDispatchComputeIndirect(
			params->indirect
		);
	}
};
struct RenderCommand_copyImageSubData : public RenderCommandBase {
	GLuint srcName;
	GLenum srcTarget;
	GLint srcLevel;
	GLint srcX;
	GLint srcY;
	GLint srcZ;
	GLuint dstName;
	GLenum dstTarget;
	GLint dstLevel;
	GLint dstX;
	GLint dstY;
	GLint dstZ;
	GLsizei srcWidth;
	GLsizei srcHeight;
	GLsizei srcDepth;

	static void APIENTRY execute(const RenderCommand_copyImageSubData* __restrict params) {
		glCopyImageSubData(
			params->srcName,
			params->srcTarget,
			params->srcLevel,
			params->srcX,
			params->srcY,
			params->srcZ,
			params->dstName,
			params->dstTarget,
			params->dstLevel,
			params->dstX,
			params->dstY,
			params->dstZ,
			params->srcWidth,
			params->srcHeight,
			params->srcDepth
		);
	}
};
struct RenderCommand_framebufferParameteri : public RenderCommandBase {
	GLenum target;
	GLenum pname;
	GLint param;

	static void APIENTRY execute(const RenderCommand_framebufferParameteri* __restrict params) {
		glFramebufferParameteri(
			params->target,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_getFramebufferParameteriv : public RenderCommandBase {
	GLenum target;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getFramebufferParameteriv* __restrict params) {
		glGetFramebufferParameteriv(
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getInternalformati64v : public RenderCommandBase {
	GLenum target;
	GLenum internalformat;
	GLenum pname;
	GLsizei bufSize;
	GLint64 *params;

	static void APIENTRY execute(const RenderCommand_getInternalformati64v* __restrict params) {
		glGetInternalformati64v(
			params->target,
			params->internalformat,
			params->pname,
			params->bufSize,
			params->params
		);
	}
};
struct RenderCommand_invalidateTexSubImage : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint zoffset;
	GLsizei width;
	GLsizei height;
	GLsizei depth;

	static void APIENTRY execute(const RenderCommand_invalidateTexSubImage* __restrict params) {
		glInvalidateTexSubImage(
			params->texture,
			params->level,
			params->xoffset,
			params->yoffset,
			params->zoffset,
			params->width,
			params->height,
			params->depth
		);
	}
};
struct RenderCommand_invalidateTexImage : public RenderCommandBase {
	GLuint texture;
	GLint level;

	static void APIENTRY execute(const RenderCommand_invalidateTexImage* __restrict params) {
		glInvalidateTexImage(
			params->texture,
			params->level
		);
	}
};
struct RenderCommand_invalidateBufferSubData : public RenderCommandBase {
	GLuint buffer;
	GLintptr offset;
	GLsizeiptr length;

	static void APIENTRY execute(const RenderCommand_invalidateBufferSubData* __restrict params) {
		glInvalidateBufferSubData(
			params->buffer,
			params->offset,
			params->length
		);
	}
};
struct RenderCommand_invalidateBufferData : public RenderCommandBase {
	GLuint buffer;

	static void APIENTRY execute(const RenderCommand_invalidateBufferData* __restrict params) {
		glInvalidateBufferData(
			params->buffer
		);
	}
};
struct RenderCommand_invalidateFramebuffer : public RenderCommandBase {
	GLenum target;
	GLsizei numAttachments;
	const GLenum *attachments;

	static void APIENTRY execute(const RenderCommand_invalidateFramebuffer* __restrict params) {
		glInvalidateFramebuffer(
			params->target,
			params->numAttachments,
			params->attachments
		);
	}
};
struct RenderCommand_invalidateSubFramebuffer : public RenderCommandBase {
	GLenum target;
	GLsizei numAttachments;
	const GLenum *attachments;
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_invalidateSubFramebuffer* __restrict params) {
		glInvalidateSubFramebuffer(
			params->target,
			params->numAttachments,
			params->attachments,
			params->x,
			params->y,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_multiDrawArraysIndirect : public RenderCommandBase {
	GLenum mode;
	const void *indirect;
	GLsizei drawcount;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_multiDrawArraysIndirect* __restrict params) {
		glMultiDrawArraysIndirect(
			params->mode,
			params->indirect,
			params->drawcount,
			params->stride
		);
	}
};
struct RenderCommand_multiDrawElementsIndirect : public RenderCommandBase {
	GLenum mode;
	GLenum type;
	const void *indirect;
	GLsizei drawcount;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_multiDrawElementsIndirect* __restrict params) {
		glMultiDrawElementsIndirect(
			params->mode,
			params->type,
			params->indirect,
			params->drawcount,
			params->stride
		);
	}
};
struct RenderCommand_getProgramInterfaceiv : public RenderCommandBase {
	GLuint program;
	GLenum programInterface;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getProgramInterfaceiv* __restrict params) {
		glGetProgramInterfaceiv(
			params->program,
			params->programInterface,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getProgramResourceIndex : public RenderCommandBase {
	GLuint program;
	GLenum programInterface;
	const GLchar *name;

	static void APIENTRY execute(const RenderCommand_getProgramResourceIndex* __restrict params) {
		glGetProgramResourceIndex(
			params->program,
			params->programInterface,
			params->name
		);
	}
};
struct RenderCommand_getProgramResourceName : public RenderCommandBase {
	GLuint program;
	GLenum programInterface;
	GLuint index;
	GLsizei bufSize;
	GLsizei *length;
	GLchar *name;

	static void APIENTRY execute(const RenderCommand_getProgramResourceName* __restrict params) {
		glGetProgramResourceName(
			params->program,
			params->programInterface,
			params->index,
			params->bufSize,
			params->length,
			params->name
		);
	}
};
struct RenderCommand_getProgramResourceiv : public RenderCommandBase {
	GLuint program;
	GLenum programInterface;
	GLuint index;
	GLsizei propCount;
	const GLenum *props;
	GLsizei bufSize;
	GLsizei *length;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getProgramResourceiv* __restrict params) {
		glGetProgramResourceiv(
			params->program,
			params->programInterface,
			params->index,
			params->propCount,
			params->props,
			params->bufSize,
			params->length,
			params->params
		);
	}
};
struct RenderCommand_getProgramResourceLocation : public RenderCommandBase {
	GLuint program;
	GLenum programInterface;
	const GLchar *name;

	static void APIENTRY execute(const RenderCommand_getProgramResourceLocation* __restrict params) {
		glGetProgramResourceLocation(
			params->program,
			params->programInterface,
			params->name
		);
	}
};
struct RenderCommand_getProgramResourceLocationIndex : public RenderCommandBase {
	GLuint program;
	GLenum programInterface;
	const GLchar *name;

	static void APIENTRY execute(const RenderCommand_getProgramResourceLocationIndex* __restrict params) {
		glGetProgramResourceLocationIndex(
			params->program,
			params->programInterface,
			params->name
		);
	}
};
struct RenderCommand_shaderStorageBlockBinding : public RenderCommandBase {
	GLuint program;
	GLuint storageBlockIndex;
	GLuint storageBlockBinding;

	static void APIENTRY execute(const RenderCommand_shaderStorageBlockBinding* __restrict params) {
		glShaderStorageBlockBinding(
			params->program,
			params->storageBlockIndex,
			params->storageBlockBinding
		);
	}
};
struct RenderCommand_texBufferRange : public RenderCommandBase {
	GLenum target;
	GLenum internalformat;
	GLuint buffer;
	GLintptr offset;
	GLsizeiptr size;

	static void APIENTRY execute(const RenderCommand_texBufferRange* __restrict params) {
		glTexBufferRange(
			params->target,
			params->internalformat,
			params->buffer,
			params->offset,
			params->size
		);
	}
};
struct RenderCommand_texStorage2DMultisample : public RenderCommandBase {
	GLenum target;
	GLsizei samples;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;
	GLboolean fixedsamplelocations;

	static void APIENTRY execute(const RenderCommand_texStorage2DMultisample* __restrict params) {
		glTexStorage2DMultisample(
			params->target,
			params->samples,
			params->internalformat,
			params->width,
			params->height,
			params->fixedsamplelocations
		);
	}
};
struct RenderCommand_texStorage3DMultisample : public RenderCommandBase {
	GLenum target;
	GLsizei samples;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLboolean fixedsamplelocations;

	static void APIENTRY execute(const RenderCommand_texStorage3DMultisample* __restrict params) {
		glTexStorage3DMultisample(
			params->target,
			params->samples,
			params->internalformat,
			params->width,
			params->height,
			params->depth,
			params->fixedsamplelocations
		);
	}
};
struct RenderCommand_textureView : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLuint origtexture;
	GLenum internalformat;
	GLuint minlevel;
	GLuint numlevels;
	GLuint minlayer;
	GLuint numlayers;

	static void APIENTRY execute(const RenderCommand_textureView* __restrict params) {
		glTextureView(
			params->texture,
			params->target,
			params->origtexture,
			params->internalformat,
			params->minlevel,
			params->numlevels,
			params->minlayer,
			params->numlayers
		);
	}
};
struct RenderCommand_bindVertexBuffer : public RenderCommandBase {
	GLuint bindingindex;
	GLuint buffer;
	GLintptr offset;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_bindVertexBuffer* __restrict params) {
		glBindVertexBuffer(
			params->bindingindex,
			params->buffer,
			params->offset,
			params->stride
		);
	}
};
struct RenderCommand_vertexAttribFormat : public RenderCommandBase {
	GLuint attribindex;
	GLint size;
	GLenum type;
	GLboolean normalized;
	GLuint relativeoffset;

	static void APIENTRY execute(const RenderCommand_vertexAttribFormat* __restrict params) {
		glVertexAttribFormat(
			params->attribindex,
			params->size,
			params->type,
			params->normalized,
			params->relativeoffset
		);
	}
};
struct RenderCommand_vertexAttribIFormat : public RenderCommandBase {
	GLuint attribindex;
	GLint size;
	GLenum type;
	GLuint relativeoffset;

	static void APIENTRY execute(const RenderCommand_vertexAttribIFormat* __restrict params) {
		glVertexAttribIFormat(
			params->attribindex,
			params->size,
			params->type,
			params->relativeoffset
		);
	}
};
struct RenderCommand_vertexAttribLFormat : public RenderCommandBase {
	GLuint attribindex;
	GLint size;
	GLenum type;
	GLuint relativeoffset;

	static void APIENTRY execute(const RenderCommand_vertexAttribLFormat* __restrict params) {
		glVertexAttribLFormat(
			params->attribindex,
			params->size,
			params->type,
			params->relativeoffset
		);
	}
};
struct RenderCommand_vertexAttribBinding : public RenderCommandBase {
	GLuint attribindex;
	GLuint bindingindex;

	static void APIENTRY execute(const RenderCommand_vertexAttribBinding* __restrict params) {
		glVertexAttribBinding(
			params->attribindex,
			params->bindingindex
		);
	}
};
struct RenderCommand_vertexBindingDivisor : public RenderCommandBase {
	GLuint bindingindex;
	GLuint divisor;

	static void APIENTRY execute(const RenderCommand_vertexBindingDivisor* __restrict params) {
		glVertexBindingDivisor(
			params->bindingindex,
			params->divisor
		);
	}
};
struct RenderCommand_debugMessageControl : public RenderCommandBase {
	GLenum source;
	GLenum type;
	GLenum severity;
	GLsizei count;
	const GLuint *ids;
	GLboolean enabled;

	static void APIENTRY execute(const RenderCommand_debugMessageControl* __restrict params) {
		glDebugMessageControl(
			params->source,
			params->type,
			params->severity,
			params->count,
			params->ids,
			params->enabled
		);
	}
};
struct RenderCommand_debugMessageInsert : public RenderCommandBase {
	GLenum source;
	GLenum type;
	GLuint id;
	GLenum severity;
	GLsizei length;
	const GLchar *buf;

	static void APIENTRY execute(const RenderCommand_debugMessageInsert* __restrict params) {
		glDebugMessageInsert(
			params->source,
			params->type,
			params->id,
			params->severity,
			params->length,
			params->buf
		);
	}
};
struct RenderCommand_debugMessageCallback : public RenderCommandBase {
	GLDEBUGPROC callback;
	const void *userParam;

	static void APIENTRY execute(const RenderCommand_debugMessageCallback* __restrict params) {
		glDebugMessageCallback(
			params->callback,
			params->userParam
		);
	}
};
struct RenderCommand_getDebugMessageLog : public RenderCommandBase {
	GLuint count;
	GLsizei bufSize;
	GLenum *sources;
	GLenum *types;
	GLuint *ids;
	GLenum *severities;
	GLsizei *lengths;
	GLchar *messageLog;

	static void APIENTRY execute(const RenderCommand_getDebugMessageLog* __restrict params) {
		glGetDebugMessageLog(
			params->count,
			params->bufSize,
			params->sources,
			params->types,
			params->ids,
			params->severities,
			params->lengths,
			params->messageLog
		);
	}
};
struct RenderCommand_pushDebugGroup : public RenderCommandBase {
	GLenum source;
	GLuint id;
	GLsizei length;
	const GLchar *message;

	static void APIENTRY execute(const RenderCommand_pushDebugGroup* __restrict params) {
		glPushDebugGroup(
			params->source,
			params->id,
			params->length,
			params->message
		);
	}
};
struct RenderCommand_popDebugGroup : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_popDebugGroup* __restrict params) {
		glPopDebugGroup(
		);
	}
};
struct RenderCommand_objectLabel : public RenderCommandBase {
	GLenum identifier;
	GLuint name;
	GLsizei length;
	const GLchar *label;

	static void APIENTRY execute(const RenderCommand_objectLabel* __restrict params) {
		glObjectLabel(
			params->identifier,
			params->name,
			params->length,
			params->label
		);
	}
};
struct RenderCommand_getObjectLabel : public RenderCommandBase {
	GLenum identifier;
	GLuint name;
	GLsizei bufSize;
	GLsizei *length;
	GLchar *label;

	static void APIENTRY execute(const RenderCommand_getObjectLabel* __restrict params) {
		glGetObjectLabel(
			params->identifier,
			params->name,
			params->bufSize,
			params->length,
			params->label
		);
	}
};
struct RenderCommand_objectPtrLabel : public RenderCommandBase {
	const void *ptr;
	GLsizei length;
	const GLchar *label;

	static void APIENTRY execute(const RenderCommand_objectPtrLabel* __restrict params) {
		glObjectPtrLabel(
			params->ptr,
			params->length,
			params->label
		);
	}
};
struct RenderCommand_getObjectPtrLabel : public RenderCommandBase {
	const void *ptr;
	GLsizei bufSize;
	GLsizei *length;
	GLchar *label;

	static void APIENTRY execute(const RenderCommand_getObjectPtrLabel* __restrict params) {
		glGetObjectPtrLabel(
			params->ptr,
			params->bufSize,
			params->length,
			params->label
		);
	}
};
struct RenderCommand_bufferStorage : public RenderCommandBase {
	GLenum target;
	GLsizeiptr size;
	const void *data;
	GLbitfield flags;

	static void APIENTRY execute(const RenderCommand_bufferStorage* __restrict params) {
		glBufferStorage(
			params->target,
			params->size,
			params->data,
			params->flags
		);
	}
};
struct RenderCommand_clearTexImage : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLenum format;
	GLenum type;
	const void *data;

	static void APIENTRY execute(const RenderCommand_clearTexImage* __restrict params) {
		glClearTexImage(
			params->texture,
			params->level,
			params->format,
			params->type,
			params->data
		);
	}
};
struct RenderCommand_clearTexSubImage : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint zoffset;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLenum format;
	GLenum type;
	const void *data;

	static void APIENTRY execute(const RenderCommand_clearTexSubImage* __restrict params) {
		glClearTexSubImage(
			params->texture,
			params->level,
			params->xoffset,
			params->yoffset,
			params->zoffset,
			params->width,
			params->height,
			params->depth,
			params->format,
			params->type,
			params->data
		);
	}
};
struct RenderCommand_bindBuffersBase : public RenderCommandBase {
	GLenum target;
	GLuint first;
	GLsizei count;
	const GLuint *buffers;

	static void APIENTRY execute(const RenderCommand_bindBuffersBase* __restrict params) {
		glBindBuffersBase(
			params->target,
			params->first,
			params->count,
			params->buffers
		);
	}
};
struct RenderCommand_bindBuffersRange : public RenderCommandBase {
	GLenum target;
	GLuint first;
	GLsizei count;
	const GLuint *buffers;
	const GLintptr *offsets;
	const GLsizeiptr *sizes;

	static void APIENTRY execute(const RenderCommand_bindBuffersRange* __restrict params) {
		glBindBuffersRange(
			params->target,
			params->first,
			params->count,
			params->buffers,
			params->offsets,
			params->sizes
		);
	}
};
struct RenderCommand_bindTextures : public RenderCommandBase {
	GLuint first;
	GLsizei count;
	const GLuint *textures;

	static void APIENTRY execute(const RenderCommand_bindTextures* __restrict params) {
		glBindTextures(
			params->first,
			params->count,
			params->textures
		);
	}
};
struct RenderCommand_bindSamplers : public RenderCommandBase {
	GLuint first;
	GLsizei count;
	const GLuint *samplers;

	static void APIENTRY execute(const RenderCommand_bindSamplers* __restrict params) {
		glBindSamplers(
			params->first,
			params->count,
			params->samplers
		);
	}
};
struct RenderCommand_bindImageTextures : public RenderCommandBase {
	GLuint first;
	GLsizei count;
	const GLuint *textures;

	static void APIENTRY execute(const RenderCommand_bindImageTextures* __restrict params) {
		glBindImageTextures(
			params->first,
			params->count,
			params->textures
		);
	}
};
struct RenderCommand_bindVertexBuffers : public RenderCommandBase {
	GLuint first;
	GLsizei count;
	const GLuint *buffers;
	const GLintptr *offsets;
	const GLsizei *strides;

	static void APIENTRY execute(const RenderCommand_bindVertexBuffers* __restrict params) {
		glBindVertexBuffers(
			params->first,
			params->count,
			params->buffers,
			params->offsets,
			params->strides
		);
	}
};
struct RenderCommand_clipControl : public RenderCommandBase {
	GLenum origin;
	GLenum depth;

	static void APIENTRY execute(const RenderCommand_clipControl* __restrict params) {
		glClipControl(
			params->origin,
			params->depth
		);
	}
};
struct RenderCommand_createTransformFeedbacks : public RenderCommandBase {
	GLsizei n;
	GLuint *ids;

	static void APIENTRY execute(const RenderCommand_createTransformFeedbacks* __restrict params) {
		glCreateTransformFeedbacks(
			params->n,
			params->ids
		);
	}
};
struct RenderCommand_transformFeedbackBufferBase : public RenderCommandBase {
	GLuint xfb;
	GLuint index;
	GLuint buffer;

	static void APIENTRY execute(const RenderCommand_transformFeedbackBufferBase* __restrict params) {
		glTransformFeedbackBufferBase(
			params->xfb,
			params->index,
			params->buffer
		);
	}
};
struct RenderCommand_transformFeedbackBufferRange : public RenderCommandBase {
	GLuint xfb;
	GLuint index;
	GLuint buffer;
	GLintptr offset;
	GLsizeiptr size;

	static void APIENTRY execute(const RenderCommand_transformFeedbackBufferRange* __restrict params) {
		glTransformFeedbackBufferRange(
			params->xfb,
			params->index,
			params->buffer,
			params->offset,
			params->size
		);
	}
};
struct RenderCommand_getTransformFeedbackiv : public RenderCommandBase {
	GLuint xfb;
	GLenum pname;
	GLint *param;

	static void APIENTRY execute(const RenderCommand_getTransformFeedbackiv* __restrict params) {
		glGetTransformFeedbackiv(
			params->xfb,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_getTransformFeedbacki_v : public RenderCommandBase {
	GLuint xfb;
	GLenum pname;
	GLuint index;
	GLint *param;

	static void APIENTRY execute(const RenderCommand_getTransformFeedbacki_v* __restrict params) {
		glGetTransformFeedbacki_v(
			params->xfb,
			params->pname,
			params->index,
			params->param
		);
	}
};
struct RenderCommand_getTransformFeedbacki64_v : public RenderCommandBase {
	GLuint xfb;
	GLenum pname;
	GLuint index;
	GLint64 *param;

	static void APIENTRY execute(const RenderCommand_getTransformFeedbacki64_v* __restrict params) {
		glGetTransformFeedbacki64_v(
			params->xfb,
			params->pname,
			params->index,
			params->param
		);
	}
};
struct RenderCommand_createBuffers : public RenderCommandBase {
	GLsizei n;
	GLuint *buffers;

	static void APIENTRY execute(const RenderCommand_createBuffers* __restrict params) {
		glCreateBuffers(
			params->n,
			params->buffers
		);
	}
};
struct RenderCommand_namedBufferStorage : public RenderCommandBase {
	GLuint buffer;
	GLsizeiptr size;
	const void *data;
	GLbitfield flags;

	static void APIENTRY execute(const RenderCommand_namedBufferStorage* __restrict params) {
		glNamedBufferStorage(
			params->buffer,
			params->size,
			params->data,
			params->flags
		);
	}
};
struct RenderCommand_namedBufferData : public RenderCommandBase {
	GLuint buffer;
	GLsizeiptr size;
	const void *data;
	GLenum usage;

	static void APIENTRY execute(const RenderCommand_namedBufferData* __restrict params) {
		glNamedBufferData(
			params->buffer,
			params->size,
			params->data,
			params->usage
		);
	}
};
struct RenderCommand_namedBufferSubData : public RenderCommandBase {
	GLuint buffer;
	GLintptr offset;
	GLsizeiptr size;
	const void *data;

	static void APIENTRY execute(const RenderCommand_namedBufferSubData* __restrict params) {
		glNamedBufferSubData(
			params->buffer,
			params->offset,
			params->size,
			params->data
		);
	}
};
struct RenderCommand_copyNamedBufferSubData : public RenderCommandBase {
	GLuint readBuffer;
	GLuint writeBuffer;
	GLintptr readOffset;
	GLintptr writeOffset;
	GLsizeiptr size;

	static void APIENTRY execute(const RenderCommand_copyNamedBufferSubData* __restrict params) {
		glCopyNamedBufferSubData(
			params->readBuffer,
			params->writeBuffer,
			params->readOffset,
			params->writeOffset,
			params->size
		);
	}
};
struct RenderCommand_clearNamedBufferData : public RenderCommandBase {
	GLuint buffer;
	GLenum internalformat;
	GLenum format;
	GLenum type;
	const void *data;

	static void APIENTRY execute(const RenderCommand_clearNamedBufferData* __restrict params) {
		glClearNamedBufferData(
			params->buffer,
			params->internalformat,
			params->format,
			params->type,
			params->data
		);
	}
};
struct RenderCommand_clearNamedBufferSubData : public RenderCommandBase {
	GLuint buffer;
	GLenum internalformat;
	GLintptr offset;
	GLsizeiptr size;
	GLenum format;
	GLenum type;
	const void *data;

	static void APIENTRY execute(const RenderCommand_clearNamedBufferSubData* __restrict params) {
		glClearNamedBufferSubData(
			params->buffer,
			params->internalformat,
			params->offset,
			params->size,
			params->format,
			params->type,
			params->data
		);
	}
};
struct RenderCommand_mapNamedBuffer : public RenderCommandBase {
	GLuint buffer;
	GLenum access;

	static void APIENTRY execute(const RenderCommand_mapNamedBuffer* __restrict params) {
		glMapNamedBuffer(
			params->buffer,
			params->access
		);
	}
};
struct RenderCommand_mapNamedBufferRange : public RenderCommandBase {
	GLuint buffer;
	GLintptr offset;
	GLsizeiptr length;
	GLbitfield access;

	static void APIENTRY execute(const RenderCommand_mapNamedBufferRange* __restrict params) {
		glMapNamedBufferRange(
			params->buffer,
			params->offset,
			params->length,
			params->access
		);
	}
};
struct RenderCommand_unmapNamedBuffer : public RenderCommandBase {
	GLuint buffer;

	static void APIENTRY execute(const RenderCommand_unmapNamedBuffer* __restrict params) {
		glUnmapNamedBuffer(
			params->buffer
		);
	}
};
struct RenderCommand_flushMappedNamedBufferRange : public RenderCommandBase {
	GLuint buffer;
	GLintptr offset;
	GLsizeiptr length;

	static void APIENTRY execute(const RenderCommand_flushMappedNamedBufferRange* __restrict params) {
		glFlushMappedNamedBufferRange(
			params->buffer,
			params->offset,
			params->length
		);
	}
};
struct RenderCommand_getNamedBufferParameteriv : public RenderCommandBase {
	GLuint buffer;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getNamedBufferParameteriv* __restrict params) {
		glGetNamedBufferParameteriv(
			params->buffer,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getNamedBufferParameteri64v : public RenderCommandBase {
	GLuint buffer;
	GLenum pname;
	GLint64 *params;

	static void APIENTRY execute(const RenderCommand_getNamedBufferParameteri64v* __restrict params) {
		glGetNamedBufferParameteri64v(
			params->buffer,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getNamedBufferPointerv : public RenderCommandBase {
	GLuint buffer;
	GLenum pname;
	void **params;

	static void APIENTRY execute(const RenderCommand_getNamedBufferPointerv* __restrict params) {
		glGetNamedBufferPointerv(
			params->buffer,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getNamedBufferSubData : public RenderCommandBase {
	GLuint buffer;
	GLintptr offset;
	GLsizeiptr size;
	void *data;

	static void APIENTRY execute(const RenderCommand_getNamedBufferSubData* __restrict params) {
		glGetNamedBufferSubData(
			params->buffer,
			params->offset,
			params->size,
			params->data
		);
	}
};
struct RenderCommand_createFramebuffers : public RenderCommandBase {
	GLsizei n;
	GLuint *framebuffers;

	static void APIENTRY execute(const RenderCommand_createFramebuffers* __restrict params) {
		glCreateFramebuffers(
			params->n,
			params->framebuffers
		);
	}
};
struct RenderCommand_namedFramebufferRenderbuffer : public RenderCommandBase {
	GLuint framebuffer;
	GLenum attachment;
	GLenum renderbuffertarget;
	GLuint renderbuffer;

	static void APIENTRY execute(const RenderCommand_namedFramebufferRenderbuffer* __restrict params) {
		glNamedFramebufferRenderbuffer(
			params->framebuffer,
			params->attachment,
			params->renderbuffertarget,
			params->renderbuffer
		);
	}
};
struct RenderCommand_namedFramebufferParameteri : public RenderCommandBase {
	GLuint framebuffer;
	GLenum pname;
	GLint param;

	static void APIENTRY execute(const RenderCommand_namedFramebufferParameteri* __restrict params) {
		glNamedFramebufferParameteri(
			params->framebuffer,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_namedFramebufferTexture : public RenderCommandBase {
	GLuint framebuffer;
	GLenum attachment;
	GLuint texture;
	GLint level;

	static void APIENTRY execute(const RenderCommand_namedFramebufferTexture* __restrict params) {
		glNamedFramebufferTexture(
			params->framebuffer,
			params->attachment,
			params->texture,
			params->level
		);
	}
};
struct RenderCommand_namedFramebufferTextureLayer : public RenderCommandBase {
	GLuint framebuffer;
	GLenum attachment;
	GLuint texture;
	GLint level;
	GLint layer;

	static void APIENTRY execute(const RenderCommand_namedFramebufferTextureLayer* __restrict params) {
		glNamedFramebufferTextureLayer(
			params->framebuffer,
			params->attachment,
			params->texture,
			params->level,
			params->layer
		);
	}
};
struct RenderCommand_namedFramebufferDrawBuffer : public RenderCommandBase {
	GLuint framebuffer;
	GLenum buf;

	static void APIENTRY execute(const RenderCommand_namedFramebufferDrawBuffer* __restrict params) {
		glNamedFramebufferDrawBuffer(
			params->framebuffer,
			params->buf
		);
	}
};
struct RenderCommand_namedFramebufferDrawBuffers : public RenderCommandBase {
	GLuint framebuffer;
	GLsizei n;
	const GLenum *bufs;

	static void APIENTRY execute(const RenderCommand_namedFramebufferDrawBuffers* __restrict params) {
		glNamedFramebufferDrawBuffers(
			params->framebuffer,
			params->n,
			params->bufs
		);
	}
};
struct RenderCommand_namedFramebufferReadBuffer : public RenderCommandBase {
	GLuint framebuffer;
	GLenum src;

	static void APIENTRY execute(const RenderCommand_namedFramebufferReadBuffer* __restrict params) {
		glNamedFramebufferReadBuffer(
			params->framebuffer,
			params->src
		);
	}
};
struct RenderCommand_invalidateNamedFramebufferData : public RenderCommandBase {
	GLuint framebuffer;
	GLsizei numAttachments;
	const GLenum *attachments;

	static void APIENTRY execute(const RenderCommand_invalidateNamedFramebufferData* __restrict params) {
		glInvalidateNamedFramebufferData(
			params->framebuffer,
			params->numAttachments,
			params->attachments
		);
	}
};
struct RenderCommand_invalidateNamedFramebufferSubData : public RenderCommandBase {
	GLuint framebuffer;
	GLsizei numAttachments;
	const GLenum *attachments;
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_invalidateNamedFramebufferSubData* __restrict params) {
		glInvalidateNamedFramebufferSubData(
			params->framebuffer,
			params->numAttachments,
			params->attachments,
			params->x,
			params->y,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_clearNamedFramebufferiv : public RenderCommandBase {
	GLuint framebuffer;
	GLenum buffer;
	GLint drawbuffer;
	const GLint *value;

	static void APIENTRY execute(const RenderCommand_clearNamedFramebufferiv* __restrict params) {
		glClearNamedFramebufferiv(
			params->framebuffer,
			params->buffer,
			params->drawbuffer,
			params->value
		);
	}
};
struct RenderCommand_clearNamedFramebufferuiv : public RenderCommandBase {
	GLuint framebuffer;
	GLenum buffer;
	GLint drawbuffer;
	const GLuint *value;

	static void APIENTRY execute(const RenderCommand_clearNamedFramebufferuiv* __restrict params) {
		glClearNamedFramebufferuiv(
			params->framebuffer,
			params->buffer,
			params->drawbuffer,
			params->value
		);
	}
};
struct RenderCommand_clearNamedFramebufferfv : public RenderCommandBase {
	GLuint framebuffer;
	GLenum buffer;
	GLint drawbuffer;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_clearNamedFramebufferfv* __restrict params) {
		glClearNamedFramebufferfv(
			params->framebuffer,
			params->buffer,
			params->drawbuffer,
			params->value
		);
	}
};
struct RenderCommand_clearNamedFramebufferfi : public RenderCommandBase {
	GLuint framebuffer;
	GLenum buffer;
	GLint drawbuffer;
	GLfloat depth;
	GLint stencil;

	static void APIENTRY execute(const RenderCommand_clearNamedFramebufferfi* __restrict params) {
		glClearNamedFramebufferfi(
			params->framebuffer,
			params->buffer,
			params->drawbuffer,
			params->depth,
			params->stencil
		);
	}
};
struct RenderCommand_blitNamedFramebuffer : public RenderCommandBase {
	GLuint readFramebuffer;
	GLuint drawFramebuffer;
	GLint srcX0;
	GLint srcY0;
	GLint srcX1;
	GLint srcY1;
	GLint dstX0;
	GLint dstY0;
	GLint dstX1;
	GLint dstY1;
	GLbitfield mask;
	GLenum filter;

	static void APIENTRY execute(const RenderCommand_blitNamedFramebuffer* __restrict params) {
		glBlitNamedFramebuffer(
			params->readFramebuffer,
			params->drawFramebuffer,
			params->srcX0,
			params->srcY0,
			params->srcX1,
			params->srcY1,
			params->dstX0,
			params->dstY0,
			params->dstX1,
			params->dstY1,
			params->mask,
			params->filter
		);
	}
};
struct RenderCommand_checkNamedFramebufferStatus : public RenderCommandBase {
	GLuint framebuffer;
	GLenum target;

	static void APIENTRY execute(const RenderCommand_checkNamedFramebufferStatus* __restrict params) {
		glCheckNamedFramebufferStatus(
			params->framebuffer,
			params->target
		);
	}
};
struct RenderCommand_getNamedFramebufferParameteriv : public RenderCommandBase {
	GLuint framebuffer;
	GLenum pname;
	GLint *param;

	static void APIENTRY execute(const RenderCommand_getNamedFramebufferParameteriv* __restrict params) {
		glGetNamedFramebufferParameteriv(
			params->framebuffer,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_getNamedFramebufferAttachmentParameteriv : public RenderCommandBase {
	GLuint framebuffer;
	GLenum attachment;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getNamedFramebufferAttachmentParameteriv* __restrict params) {
		glGetNamedFramebufferAttachmentParameteriv(
			params->framebuffer,
			params->attachment,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_createRenderbuffers : public RenderCommandBase {
	GLsizei n;
	GLuint *renderbuffers;

	static void APIENTRY execute(const RenderCommand_createRenderbuffers* __restrict params) {
		glCreateRenderbuffers(
			params->n,
			params->renderbuffers
		);
	}
};
struct RenderCommand_namedRenderbufferStorage : public RenderCommandBase {
	GLuint renderbuffer;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_namedRenderbufferStorage* __restrict params) {
		glNamedRenderbufferStorage(
			params->renderbuffer,
			params->internalformat,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_namedRenderbufferStorageMultisample : public RenderCommandBase {
	GLuint renderbuffer;
	GLsizei samples;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_namedRenderbufferStorageMultisample* __restrict params) {
		glNamedRenderbufferStorageMultisample(
			params->renderbuffer,
			params->samples,
			params->internalformat,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_getNamedRenderbufferParameteriv : public RenderCommandBase {
	GLuint renderbuffer;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getNamedRenderbufferParameteriv* __restrict params) {
		glGetNamedRenderbufferParameteriv(
			params->renderbuffer,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_createTextures : public RenderCommandBase {
	GLenum target;
	GLsizei n;
	GLuint *textures;

	static void APIENTRY execute(const RenderCommand_createTextures* __restrict params) {
		glCreateTextures(
			params->target,
			params->n,
			params->textures
		);
	}
};
struct RenderCommand_textureBuffer : public RenderCommandBase {
	GLuint texture;
	GLenum internalformat;
	GLuint buffer;

	static void APIENTRY execute(const RenderCommand_textureBuffer* __restrict params) {
		glTextureBuffer(
			params->texture,
			params->internalformat,
			params->buffer
		);
	}
};
struct RenderCommand_textureBufferRange : public RenderCommandBase {
	GLuint texture;
	GLenum internalformat;
	GLuint buffer;
	GLintptr offset;
	GLsizeiptr size;

	static void APIENTRY execute(const RenderCommand_textureBufferRange* __restrict params) {
		glTextureBufferRange(
			params->texture,
			params->internalformat,
			params->buffer,
			params->offset,
			params->size
		);
	}
};
struct RenderCommand_textureStorage1D : public RenderCommandBase {
	GLuint texture;
	GLsizei levels;
	GLenum internalformat;
	GLsizei width;

	static void APIENTRY execute(const RenderCommand_textureStorage1D* __restrict params) {
		glTextureStorage1D(
			params->texture,
			params->levels,
			params->internalformat,
			params->width
		);
	}
};
struct RenderCommand_textureStorage2D : public RenderCommandBase {
	GLuint texture;
	GLsizei levels;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_textureStorage2D* __restrict params) {
		glTextureStorage2D(
			params->texture,
			params->levels,
			params->internalformat,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_textureStorage3D : public RenderCommandBase {
	GLuint texture;
	GLsizei levels;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;
	GLsizei depth;

	static void APIENTRY execute(const RenderCommand_textureStorage3D* __restrict params) {
		glTextureStorage3D(
			params->texture,
			params->levels,
			params->internalformat,
			params->width,
			params->height,
			params->depth
		);
	}
};
struct RenderCommand_textureStorage2DMultisample : public RenderCommandBase {
	GLuint texture;
	GLsizei samples;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;
	GLboolean fixedsamplelocations;

	static void APIENTRY execute(const RenderCommand_textureStorage2DMultisample* __restrict params) {
		glTextureStorage2DMultisample(
			params->texture,
			params->samples,
			params->internalformat,
			params->width,
			params->height,
			params->fixedsamplelocations
		);
	}
};
struct RenderCommand_textureStorage3DMultisample : public RenderCommandBase {
	GLuint texture;
	GLsizei samples;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLboolean fixedsamplelocations;

	static void APIENTRY execute(const RenderCommand_textureStorage3DMultisample* __restrict params) {
		glTextureStorage3DMultisample(
			params->texture,
			params->samples,
			params->internalformat,
			params->width,
			params->height,
			params->depth,
			params->fixedsamplelocations
		);
	}
};
struct RenderCommand_textureSubImage1D : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLint xoffset;
	GLsizei width;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_textureSubImage1D* __restrict params) {
		glTextureSubImage1D(
			params->texture,
			params->level,
			params->xoffset,
			params->width,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_textureSubImage2D : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLsizei width;
	GLsizei height;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_textureSubImage2D* __restrict params) {
		glTextureSubImage2D(
			params->texture,
			params->level,
			params->xoffset,
			params->yoffset,
			params->width,
			params->height,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_textureSubImage3D : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint zoffset;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_textureSubImage3D* __restrict params) {
		glTextureSubImage3D(
			params->texture,
			params->level,
			params->xoffset,
			params->yoffset,
			params->zoffset,
			params->width,
			params->height,
			params->depth,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_compressedTextureSubImage1D : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLint xoffset;
	GLsizei width;
	GLenum format;
	GLsizei imageSize;
	const void *data;

	static void APIENTRY execute(const RenderCommand_compressedTextureSubImage1D* __restrict params) {
		glCompressedTextureSubImage1D(
			params->texture,
			params->level,
			params->xoffset,
			params->width,
			params->format,
			params->imageSize,
			params->data
		);
	}
};
struct RenderCommand_compressedTextureSubImage2D : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLsizei width;
	GLsizei height;
	GLenum format;
	GLsizei imageSize;
	const void *data;

	static void APIENTRY execute(const RenderCommand_compressedTextureSubImage2D* __restrict params) {
		glCompressedTextureSubImage2D(
			params->texture,
			params->level,
			params->xoffset,
			params->yoffset,
			params->width,
			params->height,
			params->format,
			params->imageSize,
			params->data
		);
	}
};
struct RenderCommand_compressedTextureSubImage3D : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint zoffset;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLenum format;
	GLsizei imageSize;
	const void *data;

	static void APIENTRY execute(const RenderCommand_compressedTextureSubImage3D* __restrict params) {
		glCompressedTextureSubImage3D(
			params->texture,
			params->level,
			params->xoffset,
			params->yoffset,
			params->zoffset,
			params->width,
			params->height,
			params->depth,
			params->format,
			params->imageSize,
			params->data
		);
	}
};
struct RenderCommand_copyTextureSubImage1D : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLint xoffset;
	GLint x;
	GLint y;
	GLsizei width;

	static void APIENTRY execute(const RenderCommand_copyTextureSubImage1D* __restrict params) {
		glCopyTextureSubImage1D(
			params->texture,
			params->level,
			params->xoffset,
			params->x,
			params->y,
			params->width
		);
	}
};
struct RenderCommand_copyTextureSubImage2D : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_copyTextureSubImage2D* __restrict params) {
		glCopyTextureSubImage2D(
			params->texture,
			params->level,
			params->xoffset,
			params->yoffset,
			params->x,
			params->y,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_copyTextureSubImage3D : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint zoffset;
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_copyTextureSubImage3D* __restrict params) {
		glCopyTextureSubImage3D(
			params->texture,
			params->level,
			params->xoffset,
			params->yoffset,
			params->zoffset,
			params->x,
			params->y,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_textureParameterf : public RenderCommandBase {
	GLuint texture;
	GLenum pname;
	GLfloat param;

	static void APIENTRY execute(const RenderCommand_textureParameterf* __restrict params) {
		glTextureParameterf(
			params->texture,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_textureParameterfv : public RenderCommandBase {
	GLuint texture;
	GLenum pname;
	const GLfloat *param;

	static void APIENTRY execute(const RenderCommand_textureParameterfv* __restrict params) {
		glTextureParameterfv(
			params->texture,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_textureParameteri : public RenderCommandBase {
	GLuint texture;
	GLenum pname;
	GLint param;

	static void APIENTRY execute(const RenderCommand_textureParameteri* __restrict params) {
		glTextureParameteri(
			params->texture,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_textureParameterIiv : public RenderCommandBase {
	GLuint texture;
	GLenum pname;
	const GLint *params;

	static void APIENTRY execute(const RenderCommand_textureParameterIiv* __restrict params) {
		glTextureParameterIiv(
			params->texture,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_textureParameterIuiv : public RenderCommandBase {
	GLuint texture;
	GLenum pname;
	const GLuint *params;

	static void APIENTRY execute(const RenderCommand_textureParameterIuiv* __restrict params) {
		glTextureParameterIuiv(
			params->texture,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_textureParameteriv : public RenderCommandBase {
	GLuint texture;
	GLenum pname;
	const GLint *param;

	static void APIENTRY execute(const RenderCommand_textureParameteriv* __restrict params) {
		glTextureParameteriv(
			params->texture,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_generateTextureMipmap : public RenderCommandBase {
	GLuint texture;

	static void APIENTRY execute(const RenderCommand_generateTextureMipmap* __restrict params) {
		glGenerateTextureMipmap(
			params->texture
		);
	}
};
struct RenderCommand_bindTextureUnit : public RenderCommandBase {
	GLuint unit;
	GLuint texture;

	static void APIENTRY execute(const RenderCommand_bindTextureUnit* __restrict params) {
		glBindTextureUnit(
			params->unit,
			params->texture
		);
	}
};
struct RenderCommand_getTextureImage : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLenum format;
	GLenum type;
	GLsizei bufSize;
	void *pixels;

	static void APIENTRY execute(const RenderCommand_getTextureImage* __restrict params) {
		glGetTextureImage(
			params->texture,
			params->level,
			params->format,
			params->type,
			params->bufSize,
			params->pixels
		);
	}
};
struct RenderCommand_getCompressedTextureImage : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLsizei bufSize;
	void *pixels;

	static void APIENTRY execute(const RenderCommand_getCompressedTextureImage* __restrict params) {
		glGetCompressedTextureImage(
			params->texture,
			params->level,
			params->bufSize,
			params->pixels
		);
	}
};
struct RenderCommand_getTextureLevelParameterfv : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLenum pname;
	GLfloat *params;

	static void APIENTRY execute(const RenderCommand_getTextureLevelParameterfv* __restrict params) {
		glGetTextureLevelParameterfv(
			params->texture,
			params->level,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getTextureLevelParameteriv : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getTextureLevelParameteriv* __restrict params) {
		glGetTextureLevelParameteriv(
			params->texture,
			params->level,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getTextureParameterfv : public RenderCommandBase {
	GLuint texture;
	GLenum pname;
	GLfloat *params;

	static void APIENTRY execute(const RenderCommand_getTextureParameterfv* __restrict params) {
		glGetTextureParameterfv(
			params->texture,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getTextureParameterIiv : public RenderCommandBase {
	GLuint texture;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getTextureParameterIiv* __restrict params) {
		glGetTextureParameterIiv(
			params->texture,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getTextureParameterIuiv : public RenderCommandBase {
	GLuint texture;
	GLenum pname;
	GLuint *params;

	static void APIENTRY execute(const RenderCommand_getTextureParameterIuiv* __restrict params) {
		glGetTextureParameterIuiv(
			params->texture,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getTextureParameteriv : public RenderCommandBase {
	GLuint texture;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getTextureParameteriv* __restrict params) {
		glGetTextureParameteriv(
			params->texture,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_createVertexArrays : public RenderCommandBase {
	GLsizei n;
	GLuint *arrays;

	static void APIENTRY execute(const RenderCommand_createVertexArrays* __restrict params) {
		glCreateVertexArrays(
			params->n,
			params->arrays
		);
	}
};
struct RenderCommand_disableVertexArrayAttrib : public RenderCommandBase {
	GLuint vaobj;
	GLuint index;

	static void APIENTRY execute(const RenderCommand_disableVertexArrayAttrib* __restrict params) {
		glDisableVertexArrayAttrib(
			params->vaobj,
			params->index
		);
	}
};
struct RenderCommand_enableVertexArrayAttrib : public RenderCommandBase {
	GLuint vaobj;
	GLuint index;

	static void APIENTRY execute(const RenderCommand_enableVertexArrayAttrib* __restrict params) {
		glEnableVertexArrayAttrib(
			params->vaobj,
			params->index
		);
	}
};
struct RenderCommand_vertexArrayElementBuffer : public RenderCommandBase {
	GLuint vaobj;
	GLuint buffer;

	static void APIENTRY execute(const RenderCommand_vertexArrayElementBuffer* __restrict params) {
		glVertexArrayElementBuffer(
			params->vaobj,
			params->buffer
		);
	}
};
struct RenderCommand_vertexArrayVertexBuffer : public RenderCommandBase {
	GLuint vaobj;
	GLuint bindingindex;
	GLuint buffer;
	GLintptr offset;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_vertexArrayVertexBuffer* __restrict params) {
		glVertexArrayVertexBuffer(
			params->vaobj,
			params->bindingindex,
			params->buffer,
			params->offset,
			params->stride
		);
	}
};
struct RenderCommand_vertexArrayVertexBuffers : public RenderCommandBase {
	GLuint vaobj;
	GLuint first;
	GLsizei count;
	const GLuint *buffers;
	const GLintptr *offsets;
	const GLsizei *strides;

	static void APIENTRY execute(const RenderCommand_vertexArrayVertexBuffers* __restrict params) {
		glVertexArrayVertexBuffers(
			params->vaobj,
			params->first,
			params->count,
			params->buffers,
			params->offsets,
			params->strides
		);
	}
};
struct RenderCommand_vertexArrayAttribBinding : public RenderCommandBase {
	GLuint vaobj;
	GLuint attribindex;
	GLuint bindingindex;

	static void APIENTRY execute(const RenderCommand_vertexArrayAttribBinding* __restrict params) {
		glVertexArrayAttribBinding(
			params->vaobj,
			params->attribindex,
			params->bindingindex
		);
	}
};
struct RenderCommand_vertexArrayAttribFormat : public RenderCommandBase {
	GLuint vaobj;
	GLuint attribindex;
	GLint size;
	GLenum type;
	GLboolean normalized;
	GLuint relativeoffset;

	static void APIENTRY execute(const RenderCommand_vertexArrayAttribFormat* __restrict params) {
		glVertexArrayAttribFormat(
			params->vaobj,
			params->attribindex,
			params->size,
			params->type,
			params->normalized,
			params->relativeoffset
		);
	}
};
struct RenderCommand_vertexArrayAttribIFormat : public RenderCommandBase {
	GLuint vaobj;
	GLuint attribindex;
	GLint size;
	GLenum type;
	GLuint relativeoffset;

	static void APIENTRY execute(const RenderCommand_vertexArrayAttribIFormat* __restrict params) {
		glVertexArrayAttribIFormat(
			params->vaobj,
			params->attribindex,
			params->size,
			params->type,
			params->relativeoffset
		);
	}
};
struct RenderCommand_vertexArrayAttribLFormat : public RenderCommandBase {
	GLuint vaobj;
	GLuint attribindex;
	GLint size;
	GLenum type;
	GLuint relativeoffset;

	static void APIENTRY execute(const RenderCommand_vertexArrayAttribLFormat* __restrict params) {
		glVertexArrayAttribLFormat(
			params->vaobj,
			params->attribindex,
			params->size,
			params->type,
			params->relativeoffset
		);
	}
};
struct RenderCommand_vertexArrayBindingDivisor : public RenderCommandBase {
	GLuint vaobj;
	GLuint bindingindex;
	GLuint divisor;

	static void APIENTRY execute(const RenderCommand_vertexArrayBindingDivisor* __restrict params) {
		glVertexArrayBindingDivisor(
			params->vaobj,
			params->bindingindex,
			params->divisor
		);
	}
};
struct RenderCommand_getVertexArrayiv : public RenderCommandBase {
	GLuint vaobj;
	GLenum pname;
	GLint *param;

	static void APIENTRY execute(const RenderCommand_getVertexArrayiv* __restrict params) {
		glGetVertexArrayiv(
			params->vaobj,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_getVertexArrayIndexediv : public RenderCommandBase {
	GLuint vaobj;
	GLuint index;
	GLenum pname;
	GLint *param;

	static void APIENTRY execute(const RenderCommand_getVertexArrayIndexediv* __restrict params) {
		glGetVertexArrayIndexediv(
			params->vaobj,
			params->index,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_getVertexArrayIndexed64iv : public RenderCommandBase {
	GLuint vaobj;
	GLuint index;
	GLenum pname;
	GLint64 *param;

	static void APIENTRY execute(const RenderCommand_getVertexArrayIndexed64iv* __restrict params) {
		glGetVertexArrayIndexed64iv(
			params->vaobj,
			params->index,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_createSamplers : public RenderCommandBase {
	GLsizei n;
	GLuint *samplers;

	static void APIENTRY execute(const RenderCommand_createSamplers* __restrict params) {
		glCreateSamplers(
			params->n,
			params->samplers
		);
	}
};
struct RenderCommand_createProgramPipelines : public RenderCommandBase {
	GLsizei n;
	GLuint *pipelines;

	static void APIENTRY execute(const RenderCommand_createProgramPipelines* __restrict params) {
		glCreateProgramPipelines(
			params->n,
			params->pipelines
		);
	}
};
struct RenderCommand_createQueries : public RenderCommandBase {
	GLenum target;
	GLsizei n;
	GLuint *ids;

	static void APIENTRY execute(const RenderCommand_createQueries* __restrict params) {
		glCreateQueries(
			params->target,
			params->n,
			params->ids
		);
	}
};
struct RenderCommand_getQueryBufferObjecti64v : public RenderCommandBase {
	GLuint id;
	GLuint buffer;
	GLenum pname;
	GLintptr offset;

	static void APIENTRY execute(const RenderCommand_getQueryBufferObjecti64v* __restrict params) {
		glGetQueryBufferObjecti64v(
			params->id,
			params->buffer,
			params->pname,
			params->offset
		);
	}
};
struct RenderCommand_getQueryBufferObjectiv : public RenderCommandBase {
	GLuint id;
	GLuint buffer;
	GLenum pname;
	GLintptr offset;

	static void APIENTRY execute(const RenderCommand_getQueryBufferObjectiv* __restrict params) {
		glGetQueryBufferObjectiv(
			params->id,
			params->buffer,
			params->pname,
			params->offset
		);
	}
};
struct RenderCommand_getQueryBufferObjectui64v : public RenderCommandBase {
	GLuint id;
	GLuint buffer;
	GLenum pname;
	GLintptr offset;

	static void APIENTRY execute(const RenderCommand_getQueryBufferObjectui64v* __restrict params) {
		glGetQueryBufferObjectui64v(
			params->id,
			params->buffer,
			params->pname,
			params->offset
		);
	}
};
struct RenderCommand_getQueryBufferObjectuiv : public RenderCommandBase {
	GLuint id;
	GLuint buffer;
	GLenum pname;
	GLintptr offset;

	static void APIENTRY execute(const RenderCommand_getQueryBufferObjectuiv* __restrict params) {
		glGetQueryBufferObjectuiv(
			params->id,
			params->buffer,
			params->pname,
			params->offset
		);
	}
};
struct RenderCommand_memoryBarrierByRegion : public RenderCommandBase {
	GLbitfield barriers;

	static void APIENTRY execute(const RenderCommand_memoryBarrierByRegion* __restrict params) {
		glMemoryBarrierByRegion(
			params->barriers
		);
	}
};
struct RenderCommand_getTextureSubImage : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint zoffset;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLenum format;
	GLenum type;
	GLsizei bufSize;
	void *pixels;

	static void APIENTRY execute(const RenderCommand_getTextureSubImage* __restrict params) {
		glGetTextureSubImage(
			params->texture,
			params->level,
			params->xoffset,
			params->yoffset,
			params->zoffset,
			params->width,
			params->height,
			params->depth,
			params->format,
			params->type,
			params->bufSize,
			params->pixels
		);
	}
};
struct RenderCommand_getCompressedTextureSubImage : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint zoffset;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLsizei bufSize;
	void *pixels;

	static void APIENTRY execute(const RenderCommand_getCompressedTextureSubImage* __restrict params) {
		glGetCompressedTextureSubImage(
			params->texture,
			params->level,
			params->xoffset,
			params->yoffset,
			params->zoffset,
			params->width,
			params->height,
			params->depth,
			params->bufSize,
			params->pixels
		);
	}
};
struct RenderCommand_getGraphicsResetStatus : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_getGraphicsResetStatus* __restrict params) {
		glGetGraphicsResetStatus(
		);
	}
};
struct RenderCommand_getnCompressedTexImage : public RenderCommandBase {
	GLenum target;
	GLint lod;
	GLsizei bufSize;
	void *pixels;

	static void APIENTRY execute(const RenderCommand_getnCompressedTexImage* __restrict params) {
		glGetnCompressedTexImage(
			params->target,
			params->lod,
			params->bufSize,
			params->pixels
		);
	}
};
struct RenderCommand_getnTexImage : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLenum format;
	GLenum type;
	GLsizei bufSize;
	void *pixels;

	static void APIENTRY execute(const RenderCommand_getnTexImage* __restrict params) {
		glGetnTexImage(
			params->target,
			params->level,
			params->format,
			params->type,
			params->bufSize,
			params->pixels
		);
	}
};
struct RenderCommand_getnUniformdv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei bufSize;
	GLdouble *params;

	static void APIENTRY execute(const RenderCommand_getnUniformdv* __restrict params) {
		glGetnUniformdv(
			params->program,
			params->location,
			params->bufSize,
			params->params
		);
	}
};
struct RenderCommand_getnUniformfv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei bufSize;
	GLfloat *params;

	static void APIENTRY execute(const RenderCommand_getnUniformfv* __restrict params) {
		glGetnUniformfv(
			params->program,
			params->location,
			params->bufSize,
			params->params
		);
	}
};
struct RenderCommand_getnUniformiv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei bufSize;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getnUniformiv* __restrict params) {
		glGetnUniformiv(
			params->program,
			params->location,
			params->bufSize,
			params->params
		);
	}
};
struct RenderCommand_getnUniformuiv : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei bufSize;
	GLuint *params;

	static void APIENTRY execute(const RenderCommand_getnUniformuiv* __restrict params) {
		glGetnUniformuiv(
			params->program,
			params->location,
			params->bufSize,
			params->params
		);
	}
};
struct RenderCommand_readnPixels : public RenderCommandBase {
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;
	GLenum format;
	GLenum type;
	GLsizei bufSize;
	void *data;

	static void APIENTRY execute(const RenderCommand_readnPixels* __restrict params) {
		glReadnPixels(
			params->x,
			params->y,
			params->width,
			params->height,
			params->format,
			params->type,
			params->bufSize,
			params->data
		);
	}
};
struct RenderCommand_textureBarrier : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_textureBarrier* __restrict params) {
		glTextureBarrier(
		);
	}
};
struct RenderCommand_specializeShader : public RenderCommandBase {
	GLuint shader;
	const GLchar *pEntryPoint;
	GLuint numSpecializationConstants;
	const GLuint *pConstantIndex;
	const GLuint *pConstantValue;

	static void APIENTRY execute(const RenderCommand_specializeShader* __restrict params) {
		glSpecializeShader(
			params->shader,
			params->pEntryPoint,
			params->numSpecializationConstants,
			params->pConstantIndex,
			params->pConstantValue
		);
	}
};
struct RenderCommand_multiDrawArraysIndirectCount : public RenderCommandBase {
	GLenum mode;
	const void *indirect;
	GLintptr drawcount;
	GLsizei maxdrawcount;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_multiDrawArraysIndirectCount* __restrict params) {
		glMultiDrawArraysIndirectCount(
			params->mode,
			params->indirect,
			params->drawcount,
			params->maxdrawcount,
			params->stride
		);
	}
};
struct RenderCommand_multiDrawElementsIndirectCount : public RenderCommandBase {
	GLenum mode;
	GLenum type;
	const void *indirect;
	GLintptr drawcount;
	GLsizei maxdrawcount;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_multiDrawElementsIndirectCount* __restrict params) {
		glMultiDrawElementsIndirectCount(
			params->mode,
			params->type,
			params->indirect,
			params->drawcount,
			params->maxdrawcount,
			params->stride
		);
	}
};
struct RenderCommand_polygonOffsetClamp : public RenderCommandBase {
	GLfloat factor;
	GLfloat units;
	GLfloat clamp;

	static void APIENTRY execute(const RenderCommand_polygonOffsetClamp* __restrict params) {
		glPolygonOffsetClamp(
			params->factor,
			params->units,
			params->clamp
		);
	}
};
struct RenderCommand_primitiveBoundingBoxARB : public RenderCommandBase {
	GLfloat minX;
	GLfloat minY;
	GLfloat minZ;
	GLfloat minW;
	GLfloat maxX;
	GLfloat maxY;
	GLfloat maxZ;
	GLfloat maxW;

	static void APIENTRY execute(const RenderCommand_primitiveBoundingBoxARB* __restrict params) {
		glPrimitiveBoundingBoxARB(
			params->minX,
			params->minY,
			params->minZ,
			params->minW,
			params->maxX,
			params->maxY,
			params->maxZ,
			params->maxW
		);
	}
};
struct RenderCommand_getTextureHandleARB : public RenderCommandBase {
	GLuint texture;

	static void APIENTRY execute(const RenderCommand_getTextureHandleARB* __restrict params) {
		glGetTextureHandleARB(
			params->texture
		);
	}
};
struct RenderCommand_getTextureSamplerHandleARB : public RenderCommandBase {
	GLuint texture;
	GLuint sampler;

	static void APIENTRY execute(const RenderCommand_getTextureSamplerHandleARB* __restrict params) {
		glGetTextureSamplerHandleARB(
			params->texture,
			params->sampler
		);
	}
};
struct RenderCommand_makeTextureHandleResidentARB : public RenderCommandBase {
	GLuint64 handle;

	static void APIENTRY execute(const RenderCommand_makeTextureHandleResidentARB* __restrict params) {
		glMakeTextureHandleResidentARB(
			params->handle
		);
	}
};
struct RenderCommand_makeTextureHandleNonResidentARB : public RenderCommandBase {
	GLuint64 handle;

	static void APIENTRY execute(const RenderCommand_makeTextureHandleNonResidentARB* __restrict params) {
		glMakeTextureHandleNonResidentARB(
			params->handle
		);
	}
};
struct RenderCommand_getImageHandleARB : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLboolean layered;
	GLint layer;
	GLenum format;

	static void APIENTRY execute(const RenderCommand_getImageHandleARB* __restrict params) {
		glGetImageHandleARB(
			params->texture,
			params->level,
			params->layered,
			params->layer,
			params->format
		);
	}
};
struct RenderCommand_makeImageHandleResidentARB : public RenderCommandBase {
	GLuint64 handle;
	GLenum access;

	static void APIENTRY execute(const RenderCommand_makeImageHandleResidentARB* __restrict params) {
		glMakeImageHandleResidentARB(
			params->handle,
			params->access
		);
	}
};
struct RenderCommand_makeImageHandleNonResidentARB : public RenderCommandBase {
	GLuint64 handle;

	static void APIENTRY execute(const RenderCommand_makeImageHandleNonResidentARB* __restrict params) {
		glMakeImageHandleNonResidentARB(
			params->handle
		);
	}
};
struct RenderCommand_uniformHandleui64ARB : public RenderCommandBase {
	GLint location;
	GLuint64 value;

	static void APIENTRY execute(const RenderCommand_uniformHandleui64ARB* __restrict params) {
		glUniformHandleui64ARB(
			params->location,
			params->value
		);
	}
};
struct RenderCommand_uniformHandleui64vARB : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLuint64 *value;

	static void APIENTRY execute(const RenderCommand_uniformHandleui64vARB* __restrict params) {
		glUniformHandleui64vARB(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniformHandleui64ARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint64 value;

	static void APIENTRY execute(const RenderCommand_programUniformHandleui64ARB* __restrict params) {
		glProgramUniformHandleui64ARB(
			params->program,
			params->location,
			params->value
		);
	}
};
struct RenderCommand_programUniformHandleui64vARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLuint64 *values;

	static void APIENTRY execute(const RenderCommand_programUniformHandleui64vARB* __restrict params) {
		glProgramUniformHandleui64vARB(
			params->program,
			params->location,
			params->count,
			params->values
		);
	}
};
struct RenderCommand_isTextureHandleResidentARB : public RenderCommandBase {
	GLuint64 handle;

	static void APIENTRY execute(const RenderCommand_isTextureHandleResidentARB* __restrict params) {
		glIsTextureHandleResidentARB(
			params->handle
		);
	}
};
struct RenderCommand_isImageHandleResidentARB : public RenderCommandBase {
	GLuint64 handle;

	static void APIENTRY execute(const RenderCommand_isImageHandleResidentARB* __restrict params) {
		glIsImageHandleResidentARB(
			params->handle
		);
	}
};
struct RenderCommand_vertexAttribL1ui64ARB : public RenderCommandBase {
	GLuint index;
	GLuint64EXT x;

	static void APIENTRY execute(const RenderCommand_vertexAttribL1ui64ARB* __restrict params) {
		glVertexAttribL1ui64ARB(
			params->index,
			params->x
		);
	}
};
struct RenderCommand_vertexAttribL1ui64vARB : public RenderCommandBase {
	GLuint index;
	const GLuint64EXT *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribL1ui64vARB* __restrict params) {
		glVertexAttribL1ui64vARB(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_getVertexAttribLui64vARB : public RenderCommandBase {
	GLuint index;
	GLenum pname;
	GLuint64EXT *params;

	static void APIENTRY execute(const RenderCommand_getVertexAttribLui64vARB* __restrict params) {
		glGetVertexAttribLui64vARB(
			params->index,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_createSyncFromCLeventARB : public RenderCommandBase {
	struct _cl_context *context;
	struct _cl_event *event;
	GLbitfield flags;

	static void APIENTRY execute(const RenderCommand_createSyncFromCLeventARB* __restrict params) {
		glCreateSyncFromCLeventARB(
			params->context,
			params->event,
			params->flags
		);
	}
};
struct RenderCommand_dispatchComputeGroupSizeARB : public RenderCommandBase {
	GLuint num_groups_x;
	GLuint num_groups_y;
	GLuint num_groups_z;
	GLuint group_size_x;
	GLuint group_size_y;
	GLuint group_size_z;

	static void APIENTRY execute(const RenderCommand_dispatchComputeGroupSizeARB* __restrict params) {
		glDispatchComputeGroupSizeARB(
			params->num_groups_x,
			params->num_groups_y,
			params->num_groups_z,
			params->group_size_x,
			params->group_size_y,
			params->group_size_z
		);
	}
};
struct RenderCommand_debugMessageControlARB : public RenderCommandBase {
	GLenum source;
	GLenum type;
	GLenum severity;
	GLsizei count;
	const GLuint *ids;
	GLboolean enabled;

	static void APIENTRY execute(const RenderCommand_debugMessageControlARB* __restrict params) {
		glDebugMessageControlARB(
			params->source,
			params->type,
			params->severity,
			params->count,
			params->ids,
			params->enabled
		);
	}
};
struct RenderCommand_debugMessageInsertARB : public RenderCommandBase {
	GLenum source;
	GLenum type;
	GLuint id;
	GLenum severity;
	GLsizei length;
	const GLchar *buf;

	static void APIENTRY execute(const RenderCommand_debugMessageInsertARB* __restrict params) {
		glDebugMessageInsertARB(
			params->source,
			params->type,
			params->id,
			params->severity,
			params->length,
			params->buf
		);
	}
};
struct RenderCommand_debugMessageCallbackARB : public RenderCommandBase {
	GLDEBUGPROCARB callback;
	const void *userParam;

	static void APIENTRY execute(const RenderCommand_debugMessageCallbackARB* __restrict params) {
		glDebugMessageCallbackARB(
			params->callback,
			params->userParam
		);
	}
};
struct RenderCommand_getDebugMessageLogARB : public RenderCommandBase {
	GLuint count;
	GLsizei bufSize;
	GLenum *sources;
	GLenum *types;
	GLuint *ids;
	GLenum *severities;
	GLsizei *lengths;
	GLchar *messageLog;

	static void APIENTRY execute(const RenderCommand_getDebugMessageLogARB* __restrict params) {
		glGetDebugMessageLogARB(
			params->count,
			params->bufSize,
			params->sources,
			params->types,
			params->ids,
			params->severities,
			params->lengths,
			params->messageLog
		);
	}
};
struct RenderCommand_blendEquationiARB : public RenderCommandBase {
	GLuint buf;
	GLenum mode;

	static void APIENTRY execute(const RenderCommand_blendEquationiARB* __restrict params) {
		glBlendEquationiARB(
			params->buf,
			params->mode
		);
	}
};
struct RenderCommand_blendEquationSeparateiARB : public RenderCommandBase {
	GLuint buf;
	GLenum modeRGB;
	GLenum modeAlpha;

	static void APIENTRY execute(const RenderCommand_blendEquationSeparateiARB* __restrict params) {
		glBlendEquationSeparateiARB(
			params->buf,
			params->modeRGB,
			params->modeAlpha
		);
	}
};
struct RenderCommand_blendFunciARB : public RenderCommandBase {
	GLuint buf;
	GLenum src;
	GLenum dst;

	static void APIENTRY execute(const RenderCommand_blendFunciARB* __restrict params) {
		glBlendFunciARB(
			params->buf,
			params->src,
			params->dst
		);
	}
};
struct RenderCommand_blendFuncSeparateiARB : public RenderCommandBase {
	GLuint buf;
	GLenum srcRGB;
	GLenum dstRGB;
	GLenum srcAlpha;
	GLenum dstAlpha;

	static void APIENTRY execute(const RenderCommand_blendFuncSeparateiARB* __restrict params) {
		glBlendFuncSeparateiARB(
			params->buf,
			params->srcRGB,
			params->dstRGB,
			params->srcAlpha,
			params->dstAlpha
		);
	}
};
struct RenderCommand_drawArraysInstancedARB : public RenderCommandBase {
	GLenum mode;
	GLint first;
	GLsizei count;
	GLsizei primcount;

	static void APIENTRY execute(const RenderCommand_drawArraysInstancedARB* __restrict params) {
		glDrawArraysInstancedARB(
			params->mode,
			params->first,
			params->count,
			params->primcount
		);
	}
};
struct RenderCommand_drawElementsInstancedARB : public RenderCommandBase {
	GLenum mode;
	GLsizei count;
	GLenum type;
	const void *indices;
	GLsizei primcount;

	static void APIENTRY execute(const RenderCommand_drawElementsInstancedARB* __restrict params) {
		glDrawElementsInstancedARB(
			params->mode,
			params->count,
			params->type,
			params->indices,
			params->primcount
		);
	}
};
struct RenderCommand_programParameteriARB : public RenderCommandBase {
	GLuint program;
	GLenum pname;
	GLint value;

	static void APIENTRY execute(const RenderCommand_programParameteriARB* __restrict params) {
		glProgramParameteriARB(
			params->program,
			params->pname,
			params->value
		);
	}
};
struct RenderCommand_framebufferTextureARB : public RenderCommandBase {
	GLenum target;
	GLenum attachment;
	GLuint texture;
	GLint level;

	static void APIENTRY execute(const RenderCommand_framebufferTextureARB* __restrict params) {
		glFramebufferTextureARB(
			params->target,
			params->attachment,
			params->texture,
			params->level
		);
	}
};
struct RenderCommand_framebufferTextureLayerARB : public RenderCommandBase {
	GLenum target;
	GLenum attachment;
	GLuint texture;
	GLint level;
	GLint layer;

	static void APIENTRY execute(const RenderCommand_framebufferTextureLayerARB* __restrict params) {
		glFramebufferTextureLayerARB(
			params->target,
			params->attachment,
			params->texture,
			params->level,
			params->layer
		);
	}
};
struct RenderCommand_framebufferTextureFaceARB : public RenderCommandBase {
	GLenum target;
	GLenum attachment;
	GLuint texture;
	GLint level;
	GLenum face;

	static void APIENTRY execute(const RenderCommand_framebufferTextureFaceARB* __restrict params) {
		glFramebufferTextureFaceARB(
			params->target,
			params->attachment,
			params->texture,
			params->level,
			params->face
		);
	}
};
struct RenderCommand_specializeShaderARB : public RenderCommandBase {
	GLuint shader;
	const GLchar *pEntryPoint;
	GLuint numSpecializationConstants;
	const GLuint *pConstantIndex;
	const GLuint *pConstantValue;

	static void APIENTRY execute(const RenderCommand_specializeShaderARB* __restrict params) {
		glSpecializeShaderARB(
			params->shader,
			params->pEntryPoint,
			params->numSpecializationConstants,
			params->pConstantIndex,
			params->pConstantValue
		);
	}
};
struct RenderCommand_uniform1i64ARB : public RenderCommandBase {
	GLint location;
	GLint64 x;

	static void APIENTRY execute(const RenderCommand_uniform1i64ARB* __restrict params) {
		glUniform1i64ARB(
			params->location,
			params->x
		);
	}
};
struct RenderCommand_uniform2i64ARB : public RenderCommandBase {
	GLint location;
	GLint64 x;
	GLint64 y;

	static void APIENTRY execute(const RenderCommand_uniform2i64ARB* __restrict params) {
		glUniform2i64ARB(
			params->location,
			params->x,
			params->y
		);
	}
};
struct RenderCommand_uniform3i64ARB : public RenderCommandBase {
	GLint location;
	GLint64 x;
	GLint64 y;
	GLint64 z;

	static void APIENTRY execute(const RenderCommand_uniform3i64ARB* __restrict params) {
		glUniform3i64ARB(
			params->location,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_uniform4i64ARB : public RenderCommandBase {
	GLint location;
	GLint64 x;
	GLint64 y;
	GLint64 z;
	GLint64 w;

	static void APIENTRY execute(const RenderCommand_uniform4i64ARB* __restrict params) {
		glUniform4i64ARB(
			params->location,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_uniform1i64vARB : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLint64 *value;

	static void APIENTRY execute(const RenderCommand_uniform1i64vARB* __restrict params) {
		glUniform1i64vARB(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform2i64vARB : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLint64 *value;

	static void APIENTRY execute(const RenderCommand_uniform2i64vARB* __restrict params) {
		glUniform2i64vARB(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform3i64vARB : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLint64 *value;

	static void APIENTRY execute(const RenderCommand_uniform3i64vARB* __restrict params) {
		glUniform3i64vARB(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform4i64vARB : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLint64 *value;

	static void APIENTRY execute(const RenderCommand_uniform4i64vARB* __restrict params) {
		glUniform4i64vARB(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform1ui64ARB : public RenderCommandBase {
	GLint location;
	GLuint64 x;

	static void APIENTRY execute(const RenderCommand_uniform1ui64ARB* __restrict params) {
		glUniform1ui64ARB(
			params->location,
			params->x
		);
	}
};
struct RenderCommand_uniform2ui64ARB : public RenderCommandBase {
	GLint location;
	GLuint64 x;
	GLuint64 y;

	static void APIENTRY execute(const RenderCommand_uniform2ui64ARB* __restrict params) {
		glUniform2ui64ARB(
			params->location,
			params->x,
			params->y
		);
	}
};
struct RenderCommand_uniform3ui64ARB : public RenderCommandBase {
	GLint location;
	GLuint64 x;
	GLuint64 y;
	GLuint64 z;

	static void APIENTRY execute(const RenderCommand_uniform3ui64ARB* __restrict params) {
		glUniform3ui64ARB(
			params->location,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_uniform4ui64ARB : public RenderCommandBase {
	GLint location;
	GLuint64 x;
	GLuint64 y;
	GLuint64 z;
	GLuint64 w;

	static void APIENTRY execute(const RenderCommand_uniform4ui64ARB* __restrict params) {
		glUniform4ui64ARB(
			params->location,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_uniform1ui64vARB : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLuint64 *value;

	static void APIENTRY execute(const RenderCommand_uniform1ui64vARB* __restrict params) {
		glUniform1ui64vARB(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform2ui64vARB : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLuint64 *value;

	static void APIENTRY execute(const RenderCommand_uniform2ui64vARB* __restrict params) {
		glUniform2ui64vARB(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform3ui64vARB : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLuint64 *value;

	static void APIENTRY execute(const RenderCommand_uniform3ui64vARB* __restrict params) {
		glUniform3ui64vARB(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform4ui64vARB : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLuint64 *value;

	static void APIENTRY execute(const RenderCommand_uniform4ui64vARB* __restrict params) {
		glUniform4ui64vARB(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_getUniformi64vARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLint64 *params;

	static void APIENTRY execute(const RenderCommand_getUniformi64vARB* __restrict params) {
		glGetUniformi64vARB(
			params->program,
			params->location,
			params->params
		);
	}
};
struct RenderCommand_getUniformui64vARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint64 *params;

	static void APIENTRY execute(const RenderCommand_getUniformui64vARB* __restrict params) {
		glGetUniformui64vARB(
			params->program,
			params->location,
			params->params
		);
	}
};
struct RenderCommand_getnUniformi64vARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei bufSize;
	GLint64 *params;

	static void APIENTRY execute(const RenderCommand_getnUniformi64vARB* __restrict params) {
		glGetnUniformi64vARB(
			params->program,
			params->location,
			params->bufSize,
			params->params
		);
	}
};
struct RenderCommand_getnUniformui64vARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei bufSize;
	GLuint64 *params;

	static void APIENTRY execute(const RenderCommand_getnUniformui64vARB* __restrict params) {
		glGetnUniformui64vARB(
			params->program,
			params->location,
			params->bufSize,
			params->params
		);
	}
};
struct RenderCommand_programUniform1i64ARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLint64 x;

	static void APIENTRY execute(const RenderCommand_programUniform1i64ARB* __restrict params) {
		glProgramUniform1i64ARB(
			params->program,
			params->location,
			params->x
		);
	}
};
struct RenderCommand_programUniform2i64ARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLint64 x;
	GLint64 y;

	static void APIENTRY execute(const RenderCommand_programUniform2i64ARB* __restrict params) {
		glProgramUniform2i64ARB(
			params->program,
			params->location,
			params->x,
			params->y
		);
	}
};
struct RenderCommand_programUniform3i64ARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLint64 x;
	GLint64 y;
	GLint64 z;

	static void APIENTRY execute(const RenderCommand_programUniform3i64ARB* __restrict params) {
		glProgramUniform3i64ARB(
			params->program,
			params->location,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_programUniform4i64ARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLint64 x;
	GLint64 y;
	GLint64 z;
	GLint64 w;

	static void APIENTRY execute(const RenderCommand_programUniform4i64ARB* __restrict params) {
		glProgramUniform4i64ARB(
			params->program,
			params->location,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_programUniform1i64vARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLint64 *value;

	static void APIENTRY execute(const RenderCommand_programUniform1i64vARB* __restrict params) {
		glProgramUniform1i64vARB(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform2i64vARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLint64 *value;

	static void APIENTRY execute(const RenderCommand_programUniform2i64vARB* __restrict params) {
		glProgramUniform2i64vARB(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform3i64vARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLint64 *value;

	static void APIENTRY execute(const RenderCommand_programUniform3i64vARB* __restrict params) {
		glProgramUniform3i64vARB(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform4i64vARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLint64 *value;

	static void APIENTRY execute(const RenderCommand_programUniform4i64vARB* __restrict params) {
		glProgramUniform4i64vARB(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform1ui64ARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint64 x;

	static void APIENTRY execute(const RenderCommand_programUniform1ui64ARB* __restrict params) {
		glProgramUniform1ui64ARB(
			params->program,
			params->location,
			params->x
		);
	}
};
struct RenderCommand_programUniform2ui64ARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint64 x;
	GLuint64 y;

	static void APIENTRY execute(const RenderCommand_programUniform2ui64ARB* __restrict params) {
		glProgramUniform2ui64ARB(
			params->program,
			params->location,
			params->x,
			params->y
		);
	}
};
struct RenderCommand_programUniform3ui64ARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint64 x;
	GLuint64 y;
	GLuint64 z;

	static void APIENTRY execute(const RenderCommand_programUniform3ui64ARB* __restrict params) {
		glProgramUniform3ui64ARB(
			params->program,
			params->location,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_programUniform4ui64ARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint64 x;
	GLuint64 y;
	GLuint64 z;
	GLuint64 w;

	static void APIENTRY execute(const RenderCommand_programUniform4ui64ARB* __restrict params) {
		glProgramUniform4ui64ARB(
			params->program,
			params->location,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_programUniform1ui64vARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLuint64 *value;

	static void APIENTRY execute(const RenderCommand_programUniform1ui64vARB* __restrict params) {
		glProgramUniform1ui64vARB(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform2ui64vARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLuint64 *value;

	static void APIENTRY execute(const RenderCommand_programUniform2ui64vARB* __restrict params) {
		glProgramUniform2ui64vARB(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform3ui64vARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLuint64 *value;

	static void APIENTRY execute(const RenderCommand_programUniform3ui64vARB* __restrict params) {
		glProgramUniform3ui64vARB(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform4ui64vARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLuint64 *value;

	static void APIENTRY execute(const RenderCommand_programUniform4ui64vARB* __restrict params) {
		glProgramUniform4ui64vARB(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_multiDrawArraysIndirectCountARB : public RenderCommandBase {
	GLenum mode;
	const void *indirect;
	GLintptr drawcount;
	GLsizei maxdrawcount;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_multiDrawArraysIndirectCountARB* __restrict params) {
		glMultiDrawArraysIndirectCountARB(
			params->mode,
			params->indirect,
			params->drawcount,
			params->maxdrawcount,
			params->stride
		);
	}
};
struct RenderCommand_multiDrawElementsIndirectCountARB : public RenderCommandBase {
	GLenum mode;
	GLenum type;
	const void *indirect;
	GLintptr drawcount;
	GLsizei maxdrawcount;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_multiDrawElementsIndirectCountARB* __restrict params) {
		glMultiDrawElementsIndirectCountARB(
			params->mode,
			params->type,
			params->indirect,
			params->drawcount,
			params->maxdrawcount,
			params->stride
		);
	}
};
struct RenderCommand_vertexAttribDivisorARB : public RenderCommandBase {
	GLuint index;
	GLuint divisor;

	static void APIENTRY execute(const RenderCommand_vertexAttribDivisorARB* __restrict params) {
		glVertexAttribDivisorARB(
			params->index,
			params->divisor
		);
	}
};
struct RenderCommand_maxShaderCompilerThreadsARB : public RenderCommandBase {
	GLuint count;

	static void APIENTRY execute(const RenderCommand_maxShaderCompilerThreadsARB* __restrict params) {
		glMaxShaderCompilerThreadsARB(
			params->count
		);
	}
};
struct RenderCommand_getGraphicsResetStatusARB : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_getGraphicsResetStatusARB* __restrict params) {
		glGetGraphicsResetStatusARB(
		);
	}
};
struct RenderCommand_getnTexImageARB : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLenum format;
	GLenum type;
	GLsizei bufSize;
	void *img;

	static void APIENTRY execute(const RenderCommand_getnTexImageARB* __restrict params) {
		glGetnTexImageARB(
			params->target,
			params->level,
			params->format,
			params->type,
			params->bufSize,
			params->img
		);
	}
};
struct RenderCommand_readnPixelsARB : public RenderCommandBase {
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;
	GLenum format;
	GLenum type;
	GLsizei bufSize;
	void *data;

	static void APIENTRY execute(const RenderCommand_readnPixelsARB* __restrict params) {
		glReadnPixelsARB(
			params->x,
			params->y,
			params->width,
			params->height,
			params->format,
			params->type,
			params->bufSize,
			params->data
		);
	}
};
struct RenderCommand_getnCompressedTexImageARB : public RenderCommandBase {
	GLenum target;
	GLint lod;
	GLsizei bufSize;
	void *img;

	static void APIENTRY execute(const RenderCommand_getnCompressedTexImageARB* __restrict params) {
		glGetnCompressedTexImageARB(
			params->target,
			params->lod,
			params->bufSize,
			params->img
		);
	}
};
struct RenderCommand_getnUniformfvARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei bufSize;
	GLfloat *params;

	static void APIENTRY execute(const RenderCommand_getnUniformfvARB* __restrict params) {
		glGetnUniformfvARB(
			params->program,
			params->location,
			params->bufSize,
			params->params
		);
	}
};
struct RenderCommand_getnUniformivARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei bufSize;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getnUniformivARB* __restrict params) {
		glGetnUniformivARB(
			params->program,
			params->location,
			params->bufSize,
			params->params
		);
	}
};
struct RenderCommand_getnUniformuivARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei bufSize;
	GLuint *params;

	static void APIENTRY execute(const RenderCommand_getnUniformuivARB* __restrict params) {
		glGetnUniformuivARB(
			params->program,
			params->location,
			params->bufSize,
			params->params
		);
	}
};
struct RenderCommand_getnUniformdvARB : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei bufSize;
	GLdouble *params;

	static void APIENTRY execute(const RenderCommand_getnUniformdvARB* __restrict params) {
		glGetnUniformdvARB(
			params->program,
			params->location,
			params->bufSize,
			params->params
		);
	}
};
struct RenderCommand_framebufferSampleLocationsfvARB : public RenderCommandBase {
	GLenum target;
	GLuint start;
	GLsizei count;
	const GLfloat *v;

	static void APIENTRY execute(const RenderCommand_framebufferSampleLocationsfvARB* __restrict params) {
		glFramebufferSampleLocationsfvARB(
			params->target,
			params->start,
			params->count,
			params->v
		);
	}
};
struct RenderCommand_namedFramebufferSampleLocationsfvARB : public RenderCommandBase {
	GLuint framebuffer;
	GLuint start;
	GLsizei count;
	const GLfloat *v;

	static void APIENTRY execute(const RenderCommand_namedFramebufferSampleLocationsfvARB* __restrict params) {
		glNamedFramebufferSampleLocationsfvARB(
			params->framebuffer,
			params->start,
			params->count,
			params->v
		);
	}
};
struct RenderCommand_evaluateDepthValuesARB : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_evaluateDepthValuesARB* __restrict params) {
		glEvaluateDepthValuesARB(
		);
	}
};
struct RenderCommand_minSampleShadingARB : public RenderCommandBase {
	GLfloat value;

	static void APIENTRY execute(const RenderCommand_minSampleShadingARB* __restrict params) {
		glMinSampleShadingARB(
			params->value
		);
	}
};
struct RenderCommand_namedStringARB : public RenderCommandBase {
	GLenum type;
	GLint namelen;
	const GLchar *name;
	GLint stringlen;
	const GLchar *string;

	static void APIENTRY execute(const RenderCommand_namedStringARB* __restrict params) {
		glNamedStringARB(
			params->type,
			params->namelen,
			params->name,
			params->stringlen,
			params->string
		);
	}
};
struct RenderCommand_deleteNamedStringARB : public RenderCommandBase {
	GLint namelen;
	const GLchar *name;

	static void APIENTRY execute(const RenderCommand_deleteNamedStringARB* __restrict params) {
		glDeleteNamedStringARB(
			params->namelen,
			params->name
		);
	}
};
struct RenderCommand_compileShaderIncludeARB : public RenderCommandBase {
	GLuint shader;
	GLsizei count;
	const GLchar *const*path;
	const GLint *length;

	static void APIENTRY execute(const RenderCommand_compileShaderIncludeARB* __restrict params) {
		glCompileShaderIncludeARB(
			params->shader,
			params->count,
			params->path,
			params->length
		);
	}
};
struct RenderCommand_isNamedStringARB : public RenderCommandBase {
	GLint namelen;
	const GLchar *name;

	static void APIENTRY execute(const RenderCommand_isNamedStringARB* __restrict params) {
		glIsNamedStringARB(
			params->namelen,
			params->name
		);
	}
};
struct RenderCommand_getNamedStringARB : public RenderCommandBase {
	GLint namelen;
	const GLchar *name;
	GLsizei bufSize;
	GLint *stringlen;
	GLchar *string;

	static void APIENTRY execute(const RenderCommand_getNamedStringARB* __restrict params) {
		glGetNamedStringARB(
			params->namelen,
			params->name,
			params->bufSize,
			params->stringlen,
			params->string
		);
	}
};
struct RenderCommand_getNamedStringivARB : public RenderCommandBase {
	GLint namelen;
	const GLchar *name;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getNamedStringivARB* __restrict params) {
		glGetNamedStringivARB(
			params->namelen,
			params->name,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_bufferPageCommitmentARB : public RenderCommandBase {
	GLenum target;
	GLintptr offset;
	GLsizeiptr size;
	GLboolean commit;

	static void APIENTRY execute(const RenderCommand_bufferPageCommitmentARB* __restrict params) {
		glBufferPageCommitmentARB(
			params->target,
			params->offset,
			params->size,
			params->commit
		);
	}
};
struct RenderCommand_namedBufferPageCommitmentEXT : public RenderCommandBase {
	GLuint buffer;
	GLintptr offset;
	GLsizeiptr size;
	GLboolean commit;

	static void APIENTRY execute(const RenderCommand_namedBufferPageCommitmentEXT* __restrict params) {
		glNamedBufferPageCommitmentEXT(
			params->buffer,
			params->offset,
			params->size,
			params->commit
		);
	}
};
struct RenderCommand_namedBufferPageCommitmentARB : public RenderCommandBase {
	GLuint buffer;
	GLintptr offset;
	GLsizeiptr size;
	GLboolean commit;

	static void APIENTRY execute(const RenderCommand_namedBufferPageCommitmentARB* __restrict params) {
		glNamedBufferPageCommitmentARB(
			params->buffer,
			params->offset,
			params->size,
			params->commit
		);
	}
};
struct RenderCommand_texPageCommitmentARB : public RenderCommandBase {
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint zoffset;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLboolean commit;

	static void APIENTRY execute(const RenderCommand_texPageCommitmentARB* __restrict params) {
		glTexPageCommitmentARB(
			params->target,
			params->level,
			params->xoffset,
			params->yoffset,
			params->zoffset,
			params->width,
			params->height,
			params->depth,
			params->commit
		);
	}
};
struct RenderCommand_texBufferARB : public RenderCommandBase {
	GLenum target;
	GLenum internalformat;
	GLuint buffer;

	static void APIENTRY execute(const RenderCommand_texBufferARB* __restrict params) {
		glTexBufferARB(
			params->target,
			params->internalformat,
			params->buffer
		);
	}
};
struct RenderCommand_blendBarrierKHR : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_blendBarrierKHR* __restrict params) {
		glBlendBarrierKHR(
		);
	}
};
struct RenderCommand_maxShaderCompilerThreadsKHR : public RenderCommandBase {
	GLuint count;

	static void APIENTRY execute(const RenderCommand_maxShaderCompilerThreadsKHR* __restrict params) {
		glMaxShaderCompilerThreadsKHR(
			params->count
		);
	}
};
struct RenderCommand_renderbufferStorageMultisampleAdvancedAMD : public RenderCommandBase {
	GLenum target;
	GLsizei samples;
	GLsizei storageSamples;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_renderbufferStorageMultisampleAdvancedAMD* __restrict params) {
		glRenderbufferStorageMultisampleAdvancedAMD(
			params->target,
			params->samples,
			params->storageSamples,
			params->internalformat,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_namedRenderbufferStorageMultisampleAdvancedAMD : public RenderCommandBase {
	GLuint renderbuffer;
	GLsizei samples;
	GLsizei storageSamples;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_namedRenderbufferStorageMultisampleAdvancedAMD* __restrict params) {
		glNamedRenderbufferStorageMultisampleAdvancedAMD(
			params->renderbuffer,
			params->samples,
			params->storageSamples,
			params->internalformat,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_getPerfMonitorGroupsAMD : public RenderCommandBase {
	GLint *numGroups;
	GLsizei groupsSize;
	GLuint *groups;

	static void APIENTRY execute(const RenderCommand_getPerfMonitorGroupsAMD* __restrict params) {
		glGetPerfMonitorGroupsAMD(
			params->numGroups,
			params->groupsSize,
			params->groups
		);
	}
};
struct RenderCommand_getPerfMonitorCountersAMD : public RenderCommandBase {
	GLuint group;
	GLint *numCounters;
	GLint *maxActiveCounters;
	GLsizei counterSize;
	GLuint *counters;

	static void APIENTRY execute(const RenderCommand_getPerfMonitorCountersAMD* __restrict params) {
		glGetPerfMonitorCountersAMD(
			params->group,
			params->numCounters,
			params->maxActiveCounters,
			params->counterSize,
			params->counters
		);
	}
};
struct RenderCommand_getPerfMonitorGroupStringAMD : public RenderCommandBase {
	GLuint group;
	GLsizei bufSize;
	GLsizei *length;
	GLchar *groupString;

	static void APIENTRY execute(const RenderCommand_getPerfMonitorGroupStringAMD* __restrict params) {
		glGetPerfMonitorGroupStringAMD(
			params->group,
			params->bufSize,
			params->length,
			params->groupString
		);
	}
};
struct RenderCommand_getPerfMonitorCounterStringAMD : public RenderCommandBase {
	GLuint group;
	GLuint counter;
	GLsizei bufSize;
	GLsizei *length;
	GLchar *counterString;

	static void APIENTRY execute(const RenderCommand_getPerfMonitorCounterStringAMD* __restrict params) {
		glGetPerfMonitorCounterStringAMD(
			params->group,
			params->counter,
			params->bufSize,
			params->length,
			params->counterString
		);
	}
};
struct RenderCommand_getPerfMonitorCounterInfoAMD : public RenderCommandBase {
	GLuint group;
	GLuint counter;
	GLenum pname;
	void *data;

	static void APIENTRY execute(const RenderCommand_getPerfMonitorCounterInfoAMD* __restrict params) {
		glGetPerfMonitorCounterInfoAMD(
			params->group,
			params->counter,
			params->pname,
			params->data
		);
	}
};
struct RenderCommand_genPerfMonitorsAMD : public RenderCommandBase {
	GLsizei n;
	GLuint *monitors;

	static void APIENTRY execute(const RenderCommand_genPerfMonitorsAMD* __restrict params) {
		glGenPerfMonitorsAMD(
			params->n,
			params->monitors
		);
	}
};
struct RenderCommand_deletePerfMonitorsAMD : public RenderCommandBase {
	GLsizei n;
	GLuint *monitors;

	static void APIENTRY execute(const RenderCommand_deletePerfMonitorsAMD* __restrict params) {
		glDeletePerfMonitorsAMD(
			params->n,
			params->monitors
		);
	}
};
struct RenderCommand_selectPerfMonitorCountersAMD : public RenderCommandBase {
	GLuint monitor;
	GLboolean enable;
	GLuint group;
	GLint numCounters;
	GLuint *counterList;

	static void APIENTRY execute(const RenderCommand_selectPerfMonitorCountersAMD* __restrict params) {
		glSelectPerfMonitorCountersAMD(
			params->monitor,
			params->enable,
			params->group,
			params->numCounters,
			params->counterList
		);
	}
};
struct RenderCommand_beginPerfMonitorAMD : public RenderCommandBase {
	GLuint monitor;

	static void APIENTRY execute(const RenderCommand_beginPerfMonitorAMD* __restrict params) {
		glBeginPerfMonitorAMD(
			params->monitor
		);
	}
};
struct RenderCommand_endPerfMonitorAMD : public RenderCommandBase {
	GLuint monitor;

	static void APIENTRY execute(const RenderCommand_endPerfMonitorAMD* __restrict params) {
		glEndPerfMonitorAMD(
			params->monitor
		);
	}
};
struct RenderCommand_getPerfMonitorCounterDataAMD : public RenderCommandBase {
	GLuint monitor;
	GLenum pname;
	GLsizei dataSize;
	GLuint *data;
	GLint *bytesWritten;

	static void APIENTRY execute(const RenderCommand_getPerfMonitorCounterDataAMD* __restrict params) {
		glGetPerfMonitorCounterDataAMD(
			params->monitor,
			params->pname,
			params->dataSize,
			params->data,
			params->bytesWritten
		);
	}
};
struct RenderCommand_eGLImageTargetTexStorageEXT : public RenderCommandBase {
	GLenum target;
	GLeglImageOES image;
	const GLint* attrib_list;

	static void APIENTRY execute(const RenderCommand_eGLImageTargetTexStorageEXT* __restrict params) {
		glEGLImageTargetTexStorageEXT(
			params->target,
			params->image,
			params->attrib_list
		);
	}
};
struct RenderCommand_eGLImageTargetTextureStorageEXT : public RenderCommandBase {
	GLuint texture;
	GLeglImageOES image;
	const GLint* attrib_list;

	static void APIENTRY execute(const RenderCommand_eGLImageTargetTextureStorageEXT* __restrict params) {
		glEGLImageTargetTextureStorageEXT(
			params->texture,
			params->image,
			params->attrib_list
		);
	}
};
struct RenderCommand_labelObjectEXT : public RenderCommandBase {
	GLenum type;
	GLuint object;
	GLsizei length;
	const GLchar *label;

	static void APIENTRY execute(const RenderCommand_labelObjectEXT* __restrict params) {
		glLabelObjectEXT(
			params->type,
			params->object,
			params->length,
			params->label
		);
	}
};
struct RenderCommand_getObjectLabelEXT : public RenderCommandBase {
	GLenum type;
	GLuint object;
	GLsizei bufSize;
	GLsizei *length;
	GLchar *label;

	static void APIENTRY execute(const RenderCommand_getObjectLabelEXT* __restrict params) {
		glGetObjectLabelEXT(
			params->type,
			params->object,
			params->bufSize,
			params->length,
			params->label
		);
	}
};
struct RenderCommand_insertEventMarkerEXT : public RenderCommandBase {
	GLsizei length;
	const GLchar *marker;

	static void APIENTRY execute(const RenderCommand_insertEventMarkerEXT* __restrict params) {
		glInsertEventMarkerEXT(
			params->length,
			params->marker
		);
	}
};
struct RenderCommand_pushGroupMarkerEXT : public RenderCommandBase {
	GLsizei length;
	const GLchar *marker;

	static void APIENTRY execute(const RenderCommand_pushGroupMarkerEXT* __restrict params) {
		glPushGroupMarkerEXT(
			params->length,
			params->marker
		);
	}
};
struct RenderCommand_popGroupMarkerEXT : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_popGroupMarkerEXT* __restrict params) {
		glPopGroupMarkerEXT(
		);
	}
};
struct RenderCommand_matrixLoadfEXT : public RenderCommandBase {
	GLenum mode;
	const GLfloat *m;

	static void APIENTRY execute(const RenderCommand_matrixLoadfEXT* __restrict params) {
		glMatrixLoadfEXT(
			params->mode,
			params->m
		);
	}
};
struct RenderCommand_matrixLoaddEXT : public RenderCommandBase {
	GLenum mode;
	const GLdouble *m;

	static void APIENTRY execute(const RenderCommand_matrixLoaddEXT* __restrict params) {
		glMatrixLoaddEXT(
			params->mode,
			params->m
		);
	}
};
struct RenderCommand_matrixMultfEXT : public RenderCommandBase {
	GLenum mode;
	const GLfloat *m;

	static void APIENTRY execute(const RenderCommand_matrixMultfEXT* __restrict params) {
		glMatrixMultfEXT(
			params->mode,
			params->m
		);
	}
};
struct RenderCommand_matrixMultdEXT : public RenderCommandBase {
	GLenum mode;
	const GLdouble *m;

	static void APIENTRY execute(const RenderCommand_matrixMultdEXT* __restrict params) {
		glMatrixMultdEXT(
			params->mode,
			params->m
		);
	}
};
struct RenderCommand_matrixLoadIdentityEXT : public RenderCommandBase {
	GLenum mode;

	static void APIENTRY execute(const RenderCommand_matrixLoadIdentityEXT* __restrict params) {
		glMatrixLoadIdentityEXT(
			params->mode
		);
	}
};
struct RenderCommand_matrixRotatefEXT : public RenderCommandBase {
	GLenum mode;
	GLfloat angle;
	GLfloat x;
	GLfloat y;
	GLfloat z;

	static void APIENTRY execute(const RenderCommand_matrixRotatefEXT* __restrict params) {
		glMatrixRotatefEXT(
			params->mode,
			params->angle,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_matrixRotatedEXT : public RenderCommandBase {
	GLenum mode;
	GLdouble angle;
	GLdouble x;
	GLdouble y;
	GLdouble z;

	static void APIENTRY execute(const RenderCommand_matrixRotatedEXT* __restrict params) {
		glMatrixRotatedEXT(
			params->mode,
			params->angle,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_matrixScalefEXT : public RenderCommandBase {
	GLenum mode;
	GLfloat x;
	GLfloat y;
	GLfloat z;

	static void APIENTRY execute(const RenderCommand_matrixScalefEXT* __restrict params) {
		glMatrixScalefEXT(
			params->mode,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_matrixScaledEXT : public RenderCommandBase {
	GLenum mode;
	GLdouble x;
	GLdouble y;
	GLdouble z;

	static void APIENTRY execute(const RenderCommand_matrixScaledEXT* __restrict params) {
		glMatrixScaledEXT(
			params->mode,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_matrixTranslatefEXT : public RenderCommandBase {
	GLenum mode;
	GLfloat x;
	GLfloat y;
	GLfloat z;

	static void APIENTRY execute(const RenderCommand_matrixTranslatefEXT* __restrict params) {
		glMatrixTranslatefEXT(
			params->mode,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_matrixTranslatedEXT : public RenderCommandBase {
	GLenum mode;
	GLdouble x;
	GLdouble y;
	GLdouble z;

	static void APIENTRY execute(const RenderCommand_matrixTranslatedEXT* __restrict params) {
		glMatrixTranslatedEXT(
			params->mode,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_matrixFrustumEXT : public RenderCommandBase {
	GLenum mode;
	GLdouble left;
	GLdouble right;
	GLdouble bottom;
	GLdouble top;
	GLdouble zNear;
	GLdouble zFar;

	static void APIENTRY execute(const RenderCommand_matrixFrustumEXT* __restrict params) {
		glMatrixFrustumEXT(
			params->mode,
			params->left,
			params->right,
			params->bottom,
			params->top,
			params->zNear,
			params->zFar
		);
	}
};
struct RenderCommand_matrixOrthoEXT : public RenderCommandBase {
	GLenum mode;
	GLdouble left;
	GLdouble right;
	GLdouble bottom;
	GLdouble top;
	GLdouble zNear;
	GLdouble zFar;

	static void APIENTRY execute(const RenderCommand_matrixOrthoEXT* __restrict params) {
		glMatrixOrthoEXT(
			params->mode,
			params->left,
			params->right,
			params->bottom,
			params->top,
			params->zNear,
			params->zFar
		);
	}
};
struct RenderCommand_matrixPopEXT : public RenderCommandBase {
	GLenum mode;

	static void APIENTRY execute(const RenderCommand_matrixPopEXT* __restrict params) {
		glMatrixPopEXT(
			params->mode
		);
	}
};
struct RenderCommand_matrixPushEXT : public RenderCommandBase {
	GLenum mode;

	static void APIENTRY execute(const RenderCommand_matrixPushEXT* __restrict params) {
		glMatrixPushEXT(
			params->mode
		);
	}
};
struct RenderCommand_clientAttribDefaultEXT : public RenderCommandBase {
	GLbitfield mask;

	static void APIENTRY execute(const RenderCommand_clientAttribDefaultEXT* __restrict params) {
		glClientAttribDefaultEXT(
			params->mask
		);
	}
};
struct RenderCommand_pushClientAttribDefaultEXT : public RenderCommandBase {
	GLbitfield mask;

	static void APIENTRY execute(const RenderCommand_pushClientAttribDefaultEXT* __restrict params) {
		glPushClientAttribDefaultEXT(
			params->mask
		);
	}
};
struct RenderCommand_textureParameterfEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLenum pname;
	GLfloat param;

	static void APIENTRY execute(const RenderCommand_textureParameterfEXT* __restrict params) {
		glTextureParameterfEXT(
			params->texture,
			params->target,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_textureParameterfvEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLenum pname;
	const GLfloat *params;

	static void APIENTRY execute(const RenderCommand_textureParameterfvEXT* __restrict params) {
		glTextureParameterfvEXT(
			params->texture,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_textureParameteriEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLenum pname;
	GLint param;

	static void APIENTRY execute(const RenderCommand_textureParameteriEXT* __restrict params) {
		glTextureParameteriEXT(
			params->texture,
			params->target,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_textureParameterivEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLenum pname;
	const GLint *params;

	static void APIENTRY execute(const RenderCommand_textureParameterivEXT* __restrict params) {
		glTextureParameterivEXT(
			params->texture,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_textureImage1DEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint level;
	GLint internalformat;
	GLsizei width;
	GLint border;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_textureImage1DEXT* __restrict params) {
		glTextureImage1DEXT(
			params->texture,
			params->target,
			params->level,
			params->internalformat,
			params->width,
			params->border,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_textureImage2DEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint level;
	GLint internalformat;
	GLsizei width;
	GLsizei height;
	GLint border;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_textureImage2DEXT* __restrict params) {
		glTextureImage2DEXT(
			params->texture,
			params->target,
			params->level,
			params->internalformat,
			params->width,
			params->height,
			params->border,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_textureSubImage1DEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint level;
	GLint xoffset;
	GLsizei width;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_textureSubImage1DEXT* __restrict params) {
		glTextureSubImage1DEXT(
			params->texture,
			params->target,
			params->level,
			params->xoffset,
			params->width,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_textureSubImage2DEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLsizei width;
	GLsizei height;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_textureSubImage2DEXT* __restrict params) {
		glTextureSubImage2DEXT(
			params->texture,
			params->target,
			params->level,
			params->xoffset,
			params->yoffset,
			params->width,
			params->height,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_copyTextureImage1DEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint level;
	GLenum internalformat;
	GLint x;
	GLint y;
	GLsizei width;
	GLint border;

	static void APIENTRY execute(const RenderCommand_copyTextureImage1DEXT* __restrict params) {
		glCopyTextureImage1DEXT(
			params->texture,
			params->target,
			params->level,
			params->internalformat,
			params->x,
			params->y,
			params->width,
			params->border
		);
	}
};
struct RenderCommand_copyTextureImage2DEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint level;
	GLenum internalformat;
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;
	GLint border;

	static void APIENTRY execute(const RenderCommand_copyTextureImage2DEXT* __restrict params) {
		glCopyTextureImage2DEXT(
			params->texture,
			params->target,
			params->level,
			params->internalformat,
			params->x,
			params->y,
			params->width,
			params->height,
			params->border
		);
	}
};
struct RenderCommand_copyTextureSubImage1DEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint x;
	GLint y;
	GLsizei width;

	static void APIENTRY execute(const RenderCommand_copyTextureSubImage1DEXT* __restrict params) {
		glCopyTextureSubImage1DEXT(
			params->texture,
			params->target,
			params->level,
			params->xoffset,
			params->x,
			params->y,
			params->width
		);
	}
};
struct RenderCommand_copyTextureSubImage2DEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_copyTextureSubImage2DEXT* __restrict params) {
		glCopyTextureSubImage2DEXT(
			params->texture,
			params->target,
			params->level,
			params->xoffset,
			params->yoffset,
			params->x,
			params->y,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_getTextureImageEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint level;
	GLenum format;
	GLenum type;
	void *pixels;

	static void APIENTRY execute(const RenderCommand_getTextureImageEXT* __restrict params) {
		glGetTextureImageEXT(
			params->texture,
			params->target,
			params->level,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_getTextureParameterfvEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLenum pname;
	GLfloat *params;

	static void APIENTRY execute(const RenderCommand_getTextureParameterfvEXT* __restrict params) {
		glGetTextureParameterfvEXT(
			params->texture,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getTextureParameterivEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getTextureParameterivEXT* __restrict params) {
		glGetTextureParameterivEXT(
			params->texture,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getTextureLevelParameterfvEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint level;
	GLenum pname;
	GLfloat *params;

	static void APIENTRY execute(const RenderCommand_getTextureLevelParameterfvEXT* __restrict params) {
		glGetTextureLevelParameterfvEXT(
			params->texture,
			params->target,
			params->level,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getTextureLevelParameterivEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint level;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getTextureLevelParameterivEXT* __restrict params) {
		glGetTextureLevelParameterivEXT(
			params->texture,
			params->target,
			params->level,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_textureImage3DEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint level;
	GLint internalformat;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLint border;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_textureImage3DEXT* __restrict params) {
		glTextureImage3DEXT(
			params->texture,
			params->target,
			params->level,
			params->internalformat,
			params->width,
			params->height,
			params->depth,
			params->border,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_textureSubImage3DEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint zoffset;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_textureSubImage3DEXT* __restrict params) {
		glTextureSubImage3DEXT(
			params->texture,
			params->target,
			params->level,
			params->xoffset,
			params->yoffset,
			params->zoffset,
			params->width,
			params->height,
			params->depth,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_copyTextureSubImage3DEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint zoffset;
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_copyTextureSubImage3DEXT* __restrict params) {
		glCopyTextureSubImage3DEXT(
			params->texture,
			params->target,
			params->level,
			params->xoffset,
			params->yoffset,
			params->zoffset,
			params->x,
			params->y,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_bindMultiTextureEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLuint texture;

	static void APIENTRY execute(const RenderCommand_bindMultiTextureEXT* __restrict params) {
		glBindMultiTextureEXT(
			params->texunit,
			params->target,
			params->texture
		);
	}
};
struct RenderCommand_multiTexCoordPointerEXT : public RenderCommandBase {
	GLenum texunit;
	GLint size;
	GLenum type;
	GLsizei stride;
	const void *pointer;

	static void APIENTRY execute(const RenderCommand_multiTexCoordPointerEXT* __restrict params) {
		glMultiTexCoordPointerEXT(
			params->texunit,
			params->size,
			params->type,
			params->stride,
			params->pointer
		);
	}
};
struct RenderCommand_multiTexEnvfEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLenum pname;
	GLfloat param;

	static void APIENTRY execute(const RenderCommand_multiTexEnvfEXT* __restrict params) {
		glMultiTexEnvfEXT(
			params->texunit,
			params->target,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_multiTexEnvfvEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLenum pname;
	const GLfloat *params;

	static void APIENTRY execute(const RenderCommand_multiTexEnvfvEXT* __restrict params) {
		glMultiTexEnvfvEXT(
			params->texunit,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_multiTexEnviEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLenum pname;
	GLint param;

	static void APIENTRY execute(const RenderCommand_multiTexEnviEXT* __restrict params) {
		glMultiTexEnviEXT(
			params->texunit,
			params->target,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_multiTexEnvivEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLenum pname;
	const GLint *params;

	static void APIENTRY execute(const RenderCommand_multiTexEnvivEXT* __restrict params) {
		glMultiTexEnvivEXT(
			params->texunit,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_multiTexGendEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum coord;
	GLenum pname;
	GLdouble param;

	static void APIENTRY execute(const RenderCommand_multiTexGendEXT* __restrict params) {
		glMultiTexGendEXT(
			params->texunit,
			params->coord,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_multiTexGendvEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum coord;
	GLenum pname;
	const GLdouble *params;

	static void APIENTRY execute(const RenderCommand_multiTexGendvEXT* __restrict params) {
		glMultiTexGendvEXT(
			params->texunit,
			params->coord,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_multiTexGenfEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum coord;
	GLenum pname;
	GLfloat param;

	static void APIENTRY execute(const RenderCommand_multiTexGenfEXT* __restrict params) {
		glMultiTexGenfEXT(
			params->texunit,
			params->coord,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_multiTexGenfvEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum coord;
	GLenum pname;
	const GLfloat *params;

	static void APIENTRY execute(const RenderCommand_multiTexGenfvEXT* __restrict params) {
		glMultiTexGenfvEXT(
			params->texunit,
			params->coord,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_multiTexGeniEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum coord;
	GLenum pname;
	GLint param;

	static void APIENTRY execute(const RenderCommand_multiTexGeniEXT* __restrict params) {
		glMultiTexGeniEXT(
			params->texunit,
			params->coord,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_multiTexGenivEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum coord;
	GLenum pname;
	const GLint *params;

	static void APIENTRY execute(const RenderCommand_multiTexGenivEXT* __restrict params) {
		glMultiTexGenivEXT(
			params->texunit,
			params->coord,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getMultiTexEnvfvEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLenum pname;
	GLfloat *params;

	static void APIENTRY execute(const RenderCommand_getMultiTexEnvfvEXT* __restrict params) {
		glGetMultiTexEnvfvEXT(
			params->texunit,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getMultiTexEnvivEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getMultiTexEnvivEXT* __restrict params) {
		glGetMultiTexEnvivEXT(
			params->texunit,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getMultiTexGendvEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum coord;
	GLenum pname;
	GLdouble *params;

	static void APIENTRY execute(const RenderCommand_getMultiTexGendvEXT* __restrict params) {
		glGetMultiTexGendvEXT(
			params->texunit,
			params->coord,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getMultiTexGenfvEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum coord;
	GLenum pname;
	GLfloat *params;

	static void APIENTRY execute(const RenderCommand_getMultiTexGenfvEXT* __restrict params) {
		glGetMultiTexGenfvEXT(
			params->texunit,
			params->coord,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getMultiTexGenivEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum coord;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getMultiTexGenivEXT* __restrict params) {
		glGetMultiTexGenivEXT(
			params->texunit,
			params->coord,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_multiTexParameteriEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLenum pname;
	GLint param;

	static void APIENTRY execute(const RenderCommand_multiTexParameteriEXT* __restrict params) {
		glMultiTexParameteriEXT(
			params->texunit,
			params->target,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_multiTexParameterivEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLenum pname;
	const GLint *params;

	static void APIENTRY execute(const RenderCommand_multiTexParameterivEXT* __restrict params) {
		glMultiTexParameterivEXT(
			params->texunit,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_multiTexParameterfEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLenum pname;
	GLfloat param;

	static void APIENTRY execute(const RenderCommand_multiTexParameterfEXT* __restrict params) {
		glMultiTexParameterfEXT(
			params->texunit,
			params->target,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_multiTexParameterfvEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLenum pname;
	const GLfloat *params;

	static void APIENTRY execute(const RenderCommand_multiTexParameterfvEXT* __restrict params) {
		glMultiTexParameterfvEXT(
			params->texunit,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_multiTexImage1DEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint level;
	GLint internalformat;
	GLsizei width;
	GLint border;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_multiTexImage1DEXT* __restrict params) {
		glMultiTexImage1DEXT(
			params->texunit,
			params->target,
			params->level,
			params->internalformat,
			params->width,
			params->border,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_multiTexImage2DEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint level;
	GLint internalformat;
	GLsizei width;
	GLsizei height;
	GLint border;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_multiTexImage2DEXT* __restrict params) {
		glMultiTexImage2DEXT(
			params->texunit,
			params->target,
			params->level,
			params->internalformat,
			params->width,
			params->height,
			params->border,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_multiTexSubImage1DEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint level;
	GLint xoffset;
	GLsizei width;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_multiTexSubImage1DEXT* __restrict params) {
		glMultiTexSubImage1DEXT(
			params->texunit,
			params->target,
			params->level,
			params->xoffset,
			params->width,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_multiTexSubImage2DEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLsizei width;
	GLsizei height;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_multiTexSubImage2DEXT* __restrict params) {
		glMultiTexSubImage2DEXT(
			params->texunit,
			params->target,
			params->level,
			params->xoffset,
			params->yoffset,
			params->width,
			params->height,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_copyMultiTexImage1DEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint level;
	GLenum internalformat;
	GLint x;
	GLint y;
	GLsizei width;
	GLint border;

	static void APIENTRY execute(const RenderCommand_copyMultiTexImage1DEXT* __restrict params) {
		glCopyMultiTexImage1DEXT(
			params->texunit,
			params->target,
			params->level,
			params->internalformat,
			params->x,
			params->y,
			params->width,
			params->border
		);
	}
};
struct RenderCommand_copyMultiTexImage2DEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint level;
	GLenum internalformat;
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;
	GLint border;

	static void APIENTRY execute(const RenderCommand_copyMultiTexImage2DEXT* __restrict params) {
		glCopyMultiTexImage2DEXT(
			params->texunit,
			params->target,
			params->level,
			params->internalformat,
			params->x,
			params->y,
			params->width,
			params->height,
			params->border
		);
	}
};
struct RenderCommand_copyMultiTexSubImage1DEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint x;
	GLint y;
	GLsizei width;

	static void APIENTRY execute(const RenderCommand_copyMultiTexSubImage1DEXT* __restrict params) {
		glCopyMultiTexSubImage1DEXT(
			params->texunit,
			params->target,
			params->level,
			params->xoffset,
			params->x,
			params->y,
			params->width
		);
	}
};
struct RenderCommand_copyMultiTexSubImage2DEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_copyMultiTexSubImage2DEXT* __restrict params) {
		glCopyMultiTexSubImage2DEXT(
			params->texunit,
			params->target,
			params->level,
			params->xoffset,
			params->yoffset,
			params->x,
			params->y,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_getMultiTexImageEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint level;
	GLenum format;
	GLenum type;
	void *pixels;

	static void APIENTRY execute(const RenderCommand_getMultiTexImageEXT* __restrict params) {
		glGetMultiTexImageEXT(
			params->texunit,
			params->target,
			params->level,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_getMultiTexParameterfvEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLenum pname;
	GLfloat *params;

	static void APIENTRY execute(const RenderCommand_getMultiTexParameterfvEXT* __restrict params) {
		glGetMultiTexParameterfvEXT(
			params->texunit,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getMultiTexParameterivEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getMultiTexParameterivEXT* __restrict params) {
		glGetMultiTexParameterivEXT(
			params->texunit,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getMultiTexLevelParameterfvEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint level;
	GLenum pname;
	GLfloat *params;

	static void APIENTRY execute(const RenderCommand_getMultiTexLevelParameterfvEXT* __restrict params) {
		glGetMultiTexLevelParameterfvEXT(
			params->texunit,
			params->target,
			params->level,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getMultiTexLevelParameterivEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint level;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getMultiTexLevelParameterivEXT* __restrict params) {
		glGetMultiTexLevelParameterivEXT(
			params->texunit,
			params->target,
			params->level,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_multiTexImage3DEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint level;
	GLint internalformat;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLint border;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_multiTexImage3DEXT* __restrict params) {
		glMultiTexImage3DEXT(
			params->texunit,
			params->target,
			params->level,
			params->internalformat,
			params->width,
			params->height,
			params->depth,
			params->border,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_multiTexSubImage3DEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint zoffset;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLenum format;
	GLenum type;
	const void *pixels;

	static void APIENTRY execute(const RenderCommand_multiTexSubImage3DEXT* __restrict params) {
		glMultiTexSubImage3DEXT(
			params->texunit,
			params->target,
			params->level,
			params->xoffset,
			params->yoffset,
			params->zoffset,
			params->width,
			params->height,
			params->depth,
			params->format,
			params->type,
			params->pixels
		);
	}
};
struct RenderCommand_copyMultiTexSubImage3DEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint zoffset;
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_copyMultiTexSubImage3DEXT* __restrict params) {
		glCopyMultiTexSubImage3DEXT(
			params->texunit,
			params->target,
			params->level,
			params->xoffset,
			params->yoffset,
			params->zoffset,
			params->x,
			params->y,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_enableClientStateIndexedEXT : public RenderCommandBase {
	GLenum array;
	GLuint index;

	static void APIENTRY execute(const RenderCommand_enableClientStateIndexedEXT* __restrict params) {
		glEnableClientStateIndexedEXT(
			params->array,
			params->index
		);
	}
};
struct RenderCommand_disableClientStateIndexedEXT : public RenderCommandBase {
	GLenum array;
	GLuint index;

	static void APIENTRY execute(const RenderCommand_disableClientStateIndexedEXT* __restrict params) {
		glDisableClientStateIndexedEXT(
			params->array,
			params->index
		);
	}
};
struct RenderCommand_getFloatIndexedvEXT : public RenderCommandBase {
	GLenum target;
	GLuint index;
	GLfloat *data;

	static void APIENTRY execute(const RenderCommand_getFloatIndexedvEXT* __restrict params) {
		glGetFloatIndexedvEXT(
			params->target,
			params->index,
			params->data
		);
	}
};
struct RenderCommand_getDoubleIndexedvEXT : public RenderCommandBase {
	GLenum target;
	GLuint index;
	GLdouble *data;

	static void APIENTRY execute(const RenderCommand_getDoubleIndexedvEXT* __restrict params) {
		glGetDoubleIndexedvEXT(
			params->target,
			params->index,
			params->data
		);
	}
};
struct RenderCommand_getPointerIndexedvEXT : public RenderCommandBase {
	GLenum target;
	GLuint index;
	void **data;

	static void APIENTRY execute(const RenderCommand_getPointerIndexedvEXT* __restrict params) {
		glGetPointerIndexedvEXT(
			params->target,
			params->index,
			params->data
		);
	}
};
struct RenderCommand_enableIndexedEXT : public RenderCommandBase {
	GLenum target;
	GLuint index;

	static void APIENTRY execute(const RenderCommand_enableIndexedEXT* __restrict params) {
		glEnableIndexedEXT(
			params->target,
			params->index
		);
	}
};
struct RenderCommand_disableIndexedEXT : public RenderCommandBase {
	GLenum target;
	GLuint index;

	static void APIENTRY execute(const RenderCommand_disableIndexedEXT* __restrict params) {
		glDisableIndexedEXT(
			params->target,
			params->index
		);
	}
};
struct RenderCommand_isEnabledIndexedEXT : public RenderCommandBase {
	GLenum target;
	GLuint index;

	static void APIENTRY execute(const RenderCommand_isEnabledIndexedEXT* __restrict params) {
		glIsEnabledIndexedEXT(
			params->target,
			params->index
		);
	}
};
struct RenderCommand_getIntegerIndexedvEXT : public RenderCommandBase {
	GLenum target;
	GLuint index;
	GLint *data;

	static void APIENTRY execute(const RenderCommand_getIntegerIndexedvEXT* __restrict params) {
		glGetIntegerIndexedvEXT(
			params->target,
			params->index,
			params->data
		);
	}
};
struct RenderCommand_getBooleanIndexedvEXT : public RenderCommandBase {
	GLenum target;
	GLuint index;
	GLboolean *data;

	static void APIENTRY execute(const RenderCommand_getBooleanIndexedvEXT* __restrict params) {
		glGetBooleanIndexedvEXT(
			params->target,
			params->index,
			params->data
		);
	}
};
struct RenderCommand_compressedTextureImage3DEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint level;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLint border;
	GLsizei imageSize;
	const void *bits;

	static void APIENTRY execute(const RenderCommand_compressedTextureImage3DEXT* __restrict params) {
		glCompressedTextureImage3DEXT(
			params->texture,
			params->target,
			params->level,
			params->internalformat,
			params->width,
			params->height,
			params->depth,
			params->border,
			params->imageSize,
			params->bits
		);
	}
};
struct RenderCommand_compressedTextureImage2DEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint level;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;
	GLint border;
	GLsizei imageSize;
	const void *bits;

	static void APIENTRY execute(const RenderCommand_compressedTextureImage2DEXT* __restrict params) {
		glCompressedTextureImage2DEXT(
			params->texture,
			params->target,
			params->level,
			params->internalformat,
			params->width,
			params->height,
			params->border,
			params->imageSize,
			params->bits
		);
	}
};
struct RenderCommand_compressedTextureImage1DEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint level;
	GLenum internalformat;
	GLsizei width;
	GLint border;
	GLsizei imageSize;
	const void *bits;

	static void APIENTRY execute(const RenderCommand_compressedTextureImage1DEXT* __restrict params) {
		glCompressedTextureImage1DEXT(
			params->texture,
			params->target,
			params->level,
			params->internalformat,
			params->width,
			params->border,
			params->imageSize,
			params->bits
		);
	}
};
struct RenderCommand_compressedTextureSubImage3DEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint zoffset;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLenum format;
	GLsizei imageSize;
	const void *bits;

	static void APIENTRY execute(const RenderCommand_compressedTextureSubImage3DEXT* __restrict params) {
		glCompressedTextureSubImage3DEXT(
			params->texture,
			params->target,
			params->level,
			params->xoffset,
			params->yoffset,
			params->zoffset,
			params->width,
			params->height,
			params->depth,
			params->format,
			params->imageSize,
			params->bits
		);
	}
};
struct RenderCommand_compressedTextureSubImage2DEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLsizei width;
	GLsizei height;
	GLenum format;
	GLsizei imageSize;
	const void *bits;

	static void APIENTRY execute(const RenderCommand_compressedTextureSubImage2DEXT* __restrict params) {
		glCompressedTextureSubImage2DEXT(
			params->texture,
			params->target,
			params->level,
			params->xoffset,
			params->yoffset,
			params->width,
			params->height,
			params->format,
			params->imageSize,
			params->bits
		);
	}
};
struct RenderCommand_compressedTextureSubImage1DEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint level;
	GLint xoffset;
	GLsizei width;
	GLenum format;
	GLsizei imageSize;
	const void *bits;

	static void APIENTRY execute(const RenderCommand_compressedTextureSubImage1DEXT* __restrict params) {
		glCompressedTextureSubImage1DEXT(
			params->texture,
			params->target,
			params->level,
			params->xoffset,
			params->width,
			params->format,
			params->imageSize,
			params->bits
		);
	}
};
struct RenderCommand_getCompressedTextureImageEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLint lod;
	void *img;

	static void APIENTRY execute(const RenderCommand_getCompressedTextureImageEXT* __restrict params) {
		glGetCompressedTextureImageEXT(
			params->texture,
			params->target,
			params->lod,
			params->img
		);
	}
};
struct RenderCommand_compressedMultiTexImage3DEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint level;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLint border;
	GLsizei imageSize;
	const void *bits;

	static void APIENTRY execute(const RenderCommand_compressedMultiTexImage3DEXT* __restrict params) {
		glCompressedMultiTexImage3DEXT(
			params->texunit,
			params->target,
			params->level,
			params->internalformat,
			params->width,
			params->height,
			params->depth,
			params->border,
			params->imageSize,
			params->bits
		);
	}
};
struct RenderCommand_compressedMultiTexImage2DEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint level;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;
	GLint border;
	GLsizei imageSize;
	const void *bits;

	static void APIENTRY execute(const RenderCommand_compressedMultiTexImage2DEXT* __restrict params) {
		glCompressedMultiTexImage2DEXT(
			params->texunit,
			params->target,
			params->level,
			params->internalformat,
			params->width,
			params->height,
			params->border,
			params->imageSize,
			params->bits
		);
	}
};
struct RenderCommand_compressedMultiTexImage1DEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint level;
	GLenum internalformat;
	GLsizei width;
	GLint border;
	GLsizei imageSize;
	const void *bits;

	static void APIENTRY execute(const RenderCommand_compressedMultiTexImage1DEXT* __restrict params) {
		glCompressedMultiTexImage1DEXT(
			params->texunit,
			params->target,
			params->level,
			params->internalformat,
			params->width,
			params->border,
			params->imageSize,
			params->bits
		);
	}
};
struct RenderCommand_compressedMultiTexSubImage3DEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint zoffset;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLenum format;
	GLsizei imageSize;
	const void *bits;

	static void APIENTRY execute(const RenderCommand_compressedMultiTexSubImage3DEXT* __restrict params) {
		glCompressedMultiTexSubImage3DEXT(
			params->texunit,
			params->target,
			params->level,
			params->xoffset,
			params->yoffset,
			params->zoffset,
			params->width,
			params->height,
			params->depth,
			params->format,
			params->imageSize,
			params->bits
		);
	}
};
struct RenderCommand_compressedMultiTexSubImage2DEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLsizei width;
	GLsizei height;
	GLenum format;
	GLsizei imageSize;
	const void *bits;

	static void APIENTRY execute(const RenderCommand_compressedMultiTexSubImage2DEXT* __restrict params) {
		glCompressedMultiTexSubImage2DEXT(
			params->texunit,
			params->target,
			params->level,
			params->xoffset,
			params->yoffset,
			params->width,
			params->height,
			params->format,
			params->imageSize,
			params->bits
		);
	}
};
struct RenderCommand_compressedMultiTexSubImage1DEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint level;
	GLint xoffset;
	GLsizei width;
	GLenum format;
	GLsizei imageSize;
	const void *bits;

	static void APIENTRY execute(const RenderCommand_compressedMultiTexSubImage1DEXT* __restrict params) {
		glCompressedMultiTexSubImage1DEXT(
			params->texunit,
			params->target,
			params->level,
			params->xoffset,
			params->width,
			params->format,
			params->imageSize,
			params->bits
		);
	}
};
struct RenderCommand_getCompressedMultiTexImageEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLint lod;
	void *img;

	static void APIENTRY execute(const RenderCommand_getCompressedMultiTexImageEXT* __restrict params) {
		glGetCompressedMultiTexImageEXT(
			params->texunit,
			params->target,
			params->lod,
			params->img
		);
	}
};
struct RenderCommand_matrixLoadTransposefEXT : public RenderCommandBase {
	GLenum mode;
	const GLfloat *m;

	static void APIENTRY execute(const RenderCommand_matrixLoadTransposefEXT* __restrict params) {
		glMatrixLoadTransposefEXT(
			params->mode,
			params->m
		);
	}
};
struct RenderCommand_matrixLoadTransposedEXT : public RenderCommandBase {
	GLenum mode;
	const GLdouble *m;

	static void APIENTRY execute(const RenderCommand_matrixLoadTransposedEXT* __restrict params) {
		glMatrixLoadTransposedEXT(
			params->mode,
			params->m
		);
	}
};
struct RenderCommand_matrixMultTransposefEXT : public RenderCommandBase {
	GLenum mode;
	const GLfloat *m;

	static void APIENTRY execute(const RenderCommand_matrixMultTransposefEXT* __restrict params) {
		glMatrixMultTransposefEXT(
			params->mode,
			params->m
		);
	}
};
struct RenderCommand_matrixMultTransposedEXT : public RenderCommandBase {
	GLenum mode;
	const GLdouble *m;

	static void APIENTRY execute(const RenderCommand_matrixMultTransposedEXT* __restrict params) {
		glMatrixMultTransposedEXT(
			params->mode,
			params->m
		);
	}
};
struct RenderCommand_namedBufferDataEXT : public RenderCommandBase {
	GLuint buffer;
	GLsizeiptr size;
	const void *data;
	GLenum usage;

	static void APIENTRY execute(const RenderCommand_namedBufferDataEXT* __restrict params) {
		glNamedBufferDataEXT(
			params->buffer,
			params->size,
			params->data,
			params->usage
		);
	}
};
struct RenderCommand_namedBufferSubDataEXT : public RenderCommandBase {
	GLuint buffer;
	GLintptr offset;
	GLsizeiptr size;
	const void *data;

	static void APIENTRY execute(const RenderCommand_namedBufferSubDataEXT* __restrict params) {
		glNamedBufferSubDataEXT(
			params->buffer,
			params->offset,
			params->size,
			params->data
		);
	}
};
struct RenderCommand_mapNamedBufferEXT : public RenderCommandBase {
	GLuint buffer;
	GLenum access;

	static void APIENTRY execute(const RenderCommand_mapNamedBufferEXT* __restrict params) {
		glMapNamedBufferEXT(
			params->buffer,
			params->access
		);
	}
};
struct RenderCommand_unmapNamedBufferEXT : public RenderCommandBase {
	GLuint buffer;

	static void APIENTRY execute(const RenderCommand_unmapNamedBufferEXT* __restrict params) {
		glUnmapNamedBufferEXT(
			params->buffer
		);
	}
};
struct RenderCommand_getNamedBufferParameterivEXT : public RenderCommandBase {
	GLuint buffer;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getNamedBufferParameterivEXT* __restrict params) {
		glGetNamedBufferParameterivEXT(
			params->buffer,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getNamedBufferPointervEXT : public RenderCommandBase {
	GLuint buffer;
	GLenum pname;
	void **params;

	static void APIENTRY execute(const RenderCommand_getNamedBufferPointervEXT* __restrict params) {
		glGetNamedBufferPointervEXT(
			params->buffer,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getNamedBufferSubDataEXT : public RenderCommandBase {
	GLuint buffer;
	GLintptr offset;
	GLsizeiptr size;
	void *data;

	static void APIENTRY execute(const RenderCommand_getNamedBufferSubDataEXT* __restrict params) {
		glGetNamedBufferSubDataEXT(
			params->buffer,
			params->offset,
			params->size,
			params->data
		);
	}
};
struct RenderCommand_programUniform1fEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLfloat v0;

	static void APIENTRY execute(const RenderCommand_programUniform1fEXT* __restrict params) {
		glProgramUniform1fEXT(
			params->program,
			params->location,
			params->v0
		);
	}
};
struct RenderCommand_programUniform2fEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLfloat v0;
	GLfloat v1;

	static void APIENTRY execute(const RenderCommand_programUniform2fEXT* __restrict params) {
		glProgramUniform2fEXT(
			params->program,
			params->location,
			params->v0,
			params->v1
		);
	}
};
struct RenderCommand_programUniform3fEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLfloat v0;
	GLfloat v1;
	GLfloat v2;

	static void APIENTRY execute(const RenderCommand_programUniform3fEXT* __restrict params) {
		glProgramUniform3fEXT(
			params->program,
			params->location,
			params->v0,
			params->v1,
			params->v2
		);
	}
};
struct RenderCommand_programUniform4fEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLfloat v0;
	GLfloat v1;
	GLfloat v2;
	GLfloat v3;

	static void APIENTRY execute(const RenderCommand_programUniform4fEXT* __restrict params) {
		glProgramUniform4fEXT(
			params->program,
			params->location,
			params->v0,
			params->v1,
			params->v2,
			params->v3
		);
	}
};
struct RenderCommand_programUniform1iEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLint v0;

	static void APIENTRY execute(const RenderCommand_programUniform1iEXT* __restrict params) {
		glProgramUniform1iEXT(
			params->program,
			params->location,
			params->v0
		);
	}
};
struct RenderCommand_programUniform2iEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLint v0;
	GLint v1;

	static void APIENTRY execute(const RenderCommand_programUniform2iEXT* __restrict params) {
		glProgramUniform2iEXT(
			params->program,
			params->location,
			params->v0,
			params->v1
		);
	}
};
struct RenderCommand_programUniform3iEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLint v0;
	GLint v1;
	GLint v2;

	static void APIENTRY execute(const RenderCommand_programUniform3iEXT* __restrict params) {
		glProgramUniform3iEXT(
			params->program,
			params->location,
			params->v0,
			params->v1,
			params->v2
		);
	}
};
struct RenderCommand_programUniform4iEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLint v0;
	GLint v1;
	GLint v2;
	GLint v3;

	static void APIENTRY execute(const RenderCommand_programUniform4iEXT* __restrict params) {
		glProgramUniform4iEXT(
			params->program,
			params->location,
			params->v0,
			params->v1,
			params->v2,
			params->v3
		);
	}
};
struct RenderCommand_programUniform1fvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniform1fvEXT* __restrict params) {
		glProgramUniform1fvEXT(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform2fvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniform2fvEXT* __restrict params) {
		glProgramUniform2fvEXT(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform3fvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniform3fvEXT* __restrict params) {
		glProgramUniform3fvEXT(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform4fvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniform4fvEXT* __restrict params) {
		glProgramUniform4fvEXT(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform1ivEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLint *value;

	static void APIENTRY execute(const RenderCommand_programUniform1ivEXT* __restrict params) {
		glProgramUniform1ivEXT(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform2ivEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLint *value;

	static void APIENTRY execute(const RenderCommand_programUniform2ivEXT* __restrict params) {
		glProgramUniform2ivEXT(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform3ivEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLint *value;

	static void APIENTRY execute(const RenderCommand_programUniform3ivEXT* __restrict params) {
		glProgramUniform3ivEXT(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform4ivEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLint *value;

	static void APIENTRY execute(const RenderCommand_programUniform4ivEXT* __restrict params) {
		glProgramUniform4ivEXT(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix2fvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix2fvEXT* __restrict params) {
		glProgramUniformMatrix2fvEXT(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix3fvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix3fvEXT* __restrict params) {
		glProgramUniformMatrix3fvEXT(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix4fvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix4fvEXT* __restrict params) {
		glProgramUniformMatrix4fvEXT(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix2x3fvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix2x3fvEXT* __restrict params) {
		glProgramUniformMatrix2x3fvEXT(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix3x2fvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix3x2fvEXT* __restrict params) {
		glProgramUniformMatrix3x2fvEXT(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix2x4fvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix2x4fvEXT* __restrict params) {
		glProgramUniformMatrix2x4fvEXT(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix4x2fvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix4x2fvEXT* __restrict params) {
		glProgramUniformMatrix4x2fvEXT(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix3x4fvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix3x4fvEXT* __restrict params) {
		glProgramUniformMatrix3x4fvEXT(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix4x3fvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix4x3fvEXT* __restrict params) {
		glProgramUniformMatrix4x3fvEXT(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_textureBufferEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLenum internalformat;
	GLuint buffer;

	static void APIENTRY execute(const RenderCommand_textureBufferEXT* __restrict params) {
		glTextureBufferEXT(
			params->texture,
			params->target,
			params->internalformat,
			params->buffer
		);
	}
};
struct RenderCommand_multiTexBufferEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLenum internalformat;
	GLuint buffer;

	static void APIENTRY execute(const RenderCommand_multiTexBufferEXT* __restrict params) {
		glMultiTexBufferEXT(
			params->texunit,
			params->target,
			params->internalformat,
			params->buffer
		);
	}
};
struct RenderCommand_textureParameterIivEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLenum pname;
	const GLint *params;

	static void APIENTRY execute(const RenderCommand_textureParameterIivEXT* __restrict params) {
		glTextureParameterIivEXT(
			params->texture,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_textureParameterIuivEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLenum pname;
	const GLuint *params;

	static void APIENTRY execute(const RenderCommand_textureParameterIuivEXT* __restrict params) {
		glTextureParameterIuivEXT(
			params->texture,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getTextureParameterIivEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getTextureParameterIivEXT* __restrict params) {
		glGetTextureParameterIivEXT(
			params->texture,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getTextureParameterIuivEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLenum pname;
	GLuint *params;

	static void APIENTRY execute(const RenderCommand_getTextureParameterIuivEXT* __restrict params) {
		glGetTextureParameterIuivEXT(
			params->texture,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_multiTexParameterIivEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLenum pname;
	const GLint *params;

	static void APIENTRY execute(const RenderCommand_multiTexParameterIivEXT* __restrict params) {
		glMultiTexParameterIivEXT(
			params->texunit,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_multiTexParameterIuivEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLenum pname;
	const GLuint *params;

	static void APIENTRY execute(const RenderCommand_multiTexParameterIuivEXT* __restrict params) {
		glMultiTexParameterIuivEXT(
			params->texunit,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getMultiTexParameterIivEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getMultiTexParameterIivEXT* __restrict params) {
		glGetMultiTexParameterIivEXT(
			params->texunit,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getMultiTexParameterIuivEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLenum pname;
	GLuint *params;

	static void APIENTRY execute(const RenderCommand_getMultiTexParameterIuivEXT* __restrict params) {
		glGetMultiTexParameterIuivEXT(
			params->texunit,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_programUniform1uiEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint v0;

	static void APIENTRY execute(const RenderCommand_programUniform1uiEXT* __restrict params) {
		glProgramUniform1uiEXT(
			params->program,
			params->location,
			params->v0
		);
	}
};
struct RenderCommand_programUniform2uiEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint v0;
	GLuint v1;

	static void APIENTRY execute(const RenderCommand_programUniform2uiEXT* __restrict params) {
		glProgramUniform2uiEXT(
			params->program,
			params->location,
			params->v0,
			params->v1
		);
	}
};
struct RenderCommand_programUniform3uiEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint v0;
	GLuint v1;
	GLuint v2;

	static void APIENTRY execute(const RenderCommand_programUniform3uiEXT* __restrict params) {
		glProgramUniform3uiEXT(
			params->program,
			params->location,
			params->v0,
			params->v1,
			params->v2
		);
	}
};
struct RenderCommand_programUniform4uiEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint v0;
	GLuint v1;
	GLuint v2;
	GLuint v3;

	static void APIENTRY execute(const RenderCommand_programUniform4uiEXT* __restrict params) {
		glProgramUniform4uiEXT(
			params->program,
			params->location,
			params->v0,
			params->v1,
			params->v2,
			params->v3
		);
	}
};
struct RenderCommand_programUniform1uivEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLuint *value;

	static void APIENTRY execute(const RenderCommand_programUniform1uivEXT* __restrict params) {
		glProgramUniform1uivEXT(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform2uivEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLuint *value;

	static void APIENTRY execute(const RenderCommand_programUniform2uivEXT* __restrict params) {
		glProgramUniform2uivEXT(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform3uivEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLuint *value;

	static void APIENTRY execute(const RenderCommand_programUniform3uivEXT* __restrict params) {
		glProgramUniform3uivEXT(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform4uivEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLuint *value;

	static void APIENTRY execute(const RenderCommand_programUniform4uivEXT* __restrict params) {
		glProgramUniform4uivEXT(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_namedProgramLocalParameters4fvEXT : public RenderCommandBase {
	GLuint program;
	GLenum target;
	GLuint index;
	GLsizei count;
	const GLfloat *params;

	static void APIENTRY execute(const RenderCommand_namedProgramLocalParameters4fvEXT* __restrict params) {
		glNamedProgramLocalParameters4fvEXT(
			params->program,
			params->target,
			params->index,
			params->count,
			params->params
		);
	}
};
struct RenderCommand_namedProgramLocalParameterI4iEXT : public RenderCommandBase {
	GLuint program;
	GLenum target;
	GLuint index;
	GLint x;
	GLint y;
	GLint z;
	GLint w;

	static void APIENTRY execute(const RenderCommand_namedProgramLocalParameterI4iEXT* __restrict params) {
		glNamedProgramLocalParameterI4iEXT(
			params->program,
			params->target,
			params->index,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_namedProgramLocalParameterI4ivEXT : public RenderCommandBase {
	GLuint program;
	GLenum target;
	GLuint index;
	const GLint *params;

	static void APIENTRY execute(const RenderCommand_namedProgramLocalParameterI4ivEXT* __restrict params) {
		glNamedProgramLocalParameterI4ivEXT(
			params->program,
			params->target,
			params->index,
			params->params
		);
	}
};
struct RenderCommand_namedProgramLocalParametersI4ivEXT : public RenderCommandBase {
	GLuint program;
	GLenum target;
	GLuint index;
	GLsizei count;
	const GLint *params;

	static void APIENTRY execute(const RenderCommand_namedProgramLocalParametersI4ivEXT* __restrict params) {
		glNamedProgramLocalParametersI4ivEXT(
			params->program,
			params->target,
			params->index,
			params->count,
			params->params
		);
	}
};
struct RenderCommand_namedProgramLocalParameterI4uiEXT : public RenderCommandBase {
	GLuint program;
	GLenum target;
	GLuint index;
	GLuint x;
	GLuint y;
	GLuint z;
	GLuint w;

	static void APIENTRY execute(const RenderCommand_namedProgramLocalParameterI4uiEXT* __restrict params) {
		glNamedProgramLocalParameterI4uiEXT(
			params->program,
			params->target,
			params->index,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_namedProgramLocalParameterI4uivEXT : public RenderCommandBase {
	GLuint program;
	GLenum target;
	GLuint index;
	const GLuint *params;

	static void APIENTRY execute(const RenderCommand_namedProgramLocalParameterI4uivEXT* __restrict params) {
		glNamedProgramLocalParameterI4uivEXT(
			params->program,
			params->target,
			params->index,
			params->params
		);
	}
};
struct RenderCommand_namedProgramLocalParametersI4uivEXT : public RenderCommandBase {
	GLuint program;
	GLenum target;
	GLuint index;
	GLsizei count;
	const GLuint *params;

	static void APIENTRY execute(const RenderCommand_namedProgramLocalParametersI4uivEXT* __restrict params) {
		glNamedProgramLocalParametersI4uivEXT(
			params->program,
			params->target,
			params->index,
			params->count,
			params->params
		);
	}
};
struct RenderCommand_getNamedProgramLocalParameterIivEXT : public RenderCommandBase {
	GLuint program;
	GLenum target;
	GLuint index;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getNamedProgramLocalParameterIivEXT* __restrict params) {
		glGetNamedProgramLocalParameterIivEXT(
			params->program,
			params->target,
			params->index,
			params->params
		);
	}
};
struct RenderCommand_getNamedProgramLocalParameterIuivEXT : public RenderCommandBase {
	GLuint program;
	GLenum target;
	GLuint index;
	GLuint *params;

	static void APIENTRY execute(const RenderCommand_getNamedProgramLocalParameterIuivEXT* __restrict params) {
		glGetNamedProgramLocalParameterIuivEXT(
			params->program,
			params->target,
			params->index,
			params->params
		);
	}
};
struct RenderCommand_enableClientStateiEXT : public RenderCommandBase {
	GLenum array;
	GLuint index;

	static void APIENTRY execute(const RenderCommand_enableClientStateiEXT* __restrict params) {
		glEnableClientStateiEXT(
			params->array,
			params->index
		);
	}
};
struct RenderCommand_disableClientStateiEXT : public RenderCommandBase {
	GLenum array;
	GLuint index;

	static void APIENTRY execute(const RenderCommand_disableClientStateiEXT* __restrict params) {
		glDisableClientStateiEXT(
			params->array,
			params->index
		);
	}
};
struct RenderCommand_getFloati_vEXT : public RenderCommandBase {
	GLenum pname;
	GLuint index;
	GLfloat *params;

	static void APIENTRY execute(const RenderCommand_getFloati_vEXT* __restrict params) {
		glGetFloati_vEXT(
			params->pname,
			params->index,
			params->params
		);
	}
};
struct RenderCommand_getDoublei_vEXT : public RenderCommandBase {
	GLenum pname;
	GLuint index;
	GLdouble *params;

	static void APIENTRY execute(const RenderCommand_getDoublei_vEXT* __restrict params) {
		glGetDoublei_vEXT(
			params->pname,
			params->index,
			params->params
		);
	}
};
struct RenderCommand_getPointeri_vEXT : public RenderCommandBase {
	GLenum pname;
	GLuint index;
	void **params;

	static void APIENTRY execute(const RenderCommand_getPointeri_vEXT* __restrict params) {
		glGetPointeri_vEXT(
			params->pname,
			params->index,
			params->params
		);
	}
};
struct RenderCommand_namedProgramStringEXT : public RenderCommandBase {
	GLuint program;
	GLenum target;
	GLenum format;
	GLsizei len;
	const void *string;

	static void APIENTRY execute(const RenderCommand_namedProgramStringEXT* __restrict params) {
		glNamedProgramStringEXT(
			params->program,
			params->target,
			params->format,
			params->len,
			params->string
		);
	}
};
struct RenderCommand_namedProgramLocalParameter4dEXT : public RenderCommandBase {
	GLuint program;
	GLenum target;
	GLuint index;
	GLdouble x;
	GLdouble y;
	GLdouble z;
	GLdouble w;

	static void APIENTRY execute(const RenderCommand_namedProgramLocalParameter4dEXT* __restrict params) {
		glNamedProgramLocalParameter4dEXT(
			params->program,
			params->target,
			params->index,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_namedProgramLocalParameter4dvEXT : public RenderCommandBase {
	GLuint program;
	GLenum target;
	GLuint index;
	const GLdouble *params;

	static void APIENTRY execute(const RenderCommand_namedProgramLocalParameter4dvEXT* __restrict params) {
		glNamedProgramLocalParameter4dvEXT(
			params->program,
			params->target,
			params->index,
			params->params
		);
	}
};
struct RenderCommand_namedProgramLocalParameter4fEXT : public RenderCommandBase {
	GLuint program;
	GLenum target;
	GLuint index;
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat w;

	static void APIENTRY execute(const RenderCommand_namedProgramLocalParameter4fEXT* __restrict params) {
		glNamedProgramLocalParameter4fEXT(
			params->program,
			params->target,
			params->index,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_namedProgramLocalParameter4fvEXT : public RenderCommandBase {
	GLuint program;
	GLenum target;
	GLuint index;
	const GLfloat *params;

	static void APIENTRY execute(const RenderCommand_namedProgramLocalParameter4fvEXT* __restrict params) {
		glNamedProgramLocalParameter4fvEXT(
			params->program,
			params->target,
			params->index,
			params->params
		);
	}
};
struct RenderCommand_getNamedProgramLocalParameterdvEXT : public RenderCommandBase {
	GLuint program;
	GLenum target;
	GLuint index;
	GLdouble *params;

	static void APIENTRY execute(const RenderCommand_getNamedProgramLocalParameterdvEXT* __restrict params) {
		glGetNamedProgramLocalParameterdvEXT(
			params->program,
			params->target,
			params->index,
			params->params
		);
	}
};
struct RenderCommand_getNamedProgramLocalParameterfvEXT : public RenderCommandBase {
	GLuint program;
	GLenum target;
	GLuint index;
	GLfloat *params;

	static void APIENTRY execute(const RenderCommand_getNamedProgramLocalParameterfvEXT* __restrict params) {
		glGetNamedProgramLocalParameterfvEXT(
			params->program,
			params->target,
			params->index,
			params->params
		);
	}
};
struct RenderCommand_getNamedProgramivEXT : public RenderCommandBase {
	GLuint program;
	GLenum target;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getNamedProgramivEXT* __restrict params) {
		glGetNamedProgramivEXT(
			params->program,
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getNamedProgramStringEXT : public RenderCommandBase {
	GLuint program;
	GLenum target;
	GLenum pname;
	void *string;

	static void APIENTRY execute(const RenderCommand_getNamedProgramStringEXT* __restrict params) {
		glGetNamedProgramStringEXT(
			params->program,
			params->target,
			params->pname,
			params->string
		);
	}
};
struct RenderCommand_namedRenderbufferStorageEXT : public RenderCommandBase {
	GLuint renderbuffer;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_namedRenderbufferStorageEXT* __restrict params) {
		glNamedRenderbufferStorageEXT(
			params->renderbuffer,
			params->internalformat,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_getNamedRenderbufferParameterivEXT : public RenderCommandBase {
	GLuint renderbuffer;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getNamedRenderbufferParameterivEXT* __restrict params) {
		glGetNamedRenderbufferParameterivEXT(
			params->renderbuffer,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_namedRenderbufferStorageMultisampleEXT : public RenderCommandBase {
	GLuint renderbuffer;
	GLsizei samples;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_namedRenderbufferStorageMultisampleEXT* __restrict params) {
		glNamedRenderbufferStorageMultisampleEXT(
			params->renderbuffer,
			params->samples,
			params->internalformat,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_namedRenderbufferStorageMultisampleCoverageEXT : public RenderCommandBase {
	GLuint renderbuffer;
	GLsizei coverageSamples;
	GLsizei colorSamples;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_namedRenderbufferStorageMultisampleCoverageEXT* __restrict params) {
		glNamedRenderbufferStorageMultisampleCoverageEXT(
			params->renderbuffer,
			params->coverageSamples,
			params->colorSamples,
			params->internalformat,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_checkNamedFramebufferStatusEXT : public RenderCommandBase {
	GLuint framebuffer;
	GLenum target;

	static void APIENTRY execute(const RenderCommand_checkNamedFramebufferStatusEXT* __restrict params) {
		glCheckNamedFramebufferStatusEXT(
			params->framebuffer,
			params->target
		);
	}
};
struct RenderCommand_namedFramebufferTexture1DEXT : public RenderCommandBase {
	GLuint framebuffer;
	GLenum attachment;
	GLenum textarget;
	GLuint texture;
	GLint level;

	static void APIENTRY execute(const RenderCommand_namedFramebufferTexture1DEXT* __restrict params) {
		glNamedFramebufferTexture1DEXT(
			params->framebuffer,
			params->attachment,
			params->textarget,
			params->texture,
			params->level
		);
	}
};
struct RenderCommand_namedFramebufferTexture2DEXT : public RenderCommandBase {
	GLuint framebuffer;
	GLenum attachment;
	GLenum textarget;
	GLuint texture;
	GLint level;

	static void APIENTRY execute(const RenderCommand_namedFramebufferTexture2DEXT* __restrict params) {
		glNamedFramebufferTexture2DEXT(
			params->framebuffer,
			params->attachment,
			params->textarget,
			params->texture,
			params->level
		);
	}
};
struct RenderCommand_namedFramebufferTexture3DEXT : public RenderCommandBase {
	GLuint framebuffer;
	GLenum attachment;
	GLenum textarget;
	GLuint texture;
	GLint level;
	GLint zoffset;

	static void APIENTRY execute(const RenderCommand_namedFramebufferTexture3DEXT* __restrict params) {
		glNamedFramebufferTexture3DEXT(
			params->framebuffer,
			params->attachment,
			params->textarget,
			params->texture,
			params->level,
			params->zoffset
		);
	}
};
struct RenderCommand_namedFramebufferRenderbufferEXT : public RenderCommandBase {
	GLuint framebuffer;
	GLenum attachment;
	GLenum renderbuffertarget;
	GLuint renderbuffer;

	static void APIENTRY execute(const RenderCommand_namedFramebufferRenderbufferEXT* __restrict params) {
		glNamedFramebufferRenderbufferEXT(
			params->framebuffer,
			params->attachment,
			params->renderbuffertarget,
			params->renderbuffer
		);
	}
};
struct RenderCommand_getNamedFramebufferAttachmentParameterivEXT : public RenderCommandBase {
	GLuint framebuffer;
	GLenum attachment;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getNamedFramebufferAttachmentParameterivEXT* __restrict params) {
		glGetNamedFramebufferAttachmentParameterivEXT(
			params->framebuffer,
			params->attachment,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_generateTextureMipmapEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;

	static void APIENTRY execute(const RenderCommand_generateTextureMipmapEXT* __restrict params) {
		glGenerateTextureMipmapEXT(
			params->texture,
			params->target
		);
	}
};
struct RenderCommand_generateMultiTexMipmapEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;

	static void APIENTRY execute(const RenderCommand_generateMultiTexMipmapEXT* __restrict params) {
		glGenerateMultiTexMipmapEXT(
			params->texunit,
			params->target
		);
	}
};
struct RenderCommand_framebufferDrawBufferEXT : public RenderCommandBase {
	GLuint framebuffer;
	GLenum mode;

	static void APIENTRY execute(const RenderCommand_framebufferDrawBufferEXT* __restrict params) {
		glFramebufferDrawBufferEXT(
			params->framebuffer,
			params->mode
		);
	}
};
struct RenderCommand_framebufferDrawBuffersEXT : public RenderCommandBase {
	GLuint framebuffer;
	GLsizei n;
	const GLenum *bufs;

	static void APIENTRY execute(const RenderCommand_framebufferDrawBuffersEXT* __restrict params) {
		glFramebufferDrawBuffersEXT(
			params->framebuffer,
			params->n,
			params->bufs
		);
	}
};
struct RenderCommand_framebufferReadBufferEXT : public RenderCommandBase {
	GLuint framebuffer;
	GLenum mode;

	static void APIENTRY execute(const RenderCommand_framebufferReadBufferEXT* __restrict params) {
		glFramebufferReadBufferEXT(
			params->framebuffer,
			params->mode
		);
	}
};
struct RenderCommand_getFramebufferParameterivEXT : public RenderCommandBase {
	GLuint framebuffer;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getFramebufferParameterivEXT* __restrict params) {
		glGetFramebufferParameterivEXT(
			params->framebuffer,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_namedCopyBufferSubDataEXT : public RenderCommandBase {
	GLuint readBuffer;
	GLuint writeBuffer;
	GLintptr readOffset;
	GLintptr writeOffset;
	GLsizeiptr size;

	static void APIENTRY execute(const RenderCommand_namedCopyBufferSubDataEXT* __restrict params) {
		glNamedCopyBufferSubDataEXT(
			params->readBuffer,
			params->writeBuffer,
			params->readOffset,
			params->writeOffset,
			params->size
		);
	}
};
struct RenderCommand_namedFramebufferTextureEXT : public RenderCommandBase {
	GLuint framebuffer;
	GLenum attachment;
	GLuint texture;
	GLint level;

	static void APIENTRY execute(const RenderCommand_namedFramebufferTextureEXT* __restrict params) {
		glNamedFramebufferTextureEXT(
			params->framebuffer,
			params->attachment,
			params->texture,
			params->level
		);
	}
};
struct RenderCommand_namedFramebufferTextureLayerEXT : public RenderCommandBase {
	GLuint framebuffer;
	GLenum attachment;
	GLuint texture;
	GLint level;
	GLint layer;

	static void APIENTRY execute(const RenderCommand_namedFramebufferTextureLayerEXT* __restrict params) {
		glNamedFramebufferTextureLayerEXT(
			params->framebuffer,
			params->attachment,
			params->texture,
			params->level,
			params->layer
		);
	}
};
struct RenderCommand_namedFramebufferTextureFaceEXT : public RenderCommandBase {
	GLuint framebuffer;
	GLenum attachment;
	GLuint texture;
	GLint level;
	GLenum face;

	static void APIENTRY execute(const RenderCommand_namedFramebufferTextureFaceEXT* __restrict params) {
		glNamedFramebufferTextureFaceEXT(
			params->framebuffer,
			params->attachment,
			params->texture,
			params->level,
			params->face
		);
	}
};
struct RenderCommand_textureRenderbufferEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLuint renderbuffer;

	static void APIENTRY execute(const RenderCommand_textureRenderbufferEXT* __restrict params) {
		glTextureRenderbufferEXT(
			params->texture,
			params->target,
			params->renderbuffer
		);
	}
};
struct RenderCommand_multiTexRenderbufferEXT : public RenderCommandBase {
	GLenum texunit;
	GLenum target;
	GLuint renderbuffer;

	static void APIENTRY execute(const RenderCommand_multiTexRenderbufferEXT* __restrict params) {
		glMultiTexRenderbufferEXT(
			params->texunit,
			params->target,
			params->renderbuffer
		);
	}
};
struct RenderCommand_vertexArrayVertexOffsetEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint buffer;
	GLint size;
	GLenum type;
	GLsizei stride;
	GLintptr offset;

	static void APIENTRY execute(const RenderCommand_vertexArrayVertexOffsetEXT* __restrict params) {
		glVertexArrayVertexOffsetEXT(
			params->vaobj,
			params->buffer,
			params->size,
			params->type,
			params->stride,
			params->offset
		);
	}
};
struct RenderCommand_vertexArrayColorOffsetEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint buffer;
	GLint size;
	GLenum type;
	GLsizei stride;
	GLintptr offset;

	static void APIENTRY execute(const RenderCommand_vertexArrayColorOffsetEXT* __restrict params) {
		glVertexArrayColorOffsetEXT(
			params->vaobj,
			params->buffer,
			params->size,
			params->type,
			params->stride,
			params->offset
		);
	}
};
struct RenderCommand_vertexArrayEdgeFlagOffsetEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint buffer;
	GLsizei stride;
	GLintptr offset;

	static void APIENTRY execute(const RenderCommand_vertexArrayEdgeFlagOffsetEXT* __restrict params) {
		glVertexArrayEdgeFlagOffsetEXT(
			params->vaobj,
			params->buffer,
			params->stride,
			params->offset
		);
	}
};
struct RenderCommand_vertexArrayIndexOffsetEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint buffer;
	GLenum type;
	GLsizei stride;
	GLintptr offset;

	static void APIENTRY execute(const RenderCommand_vertexArrayIndexOffsetEXT* __restrict params) {
		glVertexArrayIndexOffsetEXT(
			params->vaobj,
			params->buffer,
			params->type,
			params->stride,
			params->offset
		);
	}
};
struct RenderCommand_vertexArrayNormalOffsetEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint buffer;
	GLenum type;
	GLsizei stride;
	GLintptr offset;

	static void APIENTRY execute(const RenderCommand_vertexArrayNormalOffsetEXT* __restrict params) {
		glVertexArrayNormalOffsetEXT(
			params->vaobj,
			params->buffer,
			params->type,
			params->stride,
			params->offset
		);
	}
};
struct RenderCommand_vertexArrayTexCoordOffsetEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint buffer;
	GLint size;
	GLenum type;
	GLsizei stride;
	GLintptr offset;

	static void APIENTRY execute(const RenderCommand_vertexArrayTexCoordOffsetEXT* __restrict params) {
		glVertexArrayTexCoordOffsetEXT(
			params->vaobj,
			params->buffer,
			params->size,
			params->type,
			params->stride,
			params->offset
		);
	}
};
struct RenderCommand_vertexArrayMultiTexCoordOffsetEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint buffer;
	GLenum texunit;
	GLint size;
	GLenum type;
	GLsizei stride;
	GLintptr offset;

	static void APIENTRY execute(const RenderCommand_vertexArrayMultiTexCoordOffsetEXT* __restrict params) {
		glVertexArrayMultiTexCoordOffsetEXT(
			params->vaobj,
			params->buffer,
			params->texunit,
			params->size,
			params->type,
			params->stride,
			params->offset
		);
	}
};
struct RenderCommand_vertexArrayFogCoordOffsetEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint buffer;
	GLenum type;
	GLsizei stride;
	GLintptr offset;

	static void APIENTRY execute(const RenderCommand_vertexArrayFogCoordOffsetEXT* __restrict params) {
		glVertexArrayFogCoordOffsetEXT(
			params->vaobj,
			params->buffer,
			params->type,
			params->stride,
			params->offset
		);
	}
};
struct RenderCommand_vertexArraySecondaryColorOffsetEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint buffer;
	GLint size;
	GLenum type;
	GLsizei stride;
	GLintptr offset;

	static void APIENTRY execute(const RenderCommand_vertexArraySecondaryColorOffsetEXT* __restrict params) {
		glVertexArraySecondaryColorOffsetEXT(
			params->vaobj,
			params->buffer,
			params->size,
			params->type,
			params->stride,
			params->offset
		);
	}
};
struct RenderCommand_vertexArrayVertexAttribOffsetEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint buffer;
	GLuint index;
	GLint size;
	GLenum type;
	GLboolean normalized;
	GLsizei stride;
	GLintptr offset;

	static void APIENTRY execute(const RenderCommand_vertexArrayVertexAttribOffsetEXT* __restrict params) {
		glVertexArrayVertexAttribOffsetEXT(
			params->vaobj,
			params->buffer,
			params->index,
			params->size,
			params->type,
			params->normalized,
			params->stride,
			params->offset
		);
	}
};
struct RenderCommand_vertexArrayVertexAttribIOffsetEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint buffer;
	GLuint index;
	GLint size;
	GLenum type;
	GLsizei stride;
	GLintptr offset;

	static void APIENTRY execute(const RenderCommand_vertexArrayVertexAttribIOffsetEXT* __restrict params) {
		glVertexArrayVertexAttribIOffsetEXT(
			params->vaobj,
			params->buffer,
			params->index,
			params->size,
			params->type,
			params->stride,
			params->offset
		);
	}
};
struct RenderCommand_enableVertexArrayEXT : public RenderCommandBase {
	GLuint vaobj;
	GLenum array;

	static void APIENTRY execute(const RenderCommand_enableVertexArrayEXT* __restrict params) {
		glEnableVertexArrayEXT(
			params->vaobj,
			params->array
		);
	}
};
struct RenderCommand_disableVertexArrayEXT : public RenderCommandBase {
	GLuint vaobj;
	GLenum array;

	static void APIENTRY execute(const RenderCommand_disableVertexArrayEXT* __restrict params) {
		glDisableVertexArrayEXT(
			params->vaobj,
			params->array
		);
	}
};
struct RenderCommand_enableVertexArrayAttribEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint index;

	static void APIENTRY execute(const RenderCommand_enableVertexArrayAttribEXT* __restrict params) {
		glEnableVertexArrayAttribEXT(
			params->vaobj,
			params->index
		);
	}
};
struct RenderCommand_disableVertexArrayAttribEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint index;

	static void APIENTRY execute(const RenderCommand_disableVertexArrayAttribEXT* __restrict params) {
		glDisableVertexArrayAttribEXT(
			params->vaobj,
			params->index
		);
	}
};
struct RenderCommand_getVertexArrayIntegervEXT : public RenderCommandBase {
	GLuint vaobj;
	GLenum pname;
	GLint *param;

	static void APIENTRY execute(const RenderCommand_getVertexArrayIntegervEXT* __restrict params) {
		glGetVertexArrayIntegervEXT(
			params->vaobj,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_getVertexArrayPointervEXT : public RenderCommandBase {
	GLuint vaobj;
	GLenum pname;
	void **param;

	static void APIENTRY execute(const RenderCommand_getVertexArrayPointervEXT* __restrict params) {
		glGetVertexArrayPointervEXT(
			params->vaobj,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_getVertexArrayIntegeri_vEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint index;
	GLenum pname;
	GLint *param;

	static void APIENTRY execute(const RenderCommand_getVertexArrayIntegeri_vEXT* __restrict params) {
		glGetVertexArrayIntegeri_vEXT(
			params->vaobj,
			params->index,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_getVertexArrayPointeri_vEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint index;
	GLenum pname;
	void **param;

	static void APIENTRY execute(const RenderCommand_getVertexArrayPointeri_vEXT* __restrict params) {
		glGetVertexArrayPointeri_vEXT(
			params->vaobj,
			params->index,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_mapNamedBufferRangeEXT : public RenderCommandBase {
	GLuint buffer;
	GLintptr offset;
	GLsizeiptr length;
	GLbitfield access;

	static void APIENTRY execute(const RenderCommand_mapNamedBufferRangeEXT* __restrict params) {
		glMapNamedBufferRangeEXT(
			params->buffer,
			params->offset,
			params->length,
			params->access
		);
	}
};
struct RenderCommand_flushMappedNamedBufferRangeEXT : public RenderCommandBase {
	GLuint buffer;
	GLintptr offset;
	GLsizeiptr length;

	static void APIENTRY execute(const RenderCommand_flushMappedNamedBufferRangeEXT* __restrict params) {
		glFlushMappedNamedBufferRangeEXT(
			params->buffer,
			params->offset,
			params->length
		);
	}
};
struct RenderCommand_namedBufferStorageEXT : public RenderCommandBase {
	GLuint buffer;
	GLsizeiptr size;
	const void *data;
	GLbitfield flags;

	static void APIENTRY execute(const RenderCommand_namedBufferStorageEXT* __restrict params) {
		glNamedBufferStorageEXT(
			params->buffer,
			params->size,
			params->data,
			params->flags
		);
	}
};
struct RenderCommand_clearNamedBufferDataEXT : public RenderCommandBase {
	GLuint buffer;
	GLenum internalformat;
	GLenum format;
	GLenum type;
	const void *data;

	static void APIENTRY execute(const RenderCommand_clearNamedBufferDataEXT* __restrict params) {
		glClearNamedBufferDataEXT(
			params->buffer,
			params->internalformat,
			params->format,
			params->type,
			params->data
		);
	}
};
struct RenderCommand_clearNamedBufferSubDataEXT : public RenderCommandBase {
	GLuint buffer;
	GLenum internalformat;
	GLsizeiptr offset;
	GLsizeiptr size;
	GLenum format;
	GLenum type;
	const void *data;

	static void APIENTRY execute(const RenderCommand_clearNamedBufferSubDataEXT* __restrict params) {
		glClearNamedBufferSubDataEXT(
			params->buffer,
			params->internalformat,
			params->offset,
			params->size,
			params->format,
			params->type,
			params->data
		);
	}
};
struct RenderCommand_namedFramebufferParameteriEXT : public RenderCommandBase {
	GLuint framebuffer;
	GLenum pname;
	GLint param;

	static void APIENTRY execute(const RenderCommand_namedFramebufferParameteriEXT* __restrict params) {
		glNamedFramebufferParameteriEXT(
			params->framebuffer,
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_getNamedFramebufferParameterivEXT : public RenderCommandBase {
	GLuint framebuffer;
	GLenum pname;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getNamedFramebufferParameterivEXT* __restrict params) {
		glGetNamedFramebufferParameterivEXT(
			params->framebuffer,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_programUniform1dEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLdouble x;

	static void APIENTRY execute(const RenderCommand_programUniform1dEXT* __restrict params) {
		glProgramUniform1dEXT(
			params->program,
			params->location,
			params->x
		);
	}
};
struct RenderCommand_programUniform2dEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLdouble x;
	GLdouble y;

	static void APIENTRY execute(const RenderCommand_programUniform2dEXT* __restrict params) {
		glProgramUniform2dEXT(
			params->program,
			params->location,
			params->x,
			params->y
		);
	}
};
struct RenderCommand_programUniform3dEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLdouble x;
	GLdouble y;
	GLdouble z;

	static void APIENTRY execute(const RenderCommand_programUniform3dEXT* __restrict params) {
		glProgramUniform3dEXT(
			params->program,
			params->location,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_programUniform4dEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLdouble x;
	GLdouble y;
	GLdouble z;
	GLdouble w;

	static void APIENTRY execute(const RenderCommand_programUniform4dEXT* __restrict params) {
		glProgramUniform4dEXT(
			params->program,
			params->location,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_programUniform1dvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniform1dvEXT* __restrict params) {
		glProgramUniform1dvEXT(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform2dvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniform2dvEXT* __restrict params) {
		glProgramUniform2dvEXT(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform3dvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniform3dvEXT* __restrict params) {
		glProgramUniform3dvEXT(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform4dvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniform4dvEXT* __restrict params) {
		glProgramUniform4dvEXT(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix2dvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix2dvEXT* __restrict params) {
		glProgramUniformMatrix2dvEXT(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix3dvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix3dvEXT* __restrict params) {
		glProgramUniformMatrix3dvEXT(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix4dvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix4dvEXT* __restrict params) {
		glProgramUniformMatrix4dvEXT(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix2x3dvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix2x3dvEXT* __restrict params) {
		glProgramUniformMatrix2x3dvEXT(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix2x4dvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix2x4dvEXT* __restrict params) {
		glProgramUniformMatrix2x4dvEXT(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix3x2dvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix3x2dvEXT* __restrict params) {
		glProgramUniformMatrix3x2dvEXT(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix3x4dvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix3x4dvEXT* __restrict params) {
		glProgramUniformMatrix3x4dvEXT(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix4x2dvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix4x2dvEXT* __restrict params) {
		glProgramUniformMatrix4x2dvEXT(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_programUniformMatrix4x3dvEXT : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	GLboolean transpose;
	const GLdouble *value;

	static void APIENTRY execute(const RenderCommand_programUniformMatrix4x3dvEXT* __restrict params) {
		glProgramUniformMatrix4x3dvEXT(
			params->program,
			params->location,
			params->count,
			params->transpose,
			params->value
		);
	}
};
struct RenderCommand_textureBufferRangeEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLenum internalformat;
	GLuint buffer;
	GLintptr offset;
	GLsizeiptr size;

	static void APIENTRY execute(const RenderCommand_textureBufferRangeEXT* __restrict params) {
		glTextureBufferRangeEXT(
			params->texture,
			params->target,
			params->internalformat,
			params->buffer,
			params->offset,
			params->size
		);
	}
};
struct RenderCommand_textureStorage1DEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLsizei levels;
	GLenum internalformat;
	GLsizei width;

	static void APIENTRY execute(const RenderCommand_textureStorage1DEXT* __restrict params) {
		glTextureStorage1DEXT(
			params->texture,
			params->target,
			params->levels,
			params->internalformat,
			params->width
		);
	}
};
struct RenderCommand_textureStorage2DEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLsizei levels;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_textureStorage2DEXT* __restrict params) {
		glTextureStorage2DEXT(
			params->texture,
			params->target,
			params->levels,
			params->internalformat,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_textureStorage3DEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLsizei levels;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;
	GLsizei depth;

	static void APIENTRY execute(const RenderCommand_textureStorage3DEXT* __restrict params) {
		glTextureStorage3DEXT(
			params->texture,
			params->target,
			params->levels,
			params->internalformat,
			params->width,
			params->height,
			params->depth
		);
	}
};
struct RenderCommand_textureStorage2DMultisampleEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLsizei samples;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;
	GLboolean fixedsamplelocations;

	static void APIENTRY execute(const RenderCommand_textureStorage2DMultisampleEXT* __restrict params) {
		glTextureStorage2DMultisampleEXT(
			params->texture,
			params->target,
			params->samples,
			params->internalformat,
			params->width,
			params->height,
			params->fixedsamplelocations
		);
	}
};
struct RenderCommand_textureStorage3DMultisampleEXT : public RenderCommandBase {
	GLuint texture;
	GLenum target;
	GLsizei samples;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLboolean fixedsamplelocations;

	static void APIENTRY execute(const RenderCommand_textureStorage3DMultisampleEXT* __restrict params) {
		glTextureStorage3DMultisampleEXT(
			params->texture,
			params->target,
			params->samples,
			params->internalformat,
			params->width,
			params->height,
			params->depth,
			params->fixedsamplelocations
		);
	}
};
struct RenderCommand_vertexArrayBindVertexBufferEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint bindingindex;
	GLuint buffer;
	GLintptr offset;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_vertexArrayBindVertexBufferEXT* __restrict params) {
		glVertexArrayBindVertexBufferEXT(
			params->vaobj,
			params->bindingindex,
			params->buffer,
			params->offset,
			params->stride
		);
	}
};
struct RenderCommand_vertexArrayVertexAttribFormatEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint attribindex;
	GLint size;
	GLenum type;
	GLboolean normalized;
	GLuint relativeoffset;

	static void APIENTRY execute(const RenderCommand_vertexArrayVertexAttribFormatEXT* __restrict params) {
		glVertexArrayVertexAttribFormatEXT(
			params->vaobj,
			params->attribindex,
			params->size,
			params->type,
			params->normalized,
			params->relativeoffset
		);
	}
};
struct RenderCommand_vertexArrayVertexAttribIFormatEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint attribindex;
	GLint size;
	GLenum type;
	GLuint relativeoffset;

	static void APIENTRY execute(const RenderCommand_vertexArrayVertexAttribIFormatEXT* __restrict params) {
		glVertexArrayVertexAttribIFormatEXT(
			params->vaobj,
			params->attribindex,
			params->size,
			params->type,
			params->relativeoffset
		);
	}
};
struct RenderCommand_vertexArrayVertexAttribLFormatEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint attribindex;
	GLint size;
	GLenum type;
	GLuint relativeoffset;

	static void APIENTRY execute(const RenderCommand_vertexArrayVertexAttribLFormatEXT* __restrict params) {
		glVertexArrayVertexAttribLFormatEXT(
			params->vaobj,
			params->attribindex,
			params->size,
			params->type,
			params->relativeoffset
		);
	}
};
struct RenderCommand_vertexArrayVertexAttribBindingEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint attribindex;
	GLuint bindingindex;

	static void APIENTRY execute(const RenderCommand_vertexArrayVertexAttribBindingEXT* __restrict params) {
		glVertexArrayVertexAttribBindingEXT(
			params->vaobj,
			params->attribindex,
			params->bindingindex
		);
	}
};
struct RenderCommand_vertexArrayVertexBindingDivisorEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint bindingindex;
	GLuint divisor;

	static void APIENTRY execute(const RenderCommand_vertexArrayVertexBindingDivisorEXT* __restrict params) {
		glVertexArrayVertexBindingDivisorEXT(
			params->vaobj,
			params->bindingindex,
			params->divisor
		);
	}
};
struct RenderCommand_vertexArrayVertexAttribLOffsetEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint buffer;
	GLuint index;
	GLint size;
	GLenum type;
	GLsizei stride;
	GLintptr offset;

	static void APIENTRY execute(const RenderCommand_vertexArrayVertexAttribLOffsetEXT* __restrict params) {
		glVertexArrayVertexAttribLOffsetEXT(
			params->vaobj,
			params->buffer,
			params->index,
			params->size,
			params->type,
			params->stride,
			params->offset
		);
	}
};
struct RenderCommand_texturePageCommitmentEXT : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLint xoffset;
	GLint yoffset;
	GLint zoffset;
	GLsizei width;
	GLsizei height;
	GLsizei depth;
	GLboolean commit;

	static void APIENTRY execute(const RenderCommand_texturePageCommitmentEXT* __restrict params) {
		glTexturePageCommitmentEXT(
			params->texture,
			params->level,
			params->xoffset,
			params->yoffset,
			params->zoffset,
			params->width,
			params->height,
			params->depth,
			params->commit
		);
	}
};
struct RenderCommand_vertexArrayVertexAttribDivisorEXT : public RenderCommandBase {
	GLuint vaobj;
	GLuint index;
	GLuint divisor;

	static void APIENTRY execute(const RenderCommand_vertexArrayVertexAttribDivisorEXT* __restrict params) {
		glVertexArrayVertexAttribDivisorEXT(
			params->vaobj,
			params->index,
			params->divisor
		);
	}
};
struct RenderCommand_drawArraysInstancedEXT : public RenderCommandBase {
	GLenum mode;
	GLint start;
	GLsizei count;
	GLsizei primcount;

	static void APIENTRY execute(const RenderCommand_drawArraysInstancedEXT* __restrict params) {
		glDrawArraysInstancedEXT(
			params->mode,
			params->start,
			params->count,
			params->primcount
		);
	}
};
struct RenderCommand_drawElementsInstancedEXT : public RenderCommandBase {
	GLenum mode;
	GLsizei count;
	GLenum type;
	const void *indices;
	GLsizei primcount;

	static void APIENTRY execute(const RenderCommand_drawElementsInstancedEXT* __restrict params) {
		glDrawElementsInstancedEXT(
			params->mode,
			params->count,
			params->type,
			params->indices,
			params->primcount
		);
	}
};
struct RenderCommand_polygonOffsetClampEXT : public RenderCommandBase {
	GLfloat factor;
	GLfloat units;
	GLfloat clamp;

	static void APIENTRY execute(const RenderCommand_polygonOffsetClampEXT* __restrict params) {
		glPolygonOffsetClampEXT(
			params->factor,
			params->units,
			params->clamp
		);
	}
};
struct RenderCommand_rasterSamplesEXT : public RenderCommandBase {
	GLuint samples;
	GLboolean fixedsamplelocations;

	static void APIENTRY execute(const RenderCommand_rasterSamplesEXT* __restrict params) {
		glRasterSamplesEXT(
			params->samples,
			params->fixedsamplelocations
		);
	}
};
struct RenderCommand_useShaderProgramEXT : public RenderCommandBase {
	GLenum type;
	GLuint program;

	static void APIENTRY execute(const RenderCommand_useShaderProgramEXT* __restrict params) {
		glUseShaderProgramEXT(
			params->type,
			params->program
		);
	}
};
struct RenderCommand_activeProgramEXT : public RenderCommandBase {
	GLuint program;

	static void APIENTRY execute(const RenderCommand_activeProgramEXT* __restrict params) {
		glActiveProgramEXT(
			params->program
		);
	}
};
struct RenderCommand_createShaderProgramEXT : public RenderCommandBase {
	GLenum type;
	const GLchar *string;

	static void APIENTRY execute(const RenderCommand_createShaderProgramEXT* __restrict params) {
		glCreateShaderProgramEXT(
			params->type,
			params->string
		);
	}
};
struct RenderCommand_framebufferFetchBarrierEXT : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_framebufferFetchBarrierEXT* __restrict params) {
		glFramebufferFetchBarrierEXT(
		);
	}
};
struct RenderCommand_windowRectanglesEXT : public RenderCommandBase {
	GLenum mode;
	GLsizei count;
	const GLint *box;

	static void APIENTRY execute(const RenderCommand_windowRectanglesEXT* __restrict params) {
		glWindowRectanglesEXT(
			params->mode,
			params->count,
			params->box
		);
	}
};
struct RenderCommand_applyFramebufferAttachmentCMAAINTEL : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_applyFramebufferAttachmentCMAAINTEL* __restrict params) {
		glApplyFramebufferAttachmentCMAAINTEL(
		);
	}
};
struct RenderCommand_beginPerfQueryINTEL : public RenderCommandBase {
	GLuint queryHandle;

	static void APIENTRY execute(const RenderCommand_beginPerfQueryINTEL* __restrict params) {
		glBeginPerfQueryINTEL(
			params->queryHandle
		);
	}
};
struct RenderCommand_createPerfQueryINTEL : public RenderCommandBase {
	GLuint queryId;
	GLuint *queryHandle;

	static void APIENTRY execute(const RenderCommand_createPerfQueryINTEL* __restrict params) {
		glCreatePerfQueryINTEL(
			params->queryId,
			params->queryHandle
		);
	}
};
struct RenderCommand_deletePerfQueryINTEL : public RenderCommandBase {
	GLuint queryHandle;

	static void APIENTRY execute(const RenderCommand_deletePerfQueryINTEL* __restrict params) {
		glDeletePerfQueryINTEL(
			params->queryHandle
		);
	}
};
struct RenderCommand_endPerfQueryINTEL : public RenderCommandBase {
	GLuint queryHandle;

	static void APIENTRY execute(const RenderCommand_endPerfQueryINTEL* __restrict params) {
		glEndPerfQueryINTEL(
			params->queryHandle
		);
	}
};
struct RenderCommand_getFirstPerfQueryIdINTEL : public RenderCommandBase {
	GLuint *queryId;

	static void APIENTRY execute(const RenderCommand_getFirstPerfQueryIdINTEL* __restrict params) {
		glGetFirstPerfQueryIdINTEL(
			params->queryId
		);
	}
};
struct RenderCommand_getNextPerfQueryIdINTEL : public RenderCommandBase {
	GLuint queryId;
	GLuint *nextQueryId;

	static void APIENTRY execute(const RenderCommand_getNextPerfQueryIdINTEL* __restrict params) {
		glGetNextPerfQueryIdINTEL(
			params->queryId,
			params->nextQueryId
		);
	}
};
struct RenderCommand_getPerfCounterInfoINTEL : public RenderCommandBase {
	GLuint queryId;
	GLuint counterId;
	GLuint counterNameLength;
	GLchar *counterName;
	GLuint counterDescLength;
	GLchar *counterDesc;
	GLuint *counterOffset;
	GLuint *counterDataSize;
	GLuint *counterTypeEnum;
	GLuint *counterDataTypeEnum;
	GLuint64 *rawCounterMaxValue;

	static void APIENTRY execute(const RenderCommand_getPerfCounterInfoINTEL* __restrict params) {
		glGetPerfCounterInfoINTEL(
			params->queryId,
			params->counterId,
			params->counterNameLength,
			params->counterName,
			params->counterDescLength,
			params->counterDesc,
			params->counterOffset,
			params->counterDataSize,
			params->counterTypeEnum,
			params->counterDataTypeEnum,
			params->rawCounterMaxValue
		);
	}
};
struct RenderCommand_getPerfQueryDataINTEL : public RenderCommandBase {
	GLuint queryHandle;
	GLuint flags;
	GLsizei dataSize;
	void *data;
	GLuint *bytesWritten;

	static void APIENTRY execute(const RenderCommand_getPerfQueryDataINTEL* __restrict params) {
		glGetPerfQueryDataINTEL(
			params->queryHandle,
			params->flags,
			params->dataSize,
			params->data,
			params->bytesWritten
		);
	}
};
struct RenderCommand_getPerfQueryIdByNameINTEL : public RenderCommandBase {
	GLchar *queryName;
	GLuint *queryId;

	static void APIENTRY execute(const RenderCommand_getPerfQueryIdByNameINTEL* __restrict params) {
		glGetPerfQueryIdByNameINTEL(
			params->queryName,
			params->queryId
		);
	}
};
struct RenderCommand_getPerfQueryInfoINTEL : public RenderCommandBase {
	GLuint queryId;
	GLuint queryNameLength;
	GLchar *queryName;
	GLuint *dataSize;
	GLuint *noCounters;
	GLuint *noInstances;
	GLuint *capsMask;

	static void APIENTRY execute(const RenderCommand_getPerfQueryInfoINTEL* __restrict params) {
		glGetPerfQueryInfoINTEL(
			params->queryId,
			params->queryNameLength,
			params->queryName,
			params->dataSize,
			params->noCounters,
			params->noInstances,
			params->capsMask
		);
	}
};
struct RenderCommand_multiDrawArraysIndirectBindlessNV : public RenderCommandBase {
	GLenum mode;
	const void *indirect;
	GLsizei drawCount;
	GLsizei stride;
	GLint vertexBufferCount;

	static void APIENTRY execute(const RenderCommand_multiDrawArraysIndirectBindlessNV* __restrict params) {
		glMultiDrawArraysIndirectBindlessNV(
			params->mode,
			params->indirect,
			params->drawCount,
			params->stride,
			params->vertexBufferCount
		);
	}
};
struct RenderCommand_multiDrawElementsIndirectBindlessNV : public RenderCommandBase {
	GLenum mode;
	GLenum type;
	const void *indirect;
	GLsizei drawCount;
	GLsizei stride;
	GLint vertexBufferCount;

	static void APIENTRY execute(const RenderCommand_multiDrawElementsIndirectBindlessNV* __restrict params) {
		glMultiDrawElementsIndirectBindlessNV(
			params->mode,
			params->type,
			params->indirect,
			params->drawCount,
			params->stride,
			params->vertexBufferCount
		);
	}
};
struct RenderCommand_multiDrawArraysIndirectBindlessCountNV : public RenderCommandBase {
	GLenum mode;
	const void *indirect;
	GLsizei drawCount;
	GLsizei maxDrawCount;
	GLsizei stride;
	GLint vertexBufferCount;

	static void APIENTRY execute(const RenderCommand_multiDrawArraysIndirectBindlessCountNV* __restrict params) {
		glMultiDrawArraysIndirectBindlessCountNV(
			params->mode,
			params->indirect,
			params->drawCount,
			params->maxDrawCount,
			params->stride,
			params->vertexBufferCount
		);
	}
};
struct RenderCommand_multiDrawElementsIndirectBindlessCountNV : public RenderCommandBase {
	GLenum mode;
	GLenum type;
	const void *indirect;
	GLsizei drawCount;
	GLsizei maxDrawCount;
	GLsizei stride;
	GLint vertexBufferCount;

	static void APIENTRY execute(const RenderCommand_multiDrawElementsIndirectBindlessCountNV* __restrict params) {
		glMultiDrawElementsIndirectBindlessCountNV(
			params->mode,
			params->type,
			params->indirect,
			params->drawCount,
			params->maxDrawCount,
			params->stride,
			params->vertexBufferCount
		);
	}
};
struct RenderCommand_getTextureHandleNV : public RenderCommandBase {
	GLuint texture;

	static void APIENTRY execute(const RenderCommand_getTextureHandleNV* __restrict params) {
		glGetTextureHandleNV(
			params->texture
		);
	}
};
struct RenderCommand_getTextureSamplerHandleNV : public RenderCommandBase {
	GLuint texture;
	GLuint sampler;

	static void APIENTRY execute(const RenderCommand_getTextureSamplerHandleNV* __restrict params) {
		glGetTextureSamplerHandleNV(
			params->texture,
			params->sampler
		);
	}
};
struct RenderCommand_makeTextureHandleResidentNV : public RenderCommandBase {
	GLuint64 handle;

	static void APIENTRY execute(const RenderCommand_makeTextureHandleResidentNV* __restrict params) {
		glMakeTextureHandleResidentNV(
			params->handle
		);
	}
};
struct RenderCommand_makeTextureHandleNonResidentNV : public RenderCommandBase {
	GLuint64 handle;

	static void APIENTRY execute(const RenderCommand_makeTextureHandleNonResidentNV* __restrict params) {
		glMakeTextureHandleNonResidentNV(
			params->handle
		);
	}
};
struct RenderCommand_getImageHandleNV : public RenderCommandBase {
	GLuint texture;
	GLint level;
	GLboolean layered;
	GLint layer;
	GLenum format;

	static void APIENTRY execute(const RenderCommand_getImageHandleNV* __restrict params) {
		glGetImageHandleNV(
			params->texture,
			params->level,
			params->layered,
			params->layer,
			params->format
		);
	}
};
struct RenderCommand_makeImageHandleResidentNV : public RenderCommandBase {
	GLuint64 handle;
	GLenum access;

	static void APIENTRY execute(const RenderCommand_makeImageHandleResidentNV* __restrict params) {
		glMakeImageHandleResidentNV(
			params->handle,
			params->access
		);
	}
};
struct RenderCommand_makeImageHandleNonResidentNV : public RenderCommandBase {
	GLuint64 handle;

	static void APIENTRY execute(const RenderCommand_makeImageHandleNonResidentNV* __restrict params) {
		glMakeImageHandleNonResidentNV(
			params->handle
		);
	}
};
struct RenderCommand_uniformHandleui64NV : public RenderCommandBase {
	GLint location;
	GLuint64 value;

	static void APIENTRY execute(const RenderCommand_uniformHandleui64NV* __restrict params) {
		glUniformHandleui64NV(
			params->location,
			params->value
		);
	}
};
struct RenderCommand_uniformHandleui64vNV : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLuint64 *value;

	static void APIENTRY execute(const RenderCommand_uniformHandleui64vNV* __restrict params) {
		glUniformHandleui64vNV(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniformHandleui64NV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint64 value;

	static void APIENTRY execute(const RenderCommand_programUniformHandleui64NV* __restrict params) {
		glProgramUniformHandleui64NV(
			params->program,
			params->location,
			params->value
		);
	}
};
struct RenderCommand_programUniformHandleui64vNV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLuint64 *values;

	static void APIENTRY execute(const RenderCommand_programUniformHandleui64vNV* __restrict params) {
		glProgramUniformHandleui64vNV(
			params->program,
			params->location,
			params->count,
			params->values
		);
	}
};
struct RenderCommand_isTextureHandleResidentNV : public RenderCommandBase {
	GLuint64 handle;

	static void APIENTRY execute(const RenderCommand_isTextureHandleResidentNV* __restrict params) {
		glIsTextureHandleResidentNV(
			params->handle
		);
	}
};
struct RenderCommand_isImageHandleResidentNV : public RenderCommandBase {
	GLuint64 handle;

	static void APIENTRY execute(const RenderCommand_isImageHandleResidentNV* __restrict params) {
		glIsImageHandleResidentNV(
			params->handle
		);
	}
};
struct RenderCommand_blendParameteriNV : public RenderCommandBase {
	GLenum pname;
	GLint value;

	static void APIENTRY execute(const RenderCommand_blendParameteriNV* __restrict params) {
		glBlendParameteriNV(
			params->pname,
			params->value
		);
	}
};
struct RenderCommand_blendBarrierNV : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_blendBarrierNV* __restrict params) {
		glBlendBarrierNV(
		);
	}
};
struct RenderCommand_viewportPositionWScaleNV : public RenderCommandBase {
	GLuint index;
	GLfloat xcoeff;
	GLfloat ycoeff;

	static void APIENTRY execute(const RenderCommand_viewportPositionWScaleNV* __restrict params) {
		glViewportPositionWScaleNV(
			params->index,
			params->xcoeff,
			params->ycoeff
		);
	}
};
struct RenderCommand_createStatesNV : public RenderCommandBase {
	GLsizei n;
	GLuint *states;

	static void APIENTRY execute(const RenderCommand_createStatesNV* __restrict params) {
		glCreateStatesNV(
			params->n,
			params->states
		);
	}
};
struct RenderCommand_deleteStatesNV : public RenderCommandBase {
	GLsizei n;
	const GLuint *states;

	static void APIENTRY execute(const RenderCommand_deleteStatesNV* __restrict params) {
		glDeleteStatesNV(
			params->n,
			params->states
		);
	}
};
struct RenderCommand_isStateNV : public RenderCommandBase {
	GLuint state;

	static void APIENTRY execute(const RenderCommand_isStateNV* __restrict params) {
		glIsStateNV(
			params->state
		);
	}
};
struct RenderCommand_stateCaptureNV : public RenderCommandBase {
	GLuint state;
	GLenum mode;

	static void APIENTRY execute(const RenderCommand_stateCaptureNV* __restrict params) {
		glStateCaptureNV(
			params->state,
			params->mode
		);
	}
};
struct RenderCommand_getCommandHeaderNV : public RenderCommandBase {
	GLenum tokenID;
	GLuint size;

	static void APIENTRY execute(const RenderCommand_getCommandHeaderNV* __restrict params) {
		glGetCommandHeaderNV(
			params->tokenID,
			params->size
		);
	}
};
struct RenderCommand_getStageIndexNV : public RenderCommandBase {
	GLenum shadertype;

	static void APIENTRY execute(const RenderCommand_getStageIndexNV* __restrict params) {
		glGetStageIndexNV(
			params->shadertype
		);
	}
};
struct RenderCommand_drawCommandsNV : public RenderCommandBase {
	GLenum primitiveMode;
	GLuint buffer;
	const GLintptr *indirects;
	const GLsizei *sizes;
	GLuint count;

	static void APIENTRY execute(const RenderCommand_drawCommandsNV* __restrict params) {
		glDrawCommandsNV(
			params->primitiveMode,
			params->buffer,
			params->indirects,
			params->sizes,
			params->count
		);
	}
};
struct RenderCommand_drawCommandsAddressNV : public RenderCommandBase {
	GLenum primitiveMode;
	const GLuint64 *indirects;
	const GLsizei *sizes;
	GLuint count;

	static void APIENTRY execute(const RenderCommand_drawCommandsAddressNV* __restrict params) {
		glDrawCommandsAddressNV(
			params->primitiveMode,
			params->indirects,
			params->sizes,
			params->count
		);
	}
};
struct RenderCommand_drawCommandsStatesNV : public RenderCommandBase {
	GLuint buffer;
	const GLintptr *indirects;
	const GLsizei *sizes;
	const GLuint *states;
	const GLuint *fbos;
	GLuint count;

	static void APIENTRY execute(const RenderCommand_drawCommandsStatesNV* __restrict params) {
		glDrawCommandsStatesNV(
			params->buffer,
			params->indirects,
			params->sizes,
			params->states,
			params->fbos,
			params->count
		);
	}
};
struct RenderCommand_drawCommandsStatesAddressNV : public RenderCommandBase {
	const GLuint64 *indirects;
	const GLsizei *sizes;
	const GLuint *states;
	const GLuint *fbos;
	GLuint count;

	static void APIENTRY execute(const RenderCommand_drawCommandsStatesAddressNV* __restrict params) {
		glDrawCommandsStatesAddressNV(
			params->indirects,
			params->sizes,
			params->states,
			params->fbos,
			params->count
		);
	}
};
struct RenderCommand_createCommandListsNV : public RenderCommandBase {
	GLsizei n;
	GLuint *lists;

	static void APIENTRY execute(const RenderCommand_createCommandListsNV* __restrict params) {
		glCreateCommandListsNV(
			params->n,
			params->lists
		);
	}
};
struct RenderCommand_deleteCommandListsNV : public RenderCommandBase {
	GLsizei n;
	const GLuint *lists;

	static void APIENTRY execute(const RenderCommand_deleteCommandListsNV* __restrict params) {
		glDeleteCommandListsNV(
			params->n,
			params->lists
		);
	}
};
struct RenderCommand_isCommandListNV : public RenderCommandBase {
	GLuint list;

	static void APIENTRY execute(const RenderCommand_isCommandListNV* __restrict params) {
		glIsCommandListNV(
			params->list
		);
	}
};
struct RenderCommand_listDrawCommandsStatesClientNV : public RenderCommandBase {
	GLuint list;
	GLuint segment;
	const void **indirects;
	const GLsizei *sizes;
	const GLuint *states;
	const GLuint *fbos;
	GLuint count;

	static void APIENTRY execute(const RenderCommand_listDrawCommandsStatesClientNV* __restrict params) {
		glListDrawCommandsStatesClientNV(
			params->list,
			params->segment,
			params->indirects,
			params->sizes,
			params->states,
			params->fbos,
			params->count
		);
	}
};
struct RenderCommand_commandListSegmentsNV : public RenderCommandBase {
	GLuint list;
	GLuint segments;

	static void APIENTRY execute(const RenderCommand_commandListSegmentsNV* __restrict params) {
		glCommandListSegmentsNV(
			params->list,
			params->segments
		);
	}
};
struct RenderCommand_compileCommandListNV : public RenderCommandBase {
	GLuint list;

	static void APIENTRY execute(const RenderCommand_compileCommandListNV* __restrict params) {
		glCompileCommandListNV(
			params->list
		);
	}
};
struct RenderCommand_callCommandListNV : public RenderCommandBase {
	GLuint list;

	static void APIENTRY execute(const RenderCommand_callCommandListNV* __restrict params) {
		glCallCommandListNV(
			params->list
		);
	}
};
struct RenderCommand_beginConditionalRenderNV : public RenderCommandBase {
	GLuint id;
	GLenum mode;

	static void APIENTRY execute(const RenderCommand_beginConditionalRenderNV* __restrict params) {
		glBeginConditionalRenderNV(
			params->id,
			params->mode
		);
	}
};
struct RenderCommand_endConditionalRenderNV : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_endConditionalRenderNV* __restrict params) {
		glEndConditionalRenderNV(
		);
	}
};
struct RenderCommand_subpixelPrecisionBiasNV : public RenderCommandBase {
	GLuint xbits;
	GLuint ybits;

	static void APIENTRY execute(const RenderCommand_subpixelPrecisionBiasNV* __restrict params) {
		glSubpixelPrecisionBiasNV(
			params->xbits,
			params->ybits
		);
	}
};
struct RenderCommand_conservativeRasterParameterfNV : public RenderCommandBase {
	GLenum pname;
	GLfloat value;

	static void APIENTRY execute(const RenderCommand_conservativeRasterParameterfNV* __restrict params) {
		glConservativeRasterParameterfNV(
			params->pname,
			params->value
		);
	}
};
struct RenderCommand_conservativeRasterParameteriNV : public RenderCommandBase {
	GLenum pname;
	GLint param;

	static void APIENTRY execute(const RenderCommand_conservativeRasterParameteriNV* __restrict params) {
		glConservativeRasterParameteriNV(
			params->pname,
			params->param
		);
	}
};
struct RenderCommand_drawVkImageNV : public RenderCommandBase {
	GLuint64 vkImage;
	GLuint sampler;
	GLfloat x0;
	GLfloat y0;
	GLfloat x1;
	GLfloat y1;
	GLfloat z;
	GLfloat s0;
	GLfloat t0;
	GLfloat s1;
	GLfloat t1;

	static void APIENTRY execute(const RenderCommand_drawVkImageNV* __restrict params) {
		glDrawVkImageNV(
			params->vkImage,
			params->sampler,
			params->x0,
			params->y0,
			params->x1,
			params->y1,
			params->z,
			params->s0,
			params->t0,
			params->s1,
			params->t1
		);
	}
};
struct RenderCommand_getVkProcAddrNV : public RenderCommandBase {
	const GLchar *name;

	static void APIENTRY execute(const RenderCommand_getVkProcAddrNV* __restrict params) {
		glGetVkProcAddrNV(
			params->name
		);
	}
};
struct RenderCommand_waitVkSemaphoreNV : public RenderCommandBase {
	GLuint64 vkSemaphore;

	static void APIENTRY execute(const RenderCommand_waitVkSemaphoreNV* __restrict params) {
		glWaitVkSemaphoreNV(
			params->vkSemaphore
		);
	}
};
struct RenderCommand_signalVkSemaphoreNV : public RenderCommandBase {
	GLuint64 vkSemaphore;

	static void APIENTRY execute(const RenderCommand_signalVkSemaphoreNV* __restrict params) {
		glSignalVkSemaphoreNV(
			params->vkSemaphore
		);
	}
};
struct RenderCommand_signalVkFenceNV : public RenderCommandBase {
	GLuint64 vkFence;

	static void APIENTRY execute(const RenderCommand_signalVkFenceNV* __restrict params) {
		glSignalVkFenceNV(
			params->vkFence
		);
	}
};
struct RenderCommand_fragmentCoverageColorNV : public RenderCommandBase {
	GLuint color;

	static void APIENTRY execute(const RenderCommand_fragmentCoverageColorNV* __restrict params) {
		glFragmentCoverageColorNV(
			params->color
		);
	}
};
struct RenderCommand_coverageModulationTableNV : public RenderCommandBase {
	GLsizei n;
	const GLfloat *v;

	static void APIENTRY execute(const RenderCommand_coverageModulationTableNV* __restrict params) {
		glCoverageModulationTableNV(
			params->n,
			params->v
		);
	}
};
struct RenderCommand_getCoverageModulationTableNV : public RenderCommandBase {
	GLsizei bufsize;
	GLfloat *v;

	static void APIENTRY execute(const RenderCommand_getCoverageModulationTableNV* __restrict params) {
		glGetCoverageModulationTableNV(
			params->bufsize,
			params->v
		);
	}
};
struct RenderCommand_coverageModulationNV : public RenderCommandBase {
	GLenum components;

	static void APIENTRY execute(const RenderCommand_coverageModulationNV* __restrict params) {
		glCoverageModulationNV(
			params->components
		);
	}
};
struct RenderCommand_renderbufferStorageMultisampleCoverageNV : public RenderCommandBase {
	GLenum target;
	GLsizei coverageSamples;
	GLsizei colorSamples;
	GLenum internalformat;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_renderbufferStorageMultisampleCoverageNV* __restrict params) {
		glRenderbufferStorageMultisampleCoverageNV(
			params->target,
			params->coverageSamples,
			params->colorSamples,
			params->internalformat,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_uniform1i64NV : public RenderCommandBase {
	GLint location;
	GLint64EXT x;

	static void APIENTRY execute(const RenderCommand_uniform1i64NV* __restrict params) {
		glUniform1i64NV(
			params->location,
			params->x
		);
	}
};
struct RenderCommand_uniform2i64NV : public RenderCommandBase {
	GLint location;
	GLint64EXT x;
	GLint64EXT y;

	static void APIENTRY execute(const RenderCommand_uniform2i64NV* __restrict params) {
		glUniform2i64NV(
			params->location,
			params->x,
			params->y
		);
	}
};
struct RenderCommand_uniform3i64NV : public RenderCommandBase {
	GLint location;
	GLint64EXT x;
	GLint64EXT y;
	GLint64EXT z;

	static void APIENTRY execute(const RenderCommand_uniform3i64NV* __restrict params) {
		glUniform3i64NV(
			params->location,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_uniform4i64NV : public RenderCommandBase {
	GLint location;
	GLint64EXT x;
	GLint64EXT y;
	GLint64EXT z;
	GLint64EXT w;

	static void APIENTRY execute(const RenderCommand_uniform4i64NV* __restrict params) {
		glUniform4i64NV(
			params->location,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_uniform1i64vNV : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLint64EXT *value;

	static void APIENTRY execute(const RenderCommand_uniform1i64vNV* __restrict params) {
		glUniform1i64vNV(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform2i64vNV : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLint64EXT *value;

	static void APIENTRY execute(const RenderCommand_uniform2i64vNV* __restrict params) {
		glUniform2i64vNV(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform3i64vNV : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLint64EXT *value;

	static void APIENTRY execute(const RenderCommand_uniform3i64vNV* __restrict params) {
		glUniform3i64vNV(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform4i64vNV : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLint64EXT *value;

	static void APIENTRY execute(const RenderCommand_uniform4i64vNV* __restrict params) {
		glUniform4i64vNV(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform1ui64NV : public RenderCommandBase {
	GLint location;
	GLuint64EXT x;

	static void APIENTRY execute(const RenderCommand_uniform1ui64NV* __restrict params) {
		glUniform1ui64NV(
			params->location,
			params->x
		);
	}
};
struct RenderCommand_uniform2ui64NV : public RenderCommandBase {
	GLint location;
	GLuint64EXT x;
	GLuint64EXT y;

	static void APIENTRY execute(const RenderCommand_uniform2ui64NV* __restrict params) {
		glUniform2ui64NV(
			params->location,
			params->x,
			params->y
		);
	}
};
struct RenderCommand_uniform3ui64NV : public RenderCommandBase {
	GLint location;
	GLuint64EXT x;
	GLuint64EXT y;
	GLuint64EXT z;

	static void APIENTRY execute(const RenderCommand_uniform3ui64NV* __restrict params) {
		glUniform3ui64NV(
			params->location,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_uniform4ui64NV : public RenderCommandBase {
	GLint location;
	GLuint64EXT x;
	GLuint64EXT y;
	GLuint64EXT z;
	GLuint64EXT w;

	static void APIENTRY execute(const RenderCommand_uniform4ui64NV* __restrict params) {
		glUniform4ui64NV(
			params->location,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_uniform1ui64vNV : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLuint64EXT *value;

	static void APIENTRY execute(const RenderCommand_uniform1ui64vNV* __restrict params) {
		glUniform1ui64vNV(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform2ui64vNV : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLuint64EXT *value;

	static void APIENTRY execute(const RenderCommand_uniform2ui64vNV* __restrict params) {
		glUniform2ui64vNV(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform3ui64vNV : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLuint64EXT *value;

	static void APIENTRY execute(const RenderCommand_uniform3ui64vNV* __restrict params) {
		glUniform3ui64vNV(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_uniform4ui64vNV : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLuint64EXT *value;

	static void APIENTRY execute(const RenderCommand_uniform4ui64vNV* __restrict params) {
		glUniform4ui64vNV(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_getUniformi64vNV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLint64EXT *params;

	static void APIENTRY execute(const RenderCommand_getUniformi64vNV* __restrict params) {
		glGetUniformi64vNV(
			params->program,
			params->location,
			params->params
		);
	}
};
struct RenderCommand_programUniform1i64NV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLint64EXT x;

	static void APIENTRY execute(const RenderCommand_programUniform1i64NV* __restrict params) {
		glProgramUniform1i64NV(
			params->program,
			params->location,
			params->x
		);
	}
};
struct RenderCommand_programUniform2i64NV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLint64EXT x;
	GLint64EXT y;

	static void APIENTRY execute(const RenderCommand_programUniform2i64NV* __restrict params) {
		glProgramUniform2i64NV(
			params->program,
			params->location,
			params->x,
			params->y
		);
	}
};
struct RenderCommand_programUniform3i64NV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLint64EXT x;
	GLint64EXT y;
	GLint64EXT z;

	static void APIENTRY execute(const RenderCommand_programUniform3i64NV* __restrict params) {
		glProgramUniform3i64NV(
			params->program,
			params->location,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_programUniform4i64NV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLint64EXT x;
	GLint64EXT y;
	GLint64EXT z;
	GLint64EXT w;

	static void APIENTRY execute(const RenderCommand_programUniform4i64NV* __restrict params) {
		glProgramUniform4i64NV(
			params->program,
			params->location,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_programUniform1i64vNV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLint64EXT *value;

	static void APIENTRY execute(const RenderCommand_programUniform1i64vNV* __restrict params) {
		glProgramUniform1i64vNV(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform2i64vNV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLint64EXT *value;

	static void APIENTRY execute(const RenderCommand_programUniform2i64vNV* __restrict params) {
		glProgramUniform2i64vNV(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform3i64vNV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLint64EXT *value;

	static void APIENTRY execute(const RenderCommand_programUniform3i64vNV* __restrict params) {
		glProgramUniform3i64vNV(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform4i64vNV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLint64EXT *value;

	static void APIENTRY execute(const RenderCommand_programUniform4i64vNV* __restrict params) {
		glProgramUniform4i64vNV(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform1ui64NV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint64EXT x;

	static void APIENTRY execute(const RenderCommand_programUniform1ui64NV* __restrict params) {
		glProgramUniform1ui64NV(
			params->program,
			params->location,
			params->x
		);
	}
};
struct RenderCommand_programUniform2ui64NV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint64EXT x;
	GLuint64EXT y;

	static void APIENTRY execute(const RenderCommand_programUniform2ui64NV* __restrict params) {
		glProgramUniform2ui64NV(
			params->program,
			params->location,
			params->x,
			params->y
		);
	}
};
struct RenderCommand_programUniform3ui64NV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint64EXT x;
	GLuint64EXT y;
	GLuint64EXT z;

	static void APIENTRY execute(const RenderCommand_programUniform3ui64NV* __restrict params) {
		glProgramUniform3ui64NV(
			params->program,
			params->location,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_programUniform4ui64NV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint64EXT x;
	GLuint64EXT y;
	GLuint64EXT z;
	GLuint64EXT w;

	static void APIENTRY execute(const RenderCommand_programUniform4ui64NV* __restrict params) {
		glProgramUniform4ui64NV(
			params->program,
			params->location,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_programUniform1ui64vNV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLuint64EXT *value;

	static void APIENTRY execute(const RenderCommand_programUniform1ui64vNV* __restrict params) {
		glProgramUniform1ui64vNV(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform2ui64vNV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLuint64EXT *value;

	static void APIENTRY execute(const RenderCommand_programUniform2ui64vNV* __restrict params) {
		glProgramUniform2ui64vNV(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform3ui64vNV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLuint64EXT *value;

	static void APIENTRY execute(const RenderCommand_programUniform3ui64vNV* __restrict params) {
		glProgramUniform3ui64vNV(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_programUniform4ui64vNV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLuint64EXT *value;

	static void APIENTRY execute(const RenderCommand_programUniform4ui64vNV* __restrict params) {
		glProgramUniform4ui64vNV(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_getInternalformatSampleivNV : public RenderCommandBase {
	GLenum target;
	GLenum internalformat;
	GLsizei samples;
	GLenum pname;
	GLsizei bufSize;
	GLint *params;

	static void APIENTRY execute(const RenderCommand_getInternalformatSampleivNV* __restrict params) {
		glGetInternalformatSampleivNV(
			params->target,
			params->internalformat,
			params->samples,
			params->pname,
			params->bufSize,
			params->params
		);
	}
};
struct RenderCommand_getMemoryObjectDetachedResourcesuivNV : public RenderCommandBase {
	GLuint memory;
	GLenum pname;
	GLint first;
	GLsizei count;
	GLuint *params;

	static void APIENTRY execute(const RenderCommand_getMemoryObjectDetachedResourcesuivNV* __restrict params) {
		glGetMemoryObjectDetachedResourcesuivNV(
			params->memory,
			params->pname,
			params->first,
			params->count,
			params->params
		);
	}
};
struct RenderCommand_resetMemoryObjectParameterNV : public RenderCommandBase {
	GLuint memory;
	GLenum pname;

	static void APIENTRY execute(const RenderCommand_resetMemoryObjectParameterNV* __restrict params) {
		glResetMemoryObjectParameterNV(
			params->memory,
			params->pname
		);
	}
};
struct RenderCommand_texAttachMemoryNV : public RenderCommandBase {
	GLenum target;
	GLuint memory;
	GLuint64 offset;

	static void APIENTRY execute(const RenderCommand_texAttachMemoryNV* __restrict params) {
		glTexAttachMemoryNV(
			params->target,
			params->memory,
			params->offset
		);
	}
};
struct RenderCommand_bufferAttachMemoryNV : public RenderCommandBase {
	GLenum target;
	GLuint memory;
	GLuint64 offset;

	static void APIENTRY execute(const RenderCommand_bufferAttachMemoryNV* __restrict params) {
		glBufferAttachMemoryNV(
			params->target,
			params->memory,
			params->offset
		);
	}
};
struct RenderCommand_textureAttachMemoryNV : public RenderCommandBase {
	GLuint texture;
	GLuint memory;
	GLuint64 offset;

	static void APIENTRY execute(const RenderCommand_textureAttachMemoryNV* __restrict params) {
		glTextureAttachMemoryNV(
			params->texture,
			params->memory,
			params->offset
		);
	}
};
struct RenderCommand_namedBufferAttachMemoryNV : public RenderCommandBase {
	GLuint buffer;
	GLuint memory;
	GLuint64 offset;

	static void APIENTRY execute(const RenderCommand_namedBufferAttachMemoryNV* __restrict params) {
		glNamedBufferAttachMemoryNV(
			params->buffer,
			params->memory,
			params->offset
		);
	}
};
struct RenderCommand_drawMeshTasksNV : public RenderCommandBase {
	GLuint first;
	GLuint count;

	static void APIENTRY execute(const RenderCommand_drawMeshTasksNV* __restrict params) {
		glDrawMeshTasksNV(
			params->first,
			params->count
		);
	}
};
struct RenderCommand_drawMeshTasksIndirectNV : public RenderCommandBase {
	GLintptr indirect;

	static void APIENTRY execute(const RenderCommand_drawMeshTasksIndirectNV* __restrict params) {
		glDrawMeshTasksIndirectNV(
			params->indirect
		);
	}
};
struct RenderCommand_multiDrawMeshTasksIndirectNV : public RenderCommandBase {
	GLintptr indirect;
	GLsizei drawcount;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_multiDrawMeshTasksIndirectNV* __restrict params) {
		glMultiDrawMeshTasksIndirectNV(
			params->indirect,
			params->drawcount,
			params->stride
		);
	}
};
struct RenderCommand_multiDrawMeshTasksIndirectCountNV : public RenderCommandBase {
	GLintptr indirect;
	GLintptr drawcount;
	GLsizei maxdrawcount;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_multiDrawMeshTasksIndirectCountNV* __restrict params) {
		glMultiDrawMeshTasksIndirectCountNV(
			params->indirect,
			params->drawcount,
			params->maxdrawcount,
			params->stride
		);
	}
};
struct RenderCommand_genPathsNV : public RenderCommandBase {
	GLsizei range;

	static void APIENTRY execute(const RenderCommand_genPathsNV* __restrict params) {
		glGenPathsNV(
			params->range
		);
	}
};
struct RenderCommand_deletePathsNV : public RenderCommandBase {
	GLuint path;
	GLsizei range;

	static void APIENTRY execute(const RenderCommand_deletePathsNV* __restrict params) {
		glDeletePathsNV(
			params->path,
			params->range
		);
	}
};
struct RenderCommand_isPathNV : public RenderCommandBase {
	GLuint path;

	static void APIENTRY execute(const RenderCommand_isPathNV* __restrict params) {
		glIsPathNV(
			params->path
		);
	}
};
struct RenderCommand_pathCommandsNV : public RenderCommandBase {
	GLuint path;
	GLsizei numCommands;
	const GLubyte *commands;
	GLsizei numCoords;
	GLenum coordType;
	const void *coords;

	static void APIENTRY execute(const RenderCommand_pathCommandsNV* __restrict params) {
		glPathCommandsNV(
			params->path,
			params->numCommands,
			params->commands,
			params->numCoords,
			params->coordType,
			params->coords
		);
	}
};
struct RenderCommand_pathCoordsNV : public RenderCommandBase {
	GLuint path;
	GLsizei numCoords;
	GLenum coordType;
	const void *coords;

	static void APIENTRY execute(const RenderCommand_pathCoordsNV* __restrict params) {
		glPathCoordsNV(
			params->path,
			params->numCoords,
			params->coordType,
			params->coords
		);
	}
};
struct RenderCommand_pathSubCommandsNV : public RenderCommandBase {
	GLuint path;
	GLsizei commandStart;
	GLsizei commandsToDelete;
	GLsizei numCommands;
	const GLubyte *commands;
	GLsizei numCoords;
	GLenum coordType;
	const void *coords;

	static void APIENTRY execute(const RenderCommand_pathSubCommandsNV* __restrict params) {
		glPathSubCommandsNV(
			params->path,
			params->commandStart,
			params->commandsToDelete,
			params->numCommands,
			params->commands,
			params->numCoords,
			params->coordType,
			params->coords
		);
	}
};
struct RenderCommand_pathSubCoordsNV : public RenderCommandBase {
	GLuint path;
	GLsizei coordStart;
	GLsizei numCoords;
	GLenum coordType;
	const void *coords;

	static void APIENTRY execute(const RenderCommand_pathSubCoordsNV* __restrict params) {
		glPathSubCoordsNV(
			params->path,
			params->coordStart,
			params->numCoords,
			params->coordType,
			params->coords
		);
	}
};
struct RenderCommand_pathStringNV : public RenderCommandBase {
	GLuint path;
	GLenum format;
	GLsizei length;
	const void *pathString;

	static void APIENTRY execute(const RenderCommand_pathStringNV* __restrict params) {
		glPathStringNV(
			params->path,
			params->format,
			params->length,
			params->pathString
		);
	}
};
struct RenderCommand_pathGlyphsNV : public RenderCommandBase {
	GLuint firstPathName;
	GLenum fontTarget;
	const void *fontName;
	GLbitfield fontStyle;
	GLsizei numGlyphs;
	GLenum type;
	const void *charcodes;
	GLenum handleMissingGlyphs;
	GLuint pathParameterTemplate;
	GLfloat emScale;

	static void APIENTRY execute(const RenderCommand_pathGlyphsNV* __restrict params) {
		glPathGlyphsNV(
			params->firstPathName,
			params->fontTarget,
			params->fontName,
			params->fontStyle,
			params->numGlyphs,
			params->type,
			params->charcodes,
			params->handleMissingGlyphs,
			params->pathParameterTemplate,
			params->emScale
		);
	}
};
struct RenderCommand_pathGlyphRangeNV : public RenderCommandBase {
	GLuint firstPathName;
	GLenum fontTarget;
	const void *fontName;
	GLbitfield fontStyle;
	GLuint firstGlyph;
	GLsizei numGlyphs;
	GLenum handleMissingGlyphs;
	GLuint pathParameterTemplate;
	GLfloat emScale;

	static void APIENTRY execute(const RenderCommand_pathGlyphRangeNV* __restrict params) {
		glPathGlyphRangeNV(
			params->firstPathName,
			params->fontTarget,
			params->fontName,
			params->fontStyle,
			params->firstGlyph,
			params->numGlyphs,
			params->handleMissingGlyphs,
			params->pathParameterTemplate,
			params->emScale
		);
	}
};
struct RenderCommand_weightPathsNV : public RenderCommandBase {
	GLuint resultPath;
	GLsizei numPaths;
	const GLuint *paths;
	const GLfloat *weights;

	static void APIENTRY execute(const RenderCommand_weightPathsNV* __restrict params) {
		glWeightPathsNV(
			params->resultPath,
			params->numPaths,
			params->paths,
			params->weights
		);
	}
};
struct RenderCommand_copyPathNV : public RenderCommandBase {
	GLuint resultPath;
	GLuint srcPath;

	static void APIENTRY execute(const RenderCommand_copyPathNV* __restrict params) {
		glCopyPathNV(
			params->resultPath,
			params->srcPath
		);
	}
};
struct RenderCommand_interpolatePathsNV : public RenderCommandBase {
	GLuint resultPath;
	GLuint pathA;
	GLuint pathB;
	GLfloat weight;

	static void APIENTRY execute(const RenderCommand_interpolatePathsNV* __restrict params) {
		glInterpolatePathsNV(
			params->resultPath,
			params->pathA,
			params->pathB,
			params->weight
		);
	}
};
struct RenderCommand_transformPathNV : public RenderCommandBase {
	GLuint resultPath;
	GLuint srcPath;
	GLenum transformType;
	const GLfloat *transformValues;

	static void APIENTRY execute(const RenderCommand_transformPathNV* __restrict params) {
		glTransformPathNV(
			params->resultPath,
			params->srcPath,
			params->transformType,
			params->transformValues
		);
	}
};
struct RenderCommand_pathParameterivNV : public RenderCommandBase {
	GLuint path;
	GLenum pname;
	const GLint *value;

	static void APIENTRY execute(const RenderCommand_pathParameterivNV* __restrict params) {
		glPathParameterivNV(
			params->path,
			params->pname,
			params->value
		);
	}
};
struct RenderCommand_pathParameteriNV : public RenderCommandBase {
	GLuint path;
	GLenum pname;
	GLint value;

	static void APIENTRY execute(const RenderCommand_pathParameteriNV* __restrict params) {
		glPathParameteriNV(
			params->path,
			params->pname,
			params->value
		);
	}
};
struct RenderCommand_pathParameterfvNV : public RenderCommandBase {
	GLuint path;
	GLenum pname;
	const GLfloat *value;

	static void APIENTRY execute(const RenderCommand_pathParameterfvNV* __restrict params) {
		glPathParameterfvNV(
			params->path,
			params->pname,
			params->value
		);
	}
};
struct RenderCommand_pathParameterfNV : public RenderCommandBase {
	GLuint path;
	GLenum pname;
	GLfloat value;

	static void APIENTRY execute(const RenderCommand_pathParameterfNV* __restrict params) {
		glPathParameterfNV(
			params->path,
			params->pname,
			params->value
		);
	}
};
struct RenderCommand_pathDashArrayNV : public RenderCommandBase {
	GLuint path;
	GLsizei dashCount;
	const GLfloat *dashArray;

	static void APIENTRY execute(const RenderCommand_pathDashArrayNV* __restrict params) {
		glPathDashArrayNV(
			params->path,
			params->dashCount,
			params->dashArray
		);
	}
};
struct RenderCommand_pathStencilFuncNV : public RenderCommandBase {
	GLenum func;
	GLint ref;
	GLuint mask;

	static void APIENTRY execute(const RenderCommand_pathStencilFuncNV* __restrict params) {
		glPathStencilFuncNV(
			params->func,
			params->ref,
			params->mask
		);
	}
};
struct RenderCommand_pathStencilDepthOffsetNV : public RenderCommandBase {
	GLfloat factor;
	GLfloat units;

	static void APIENTRY execute(const RenderCommand_pathStencilDepthOffsetNV* __restrict params) {
		glPathStencilDepthOffsetNV(
			params->factor,
			params->units
		);
	}
};
struct RenderCommand_stencilFillPathNV : public RenderCommandBase {
	GLuint path;
	GLenum fillMode;
	GLuint mask;

	static void APIENTRY execute(const RenderCommand_stencilFillPathNV* __restrict params) {
		glStencilFillPathNV(
			params->path,
			params->fillMode,
			params->mask
		);
	}
};
struct RenderCommand_stencilStrokePathNV : public RenderCommandBase {
	GLuint path;
	GLint reference;
	GLuint mask;

	static void APIENTRY execute(const RenderCommand_stencilStrokePathNV* __restrict params) {
		glStencilStrokePathNV(
			params->path,
			params->reference,
			params->mask
		);
	}
};
struct RenderCommand_stencilFillPathInstancedNV : public RenderCommandBase {
	GLsizei numPaths;
	GLenum pathNameType;
	const void *paths;
	GLuint pathBase;
	GLenum fillMode;
	GLuint mask;
	GLenum transformType;
	const GLfloat *transformValues;

	static void APIENTRY execute(const RenderCommand_stencilFillPathInstancedNV* __restrict params) {
		glStencilFillPathInstancedNV(
			params->numPaths,
			params->pathNameType,
			params->paths,
			params->pathBase,
			params->fillMode,
			params->mask,
			params->transformType,
			params->transformValues
		);
	}
};
struct RenderCommand_stencilStrokePathInstancedNV : public RenderCommandBase {
	GLsizei numPaths;
	GLenum pathNameType;
	const void *paths;
	GLuint pathBase;
	GLint reference;
	GLuint mask;
	GLenum transformType;
	const GLfloat *transformValues;

	static void APIENTRY execute(const RenderCommand_stencilStrokePathInstancedNV* __restrict params) {
		glStencilStrokePathInstancedNV(
			params->numPaths,
			params->pathNameType,
			params->paths,
			params->pathBase,
			params->reference,
			params->mask,
			params->transformType,
			params->transformValues
		);
	}
};
struct RenderCommand_pathCoverDepthFuncNV : public RenderCommandBase {
	GLenum func;

	static void APIENTRY execute(const RenderCommand_pathCoverDepthFuncNV* __restrict params) {
		glPathCoverDepthFuncNV(
			params->func
		);
	}
};
struct RenderCommand_coverFillPathNV : public RenderCommandBase {
	GLuint path;
	GLenum coverMode;

	static void APIENTRY execute(const RenderCommand_coverFillPathNV* __restrict params) {
		glCoverFillPathNV(
			params->path,
			params->coverMode
		);
	}
};
struct RenderCommand_coverStrokePathNV : public RenderCommandBase {
	GLuint path;
	GLenum coverMode;

	static void APIENTRY execute(const RenderCommand_coverStrokePathNV* __restrict params) {
		glCoverStrokePathNV(
			params->path,
			params->coverMode
		);
	}
};
struct RenderCommand_coverFillPathInstancedNV : public RenderCommandBase {
	GLsizei numPaths;
	GLenum pathNameType;
	const void *paths;
	GLuint pathBase;
	GLenum coverMode;
	GLenum transformType;
	const GLfloat *transformValues;

	static void APIENTRY execute(const RenderCommand_coverFillPathInstancedNV* __restrict params) {
		glCoverFillPathInstancedNV(
			params->numPaths,
			params->pathNameType,
			params->paths,
			params->pathBase,
			params->coverMode,
			params->transformType,
			params->transformValues
		);
	}
};
struct RenderCommand_coverStrokePathInstancedNV : public RenderCommandBase {
	GLsizei numPaths;
	GLenum pathNameType;
	const void *paths;
	GLuint pathBase;
	GLenum coverMode;
	GLenum transformType;
	const GLfloat *transformValues;

	static void APIENTRY execute(const RenderCommand_coverStrokePathInstancedNV* __restrict params) {
		glCoverStrokePathInstancedNV(
			params->numPaths,
			params->pathNameType,
			params->paths,
			params->pathBase,
			params->coverMode,
			params->transformType,
			params->transformValues
		);
	}
};
struct RenderCommand_getPathParameterivNV : public RenderCommandBase {
	GLuint path;
	GLenum pname;
	GLint *value;

	static void APIENTRY execute(const RenderCommand_getPathParameterivNV* __restrict params) {
		glGetPathParameterivNV(
			params->path,
			params->pname,
			params->value
		);
	}
};
struct RenderCommand_getPathParameterfvNV : public RenderCommandBase {
	GLuint path;
	GLenum pname;
	GLfloat *value;

	static void APIENTRY execute(const RenderCommand_getPathParameterfvNV* __restrict params) {
		glGetPathParameterfvNV(
			params->path,
			params->pname,
			params->value
		);
	}
};
struct RenderCommand_getPathCommandsNV : public RenderCommandBase {
	GLuint path;
	GLubyte *commands;

	static void APIENTRY execute(const RenderCommand_getPathCommandsNV* __restrict params) {
		glGetPathCommandsNV(
			params->path,
			params->commands
		);
	}
};
struct RenderCommand_getPathCoordsNV : public RenderCommandBase {
	GLuint path;
	GLfloat *coords;

	static void APIENTRY execute(const RenderCommand_getPathCoordsNV* __restrict params) {
		glGetPathCoordsNV(
			params->path,
			params->coords
		);
	}
};
struct RenderCommand_getPathDashArrayNV : public RenderCommandBase {
	GLuint path;
	GLfloat *dashArray;

	static void APIENTRY execute(const RenderCommand_getPathDashArrayNV* __restrict params) {
		glGetPathDashArrayNV(
			params->path,
			params->dashArray
		);
	}
};
struct RenderCommand_getPathMetricsNV : public RenderCommandBase {
	GLbitfield metricQueryMask;
	GLsizei numPaths;
	GLenum pathNameType;
	const void *paths;
	GLuint pathBase;
	GLsizei stride;
	GLfloat *metrics;

	static void APIENTRY execute(const RenderCommand_getPathMetricsNV* __restrict params) {
		glGetPathMetricsNV(
			params->metricQueryMask,
			params->numPaths,
			params->pathNameType,
			params->paths,
			params->pathBase,
			params->stride,
			params->metrics
		);
	}
};
struct RenderCommand_getPathMetricRangeNV : public RenderCommandBase {
	GLbitfield metricQueryMask;
	GLuint firstPathName;
	GLsizei numPaths;
	GLsizei stride;
	GLfloat *metrics;

	static void APIENTRY execute(const RenderCommand_getPathMetricRangeNV* __restrict params) {
		glGetPathMetricRangeNV(
			params->metricQueryMask,
			params->firstPathName,
			params->numPaths,
			params->stride,
			params->metrics
		);
	}
};
struct RenderCommand_getPathSpacingNV : public RenderCommandBase {
	GLenum pathListMode;
	GLsizei numPaths;
	GLenum pathNameType;
	const void *paths;
	GLuint pathBase;
	GLfloat advanceScale;
	GLfloat kerningScale;
	GLenum transformType;
	GLfloat *returnedSpacing;

	static void APIENTRY execute(const RenderCommand_getPathSpacingNV* __restrict params) {
		glGetPathSpacingNV(
			params->pathListMode,
			params->numPaths,
			params->pathNameType,
			params->paths,
			params->pathBase,
			params->advanceScale,
			params->kerningScale,
			params->transformType,
			params->returnedSpacing
		);
	}
};
struct RenderCommand_isPointInFillPathNV : public RenderCommandBase {
	GLuint path;
	GLuint mask;
	GLfloat x;
	GLfloat y;

	static void APIENTRY execute(const RenderCommand_isPointInFillPathNV* __restrict params) {
		glIsPointInFillPathNV(
			params->path,
			params->mask,
			params->x,
			params->y
		);
	}
};
struct RenderCommand_isPointInStrokePathNV : public RenderCommandBase {
	GLuint path;
	GLfloat x;
	GLfloat y;

	static void APIENTRY execute(const RenderCommand_isPointInStrokePathNV* __restrict params) {
		glIsPointInStrokePathNV(
			params->path,
			params->x,
			params->y
		);
	}
};
struct RenderCommand_getPathLengthNV : public RenderCommandBase {
	GLuint path;
	GLsizei startSegment;
	GLsizei numSegments;

	static void APIENTRY execute(const RenderCommand_getPathLengthNV* __restrict params) {
		glGetPathLengthNV(
			params->path,
			params->startSegment,
			params->numSegments
		);
	}
};
struct RenderCommand_pointAlongPathNV : public RenderCommandBase {
	GLuint path;
	GLsizei startSegment;
	GLsizei numSegments;
	GLfloat distance;
	GLfloat *x;
	GLfloat *y;
	GLfloat *tangentX;
	GLfloat *tangentY;

	static void APIENTRY execute(const RenderCommand_pointAlongPathNV* __restrict params) {
		glPointAlongPathNV(
			params->path,
			params->startSegment,
			params->numSegments,
			params->distance,
			params->x,
			params->y,
			params->tangentX,
			params->tangentY
		);
	}
};
struct RenderCommand_matrixLoad3x2fNV : public RenderCommandBase {
	GLenum matrixMode;
	const GLfloat *m;

	static void APIENTRY execute(const RenderCommand_matrixLoad3x2fNV* __restrict params) {
		glMatrixLoad3x2fNV(
			params->matrixMode,
			params->m
		);
	}
};
struct RenderCommand_matrixLoad3x3fNV : public RenderCommandBase {
	GLenum matrixMode;
	const GLfloat *m;

	static void APIENTRY execute(const RenderCommand_matrixLoad3x3fNV* __restrict params) {
		glMatrixLoad3x3fNV(
			params->matrixMode,
			params->m
		);
	}
};
struct RenderCommand_matrixLoadTranspose3x3fNV : public RenderCommandBase {
	GLenum matrixMode;
	const GLfloat *m;

	static void APIENTRY execute(const RenderCommand_matrixLoadTranspose3x3fNV* __restrict params) {
		glMatrixLoadTranspose3x3fNV(
			params->matrixMode,
			params->m
		);
	}
};
struct RenderCommand_matrixMult3x2fNV : public RenderCommandBase {
	GLenum matrixMode;
	const GLfloat *m;

	static void APIENTRY execute(const RenderCommand_matrixMult3x2fNV* __restrict params) {
		glMatrixMult3x2fNV(
			params->matrixMode,
			params->m
		);
	}
};
struct RenderCommand_matrixMult3x3fNV : public RenderCommandBase {
	GLenum matrixMode;
	const GLfloat *m;

	static void APIENTRY execute(const RenderCommand_matrixMult3x3fNV* __restrict params) {
		glMatrixMult3x3fNV(
			params->matrixMode,
			params->m
		);
	}
};
struct RenderCommand_matrixMultTranspose3x3fNV : public RenderCommandBase {
	GLenum matrixMode;
	const GLfloat *m;

	static void APIENTRY execute(const RenderCommand_matrixMultTranspose3x3fNV* __restrict params) {
		glMatrixMultTranspose3x3fNV(
			params->matrixMode,
			params->m
		);
	}
};
struct RenderCommand_stencilThenCoverFillPathNV : public RenderCommandBase {
	GLuint path;
	GLenum fillMode;
	GLuint mask;
	GLenum coverMode;

	static void APIENTRY execute(const RenderCommand_stencilThenCoverFillPathNV* __restrict params) {
		glStencilThenCoverFillPathNV(
			params->path,
			params->fillMode,
			params->mask,
			params->coverMode
		);
	}
};
struct RenderCommand_stencilThenCoverStrokePathNV : public RenderCommandBase {
	GLuint path;
	GLint reference;
	GLuint mask;
	GLenum coverMode;

	static void APIENTRY execute(const RenderCommand_stencilThenCoverStrokePathNV* __restrict params) {
		glStencilThenCoverStrokePathNV(
			params->path,
			params->reference,
			params->mask,
			params->coverMode
		);
	}
};
struct RenderCommand_stencilThenCoverFillPathInstancedNV : public RenderCommandBase {
	GLsizei numPaths;
	GLenum pathNameType;
	const void *paths;
	GLuint pathBase;
	GLenum fillMode;
	GLuint mask;
	GLenum coverMode;
	GLenum transformType;
	const GLfloat *transformValues;

	static void APIENTRY execute(const RenderCommand_stencilThenCoverFillPathInstancedNV* __restrict params) {
		glStencilThenCoverFillPathInstancedNV(
			params->numPaths,
			params->pathNameType,
			params->paths,
			params->pathBase,
			params->fillMode,
			params->mask,
			params->coverMode,
			params->transformType,
			params->transformValues
		);
	}
};
struct RenderCommand_stencilThenCoverStrokePathInstancedNV : public RenderCommandBase {
	GLsizei numPaths;
	GLenum pathNameType;
	const void *paths;
	GLuint pathBase;
	GLint reference;
	GLuint mask;
	GLenum coverMode;
	GLenum transformType;
	const GLfloat *transformValues;

	static void APIENTRY execute(const RenderCommand_stencilThenCoverStrokePathInstancedNV* __restrict params) {
		glStencilThenCoverStrokePathInstancedNV(
			params->numPaths,
			params->pathNameType,
			params->paths,
			params->pathBase,
			params->reference,
			params->mask,
			params->coverMode,
			params->transformType,
			params->transformValues
		);
	}
};
struct RenderCommand_pathGlyphIndexRangeNV : public RenderCommandBase {
	GLenum fontTarget;
	const void *fontName;
	GLbitfield fontStyle;
	GLuint pathParameterTemplate;
	GLfloat emScale;
	GLuint baseAndCount[2];

	static void APIENTRY execute(const RenderCommand_pathGlyphIndexRangeNV* __restrict params) {
		glPathGlyphIndexRangeNV(
			params->fontTarget,
			params->fontName,
			params->fontStyle,
			params->pathParameterTemplate,
			params->emScale,
			const_cast<GLuint*>(params->baseAndCount)
		);
	}
};
struct RenderCommand_pathGlyphIndexArrayNV : public RenderCommandBase {
	GLuint firstPathName;
	GLenum fontTarget;
	const void *fontName;
	GLbitfield fontStyle;
	GLuint firstGlyphIndex;
	GLsizei numGlyphs;
	GLuint pathParameterTemplate;
	GLfloat emScale;

	static void APIENTRY execute(const RenderCommand_pathGlyphIndexArrayNV* __restrict params) {
		glPathGlyphIndexArrayNV(
			params->firstPathName,
			params->fontTarget,
			params->fontName,
			params->fontStyle,
			params->firstGlyphIndex,
			params->numGlyphs,
			params->pathParameterTemplate,
			params->emScale
		);
	}
};
struct RenderCommand_pathMemoryGlyphIndexArrayNV : public RenderCommandBase {
	GLuint firstPathName;
	GLenum fontTarget;
	GLsizeiptr fontSize;
	const void *fontData;
	GLsizei faceIndex;
	GLuint firstGlyphIndex;
	GLsizei numGlyphs;
	GLuint pathParameterTemplate;
	GLfloat emScale;

	static void APIENTRY execute(const RenderCommand_pathMemoryGlyphIndexArrayNV* __restrict params) {
		glPathMemoryGlyphIndexArrayNV(
			params->firstPathName,
			params->fontTarget,
			params->fontSize,
			params->fontData,
			params->faceIndex,
			params->firstGlyphIndex,
			params->numGlyphs,
			params->pathParameterTemplate,
			params->emScale
		);
	}
};
struct RenderCommand_programPathFragmentInputGenNV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLenum genMode;
	GLint components;
	const GLfloat *coeffs;

	static void APIENTRY execute(const RenderCommand_programPathFragmentInputGenNV* __restrict params) {
		glProgramPathFragmentInputGenNV(
			params->program,
			params->location,
			params->genMode,
			params->components,
			params->coeffs
		);
	}
};
struct RenderCommand_getProgramResourcefvNV : public RenderCommandBase {
	GLuint program;
	GLenum programInterface;
	GLuint index;
	GLsizei propCount;
	const GLenum *props;
	GLsizei bufSize;
	GLsizei *length;
	GLfloat *params;

	static void APIENTRY execute(const RenderCommand_getProgramResourcefvNV* __restrict params) {
		glGetProgramResourcefvNV(
			params->program,
			params->programInterface,
			params->index,
			params->propCount,
			params->props,
			params->bufSize,
			params->length,
			params->params
		);
	}
};
struct RenderCommand_framebufferSampleLocationsfvNV : public RenderCommandBase {
	GLenum target;
	GLuint start;
	GLsizei count;
	const GLfloat *v;

	static void APIENTRY execute(const RenderCommand_framebufferSampleLocationsfvNV* __restrict params) {
		glFramebufferSampleLocationsfvNV(
			params->target,
			params->start,
			params->count,
			params->v
		);
	}
};
struct RenderCommand_namedFramebufferSampleLocationsfvNV : public RenderCommandBase {
	GLuint framebuffer;
	GLuint start;
	GLsizei count;
	const GLfloat *v;

	static void APIENTRY execute(const RenderCommand_namedFramebufferSampleLocationsfvNV* __restrict params) {
		glNamedFramebufferSampleLocationsfvNV(
			params->framebuffer,
			params->start,
			params->count,
			params->v
		);
	}
};
struct RenderCommand_resolveDepthValuesNV : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_resolveDepthValuesNV* __restrict params) {
		glResolveDepthValuesNV(
		);
	}
};
struct RenderCommand_scissorExclusiveNV : public RenderCommandBase {
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;

	static void APIENTRY execute(const RenderCommand_scissorExclusiveNV* __restrict params) {
		glScissorExclusiveNV(
			params->x,
			params->y,
			params->width,
			params->height
		);
	}
};
struct RenderCommand_scissorExclusiveArrayvNV : public RenderCommandBase {
	GLuint first;
	GLsizei count;
	const GLint *v;

	static void APIENTRY execute(const RenderCommand_scissorExclusiveArrayvNV* __restrict params) {
		glScissorExclusiveArrayvNV(
			params->first,
			params->count,
			params->v
		);
	}
};
struct RenderCommand_makeBufferResidentNV : public RenderCommandBase {
	GLenum target;
	GLenum access;

	static void APIENTRY execute(const RenderCommand_makeBufferResidentNV* __restrict params) {
		glMakeBufferResidentNV(
			params->target,
			params->access
		);
	}
};
struct RenderCommand_makeBufferNonResidentNV : public RenderCommandBase {
	GLenum target;

	static void APIENTRY execute(const RenderCommand_makeBufferNonResidentNV* __restrict params) {
		glMakeBufferNonResidentNV(
			params->target
		);
	}
};
struct RenderCommand_isBufferResidentNV : public RenderCommandBase {
	GLenum target;

	static void APIENTRY execute(const RenderCommand_isBufferResidentNV* __restrict params) {
		glIsBufferResidentNV(
			params->target
		);
	}
};
struct RenderCommand_makeNamedBufferResidentNV : public RenderCommandBase {
	GLuint buffer;
	GLenum access;

	static void APIENTRY execute(const RenderCommand_makeNamedBufferResidentNV* __restrict params) {
		glMakeNamedBufferResidentNV(
			params->buffer,
			params->access
		);
	}
};
struct RenderCommand_makeNamedBufferNonResidentNV : public RenderCommandBase {
	GLuint buffer;

	static void APIENTRY execute(const RenderCommand_makeNamedBufferNonResidentNV* __restrict params) {
		glMakeNamedBufferNonResidentNV(
			params->buffer
		);
	}
};
struct RenderCommand_isNamedBufferResidentNV : public RenderCommandBase {
	GLuint buffer;

	static void APIENTRY execute(const RenderCommand_isNamedBufferResidentNV* __restrict params) {
		glIsNamedBufferResidentNV(
			params->buffer
		);
	}
};
struct RenderCommand_getBufferParameterui64vNV : public RenderCommandBase {
	GLenum target;
	GLenum pname;
	GLuint64EXT *params;

	static void APIENTRY execute(const RenderCommand_getBufferParameterui64vNV* __restrict params) {
		glGetBufferParameterui64vNV(
			params->target,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getNamedBufferParameterui64vNV : public RenderCommandBase {
	GLuint buffer;
	GLenum pname;
	GLuint64EXT *params;

	static void APIENTRY execute(const RenderCommand_getNamedBufferParameterui64vNV* __restrict params) {
		glGetNamedBufferParameterui64vNV(
			params->buffer,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getIntegerui64vNV : public RenderCommandBase {
	GLenum value;
	GLuint64EXT *result;

	static void APIENTRY execute(const RenderCommand_getIntegerui64vNV* __restrict params) {
		glGetIntegerui64vNV(
			params->value,
			params->result
		);
	}
};
struct RenderCommand_uniformui64NV : public RenderCommandBase {
	GLint location;
	GLuint64EXT value;

	static void APIENTRY execute(const RenderCommand_uniformui64NV* __restrict params) {
		glUniformui64NV(
			params->location,
			params->value
		);
	}
};
struct RenderCommand_uniformui64vNV : public RenderCommandBase {
	GLint location;
	GLsizei count;
	const GLuint64EXT *value;

	static void APIENTRY execute(const RenderCommand_uniformui64vNV* __restrict params) {
		glUniformui64vNV(
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_getUniformui64vNV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint64EXT *params;

	static void APIENTRY execute(const RenderCommand_getUniformui64vNV* __restrict params) {
		glGetUniformui64vNV(
			params->program,
			params->location,
			params->params
		);
	}
};
struct RenderCommand_programUniformui64NV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLuint64EXT value;

	static void APIENTRY execute(const RenderCommand_programUniformui64NV* __restrict params) {
		glProgramUniformui64NV(
			params->program,
			params->location,
			params->value
		);
	}
};
struct RenderCommand_programUniformui64vNV : public RenderCommandBase {
	GLuint program;
	GLint location;
	GLsizei count;
	const GLuint64EXT *value;

	static void APIENTRY execute(const RenderCommand_programUniformui64vNV* __restrict params) {
		glProgramUniformui64vNV(
			params->program,
			params->location,
			params->count,
			params->value
		);
	}
};
struct RenderCommand_bindShadingRateImageNV : public RenderCommandBase {
	GLuint texture;

	static void APIENTRY execute(const RenderCommand_bindShadingRateImageNV* __restrict params) {
		glBindShadingRateImageNV(
			params->texture
		);
	}
};
struct RenderCommand_getShadingRateImagePaletteNV : public RenderCommandBase {
	GLuint viewport;
	GLuint entry;
	GLenum *rate;

	static void APIENTRY execute(const RenderCommand_getShadingRateImagePaletteNV* __restrict params) {
		glGetShadingRateImagePaletteNV(
			params->viewport,
			params->entry,
			params->rate
		);
	}
};
struct RenderCommand_getShadingRateSampleLocationivNV : public RenderCommandBase {
	GLenum rate;
	GLuint samples;
	GLuint index;
	GLint *location;

	static void APIENTRY execute(const RenderCommand_getShadingRateSampleLocationivNV* __restrict params) {
		glGetShadingRateSampleLocationivNV(
			params->rate,
			params->samples,
			params->index,
			params->location
		);
	}
};
struct RenderCommand_shadingRateImageBarrierNV : public RenderCommandBase {
	GLboolean synchronize;

	static void APIENTRY execute(const RenderCommand_shadingRateImageBarrierNV* __restrict params) {
		glShadingRateImageBarrierNV(
			params->synchronize
		);
	}
};
struct RenderCommand_shadingRateImagePaletteNV : public RenderCommandBase {
	GLuint viewport;
	GLuint first;
	GLsizei count;
	const GLenum *rates;

	static void APIENTRY execute(const RenderCommand_shadingRateImagePaletteNV* __restrict params) {
		glShadingRateImagePaletteNV(
			params->viewport,
			params->first,
			params->count,
			params->rates
		);
	}
};
struct RenderCommand_shadingRateSampleOrderNV : public RenderCommandBase {
	GLenum order;

	static void APIENTRY execute(const RenderCommand_shadingRateSampleOrderNV* __restrict params) {
		glShadingRateSampleOrderNV(
			params->order
		);
	}
};
struct RenderCommand_shadingRateSampleOrderCustomNV : public RenderCommandBase {
	GLenum rate;
	GLuint samples;
	const GLint *locations;

	static void APIENTRY execute(const RenderCommand_shadingRateSampleOrderCustomNV* __restrict params) {
		glShadingRateSampleOrderCustomNV(
			params->rate,
			params->samples,
			params->locations
		);
	}
};
struct RenderCommand_textureBarrierNV : public RenderCommandBase {

	static void APIENTRY execute(const RenderCommand_textureBarrierNV* __restrict params) {
		glTextureBarrierNV(
		);
	}
};
struct RenderCommand_vertexAttribL1i64NV : public RenderCommandBase {
	GLuint index;
	GLint64EXT x;

	static void APIENTRY execute(const RenderCommand_vertexAttribL1i64NV* __restrict params) {
		glVertexAttribL1i64NV(
			params->index,
			params->x
		);
	}
};
struct RenderCommand_vertexAttribL2i64NV : public RenderCommandBase {
	GLuint index;
	GLint64EXT x;
	GLint64EXT y;

	static void APIENTRY execute(const RenderCommand_vertexAttribL2i64NV* __restrict params) {
		glVertexAttribL2i64NV(
			params->index,
			params->x,
			params->y
		);
	}
};
struct RenderCommand_vertexAttribL3i64NV : public RenderCommandBase {
	GLuint index;
	GLint64EXT x;
	GLint64EXT y;
	GLint64EXT z;

	static void APIENTRY execute(const RenderCommand_vertexAttribL3i64NV* __restrict params) {
		glVertexAttribL3i64NV(
			params->index,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_vertexAttribL4i64NV : public RenderCommandBase {
	GLuint index;
	GLint64EXT x;
	GLint64EXT y;
	GLint64EXT z;
	GLint64EXT w;

	static void APIENTRY execute(const RenderCommand_vertexAttribL4i64NV* __restrict params) {
		glVertexAttribL4i64NV(
			params->index,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_vertexAttribL1i64vNV : public RenderCommandBase {
	GLuint index;
	const GLint64EXT *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribL1i64vNV* __restrict params) {
		glVertexAttribL1i64vNV(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribL2i64vNV : public RenderCommandBase {
	GLuint index;
	const GLint64EXT *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribL2i64vNV* __restrict params) {
		glVertexAttribL2i64vNV(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribL3i64vNV : public RenderCommandBase {
	GLuint index;
	const GLint64EXT *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribL3i64vNV* __restrict params) {
		glVertexAttribL3i64vNV(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribL4i64vNV : public RenderCommandBase {
	GLuint index;
	const GLint64EXT *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribL4i64vNV* __restrict params) {
		glVertexAttribL4i64vNV(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribL1ui64NV : public RenderCommandBase {
	GLuint index;
	GLuint64EXT x;

	static void APIENTRY execute(const RenderCommand_vertexAttribL1ui64NV* __restrict params) {
		glVertexAttribL1ui64NV(
			params->index,
			params->x
		);
	}
};
struct RenderCommand_vertexAttribL2ui64NV : public RenderCommandBase {
	GLuint index;
	GLuint64EXT x;
	GLuint64EXT y;

	static void APIENTRY execute(const RenderCommand_vertexAttribL2ui64NV* __restrict params) {
		glVertexAttribL2ui64NV(
			params->index,
			params->x,
			params->y
		);
	}
};
struct RenderCommand_vertexAttribL3ui64NV : public RenderCommandBase {
	GLuint index;
	GLuint64EXT x;
	GLuint64EXT y;
	GLuint64EXT z;

	static void APIENTRY execute(const RenderCommand_vertexAttribL3ui64NV* __restrict params) {
		glVertexAttribL3ui64NV(
			params->index,
			params->x,
			params->y,
			params->z
		);
	}
};
struct RenderCommand_vertexAttribL4ui64NV : public RenderCommandBase {
	GLuint index;
	GLuint64EXT x;
	GLuint64EXT y;
	GLuint64EXT z;
	GLuint64EXT w;

	static void APIENTRY execute(const RenderCommand_vertexAttribL4ui64NV* __restrict params) {
		glVertexAttribL4ui64NV(
			params->index,
			params->x,
			params->y,
			params->z,
			params->w
		);
	}
};
struct RenderCommand_vertexAttribL1ui64vNV : public RenderCommandBase {
	GLuint index;
	const GLuint64EXT *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribL1ui64vNV* __restrict params) {
		glVertexAttribL1ui64vNV(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribL2ui64vNV : public RenderCommandBase {
	GLuint index;
	const GLuint64EXT *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribL2ui64vNV* __restrict params) {
		glVertexAttribL2ui64vNV(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribL3ui64vNV : public RenderCommandBase {
	GLuint index;
	const GLuint64EXT *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribL3ui64vNV* __restrict params) {
		glVertexAttribL3ui64vNV(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_vertexAttribL4ui64vNV : public RenderCommandBase {
	GLuint index;
	const GLuint64EXT *v;

	static void APIENTRY execute(const RenderCommand_vertexAttribL4ui64vNV* __restrict params) {
		glVertexAttribL4ui64vNV(
			params->index,
			params->v
		);
	}
};
struct RenderCommand_getVertexAttribLi64vNV : public RenderCommandBase {
	GLuint index;
	GLenum pname;
	GLint64EXT *params;

	static void APIENTRY execute(const RenderCommand_getVertexAttribLi64vNV* __restrict params) {
		glGetVertexAttribLi64vNV(
			params->index,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_getVertexAttribLui64vNV : public RenderCommandBase {
	GLuint index;
	GLenum pname;
	GLuint64EXT *params;

	static void APIENTRY execute(const RenderCommand_getVertexAttribLui64vNV* __restrict params) {
		glGetVertexAttribLui64vNV(
			params->index,
			params->pname,
			params->params
		);
	}
};
struct RenderCommand_vertexAttribLFormatNV : public RenderCommandBase {
	GLuint index;
	GLint size;
	GLenum type;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_vertexAttribLFormatNV* __restrict params) {
		glVertexAttribLFormatNV(
			params->index,
			params->size,
			params->type,
			params->stride
		);
	}
};
struct RenderCommand_bufferAddressRangeNV : public RenderCommandBase {
	GLenum pname;
	GLuint index;
	GLuint64EXT address;
	GLsizeiptr length;

	static void APIENTRY execute(const RenderCommand_bufferAddressRangeNV* __restrict params) {
		glBufferAddressRangeNV(
			params->pname,
			params->index,
			params->address,
			params->length
		);
	}
};
struct RenderCommand_vertexFormatNV : public RenderCommandBase {
	GLint size;
	GLenum type;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_vertexFormatNV* __restrict params) {
		glVertexFormatNV(
			params->size,
			params->type,
			params->stride
		);
	}
};
struct RenderCommand_normalFormatNV : public RenderCommandBase {
	GLenum type;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_normalFormatNV* __restrict params) {
		glNormalFormatNV(
			params->type,
			params->stride
		);
	}
};
struct RenderCommand_colorFormatNV : public RenderCommandBase {
	GLint size;
	GLenum type;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_colorFormatNV* __restrict params) {
		glColorFormatNV(
			params->size,
			params->type,
			params->stride
		);
	}
};
struct RenderCommand_indexFormatNV : public RenderCommandBase {
	GLenum type;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_indexFormatNV* __restrict params) {
		glIndexFormatNV(
			params->type,
			params->stride
		);
	}
};
struct RenderCommand_texCoordFormatNV : public RenderCommandBase {
	GLint size;
	GLenum type;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_texCoordFormatNV* __restrict params) {
		glTexCoordFormatNV(
			params->size,
			params->type,
			params->stride
		);
	}
};
struct RenderCommand_edgeFlagFormatNV : public RenderCommandBase {
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_edgeFlagFormatNV* __restrict params) {
		glEdgeFlagFormatNV(
			params->stride
		);
	}
};
struct RenderCommand_secondaryColorFormatNV : public RenderCommandBase {
	GLint size;
	GLenum type;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_secondaryColorFormatNV* __restrict params) {
		glSecondaryColorFormatNV(
			params->size,
			params->type,
			params->stride
		);
	}
};
struct RenderCommand_fogCoordFormatNV : public RenderCommandBase {
	GLenum type;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_fogCoordFormatNV* __restrict params) {
		glFogCoordFormatNV(
			params->type,
			params->stride
		);
	}
};
struct RenderCommand_vertexAttribFormatNV : public RenderCommandBase {
	GLuint index;
	GLint size;
	GLenum type;
	GLboolean normalized;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_vertexAttribFormatNV* __restrict params) {
		glVertexAttribFormatNV(
			params->index,
			params->size,
			params->type,
			params->normalized,
			params->stride
		);
	}
};
struct RenderCommand_vertexAttribIFormatNV : public RenderCommandBase {
	GLuint index;
	GLint size;
	GLenum type;
	GLsizei stride;

	static void APIENTRY execute(const RenderCommand_vertexAttribIFormatNV* __restrict params) {
		glVertexAttribIFormatNV(
			params->index,
			params->size,
			params->type,
			params->stride
		);
	}
};
struct RenderCommand_getIntegerui64i_vNV : public RenderCommandBase {
	GLenum value;
	GLuint index;
	GLuint64EXT *result;

	static void APIENTRY execute(const RenderCommand_getIntegerui64i_vNV* __restrict params) {
		glGetIntegerui64i_vNV(
			params->value,
			params->index,
			params->result
		);
	}
};
struct RenderCommand_viewportSwizzleNV : public RenderCommandBase {
	GLuint index;
	GLenum swizzlex;
	GLenum swizzley;
	GLenum swizzlez;
	GLenum swizzlew;

	static void APIENTRY execute(const RenderCommand_viewportSwizzleNV* __restrict params) {
		glViewportSwizzleNV(
			params->index,
			params->swizzlex,
			params->swizzley,
			params->swizzlez,
			params->swizzlew
		);
	}
};
struct RenderCommand_framebufferTextureMultiviewOVR : public RenderCommandBase {
	GLenum target;
	GLenum attachment;
	GLuint texture;
	GLint level;
	GLint baseViewIndex;
	GLsizei numViews;

	static void APIENTRY execute(const RenderCommand_framebufferTextureMultiviewOVR* __restrict params) {
		glFramebufferTextureMultiviewOVR(
			params->target,
			params->attachment,
			params->texture,
			params->level,
			params->baseViewIndex,
			params->numViews
		);
	}
};
union RenderCommandPacketUnion {
public:
	PFN_EXECUTE execute;
private:
	RenderCommand_cullFace cullFace;
	RenderCommand_frontFace frontFace;
	RenderCommand_hint hint;
	RenderCommand_lineWidth lineWidth;
	RenderCommand_pointSize pointSize;
	RenderCommand_polygonMode polygonMode;
	RenderCommand_scissor scissor;
	RenderCommand_texParameterf texParameterf;
	RenderCommand_texParameterfv texParameterfv;
	RenderCommand_texParameteri texParameteri;
	RenderCommand_texParameteriv texParameteriv;
	RenderCommand_texImage1D texImage1D;
	RenderCommand_texImage2D texImage2D;
	RenderCommand_drawBuffer drawBuffer;
	RenderCommand_clear clear;
	RenderCommand_clearColor clearColor;
	RenderCommand_clearStencil clearStencil;
	RenderCommand_clearDepth clearDepth;
	RenderCommand_stencilMask stencilMask;
	RenderCommand_colorMask colorMask;
	RenderCommand_depthMask depthMask;
	RenderCommand_disable disable;
	RenderCommand_enable enable;
	RenderCommand_finish finish;
	RenderCommand_flush flush;
	RenderCommand_blendFunc blendFunc;
	RenderCommand_logicOp logicOp;
	RenderCommand_stencilFunc stencilFunc;
	RenderCommand_stencilOp stencilOp;
	RenderCommand_depthFunc depthFunc;
	RenderCommand_pixelStoref pixelStoref;
	RenderCommand_pixelStorei pixelStorei;
	RenderCommand_readBuffer readBuffer;
	RenderCommand_readPixels readPixels;
	RenderCommand_getBooleanv getBooleanv;
	RenderCommand_getDoublev getDoublev;
	RenderCommand_getError getError;
	RenderCommand_getFloatv getFloatv;
	RenderCommand_getIntegerv getIntegerv;
	RenderCommand_getString getString;
	RenderCommand_getTexImage getTexImage;
	RenderCommand_getTexParameterfv getTexParameterfv;
	RenderCommand_getTexParameteriv getTexParameteriv;
	RenderCommand_getTexLevelParameterfv getTexLevelParameterfv;
	RenderCommand_getTexLevelParameteriv getTexLevelParameteriv;
	RenderCommand_isEnabled isEnabled;
	RenderCommand_depthRange depthRange;
	RenderCommand_viewport viewport;
	RenderCommand_drawArrays drawArrays;
	RenderCommand_drawElements drawElements;
	RenderCommand_getPointerv getPointerv;
	RenderCommand_polygonOffset polygonOffset;
	RenderCommand_copyTexImage1D copyTexImage1D;
	RenderCommand_copyTexImage2D copyTexImage2D;
	RenderCommand_copyTexSubImage1D copyTexSubImage1D;
	RenderCommand_copyTexSubImage2D copyTexSubImage2D;
	RenderCommand_texSubImage1D texSubImage1D;
	RenderCommand_texSubImage2D texSubImage2D;
	RenderCommand_bindTexture bindTexture;
	RenderCommand_deleteTextures deleteTextures;
	RenderCommand_genTextures genTextures;
	RenderCommand_isTexture isTexture;
	RenderCommand_drawRangeElements drawRangeElements;
	RenderCommand_texImage3D texImage3D;
	RenderCommand_texSubImage3D texSubImage3D;
	RenderCommand_copyTexSubImage3D copyTexSubImage3D;
	RenderCommand_activeTexture activeTexture;
	RenderCommand_sampleCoverage sampleCoverage;
	RenderCommand_compressedTexImage3D compressedTexImage3D;
	RenderCommand_compressedTexImage2D compressedTexImage2D;
	RenderCommand_compressedTexImage1D compressedTexImage1D;
	RenderCommand_compressedTexSubImage3D compressedTexSubImage3D;
	RenderCommand_compressedTexSubImage2D compressedTexSubImage2D;
	RenderCommand_compressedTexSubImage1D compressedTexSubImage1D;
	RenderCommand_getCompressedTexImage getCompressedTexImage;
	RenderCommand_blendFuncSeparate blendFuncSeparate;
	RenderCommand_multiDrawArrays multiDrawArrays;
	RenderCommand_multiDrawElements multiDrawElements;
	RenderCommand_pointParameterf pointParameterf;
	RenderCommand_pointParameterfv pointParameterfv;
	RenderCommand_pointParameteri pointParameteri;
	RenderCommand_pointParameteriv pointParameteriv;
	RenderCommand_blendColor blendColor;
	RenderCommand_blendEquation blendEquation;
	RenderCommand_genQueries genQueries;
	RenderCommand_deleteQueries deleteQueries;
	RenderCommand_isQuery isQuery;
	RenderCommand_beginQuery beginQuery;
	RenderCommand_endQuery endQuery;
	RenderCommand_getQueryiv getQueryiv;
	RenderCommand_getQueryObjectiv getQueryObjectiv;
	RenderCommand_getQueryObjectuiv getQueryObjectuiv;
	RenderCommand_bindBuffer bindBuffer;
	RenderCommand_deleteBuffers deleteBuffers;
	RenderCommand_genBuffers genBuffers;
	RenderCommand_isBuffer isBuffer;
	RenderCommand_bufferData bufferData;
	RenderCommand_bufferSubData bufferSubData;
	RenderCommand_getBufferSubData getBufferSubData;
	RenderCommand_mapBuffer mapBuffer;
	RenderCommand_unmapBuffer unmapBuffer;
	RenderCommand_getBufferParameteriv getBufferParameteriv;
	RenderCommand_getBufferPointerv getBufferPointerv;
	RenderCommand_blendEquationSeparate blendEquationSeparate;
	RenderCommand_drawBuffers drawBuffers;
	RenderCommand_stencilOpSeparate stencilOpSeparate;
	RenderCommand_stencilFuncSeparate stencilFuncSeparate;
	RenderCommand_stencilMaskSeparate stencilMaskSeparate;
	RenderCommand_attachShader attachShader;
	RenderCommand_bindAttribLocation bindAttribLocation;
	RenderCommand_compileShader compileShader;
	RenderCommand_createProgram createProgram;
	RenderCommand_createShader createShader;
	RenderCommand_deleteProgram deleteProgram;
	RenderCommand_deleteShader deleteShader;
	RenderCommand_detachShader detachShader;
	RenderCommand_disableVertexAttribArray disableVertexAttribArray;
	RenderCommand_enableVertexAttribArray enableVertexAttribArray;
	RenderCommand_getActiveAttrib getActiveAttrib;
	RenderCommand_getActiveUniform getActiveUniform;
	RenderCommand_getAttachedShaders getAttachedShaders;
	RenderCommand_getAttribLocation getAttribLocation;
	RenderCommand_getProgramiv getProgramiv;
	RenderCommand_getProgramInfoLog getProgramInfoLog;
	RenderCommand_getShaderiv getShaderiv;
	RenderCommand_getShaderInfoLog getShaderInfoLog;
	RenderCommand_getShaderSource getShaderSource;
	RenderCommand_getUniformLocation getUniformLocation;
	RenderCommand_getUniformfv getUniformfv;
	RenderCommand_getUniformiv getUniformiv;
	RenderCommand_getVertexAttribdv getVertexAttribdv;
	RenderCommand_getVertexAttribfv getVertexAttribfv;
	RenderCommand_getVertexAttribiv getVertexAttribiv;
	RenderCommand_getVertexAttribPointerv getVertexAttribPointerv;
	RenderCommand_isProgram isProgram;
	RenderCommand_isShader isShader;
	RenderCommand_linkProgram linkProgram;
	RenderCommand_shaderSource shaderSource;
	RenderCommand_useProgram useProgram;
	RenderCommand_uniform1f uniform1f;
	RenderCommand_uniform2f uniform2f;
	RenderCommand_uniform3f uniform3f;
	RenderCommand_uniform4f uniform4f;
	RenderCommand_uniform1i uniform1i;
	RenderCommand_uniform2i uniform2i;
	RenderCommand_uniform3i uniform3i;
	RenderCommand_uniform4i uniform4i;
	RenderCommand_uniform1fv uniform1fv;
	RenderCommand_uniform2fv uniform2fv;
	RenderCommand_uniform3fv uniform3fv;
	RenderCommand_uniform4fv uniform4fv;
	RenderCommand_uniform1iv uniform1iv;
	RenderCommand_uniform2iv uniform2iv;
	RenderCommand_uniform3iv uniform3iv;
	RenderCommand_uniform4iv uniform4iv;
	RenderCommand_uniformMatrix2fv uniformMatrix2fv;
	RenderCommand_uniformMatrix3fv uniformMatrix3fv;
	RenderCommand_uniformMatrix4fv uniformMatrix4fv;
	RenderCommand_validateProgram validateProgram;
	RenderCommand_vertexAttrib1d vertexAttrib1d;
	RenderCommand_vertexAttrib1dv vertexAttrib1dv;
	RenderCommand_vertexAttrib1f vertexAttrib1f;
	RenderCommand_vertexAttrib1fv vertexAttrib1fv;
	RenderCommand_vertexAttrib1s vertexAttrib1s;
	RenderCommand_vertexAttrib1sv vertexAttrib1sv;
	RenderCommand_vertexAttrib2d vertexAttrib2d;
	RenderCommand_vertexAttrib2dv vertexAttrib2dv;
	RenderCommand_vertexAttrib2f vertexAttrib2f;
	RenderCommand_vertexAttrib2fv vertexAttrib2fv;
	RenderCommand_vertexAttrib2s vertexAttrib2s;
	RenderCommand_vertexAttrib2sv vertexAttrib2sv;
	RenderCommand_vertexAttrib3d vertexAttrib3d;
	RenderCommand_vertexAttrib3dv vertexAttrib3dv;
	RenderCommand_vertexAttrib3f vertexAttrib3f;
	RenderCommand_vertexAttrib3fv vertexAttrib3fv;
	RenderCommand_vertexAttrib3s vertexAttrib3s;
	RenderCommand_vertexAttrib3sv vertexAttrib3sv;
	RenderCommand_vertexAttrib4Nbv vertexAttrib4Nbv;
	RenderCommand_vertexAttrib4Niv vertexAttrib4Niv;
	RenderCommand_vertexAttrib4Nsv vertexAttrib4Nsv;
	RenderCommand_vertexAttrib4Nub vertexAttrib4Nub;
	RenderCommand_vertexAttrib4Nubv vertexAttrib4Nubv;
	RenderCommand_vertexAttrib4Nuiv vertexAttrib4Nuiv;
	RenderCommand_vertexAttrib4Nusv vertexAttrib4Nusv;
	RenderCommand_vertexAttrib4bv vertexAttrib4bv;
	RenderCommand_vertexAttrib4d vertexAttrib4d;
	RenderCommand_vertexAttrib4dv vertexAttrib4dv;
	RenderCommand_vertexAttrib4f vertexAttrib4f;
	RenderCommand_vertexAttrib4fv vertexAttrib4fv;
	RenderCommand_vertexAttrib4iv vertexAttrib4iv;
	RenderCommand_vertexAttrib4s vertexAttrib4s;
	RenderCommand_vertexAttrib4sv vertexAttrib4sv;
	RenderCommand_vertexAttrib4ubv vertexAttrib4ubv;
	RenderCommand_vertexAttrib4uiv vertexAttrib4uiv;
	RenderCommand_vertexAttrib4usv vertexAttrib4usv;
	RenderCommand_vertexAttribPointer vertexAttribPointer;
	RenderCommand_uniformMatrix2x3fv uniformMatrix2x3fv;
	RenderCommand_uniformMatrix3x2fv uniformMatrix3x2fv;
	RenderCommand_uniformMatrix2x4fv uniformMatrix2x4fv;
	RenderCommand_uniformMatrix4x2fv uniformMatrix4x2fv;
	RenderCommand_uniformMatrix3x4fv uniformMatrix3x4fv;
	RenderCommand_uniformMatrix4x3fv uniformMatrix4x3fv;
	RenderCommand_colorMaski colorMaski;
	RenderCommand_getBooleani_v getBooleani_v;
	RenderCommand_getIntegeri_v getIntegeri_v;
	RenderCommand_enablei enablei;
	RenderCommand_disablei disablei;
	RenderCommand_isEnabledi isEnabledi;
	RenderCommand_beginTransformFeedback beginTransformFeedback;
	RenderCommand_endTransformFeedback endTransformFeedback;
	RenderCommand_bindBufferRange bindBufferRange;
	RenderCommand_bindBufferBase bindBufferBase;
	RenderCommand_transformFeedbackVaryings transformFeedbackVaryings;
	RenderCommand_getTransformFeedbackVarying getTransformFeedbackVarying;
	RenderCommand_clampColor clampColor;
	RenderCommand_beginConditionalRender beginConditionalRender;
	RenderCommand_endConditionalRender endConditionalRender;
	RenderCommand_vertexAttribIPointer vertexAttribIPointer;
	RenderCommand_getVertexAttribIiv getVertexAttribIiv;
	RenderCommand_getVertexAttribIuiv getVertexAttribIuiv;
	RenderCommand_vertexAttribI1i vertexAttribI1i;
	RenderCommand_vertexAttribI2i vertexAttribI2i;
	RenderCommand_vertexAttribI3i vertexAttribI3i;
	RenderCommand_vertexAttribI4i vertexAttribI4i;
	RenderCommand_vertexAttribI1ui vertexAttribI1ui;
	RenderCommand_vertexAttribI2ui vertexAttribI2ui;
	RenderCommand_vertexAttribI3ui vertexAttribI3ui;
	RenderCommand_vertexAttribI4ui vertexAttribI4ui;
	RenderCommand_vertexAttribI1iv vertexAttribI1iv;
	RenderCommand_vertexAttribI2iv vertexAttribI2iv;
	RenderCommand_vertexAttribI3iv vertexAttribI3iv;
	RenderCommand_vertexAttribI4iv vertexAttribI4iv;
	RenderCommand_vertexAttribI1uiv vertexAttribI1uiv;
	RenderCommand_vertexAttribI2uiv vertexAttribI2uiv;
	RenderCommand_vertexAttribI3uiv vertexAttribI3uiv;
	RenderCommand_vertexAttribI4uiv vertexAttribI4uiv;
	RenderCommand_vertexAttribI4bv vertexAttribI4bv;
	RenderCommand_vertexAttribI4sv vertexAttribI4sv;
	RenderCommand_vertexAttribI4ubv vertexAttribI4ubv;
	RenderCommand_vertexAttribI4usv vertexAttribI4usv;
	RenderCommand_getUniformuiv getUniformuiv;
	RenderCommand_bindFragDataLocation bindFragDataLocation;
	RenderCommand_getFragDataLocation getFragDataLocation;
	RenderCommand_uniform1ui uniform1ui;
	RenderCommand_uniform2ui uniform2ui;
	RenderCommand_uniform3ui uniform3ui;
	RenderCommand_uniform4ui uniform4ui;
	RenderCommand_uniform1uiv uniform1uiv;
	RenderCommand_uniform2uiv uniform2uiv;
	RenderCommand_uniform3uiv uniform3uiv;
	RenderCommand_uniform4uiv uniform4uiv;
	RenderCommand_texParameterIiv texParameterIiv;
	RenderCommand_texParameterIuiv texParameterIuiv;
	RenderCommand_getTexParameterIiv getTexParameterIiv;
	RenderCommand_getTexParameterIuiv getTexParameterIuiv;
	RenderCommand_clearBufferiv clearBufferiv;
	RenderCommand_clearBufferuiv clearBufferuiv;
	RenderCommand_clearBufferfv clearBufferfv;
	RenderCommand_clearBufferfi clearBufferfi;
	RenderCommand_getStringi getStringi;
	RenderCommand_isRenderbuffer isRenderbuffer;
	RenderCommand_bindRenderbuffer bindRenderbuffer;
	RenderCommand_deleteRenderbuffers deleteRenderbuffers;
	RenderCommand_genRenderbuffers genRenderbuffers;
	RenderCommand_renderbufferStorage renderbufferStorage;
	RenderCommand_getRenderbufferParameteriv getRenderbufferParameteriv;
	RenderCommand_isFramebuffer isFramebuffer;
	RenderCommand_bindFramebuffer bindFramebuffer;
	RenderCommand_deleteFramebuffers deleteFramebuffers;
	RenderCommand_genFramebuffers genFramebuffers;
	RenderCommand_checkFramebufferStatus checkFramebufferStatus;
	RenderCommand_framebufferTexture1D framebufferTexture1D;
	RenderCommand_framebufferTexture2D framebufferTexture2D;
	RenderCommand_framebufferTexture3D framebufferTexture3D;
	RenderCommand_framebufferRenderbuffer framebufferRenderbuffer;
	RenderCommand_getFramebufferAttachmentParameteriv getFramebufferAttachmentParameteriv;
	RenderCommand_generateMipmap generateMipmap;
	RenderCommand_blitFramebuffer blitFramebuffer;
	RenderCommand_renderbufferStorageMultisample renderbufferStorageMultisample;
	RenderCommand_framebufferTextureLayer framebufferTextureLayer;
	RenderCommand_mapBufferRange mapBufferRange;
	RenderCommand_flushMappedBufferRange flushMappedBufferRange;
	RenderCommand_bindVertexArray bindVertexArray;
	RenderCommand_deleteVertexArrays deleteVertexArrays;
	RenderCommand_genVertexArrays genVertexArrays;
	RenderCommand_isVertexArray isVertexArray;
	RenderCommand_drawArraysInstanced drawArraysInstanced;
	RenderCommand_drawElementsInstanced drawElementsInstanced;
	RenderCommand_texBuffer texBuffer;
	RenderCommand_primitiveRestartIndex primitiveRestartIndex;
	RenderCommand_copyBufferSubData copyBufferSubData;
	RenderCommand_getUniformIndices getUniformIndices;
	RenderCommand_getActiveUniformsiv getActiveUniformsiv;
	RenderCommand_getActiveUniformName getActiveUniformName;
	RenderCommand_getUniformBlockIndex getUniformBlockIndex;
	RenderCommand_getActiveUniformBlockiv getActiveUniformBlockiv;
	RenderCommand_getActiveUniformBlockName getActiveUniformBlockName;
	RenderCommand_uniformBlockBinding uniformBlockBinding;
	RenderCommand_drawElementsBaseVertex drawElementsBaseVertex;
	RenderCommand_drawRangeElementsBaseVertex drawRangeElementsBaseVertex;
	RenderCommand_drawElementsInstancedBaseVertex drawElementsInstancedBaseVertex;
	RenderCommand_multiDrawElementsBaseVertex multiDrawElementsBaseVertex;
	RenderCommand_provokingVertex provokingVertex;
	RenderCommand_fenceSync fenceSync;
	RenderCommand_isSync isSync;
	RenderCommand_deleteSync deleteSync;
	RenderCommand_clientWaitSync clientWaitSync;
	RenderCommand_waitSync waitSync;
	RenderCommand_getInteger64v getInteger64v;
	RenderCommand_getSynciv getSynciv;
	RenderCommand_getInteger64i_v getInteger64i_v;
	RenderCommand_getBufferParameteri64v getBufferParameteri64v;
	RenderCommand_framebufferTexture framebufferTexture;
	RenderCommand_texImage2DMultisample texImage2DMultisample;
	RenderCommand_texImage3DMultisample texImage3DMultisample;
	RenderCommand_getMultisamplefv getMultisamplefv;
	RenderCommand_sampleMaski sampleMaski;
	RenderCommand_bindFragDataLocationIndexed bindFragDataLocationIndexed;
	RenderCommand_getFragDataIndex getFragDataIndex;
	RenderCommand_genSamplers genSamplers;
	RenderCommand_deleteSamplers deleteSamplers;
	RenderCommand_isSampler isSampler;
	RenderCommand_bindSampler bindSampler;
	RenderCommand_samplerParameteri samplerParameteri;
	RenderCommand_samplerParameteriv samplerParameteriv;
	RenderCommand_samplerParameterf samplerParameterf;
	RenderCommand_samplerParameterfv samplerParameterfv;
	RenderCommand_samplerParameterIiv samplerParameterIiv;
	RenderCommand_samplerParameterIuiv samplerParameterIuiv;
	RenderCommand_getSamplerParameteriv getSamplerParameteriv;
	RenderCommand_getSamplerParameterIiv getSamplerParameterIiv;
	RenderCommand_getSamplerParameterfv getSamplerParameterfv;
	RenderCommand_getSamplerParameterIuiv getSamplerParameterIuiv;
	RenderCommand_queryCounter queryCounter;
	RenderCommand_getQueryObjecti64v getQueryObjecti64v;
	RenderCommand_getQueryObjectui64v getQueryObjectui64v;
	RenderCommand_vertexAttribDivisor vertexAttribDivisor;
	RenderCommand_vertexAttribP1ui vertexAttribP1ui;
	RenderCommand_vertexAttribP1uiv vertexAttribP1uiv;
	RenderCommand_vertexAttribP2ui vertexAttribP2ui;
	RenderCommand_vertexAttribP2uiv vertexAttribP2uiv;
	RenderCommand_vertexAttribP3ui vertexAttribP3ui;
	RenderCommand_vertexAttribP3uiv vertexAttribP3uiv;
	RenderCommand_vertexAttribP4ui vertexAttribP4ui;
	RenderCommand_vertexAttribP4uiv vertexAttribP4uiv;
	RenderCommand_minSampleShading minSampleShading;
	RenderCommand_blendEquationi blendEquationi;
	RenderCommand_blendEquationSeparatei blendEquationSeparatei;
	RenderCommand_blendFunci blendFunci;
	RenderCommand_blendFuncSeparatei blendFuncSeparatei;
	RenderCommand_drawArraysIndirect drawArraysIndirect;
	RenderCommand_drawElementsIndirect drawElementsIndirect;
	RenderCommand_uniform1d uniform1d;
	RenderCommand_uniform2d uniform2d;
	RenderCommand_uniform3d uniform3d;
	RenderCommand_uniform4d uniform4d;
	RenderCommand_uniform1dv uniform1dv;
	RenderCommand_uniform2dv uniform2dv;
	RenderCommand_uniform3dv uniform3dv;
	RenderCommand_uniform4dv uniform4dv;
	RenderCommand_uniformMatrix2dv uniformMatrix2dv;
	RenderCommand_uniformMatrix3dv uniformMatrix3dv;
	RenderCommand_uniformMatrix4dv uniformMatrix4dv;
	RenderCommand_uniformMatrix2x3dv uniformMatrix2x3dv;
	RenderCommand_uniformMatrix2x4dv uniformMatrix2x4dv;
	RenderCommand_uniformMatrix3x2dv uniformMatrix3x2dv;
	RenderCommand_uniformMatrix3x4dv uniformMatrix3x4dv;
	RenderCommand_uniformMatrix4x2dv uniformMatrix4x2dv;
	RenderCommand_uniformMatrix4x3dv uniformMatrix4x3dv;
	RenderCommand_getUniformdv getUniformdv;
	RenderCommand_getSubroutineUniformLocation getSubroutineUniformLocation;
	RenderCommand_getSubroutineIndex getSubroutineIndex;
	RenderCommand_getActiveSubroutineUniformiv getActiveSubroutineUniformiv;
	RenderCommand_getActiveSubroutineUniformName getActiveSubroutineUniformName;
	RenderCommand_getActiveSubroutineName getActiveSubroutineName;
	RenderCommand_uniformSubroutinesuiv uniformSubroutinesuiv;
	RenderCommand_getUniformSubroutineuiv getUniformSubroutineuiv;
	RenderCommand_getProgramStageiv getProgramStageiv;
	RenderCommand_patchParameteri patchParameteri;
	RenderCommand_patchParameterfv patchParameterfv;
	RenderCommand_bindTransformFeedback bindTransformFeedback;
	RenderCommand_deleteTransformFeedbacks deleteTransformFeedbacks;
	RenderCommand_genTransformFeedbacks genTransformFeedbacks;
	RenderCommand_isTransformFeedback isTransformFeedback;
	RenderCommand_pauseTransformFeedback pauseTransformFeedback;
	RenderCommand_resumeTransformFeedback resumeTransformFeedback;
	RenderCommand_drawTransformFeedback drawTransformFeedback;
	RenderCommand_drawTransformFeedbackStream drawTransformFeedbackStream;
	RenderCommand_beginQueryIndexed beginQueryIndexed;
	RenderCommand_endQueryIndexed endQueryIndexed;
	RenderCommand_getQueryIndexediv getQueryIndexediv;
	RenderCommand_releaseShaderCompiler releaseShaderCompiler;
	RenderCommand_shaderBinary shaderBinary;
	RenderCommand_getShaderPrecisionFormat getShaderPrecisionFormat;
	RenderCommand_depthRangef depthRangef;
	RenderCommand_clearDepthf clearDepthf;
	RenderCommand_getProgramBinary getProgramBinary;
	RenderCommand_programBinary programBinary;
	RenderCommand_programParameteri programParameteri;
	RenderCommand_useProgramStages useProgramStages;
	RenderCommand_activeShaderProgram activeShaderProgram;
	RenderCommand_createShaderProgramv createShaderProgramv;
	RenderCommand_bindProgramPipeline bindProgramPipeline;
	RenderCommand_deleteProgramPipelines deleteProgramPipelines;
	RenderCommand_genProgramPipelines genProgramPipelines;
	RenderCommand_isProgramPipeline isProgramPipeline;
	RenderCommand_getProgramPipelineiv getProgramPipelineiv;
	RenderCommand_programUniform1i programUniform1i;
	RenderCommand_programUniform1iv programUniform1iv;
	RenderCommand_programUniform1f programUniform1f;
	RenderCommand_programUniform1fv programUniform1fv;
	RenderCommand_programUniform1d programUniform1d;
	RenderCommand_programUniform1dv programUniform1dv;
	RenderCommand_programUniform1ui programUniform1ui;
	RenderCommand_programUniform1uiv programUniform1uiv;
	RenderCommand_programUniform2i programUniform2i;
	RenderCommand_programUniform2iv programUniform2iv;
	RenderCommand_programUniform2f programUniform2f;
	RenderCommand_programUniform2fv programUniform2fv;
	RenderCommand_programUniform2d programUniform2d;
	RenderCommand_programUniform2dv programUniform2dv;
	RenderCommand_programUniform2ui programUniform2ui;
	RenderCommand_programUniform2uiv programUniform2uiv;
	RenderCommand_programUniform3i programUniform3i;
	RenderCommand_programUniform3iv programUniform3iv;
	RenderCommand_programUniform3f programUniform3f;
	RenderCommand_programUniform3fv programUniform3fv;
	RenderCommand_programUniform3d programUniform3d;
	RenderCommand_programUniform3dv programUniform3dv;
	RenderCommand_programUniform3ui programUniform3ui;
	RenderCommand_programUniform3uiv programUniform3uiv;
	RenderCommand_programUniform4i programUniform4i;
	RenderCommand_programUniform4iv programUniform4iv;
	RenderCommand_programUniform4f programUniform4f;
	RenderCommand_programUniform4fv programUniform4fv;
	RenderCommand_programUniform4d programUniform4d;
	RenderCommand_programUniform4dv programUniform4dv;
	RenderCommand_programUniform4ui programUniform4ui;
	RenderCommand_programUniform4uiv programUniform4uiv;
	RenderCommand_programUniformMatrix2fv programUniformMatrix2fv;
	RenderCommand_programUniformMatrix3fv programUniformMatrix3fv;
	RenderCommand_programUniformMatrix4fv programUniformMatrix4fv;
	RenderCommand_programUniformMatrix2dv programUniformMatrix2dv;
	RenderCommand_programUniformMatrix3dv programUniformMatrix3dv;
	RenderCommand_programUniformMatrix4dv programUniformMatrix4dv;
	RenderCommand_programUniformMatrix2x3fv programUniformMatrix2x3fv;
	RenderCommand_programUniformMatrix3x2fv programUniformMatrix3x2fv;
	RenderCommand_programUniformMatrix2x4fv programUniformMatrix2x4fv;
	RenderCommand_programUniformMatrix4x2fv programUniformMatrix4x2fv;
	RenderCommand_programUniformMatrix3x4fv programUniformMatrix3x4fv;
	RenderCommand_programUniformMatrix4x3fv programUniformMatrix4x3fv;
	RenderCommand_programUniformMatrix2x3dv programUniformMatrix2x3dv;
	RenderCommand_programUniformMatrix3x2dv programUniformMatrix3x2dv;
	RenderCommand_programUniformMatrix2x4dv programUniformMatrix2x4dv;
	RenderCommand_programUniformMatrix4x2dv programUniformMatrix4x2dv;
	RenderCommand_programUniformMatrix3x4dv programUniformMatrix3x4dv;
	RenderCommand_programUniformMatrix4x3dv programUniformMatrix4x3dv;
	RenderCommand_validateProgramPipeline validateProgramPipeline;
	RenderCommand_getProgramPipelineInfoLog getProgramPipelineInfoLog;
	RenderCommand_vertexAttribL1d vertexAttribL1d;
	RenderCommand_vertexAttribL2d vertexAttribL2d;
	RenderCommand_vertexAttribL3d vertexAttribL3d;
	RenderCommand_vertexAttribL4d vertexAttribL4d;
	RenderCommand_vertexAttribL1dv vertexAttribL1dv;
	RenderCommand_vertexAttribL2dv vertexAttribL2dv;
	RenderCommand_vertexAttribL3dv vertexAttribL3dv;
	RenderCommand_vertexAttribL4dv vertexAttribL4dv;
	RenderCommand_vertexAttribLPointer vertexAttribLPointer;
	RenderCommand_getVertexAttribLdv getVertexAttribLdv;
	RenderCommand_viewportArrayv viewportArrayv;
	RenderCommand_viewportIndexedf viewportIndexedf;
	RenderCommand_viewportIndexedfv viewportIndexedfv;
	RenderCommand_scissorArrayv scissorArrayv;
	RenderCommand_scissorIndexed scissorIndexed;
	RenderCommand_scissorIndexedv scissorIndexedv;
	RenderCommand_depthRangeArrayv depthRangeArrayv;
	RenderCommand_depthRangeIndexed depthRangeIndexed;
	RenderCommand_getFloati_v getFloati_v;
	RenderCommand_getDoublei_v getDoublei_v;
	RenderCommand_drawArraysInstancedBaseInstance drawArraysInstancedBaseInstance;
	RenderCommand_drawElementsInstancedBaseInstance drawElementsInstancedBaseInstance;
	RenderCommand_drawElementsInstancedBaseVertexBaseInstance drawElementsInstancedBaseVertexBaseInstance;
	RenderCommand_getInternalformativ getInternalformativ;
	RenderCommand_getActiveAtomicCounterBufferiv getActiveAtomicCounterBufferiv;
	RenderCommand_bindImageTexture bindImageTexture;
	RenderCommand_memoryBarrier memoryBarrier;
	RenderCommand_texStorage1D texStorage1D;
	RenderCommand_texStorage2D texStorage2D;
	RenderCommand_texStorage3D texStorage3D;
	RenderCommand_drawTransformFeedbackInstanced drawTransformFeedbackInstanced;
	RenderCommand_drawTransformFeedbackStreamInstanced drawTransformFeedbackStreamInstanced;
	RenderCommand_clearBufferData clearBufferData;
	RenderCommand_clearBufferSubData clearBufferSubData;
	RenderCommand_dispatchCompute dispatchCompute;
	RenderCommand_dispatchComputeIndirect dispatchComputeIndirect;
	RenderCommand_copyImageSubData copyImageSubData;
	RenderCommand_framebufferParameteri framebufferParameteri;
	RenderCommand_getFramebufferParameteriv getFramebufferParameteriv;
	RenderCommand_getInternalformati64v getInternalformati64v;
	RenderCommand_invalidateTexSubImage invalidateTexSubImage;
	RenderCommand_invalidateTexImage invalidateTexImage;
	RenderCommand_invalidateBufferSubData invalidateBufferSubData;
	RenderCommand_invalidateBufferData invalidateBufferData;
	RenderCommand_invalidateFramebuffer invalidateFramebuffer;
	RenderCommand_invalidateSubFramebuffer invalidateSubFramebuffer;
	RenderCommand_multiDrawArraysIndirect multiDrawArraysIndirect;
	RenderCommand_multiDrawElementsIndirect multiDrawElementsIndirect;
	RenderCommand_getProgramInterfaceiv getProgramInterfaceiv;
	RenderCommand_getProgramResourceIndex getProgramResourceIndex;
	RenderCommand_getProgramResourceName getProgramResourceName;
	RenderCommand_getProgramResourceiv getProgramResourceiv;
	RenderCommand_getProgramResourceLocation getProgramResourceLocation;
	RenderCommand_getProgramResourceLocationIndex getProgramResourceLocationIndex;
	RenderCommand_shaderStorageBlockBinding shaderStorageBlockBinding;
	RenderCommand_texBufferRange texBufferRange;
	RenderCommand_texStorage2DMultisample texStorage2DMultisample;
	RenderCommand_texStorage3DMultisample texStorage3DMultisample;
	RenderCommand_textureView textureView;
	RenderCommand_bindVertexBuffer bindVertexBuffer;
	RenderCommand_vertexAttribFormat vertexAttribFormat;
	RenderCommand_vertexAttribIFormat vertexAttribIFormat;
	RenderCommand_vertexAttribLFormat vertexAttribLFormat;
	RenderCommand_vertexAttribBinding vertexAttribBinding;
	RenderCommand_vertexBindingDivisor vertexBindingDivisor;
	RenderCommand_debugMessageControl debugMessageControl;
	RenderCommand_debugMessageInsert debugMessageInsert;
	RenderCommand_debugMessageCallback debugMessageCallback;
	RenderCommand_getDebugMessageLog getDebugMessageLog;
	RenderCommand_pushDebugGroup pushDebugGroup;
	RenderCommand_popDebugGroup popDebugGroup;
	RenderCommand_objectLabel objectLabel;
	RenderCommand_getObjectLabel getObjectLabel;
	RenderCommand_objectPtrLabel objectPtrLabel;
	RenderCommand_getObjectPtrLabel getObjectPtrLabel;
	RenderCommand_bufferStorage bufferStorage;
	RenderCommand_clearTexImage clearTexImage;
	RenderCommand_clearTexSubImage clearTexSubImage;
	RenderCommand_bindBuffersBase bindBuffersBase;
	RenderCommand_bindBuffersRange bindBuffersRange;
	RenderCommand_bindTextures bindTextures;
	RenderCommand_bindSamplers bindSamplers;
	RenderCommand_bindImageTextures bindImageTextures;
	RenderCommand_bindVertexBuffers bindVertexBuffers;
	RenderCommand_clipControl clipControl;
	RenderCommand_createTransformFeedbacks createTransformFeedbacks;
	RenderCommand_transformFeedbackBufferBase transformFeedbackBufferBase;
	RenderCommand_transformFeedbackBufferRange transformFeedbackBufferRange;
	RenderCommand_getTransformFeedbackiv getTransformFeedbackiv;
	RenderCommand_getTransformFeedbacki_v getTransformFeedbacki_v;
	RenderCommand_getTransformFeedbacki64_v getTransformFeedbacki64_v;
	RenderCommand_createBuffers createBuffers;
	RenderCommand_namedBufferStorage namedBufferStorage;
	RenderCommand_namedBufferData namedBufferData;
	RenderCommand_namedBufferSubData namedBufferSubData;
	RenderCommand_copyNamedBufferSubData copyNamedBufferSubData;
	RenderCommand_clearNamedBufferData clearNamedBufferData;
	RenderCommand_clearNamedBufferSubData clearNamedBufferSubData;
	RenderCommand_mapNamedBuffer mapNamedBuffer;
	RenderCommand_mapNamedBufferRange mapNamedBufferRange;
	RenderCommand_unmapNamedBuffer unmapNamedBuffer;
	RenderCommand_flushMappedNamedBufferRange flushMappedNamedBufferRange;
	RenderCommand_getNamedBufferParameteriv getNamedBufferParameteriv;
	RenderCommand_getNamedBufferParameteri64v getNamedBufferParameteri64v;
	RenderCommand_getNamedBufferPointerv getNamedBufferPointerv;
	RenderCommand_getNamedBufferSubData getNamedBufferSubData;
	RenderCommand_createFramebuffers createFramebuffers;
	RenderCommand_namedFramebufferRenderbuffer namedFramebufferRenderbuffer;
	RenderCommand_namedFramebufferParameteri namedFramebufferParameteri;
	RenderCommand_namedFramebufferTexture namedFramebufferTexture;
	RenderCommand_namedFramebufferTextureLayer namedFramebufferTextureLayer;
	RenderCommand_namedFramebufferDrawBuffer namedFramebufferDrawBuffer;
	RenderCommand_namedFramebufferDrawBuffers namedFramebufferDrawBuffers;
	RenderCommand_namedFramebufferReadBuffer namedFramebufferReadBuffer;
	RenderCommand_invalidateNamedFramebufferData invalidateNamedFramebufferData;
	RenderCommand_invalidateNamedFramebufferSubData invalidateNamedFramebufferSubData;
	RenderCommand_clearNamedFramebufferiv clearNamedFramebufferiv;
	RenderCommand_clearNamedFramebufferuiv clearNamedFramebufferuiv;
	RenderCommand_clearNamedFramebufferfv clearNamedFramebufferfv;
	RenderCommand_clearNamedFramebufferfi clearNamedFramebufferfi;
	RenderCommand_blitNamedFramebuffer blitNamedFramebuffer;
	RenderCommand_checkNamedFramebufferStatus checkNamedFramebufferStatus;
	RenderCommand_getNamedFramebufferParameteriv getNamedFramebufferParameteriv;
	RenderCommand_getNamedFramebufferAttachmentParameteriv getNamedFramebufferAttachmentParameteriv;
	RenderCommand_createRenderbuffers createRenderbuffers;
	RenderCommand_namedRenderbufferStorage namedRenderbufferStorage;
	RenderCommand_namedRenderbufferStorageMultisample namedRenderbufferStorageMultisample;
	RenderCommand_getNamedRenderbufferParameteriv getNamedRenderbufferParameteriv;
	RenderCommand_createTextures createTextures;
	RenderCommand_textureBuffer textureBuffer;
	RenderCommand_textureBufferRange textureBufferRange;
	RenderCommand_textureStorage1D textureStorage1D;
	RenderCommand_textureStorage2D textureStorage2D;
	RenderCommand_textureStorage3D textureStorage3D;
	RenderCommand_textureStorage2DMultisample textureStorage2DMultisample;
	RenderCommand_textureStorage3DMultisample textureStorage3DMultisample;
	RenderCommand_textureSubImage1D textureSubImage1D;
	RenderCommand_textureSubImage2D textureSubImage2D;
	RenderCommand_textureSubImage3D textureSubImage3D;
	RenderCommand_compressedTextureSubImage1D compressedTextureSubImage1D;
	RenderCommand_compressedTextureSubImage2D compressedTextureSubImage2D;
	RenderCommand_compressedTextureSubImage3D compressedTextureSubImage3D;
	RenderCommand_copyTextureSubImage1D copyTextureSubImage1D;
	RenderCommand_copyTextureSubImage2D copyTextureSubImage2D;
	RenderCommand_copyTextureSubImage3D copyTextureSubImage3D;
	RenderCommand_textureParameterf textureParameterf;
	RenderCommand_textureParameterfv textureParameterfv;
	RenderCommand_textureParameteri textureParameteri;
	RenderCommand_textureParameterIiv textureParameterIiv;
	RenderCommand_textureParameterIuiv textureParameterIuiv;
	RenderCommand_textureParameteriv textureParameteriv;
	RenderCommand_generateTextureMipmap generateTextureMipmap;
	RenderCommand_bindTextureUnit bindTextureUnit;
	RenderCommand_getTextureImage getTextureImage;
	RenderCommand_getCompressedTextureImage getCompressedTextureImage;
	RenderCommand_getTextureLevelParameterfv getTextureLevelParameterfv;
	RenderCommand_getTextureLevelParameteriv getTextureLevelParameteriv;
	RenderCommand_getTextureParameterfv getTextureParameterfv;
	RenderCommand_getTextureParameterIiv getTextureParameterIiv;
	RenderCommand_getTextureParameterIuiv getTextureParameterIuiv;
	RenderCommand_getTextureParameteriv getTextureParameteriv;
	RenderCommand_createVertexArrays createVertexArrays;
	RenderCommand_disableVertexArrayAttrib disableVertexArrayAttrib;
	RenderCommand_enableVertexArrayAttrib enableVertexArrayAttrib;
	RenderCommand_vertexArrayElementBuffer vertexArrayElementBuffer;
	RenderCommand_vertexArrayVertexBuffer vertexArrayVertexBuffer;
	RenderCommand_vertexArrayVertexBuffers vertexArrayVertexBuffers;
	RenderCommand_vertexArrayAttribBinding vertexArrayAttribBinding;
	RenderCommand_vertexArrayAttribFormat vertexArrayAttribFormat;
	RenderCommand_vertexArrayAttribIFormat vertexArrayAttribIFormat;
	RenderCommand_vertexArrayAttribLFormat vertexArrayAttribLFormat;
	RenderCommand_vertexArrayBindingDivisor vertexArrayBindingDivisor;
	RenderCommand_getVertexArrayiv getVertexArrayiv;
	RenderCommand_getVertexArrayIndexediv getVertexArrayIndexediv;
	RenderCommand_getVertexArrayIndexed64iv getVertexArrayIndexed64iv;
	RenderCommand_createSamplers createSamplers;
	RenderCommand_createProgramPipelines createProgramPipelines;
	RenderCommand_createQueries createQueries;
	RenderCommand_getQueryBufferObjecti64v getQueryBufferObjecti64v;
	RenderCommand_getQueryBufferObjectiv getQueryBufferObjectiv;
	RenderCommand_getQueryBufferObjectui64v getQueryBufferObjectui64v;
	RenderCommand_getQueryBufferObjectuiv getQueryBufferObjectuiv;
	RenderCommand_memoryBarrierByRegion memoryBarrierByRegion;
	RenderCommand_getTextureSubImage getTextureSubImage;
	RenderCommand_getCompressedTextureSubImage getCompressedTextureSubImage;
	RenderCommand_getGraphicsResetStatus getGraphicsResetStatus;
	RenderCommand_getnCompressedTexImage getnCompressedTexImage;
	RenderCommand_getnTexImage getnTexImage;
	RenderCommand_getnUniformdv getnUniformdv;
	RenderCommand_getnUniformfv getnUniformfv;
	RenderCommand_getnUniformiv getnUniformiv;
	RenderCommand_getnUniformuiv getnUniformuiv;
	RenderCommand_readnPixels readnPixels;
	RenderCommand_textureBarrier textureBarrier;
	RenderCommand_specializeShader specializeShader;
	RenderCommand_multiDrawArraysIndirectCount multiDrawArraysIndirectCount;
	RenderCommand_multiDrawElementsIndirectCount multiDrawElementsIndirectCount;
	RenderCommand_polygonOffsetClamp polygonOffsetClamp;
	RenderCommand_primitiveBoundingBoxARB primitiveBoundingBoxARB;
	RenderCommand_getTextureHandleARB getTextureHandleARB;
	RenderCommand_getTextureSamplerHandleARB getTextureSamplerHandleARB;
	RenderCommand_makeTextureHandleResidentARB makeTextureHandleResidentARB;
	RenderCommand_makeTextureHandleNonResidentARB makeTextureHandleNonResidentARB;
	RenderCommand_getImageHandleARB getImageHandleARB;
	RenderCommand_makeImageHandleResidentARB makeImageHandleResidentARB;
	RenderCommand_makeImageHandleNonResidentARB makeImageHandleNonResidentARB;
	RenderCommand_uniformHandleui64ARB uniformHandleui64ARB;
	RenderCommand_uniformHandleui64vARB uniformHandleui64vARB;
	RenderCommand_programUniformHandleui64ARB programUniformHandleui64ARB;
	RenderCommand_programUniformHandleui64vARB programUniformHandleui64vARB;
	RenderCommand_isTextureHandleResidentARB isTextureHandleResidentARB;
	RenderCommand_isImageHandleResidentARB isImageHandleResidentARB;
	RenderCommand_vertexAttribL1ui64ARB vertexAttribL1ui64ARB;
	RenderCommand_vertexAttribL1ui64vARB vertexAttribL1ui64vARB;
	RenderCommand_getVertexAttribLui64vARB getVertexAttribLui64vARB;
	RenderCommand_createSyncFromCLeventARB createSyncFromCLeventARB;
	RenderCommand_dispatchComputeGroupSizeARB dispatchComputeGroupSizeARB;
	RenderCommand_debugMessageControlARB debugMessageControlARB;
	RenderCommand_debugMessageInsertARB debugMessageInsertARB;
	RenderCommand_debugMessageCallbackARB debugMessageCallbackARB;
	RenderCommand_getDebugMessageLogARB getDebugMessageLogARB;
	RenderCommand_blendEquationiARB blendEquationiARB;
	RenderCommand_blendEquationSeparateiARB blendEquationSeparateiARB;
	RenderCommand_blendFunciARB blendFunciARB;
	RenderCommand_blendFuncSeparateiARB blendFuncSeparateiARB;
	RenderCommand_drawArraysInstancedARB drawArraysInstancedARB;
	RenderCommand_drawElementsInstancedARB drawElementsInstancedARB;
	RenderCommand_programParameteriARB programParameteriARB;
	RenderCommand_framebufferTextureARB framebufferTextureARB;
	RenderCommand_framebufferTextureLayerARB framebufferTextureLayerARB;
	RenderCommand_framebufferTextureFaceARB framebufferTextureFaceARB;
	RenderCommand_specializeShaderARB specializeShaderARB;
	RenderCommand_uniform1i64ARB uniform1i64ARB;
	RenderCommand_uniform2i64ARB uniform2i64ARB;
	RenderCommand_uniform3i64ARB uniform3i64ARB;
	RenderCommand_uniform4i64ARB uniform4i64ARB;
	RenderCommand_uniform1i64vARB uniform1i64vARB;
	RenderCommand_uniform2i64vARB uniform2i64vARB;
	RenderCommand_uniform3i64vARB uniform3i64vARB;
	RenderCommand_uniform4i64vARB uniform4i64vARB;
	RenderCommand_uniform1ui64ARB uniform1ui64ARB;
	RenderCommand_uniform2ui64ARB uniform2ui64ARB;
	RenderCommand_uniform3ui64ARB uniform3ui64ARB;
	RenderCommand_uniform4ui64ARB uniform4ui64ARB;
	RenderCommand_uniform1ui64vARB uniform1ui64vARB;
	RenderCommand_uniform2ui64vARB uniform2ui64vARB;
	RenderCommand_uniform3ui64vARB uniform3ui64vARB;
	RenderCommand_uniform4ui64vARB uniform4ui64vARB;
	RenderCommand_getUniformi64vARB getUniformi64vARB;
	RenderCommand_getUniformui64vARB getUniformui64vARB;
	RenderCommand_getnUniformi64vARB getnUniformi64vARB;
	RenderCommand_getnUniformui64vARB getnUniformui64vARB;
	RenderCommand_programUniform1i64ARB programUniform1i64ARB;
	RenderCommand_programUniform2i64ARB programUniform2i64ARB;
	RenderCommand_programUniform3i64ARB programUniform3i64ARB;
	RenderCommand_programUniform4i64ARB programUniform4i64ARB;
	RenderCommand_programUniform1i64vARB programUniform1i64vARB;
	RenderCommand_programUniform2i64vARB programUniform2i64vARB;
	RenderCommand_programUniform3i64vARB programUniform3i64vARB;
	RenderCommand_programUniform4i64vARB programUniform4i64vARB;
	RenderCommand_programUniform1ui64ARB programUniform1ui64ARB;
	RenderCommand_programUniform2ui64ARB programUniform2ui64ARB;
	RenderCommand_programUniform3ui64ARB programUniform3ui64ARB;
	RenderCommand_programUniform4ui64ARB programUniform4ui64ARB;
	RenderCommand_programUniform1ui64vARB programUniform1ui64vARB;
	RenderCommand_programUniform2ui64vARB programUniform2ui64vARB;
	RenderCommand_programUniform3ui64vARB programUniform3ui64vARB;
	RenderCommand_programUniform4ui64vARB programUniform4ui64vARB;
	RenderCommand_multiDrawArraysIndirectCountARB multiDrawArraysIndirectCountARB;
	RenderCommand_multiDrawElementsIndirectCountARB multiDrawElementsIndirectCountARB;
	RenderCommand_vertexAttribDivisorARB vertexAttribDivisorARB;
	RenderCommand_maxShaderCompilerThreadsARB maxShaderCompilerThreadsARB;
	RenderCommand_getGraphicsResetStatusARB getGraphicsResetStatusARB;
	RenderCommand_getnTexImageARB getnTexImageARB;
	RenderCommand_readnPixelsARB readnPixelsARB;
	RenderCommand_getnCompressedTexImageARB getnCompressedTexImageARB;
	RenderCommand_getnUniformfvARB getnUniformfvARB;
	RenderCommand_getnUniformivARB getnUniformivARB;
	RenderCommand_getnUniformuivARB getnUniformuivARB;
	RenderCommand_getnUniformdvARB getnUniformdvARB;
	RenderCommand_framebufferSampleLocationsfvARB framebufferSampleLocationsfvARB;
	RenderCommand_namedFramebufferSampleLocationsfvARB namedFramebufferSampleLocationsfvARB;
	RenderCommand_evaluateDepthValuesARB evaluateDepthValuesARB;
	RenderCommand_minSampleShadingARB minSampleShadingARB;
	RenderCommand_namedStringARB namedStringARB;
	RenderCommand_deleteNamedStringARB deleteNamedStringARB;
	RenderCommand_compileShaderIncludeARB compileShaderIncludeARB;
	RenderCommand_isNamedStringARB isNamedStringARB;
	RenderCommand_getNamedStringARB getNamedStringARB;
	RenderCommand_getNamedStringivARB getNamedStringivARB;
	RenderCommand_bufferPageCommitmentARB bufferPageCommitmentARB;
	RenderCommand_namedBufferPageCommitmentEXT namedBufferPageCommitmentEXT;
	RenderCommand_namedBufferPageCommitmentARB namedBufferPageCommitmentARB;
	RenderCommand_texPageCommitmentARB texPageCommitmentARB;
	RenderCommand_texBufferARB texBufferARB;
	RenderCommand_blendBarrierKHR blendBarrierKHR;
	RenderCommand_maxShaderCompilerThreadsKHR maxShaderCompilerThreadsKHR;
	RenderCommand_renderbufferStorageMultisampleAdvancedAMD renderbufferStorageMultisampleAdvancedAMD;
	RenderCommand_namedRenderbufferStorageMultisampleAdvancedAMD namedRenderbufferStorageMultisampleAdvancedAMD;
	RenderCommand_getPerfMonitorGroupsAMD getPerfMonitorGroupsAMD;
	RenderCommand_getPerfMonitorCountersAMD getPerfMonitorCountersAMD;
	RenderCommand_getPerfMonitorGroupStringAMD getPerfMonitorGroupStringAMD;
	RenderCommand_getPerfMonitorCounterStringAMD getPerfMonitorCounterStringAMD;
	RenderCommand_getPerfMonitorCounterInfoAMD getPerfMonitorCounterInfoAMD;
	RenderCommand_genPerfMonitorsAMD genPerfMonitorsAMD;
	RenderCommand_deletePerfMonitorsAMD deletePerfMonitorsAMD;
	RenderCommand_selectPerfMonitorCountersAMD selectPerfMonitorCountersAMD;
	RenderCommand_beginPerfMonitorAMD beginPerfMonitorAMD;
	RenderCommand_endPerfMonitorAMD endPerfMonitorAMD;
	RenderCommand_getPerfMonitorCounterDataAMD getPerfMonitorCounterDataAMD;
	RenderCommand_eGLImageTargetTexStorageEXT eGLImageTargetTexStorageEXT;
	RenderCommand_eGLImageTargetTextureStorageEXT eGLImageTargetTextureStorageEXT;
	RenderCommand_labelObjectEXT labelObjectEXT;
	RenderCommand_getObjectLabelEXT getObjectLabelEXT;
	RenderCommand_insertEventMarkerEXT insertEventMarkerEXT;
	RenderCommand_pushGroupMarkerEXT pushGroupMarkerEXT;
	RenderCommand_popGroupMarkerEXT popGroupMarkerEXT;
	RenderCommand_matrixLoadfEXT matrixLoadfEXT;
	RenderCommand_matrixLoaddEXT matrixLoaddEXT;
	RenderCommand_matrixMultfEXT matrixMultfEXT;
	RenderCommand_matrixMultdEXT matrixMultdEXT;
	RenderCommand_matrixLoadIdentityEXT matrixLoadIdentityEXT;
	RenderCommand_matrixRotatefEXT matrixRotatefEXT;
	RenderCommand_matrixRotatedEXT matrixRotatedEXT;
	RenderCommand_matrixScalefEXT matrixScalefEXT;
	RenderCommand_matrixScaledEXT matrixScaledEXT;
	RenderCommand_matrixTranslatefEXT matrixTranslatefEXT;
	RenderCommand_matrixTranslatedEXT matrixTranslatedEXT;
	RenderCommand_matrixFrustumEXT matrixFrustumEXT;
	RenderCommand_matrixOrthoEXT matrixOrthoEXT;
	RenderCommand_matrixPopEXT matrixPopEXT;
	RenderCommand_matrixPushEXT matrixPushEXT;
	RenderCommand_clientAttribDefaultEXT clientAttribDefaultEXT;
	RenderCommand_pushClientAttribDefaultEXT pushClientAttribDefaultEXT;
	RenderCommand_textureParameterfEXT textureParameterfEXT;
	RenderCommand_textureParameterfvEXT textureParameterfvEXT;
	RenderCommand_textureParameteriEXT textureParameteriEXT;
	RenderCommand_textureParameterivEXT textureParameterivEXT;
	RenderCommand_textureImage1DEXT textureImage1DEXT;
	RenderCommand_textureImage2DEXT textureImage2DEXT;
	RenderCommand_textureSubImage1DEXT textureSubImage1DEXT;
	RenderCommand_textureSubImage2DEXT textureSubImage2DEXT;
	RenderCommand_copyTextureImage1DEXT copyTextureImage1DEXT;
	RenderCommand_copyTextureImage2DEXT copyTextureImage2DEXT;
	RenderCommand_copyTextureSubImage1DEXT copyTextureSubImage1DEXT;
	RenderCommand_copyTextureSubImage2DEXT copyTextureSubImage2DEXT;
	RenderCommand_getTextureImageEXT getTextureImageEXT;
	RenderCommand_getTextureParameterfvEXT getTextureParameterfvEXT;
	RenderCommand_getTextureParameterivEXT getTextureParameterivEXT;
	RenderCommand_getTextureLevelParameterfvEXT getTextureLevelParameterfvEXT;
	RenderCommand_getTextureLevelParameterivEXT getTextureLevelParameterivEXT;
	RenderCommand_textureImage3DEXT textureImage3DEXT;
	RenderCommand_textureSubImage3DEXT textureSubImage3DEXT;
	RenderCommand_copyTextureSubImage3DEXT copyTextureSubImage3DEXT;
	RenderCommand_bindMultiTextureEXT bindMultiTextureEXT;
	RenderCommand_multiTexCoordPointerEXT multiTexCoordPointerEXT;
	RenderCommand_multiTexEnvfEXT multiTexEnvfEXT;
	RenderCommand_multiTexEnvfvEXT multiTexEnvfvEXT;
	RenderCommand_multiTexEnviEXT multiTexEnviEXT;
	RenderCommand_multiTexEnvivEXT multiTexEnvivEXT;
	RenderCommand_multiTexGendEXT multiTexGendEXT;
	RenderCommand_multiTexGendvEXT multiTexGendvEXT;
	RenderCommand_multiTexGenfEXT multiTexGenfEXT;
	RenderCommand_multiTexGenfvEXT multiTexGenfvEXT;
	RenderCommand_multiTexGeniEXT multiTexGeniEXT;
	RenderCommand_multiTexGenivEXT multiTexGenivEXT;
	RenderCommand_getMultiTexEnvfvEXT getMultiTexEnvfvEXT;
	RenderCommand_getMultiTexEnvivEXT getMultiTexEnvivEXT;
	RenderCommand_getMultiTexGendvEXT getMultiTexGendvEXT;
	RenderCommand_getMultiTexGenfvEXT getMultiTexGenfvEXT;
	RenderCommand_getMultiTexGenivEXT getMultiTexGenivEXT;
	RenderCommand_multiTexParameteriEXT multiTexParameteriEXT;
	RenderCommand_multiTexParameterivEXT multiTexParameterivEXT;
	RenderCommand_multiTexParameterfEXT multiTexParameterfEXT;
	RenderCommand_multiTexParameterfvEXT multiTexParameterfvEXT;
	RenderCommand_multiTexImage1DEXT multiTexImage1DEXT;
	RenderCommand_multiTexImage2DEXT multiTexImage2DEXT;
	RenderCommand_multiTexSubImage1DEXT multiTexSubImage1DEXT;
	RenderCommand_multiTexSubImage2DEXT multiTexSubImage2DEXT;
	RenderCommand_copyMultiTexImage1DEXT copyMultiTexImage1DEXT;
	RenderCommand_copyMultiTexImage2DEXT copyMultiTexImage2DEXT;
	RenderCommand_copyMultiTexSubImage1DEXT copyMultiTexSubImage1DEXT;
	RenderCommand_copyMultiTexSubImage2DEXT copyMultiTexSubImage2DEXT;
	RenderCommand_getMultiTexImageEXT getMultiTexImageEXT;
	RenderCommand_getMultiTexParameterfvEXT getMultiTexParameterfvEXT;
	RenderCommand_getMultiTexParameterivEXT getMultiTexParameterivEXT;
	RenderCommand_getMultiTexLevelParameterfvEXT getMultiTexLevelParameterfvEXT;
	RenderCommand_getMultiTexLevelParameterivEXT getMultiTexLevelParameterivEXT;
	RenderCommand_multiTexImage3DEXT multiTexImage3DEXT;
	RenderCommand_multiTexSubImage3DEXT multiTexSubImage3DEXT;
	RenderCommand_copyMultiTexSubImage3DEXT copyMultiTexSubImage3DEXT;
	RenderCommand_enableClientStateIndexedEXT enableClientStateIndexedEXT;
	RenderCommand_disableClientStateIndexedEXT disableClientStateIndexedEXT;
	RenderCommand_getFloatIndexedvEXT getFloatIndexedvEXT;
	RenderCommand_getDoubleIndexedvEXT getDoubleIndexedvEXT;
	RenderCommand_getPointerIndexedvEXT getPointerIndexedvEXT;
	RenderCommand_enableIndexedEXT enableIndexedEXT;
	RenderCommand_disableIndexedEXT disableIndexedEXT;
	RenderCommand_isEnabledIndexedEXT isEnabledIndexedEXT;
	RenderCommand_getIntegerIndexedvEXT getIntegerIndexedvEXT;
	RenderCommand_getBooleanIndexedvEXT getBooleanIndexedvEXT;
	RenderCommand_compressedTextureImage3DEXT compressedTextureImage3DEXT;
	RenderCommand_compressedTextureImage2DEXT compressedTextureImage2DEXT;
	RenderCommand_compressedTextureImage1DEXT compressedTextureImage1DEXT;
	RenderCommand_compressedTextureSubImage3DEXT compressedTextureSubImage3DEXT;
	RenderCommand_compressedTextureSubImage2DEXT compressedTextureSubImage2DEXT;
	RenderCommand_compressedTextureSubImage1DEXT compressedTextureSubImage1DEXT;
	RenderCommand_getCompressedTextureImageEXT getCompressedTextureImageEXT;
	RenderCommand_compressedMultiTexImage3DEXT compressedMultiTexImage3DEXT;
	RenderCommand_compressedMultiTexImage2DEXT compressedMultiTexImage2DEXT;
	RenderCommand_compressedMultiTexImage1DEXT compressedMultiTexImage1DEXT;
	RenderCommand_compressedMultiTexSubImage3DEXT compressedMultiTexSubImage3DEXT;
	RenderCommand_compressedMultiTexSubImage2DEXT compressedMultiTexSubImage2DEXT;
	RenderCommand_compressedMultiTexSubImage1DEXT compressedMultiTexSubImage1DEXT;
	RenderCommand_getCompressedMultiTexImageEXT getCompressedMultiTexImageEXT;
	RenderCommand_matrixLoadTransposefEXT matrixLoadTransposefEXT;
	RenderCommand_matrixLoadTransposedEXT matrixLoadTransposedEXT;
	RenderCommand_matrixMultTransposefEXT matrixMultTransposefEXT;
	RenderCommand_matrixMultTransposedEXT matrixMultTransposedEXT;
	RenderCommand_namedBufferDataEXT namedBufferDataEXT;
	RenderCommand_namedBufferSubDataEXT namedBufferSubDataEXT;
	RenderCommand_mapNamedBufferEXT mapNamedBufferEXT;
	RenderCommand_unmapNamedBufferEXT unmapNamedBufferEXT;
	RenderCommand_getNamedBufferParameterivEXT getNamedBufferParameterivEXT;
	RenderCommand_getNamedBufferPointervEXT getNamedBufferPointervEXT;
	RenderCommand_getNamedBufferSubDataEXT getNamedBufferSubDataEXT;
	RenderCommand_programUniform1fEXT programUniform1fEXT;
	RenderCommand_programUniform2fEXT programUniform2fEXT;
	RenderCommand_programUniform3fEXT programUniform3fEXT;
	RenderCommand_programUniform4fEXT programUniform4fEXT;
	RenderCommand_programUniform1iEXT programUniform1iEXT;
	RenderCommand_programUniform2iEXT programUniform2iEXT;
	RenderCommand_programUniform3iEXT programUniform3iEXT;
	RenderCommand_programUniform4iEXT programUniform4iEXT;
	RenderCommand_programUniform1fvEXT programUniform1fvEXT;
	RenderCommand_programUniform2fvEXT programUniform2fvEXT;
	RenderCommand_programUniform3fvEXT programUniform3fvEXT;
	RenderCommand_programUniform4fvEXT programUniform4fvEXT;
	RenderCommand_programUniform1ivEXT programUniform1ivEXT;
	RenderCommand_programUniform2ivEXT programUniform2ivEXT;
	RenderCommand_programUniform3ivEXT programUniform3ivEXT;
	RenderCommand_programUniform4ivEXT programUniform4ivEXT;
	RenderCommand_programUniformMatrix2fvEXT programUniformMatrix2fvEXT;
	RenderCommand_programUniformMatrix3fvEXT programUniformMatrix3fvEXT;
	RenderCommand_programUniformMatrix4fvEXT programUniformMatrix4fvEXT;
	RenderCommand_programUniformMatrix2x3fvEXT programUniformMatrix2x3fvEXT;
	RenderCommand_programUniformMatrix3x2fvEXT programUniformMatrix3x2fvEXT;
	RenderCommand_programUniformMatrix2x4fvEXT programUniformMatrix2x4fvEXT;
	RenderCommand_programUniformMatrix4x2fvEXT programUniformMatrix4x2fvEXT;
	RenderCommand_programUniformMatrix3x4fvEXT programUniformMatrix3x4fvEXT;
	RenderCommand_programUniformMatrix4x3fvEXT programUniformMatrix4x3fvEXT;
	RenderCommand_textureBufferEXT textureBufferEXT;
	RenderCommand_multiTexBufferEXT multiTexBufferEXT;
	RenderCommand_textureParameterIivEXT textureParameterIivEXT;
	RenderCommand_textureParameterIuivEXT textureParameterIuivEXT;
	RenderCommand_getTextureParameterIivEXT getTextureParameterIivEXT;
	RenderCommand_getTextureParameterIuivEXT getTextureParameterIuivEXT;
	RenderCommand_multiTexParameterIivEXT multiTexParameterIivEXT;
	RenderCommand_multiTexParameterIuivEXT multiTexParameterIuivEXT;
	RenderCommand_getMultiTexParameterIivEXT getMultiTexParameterIivEXT;
	RenderCommand_getMultiTexParameterIuivEXT getMultiTexParameterIuivEXT;
	RenderCommand_programUniform1uiEXT programUniform1uiEXT;
	RenderCommand_programUniform2uiEXT programUniform2uiEXT;
	RenderCommand_programUniform3uiEXT programUniform3uiEXT;
	RenderCommand_programUniform4uiEXT programUniform4uiEXT;
	RenderCommand_programUniform1uivEXT programUniform1uivEXT;
	RenderCommand_programUniform2uivEXT programUniform2uivEXT;
	RenderCommand_programUniform3uivEXT programUniform3uivEXT;
	RenderCommand_programUniform4uivEXT programUniform4uivEXT;
	RenderCommand_namedProgramLocalParameters4fvEXT namedProgramLocalParameters4fvEXT;
	RenderCommand_namedProgramLocalParameterI4iEXT namedProgramLocalParameterI4iEXT;
	RenderCommand_namedProgramLocalParameterI4ivEXT namedProgramLocalParameterI4ivEXT;
	RenderCommand_namedProgramLocalParametersI4ivEXT namedProgramLocalParametersI4ivEXT;
	RenderCommand_namedProgramLocalParameterI4uiEXT namedProgramLocalParameterI4uiEXT;
	RenderCommand_namedProgramLocalParameterI4uivEXT namedProgramLocalParameterI4uivEXT;
	RenderCommand_namedProgramLocalParametersI4uivEXT namedProgramLocalParametersI4uivEXT;
	RenderCommand_getNamedProgramLocalParameterIivEXT getNamedProgramLocalParameterIivEXT;
	RenderCommand_getNamedProgramLocalParameterIuivEXT getNamedProgramLocalParameterIuivEXT;
	RenderCommand_enableClientStateiEXT enableClientStateiEXT;
	RenderCommand_disableClientStateiEXT disableClientStateiEXT;
	RenderCommand_getFloati_vEXT getFloati_vEXT;
	RenderCommand_getDoublei_vEXT getDoublei_vEXT;
	RenderCommand_getPointeri_vEXT getPointeri_vEXT;
	RenderCommand_namedProgramStringEXT namedProgramStringEXT;
	RenderCommand_namedProgramLocalParameter4dEXT namedProgramLocalParameter4dEXT;
	RenderCommand_namedProgramLocalParameter4dvEXT namedProgramLocalParameter4dvEXT;
	RenderCommand_namedProgramLocalParameter4fEXT namedProgramLocalParameter4fEXT;
	RenderCommand_namedProgramLocalParameter4fvEXT namedProgramLocalParameter4fvEXT;
	RenderCommand_getNamedProgramLocalParameterdvEXT getNamedProgramLocalParameterdvEXT;
	RenderCommand_getNamedProgramLocalParameterfvEXT getNamedProgramLocalParameterfvEXT;
	RenderCommand_getNamedProgramivEXT getNamedProgramivEXT;
	RenderCommand_getNamedProgramStringEXT getNamedProgramStringEXT;
	RenderCommand_namedRenderbufferStorageEXT namedRenderbufferStorageEXT;
	RenderCommand_getNamedRenderbufferParameterivEXT getNamedRenderbufferParameterivEXT;
	RenderCommand_namedRenderbufferStorageMultisampleEXT namedRenderbufferStorageMultisampleEXT;
	RenderCommand_namedRenderbufferStorageMultisampleCoverageEXT namedRenderbufferStorageMultisampleCoverageEXT;
	RenderCommand_checkNamedFramebufferStatusEXT checkNamedFramebufferStatusEXT;
	RenderCommand_namedFramebufferTexture1DEXT namedFramebufferTexture1DEXT;
	RenderCommand_namedFramebufferTexture2DEXT namedFramebufferTexture2DEXT;
	RenderCommand_namedFramebufferTexture3DEXT namedFramebufferTexture3DEXT;
	RenderCommand_namedFramebufferRenderbufferEXT namedFramebufferRenderbufferEXT;
	RenderCommand_getNamedFramebufferAttachmentParameterivEXT getNamedFramebufferAttachmentParameterivEXT;
	RenderCommand_generateTextureMipmapEXT generateTextureMipmapEXT;
	RenderCommand_generateMultiTexMipmapEXT generateMultiTexMipmapEXT;
	RenderCommand_framebufferDrawBufferEXT framebufferDrawBufferEXT;
	RenderCommand_framebufferDrawBuffersEXT framebufferDrawBuffersEXT;
	RenderCommand_framebufferReadBufferEXT framebufferReadBufferEXT;
	RenderCommand_getFramebufferParameterivEXT getFramebufferParameterivEXT;
	RenderCommand_namedCopyBufferSubDataEXT namedCopyBufferSubDataEXT;
	RenderCommand_namedFramebufferTextureEXT namedFramebufferTextureEXT;
	RenderCommand_namedFramebufferTextureLayerEXT namedFramebufferTextureLayerEXT;
	RenderCommand_namedFramebufferTextureFaceEXT namedFramebufferTextureFaceEXT;
	RenderCommand_textureRenderbufferEXT textureRenderbufferEXT;
	RenderCommand_multiTexRenderbufferEXT multiTexRenderbufferEXT;
	RenderCommand_vertexArrayVertexOffsetEXT vertexArrayVertexOffsetEXT;
	RenderCommand_vertexArrayColorOffsetEXT vertexArrayColorOffsetEXT;
	RenderCommand_vertexArrayEdgeFlagOffsetEXT vertexArrayEdgeFlagOffsetEXT;
	RenderCommand_vertexArrayIndexOffsetEXT vertexArrayIndexOffsetEXT;
	RenderCommand_vertexArrayNormalOffsetEXT vertexArrayNormalOffsetEXT;
	RenderCommand_vertexArrayTexCoordOffsetEXT vertexArrayTexCoordOffsetEXT;
	RenderCommand_vertexArrayMultiTexCoordOffsetEXT vertexArrayMultiTexCoordOffsetEXT;
	RenderCommand_vertexArrayFogCoordOffsetEXT vertexArrayFogCoordOffsetEXT;
	RenderCommand_vertexArraySecondaryColorOffsetEXT vertexArraySecondaryColorOffsetEXT;
	RenderCommand_vertexArrayVertexAttribOffsetEXT vertexArrayVertexAttribOffsetEXT;
	RenderCommand_vertexArrayVertexAttribIOffsetEXT vertexArrayVertexAttribIOffsetEXT;
	RenderCommand_enableVertexArrayEXT enableVertexArrayEXT;
	RenderCommand_disableVertexArrayEXT disableVertexArrayEXT;
	RenderCommand_enableVertexArrayAttribEXT enableVertexArrayAttribEXT;
	RenderCommand_disableVertexArrayAttribEXT disableVertexArrayAttribEXT;
	RenderCommand_getVertexArrayIntegervEXT getVertexArrayIntegervEXT;
	RenderCommand_getVertexArrayPointervEXT getVertexArrayPointervEXT;
	RenderCommand_getVertexArrayIntegeri_vEXT getVertexArrayIntegeri_vEXT;
	RenderCommand_getVertexArrayPointeri_vEXT getVertexArrayPointeri_vEXT;
	RenderCommand_mapNamedBufferRangeEXT mapNamedBufferRangeEXT;
	RenderCommand_flushMappedNamedBufferRangeEXT flushMappedNamedBufferRangeEXT;
	RenderCommand_namedBufferStorageEXT namedBufferStorageEXT;
	RenderCommand_clearNamedBufferDataEXT clearNamedBufferDataEXT;
	RenderCommand_clearNamedBufferSubDataEXT clearNamedBufferSubDataEXT;
	RenderCommand_namedFramebufferParameteriEXT namedFramebufferParameteriEXT;
	RenderCommand_getNamedFramebufferParameterivEXT getNamedFramebufferParameterivEXT;
	RenderCommand_programUniform1dEXT programUniform1dEXT;
	RenderCommand_programUniform2dEXT programUniform2dEXT;
	RenderCommand_programUniform3dEXT programUniform3dEXT;
	RenderCommand_programUniform4dEXT programUniform4dEXT;
	RenderCommand_programUniform1dvEXT programUniform1dvEXT;
	RenderCommand_programUniform2dvEXT programUniform2dvEXT;
	RenderCommand_programUniform3dvEXT programUniform3dvEXT;
	RenderCommand_programUniform4dvEXT programUniform4dvEXT;
	RenderCommand_programUniformMatrix2dvEXT programUniformMatrix2dvEXT;
	RenderCommand_programUniformMatrix3dvEXT programUniformMatrix3dvEXT;
	RenderCommand_programUniformMatrix4dvEXT programUniformMatrix4dvEXT;
	RenderCommand_programUniformMatrix2x3dvEXT programUniformMatrix2x3dvEXT;
	RenderCommand_programUniformMatrix2x4dvEXT programUniformMatrix2x4dvEXT;
	RenderCommand_programUniformMatrix3x2dvEXT programUniformMatrix3x2dvEXT;
	RenderCommand_programUniformMatrix3x4dvEXT programUniformMatrix3x4dvEXT;
	RenderCommand_programUniformMatrix4x2dvEXT programUniformMatrix4x2dvEXT;
	RenderCommand_programUniformMatrix4x3dvEXT programUniformMatrix4x3dvEXT;
	RenderCommand_textureBufferRangeEXT textureBufferRangeEXT;
	RenderCommand_textureStorage1DEXT textureStorage1DEXT;
	RenderCommand_textureStorage2DEXT textureStorage2DEXT;
	RenderCommand_textureStorage3DEXT textureStorage3DEXT;
	RenderCommand_textureStorage2DMultisampleEXT textureStorage2DMultisampleEXT;
	RenderCommand_textureStorage3DMultisampleEXT textureStorage3DMultisampleEXT;
	RenderCommand_vertexArrayBindVertexBufferEXT vertexArrayBindVertexBufferEXT;
	RenderCommand_vertexArrayVertexAttribFormatEXT vertexArrayVertexAttribFormatEXT;
	RenderCommand_vertexArrayVertexAttribIFormatEXT vertexArrayVertexAttribIFormatEXT;
	RenderCommand_vertexArrayVertexAttribLFormatEXT vertexArrayVertexAttribLFormatEXT;
	RenderCommand_vertexArrayVertexAttribBindingEXT vertexArrayVertexAttribBindingEXT;
	RenderCommand_vertexArrayVertexBindingDivisorEXT vertexArrayVertexBindingDivisorEXT;
	RenderCommand_vertexArrayVertexAttribLOffsetEXT vertexArrayVertexAttribLOffsetEXT;
	RenderCommand_texturePageCommitmentEXT texturePageCommitmentEXT;
	RenderCommand_vertexArrayVertexAttribDivisorEXT vertexArrayVertexAttribDivisorEXT;
	RenderCommand_drawArraysInstancedEXT drawArraysInstancedEXT;
	RenderCommand_drawElementsInstancedEXT drawElementsInstancedEXT;
	RenderCommand_polygonOffsetClampEXT polygonOffsetClampEXT;
	RenderCommand_rasterSamplesEXT rasterSamplesEXT;
	RenderCommand_useShaderProgramEXT useShaderProgramEXT;
	RenderCommand_activeProgramEXT activeProgramEXT;
	RenderCommand_createShaderProgramEXT createShaderProgramEXT;
	RenderCommand_framebufferFetchBarrierEXT framebufferFetchBarrierEXT;
	RenderCommand_windowRectanglesEXT windowRectanglesEXT;
	RenderCommand_applyFramebufferAttachmentCMAAINTEL applyFramebufferAttachmentCMAAINTEL;
	RenderCommand_beginPerfQueryINTEL beginPerfQueryINTEL;
	RenderCommand_createPerfQueryINTEL createPerfQueryINTEL;
	RenderCommand_deletePerfQueryINTEL deletePerfQueryINTEL;
	RenderCommand_endPerfQueryINTEL endPerfQueryINTEL;
	RenderCommand_getFirstPerfQueryIdINTEL getFirstPerfQueryIdINTEL;
	RenderCommand_getNextPerfQueryIdINTEL getNextPerfQueryIdINTEL;
	RenderCommand_getPerfCounterInfoINTEL getPerfCounterInfoINTEL;
	RenderCommand_getPerfQueryDataINTEL getPerfQueryDataINTEL;
	RenderCommand_getPerfQueryIdByNameINTEL getPerfQueryIdByNameINTEL;
	RenderCommand_getPerfQueryInfoINTEL getPerfQueryInfoINTEL;
	RenderCommand_multiDrawArraysIndirectBindlessNV multiDrawArraysIndirectBindlessNV;
	RenderCommand_multiDrawElementsIndirectBindlessNV multiDrawElementsIndirectBindlessNV;
	RenderCommand_multiDrawArraysIndirectBindlessCountNV multiDrawArraysIndirectBindlessCountNV;
	RenderCommand_multiDrawElementsIndirectBindlessCountNV multiDrawElementsIndirectBindlessCountNV;
	RenderCommand_getTextureHandleNV getTextureHandleNV;
	RenderCommand_getTextureSamplerHandleNV getTextureSamplerHandleNV;
	RenderCommand_makeTextureHandleResidentNV makeTextureHandleResidentNV;
	RenderCommand_makeTextureHandleNonResidentNV makeTextureHandleNonResidentNV;
	RenderCommand_getImageHandleNV getImageHandleNV;
	RenderCommand_makeImageHandleResidentNV makeImageHandleResidentNV;
	RenderCommand_makeImageHandleNonResidentNV makeImageHandleNonResidentNV;
	RenderCommand_uniformHandleui64NV uniformHandleui64NV;
	RenderCommand_uniformHandleui64vNV uniformHandleui64vNV;
	RenderCommand_programUniformHandleui64NV programUniformHandleui64NV;
	RenderCommand_programUniformHandleui64vNV programUniformHandleui64vNV;
	RenderCommand_isTextureHandleResidentNV isTextureHandleResidentNV;
	RenderCommand_isImageHandleResidentNV isImageHandleResidentNV;
	RenderCommand_blendParameteriNV blendParameteriNV;
	RenderCommand_blendBarrierNV blendBarrierNV;
	RenderCommand_viewportPositionWScaleNV viewportPositionWScaleNV;
	RenderCommand_createStatesNV createStatesNV;
	RenderCommand_deleteStatesNV deleteStatesNV;
	RenderCommand_isStateNV isStateNV;
	RenderCommand_stateCaptureNV stateCaptureNV;
	RenderCommand_getCommandHeaderNV getCommandHeaderNV;
	RenderCommand_getStageIndexNV getStageIndexNV;
	RenderCommand_drawCommandsNV drawCommandsNV;
	RenderCommand_drawCommandsAddressNV drawCommandsAddressNV;
	RenderCommand_drawCommandsStatesNV drawCommandsStatesNV;
	RenderCommand_drawCommandsStatesAddressNV drawCommandsStatesAddressNV;
	RenderCommand_createCommandListsNV createCommandListsNV;
	RenderCommand_deleteCommandListsNV deleteCommandListsNV;
	RenderCommand_isCommandListNV isCommandListNV;
	RenderCommand_listDrawCommandsStatesClientNV listDrawCommandsStatesClientNV;
	RenderCommand_commandListSegmentsNV commandListSegmentsNV;
	RenderCommand_compileCommandListNV compileCommandListNV;
	RenderCommand_callCommandListNV callCommandListNV;
	RenderCommand_beginConditionalRenderNV beginConditionalRenderNV;
	RenderCommand_endConditionalRenderNV endConditionalRenderNV;
	RenderCommand_subpixelPrecisionBiasNV subpixelPrecisionBiasNV;
	RenderCommand_conservativeRasterParameterfNV conservativeRasterParameterfNV;
	RenderCommand_conservativeRasterParameteriNV conservativeRasterParameteriNV;
	RenderCommand_drawVkImageNV drawVkImageNV;
	RenderCommand_getVkProcAddrNV getVkProcAddrNV;
	RenderCommand_waitVkSemaphoreNV waitVkSemaphoreNV;
	RenderCommand_signalVkSemaphoreNV signalVkSemaphoreNV;
	RenderCommand_signalVkFenceNV signalVkFenceNV;
	RenderCommand_fragmentCoverageColorNV fragmentCoverageColorNV;
	RenderCommand_coverageModulationTableNV coverageModulationTableNV;
	RenderCommand_getCoverageModulationTableNV getCoverageModulationTableNV;
	RenderCommand_coverageModulationNV coverageModulationNV;
	RenderCommand_renderbufferStorageMultisampleCoverageNV renderbufferStorageMultisampleCoverageNV;
	RenderCommand_uniform1i64NV uniform1i64NV;
	RenderCommand_uniform2i64NV uniform2i64NV;
	RenderCommand_uniform3i64NV uniform3i64NV;
	RenderCommand_uniform4i64NV uniform4i64NV;
	RenderCommand_uniform1i64vNV uniform1i64vNV;
	RenderCommand_uniform2i64vNV uniform2i64vNV;
	RenderCommand_uniform3i64vNV uniform3i64vNV;
	RenderCommand_uniform4i64vNV uniform4i64vNV;
	RenderCommand_uniform1ui64NV uniform1ui64NV;
	RenderCommand_uniform2ui64NV uniform2ui64NV;
	RenderCommand_uniform3ui64NV uniform3ui64NV;
	RenderCommand_uniform4ui64NV uniform4ui64NV;
	RenderCommand_uniform1ui64vNV uniform1ui64vNV;
	RenderCommand_uniform2ui64vNV uniform2ui64vNV;
	RenderCommand_uniform3ui64vNV uniform3ui64vNV;
	RenderCommand_uniform4ui64vNV uniform4ui64vNV;
	RenderCommand_getUniformi64vNV getUniformi64vNV;
	RenderCommand_programUniform1i64NV programUniform1i64NV;
	RenderCommand_programUniform2i64NV programUniform2i64NV;
	RenderCommand_programUniform3i64NV programUniform3i64NV;
	RenderCommand_programUniform4i64NV programUniform4i64NV;
	RenderCommand_programUniform1i64vNV programUniform1i64vNV;
	RenderCommand_programUniform2i64vNV programUniform2i64vNV;
	RenderCommand_programUniform3i64vNV programUniform3i64vNV;
	RenderCommand_programUniform4i64vNV programUniform4i64vNV;
	RenderCommand_programUniform1ui64NV programUniform1ui64NV;
	RenderCommand_programUniform2ui64NV programUniform2ui64NV;
	RenderCommand_programUniform3ui64NV programUniform3ui64NV;
	RenderCommand_programUniform4ui64NV programUniform4ui64NV;
	RenderCommand_programUniform1ui64vNV programUniform1ui64vNV;
	RenderCommand_programUniform2ui64vNV programUniform2ui64vNV;
	RenderCommand_programUniform3ui64vNV programUniform3ui64vNV;
	RenderCommand_programUniform4ui64vNV programUniform4ui64vNV;
	RenderCommand_getInternalformatSampleivNV getInternalformatSampleivNV;
	RenderCommand_getMemoryObjectDetachedResourcesuivNV getMemoryObjectDetachedResourcesuivNV;
	RenderCommand_resetMemoryObjectParameterNV resetMemoryObjectParameterNV;
	RenderCommand_texAttachMemoryNV texAttachMemoryNV;
	RenderCommand_bufferAttachMemoryNV bufferAttachMemoryNV;
	RenderCommand_textureAttachMemoryNV textureAttachMemoryNV;
	RenderCommand_namedBufferAttachMemoryNV namedBufferAttachMemoryNV;
	RenderCommand_drawMeshTasksNV drawMeshTasksNV;
	RenderCommand_drawMeshTasksIndirectNV drawMeshTasksIndirectNV;
	RenderCommand_multiDrawMeshTasksIndirectNV multiDrawMeshTasksIndirectNV;
	RenderCommand_multiDrawMeshTasksIndirectCountNV multiDrawMeshTasksIndirectCountNV;
	RenderCommand_genPathsNV genPathsNV;
	RenderCommand_deletePathsNV deletePathsNV;
	RenderCommand_isPathNV isPathNV;
	RenderCommand_pathCommandsNV pathCommandsNV;
	RenderCommand_pathCoordsNV pathCoordsNV;
	RenderCommand_pathSubCommandsNV pathSubCommandsNV;
	RenderCommand_pathSubCoordsNV pathSubCoordsNV;
	RenderCommand_pathStringNV pathStringNV;
	RenderCommand_pathGlyphsNV pathGlyphsNV;
	RenderCommand_pathGlyphRangeNV pathGlyphRangeNV;
	RenderCommand_weightPathsNV weightPathsNV;
	RenderCommand_copyPathNV copyPathNV;
	RenderCommand_interpolatePathsNV interpolatePathsNV;
	RenderCommand_transformPathNV transformPathNV;
	RenderCommand_pathParameterivNV pathParameterivNV;
	RenderCommand_pathParameteriNV pathParameteriNV;
	RenderCommand_pathParameterfvNV pathParameterfvNV;
	RenderCommand_pathParameterfNV pathParameterfNV;
	RenderCommand_pathDashArrayNV pathDashArrayNV;
	RenderCommand_pathStencilFuncNV pathStencilFuncNV;
	RenderCommand_pathStencilDepthOffsetNV pathStencilDepthOffsetNV;
	RenderCommand_stencilFillPathNV stencilFillPathNV;
	RenderCommand_stencilStrokePathNV stencilStrokePathNV;
	RenderCommand_stencilFillPathInstancedNV stencilFillPathInstancedNV;
	RenderCommand_stencilStrokePathInstancedNV stencilStrokePathInstancedNV;
	RenderCommand_pathCoverDepthFuncNV pathCoverDepthFuncNV;
	RenderCommand_coverFillPathNV coverFillPathNV;
	RenderCommand_coverStrokePathNV coverStrokePathNV;
	RenderCommand_coverFillPathInstancedNV coverFillPathInstancedNV;
	RenderCommand_coverStrokePathInstancedNV coverStrokePathInstancedNV;
	RenderCommand_getPathParameterivNV getPathParameterivNV;
	RenderCommand_getPathParameterfvNV getPathParameterfvNV;
	RenderCommand_getPathCommandsNV getPathCommandsNV;
	RenderCommand_getPathCoordsNV getPathCoordsNV;
	RenderCommand_getPathDashArrayNV getPathDashArrayNV;
	RenderCommand_getPathMetricsNV getPathMetricsNV;
	RenderCommand_getPathMetricRangeNV getPathMetricRangeNV;
	RenderCommand_getPathSpacingNV getPathSpacingNV;
	RenderCommand_isPointInFillPathNV isPointInFillPathNV;
	RenderCommand_isPointInStrokePathNV isPointInStrokePathNV;
	RenderCommand_getPathLengthNV getPathLengthNV;
	RenderCommand_pointAlongPathNV pointAlongPathNV;
	RenderCommand_matrixLoad3x2fNV matrixLoad3x2fNV;
	RenderCommand_matrixLoad3x3fNV matrixLoad3x3fNV;
	RenderCommand_matrixLoadTranspose3x3fNV matrixLoadTranspose3x3fNV;
	RenderCommand_matrixMult3x2fNV matrixMult3x2fNV;
	RenderCommand_matrixMult3x3fNV matrixMult3x3fNV;
	RenderCommand_matrixMultTranspose3x3fNV matrixMultTranspose3x3fNV;
	RenderCommand_stencilThenCoverFillPathNV stencilThenCoverFillPathNV;
	RenderCommand_stencilThenCoverStrokePathNV stencilThenCoverStrokePathNV;
	RenderCommand_stencilThenCoverFillPathInstancedNV stencilThenCoverFillPathInstancedNV;
	RenderCommand_stencilThenCoverStrokePathInstancedNV stencilThenCoverStrokePathInstancedNV;
	RenderCommand_pathGlyphIndexRangeNV pathGlyphIndexRangeNV;
	RenderCommand_pathGlyphIndexArrayNV pathGlyphIndexArrayNV;
	RenderCommand_pathMemoryGlyphIndexArrayNV pathMemoryGlyphIndexArrayNV;
	RenderCommand_programPathFragmentInputGenNV programPathFragmentInputGenNV;
	RenderCommand_getProgramResourcefvNV getProgramResourcefvNV;
	RenderCommand_framebufferSampleLocationsfvNV framebufferSampleLocationsfvNV;
	RenderCommand_namedFramebufferSampleLocationsfvNV namedFramebufferSampleLocationsfvNV;
	RenderCommand_resolveDepthValuesNV resolveDepthValuesNV;
	RenderCommand_scissorExclusiveNV scissorExclusiveNV;
	RenderCommand_scissorExclusiveArrayvNV scissorExclusiveArrayvNV;
	RenderCommand_makeBufferResidentNV makeBufferResidentNV;
	RenderCommand_makeBufferNonResidentNV makeBufferNonResidentNV;
	RenderCommand_isBufferResidentNV isBufferResidentNV;
	RenderCommand_makeNamedBufferResidentNV makeNamedBufferResidentNV;
	RenderCommand_makeNamedBufferNonResidentNV makeNamedBufferNonResidentNV;
	RenderCommand_isNamedBufferResidentNV isNamedBufferResidentNV;
	RenderCommand_getBufferParameterui64vNV getBufferParameterui64vNV;
	RenderCommand_getNamedBufferParameterui64vNV getNamedBufferParameterui64vNV;
	RenderCommand_getIntegerui64vNV getIntegerui64vNV;
	RenderCommand_uniformui64NV uniformui64NV;
	RenderCommand_uniformui64vNV uniformui64vNV;
	RenderCommand_getUniformui64vNV getUniformui64vNV;
	RenderCommand_programUniformui64NV programUniformui64NV;
	RenderCommand_programUniformui64vNV programUniformui64vNV;
	RenderCommand_bindShadingRateImageNV bindShadingRateImageNV;
	RenderCommand_getShadingRateImagePaletteNV getShadingRateImagePaletteNV;
	RenderCommand_getShadingRateSampleLocationivNV getShadingRateSampleLocationivNV;
	RenderCommand_shadingRateImageBarrierNV shadingRateImageBarrierNV;
	RenderCommand_shadingRateImagePaletteNV shadingRateImagePaletteNV;
	RenderCommand_shadingRateSampleOrderNV shadingRateSampleOrderNV;
	RenderCommand_shadingRateSampleOrderCustomNV shadingRateSampleOrderCustomNV;
	RenderCommand_textureBarrierNV textureBarrierNV;
	RenderCommand_vertexAttribL1i64NV vertexAttribL1i64NV;
	RenderCommand_vertexAttribL2i64NV vertexAttribL2i64NV;
	RenderCommand_vertexAttribL3i64NV vertexAttribL3i64NV;
	RenderCommand_vertexAttribL4i64NV vertexAttribL4i64NV;
	RenderCommand_vertexAttribL1i64vNV vertexAttribL1i64vNV;
	RenderCommand_vertexAttribL2i64vNV vertexAttribL2i64vNV;
	RenderCommand_vertexAttribL3i64vNV vertexAttribL3i64vNV;
	RenderCommand_vertexAttribL4i64vNV vertexAttribL4i64vNV;
	RenderCommand_vertexAttribL1ui64NV vertexAttribL1ui64NV;
	RenderCommand_vertexAttribL2ui64NV vertexAttribL2ui64NV;
	RenderCommand_vertexAttribL3ui64NV vertexAttribL3ui64NV;
	RenderCommand_vertexAttribL4ui64NV vertexAttribL4ui64NV;
	RenderCommand_vertexAttribL1ui64vNV vertexAttribL1ui64vNV;
	RenderCommand_vertexAttribL2ui64vNV vertexAttribL2ui64vNV;
	RenderCommand_vertexAttribL3ui64vNV vertexAttribL3ui64vNV;
	RenderCommand_vertexAttribL4ui64vNV vertexAttribL4ui64vNV;
	RenderCommand_getVertexAttribLi64vNV getVertexAttribLi64vNV;
	RenderCommand_getVertexAttribLui64vNV getVertexAttribLui64vNV;
	RenderCommand_vertexAttribLFormatNV vertexAttribLFormatNV;
	RenderCommand_bufferAddressRangeNV bufferAddressRangeNV;
	RenderCommand_vertexFormatNV vertexFormatNV;
	RenderCommand_normalFormatNV normalFormatNV;
	RenderCommand_colorFormatNV colorFormatNV;
	RenderCommand_indexFormatNV indexFormatNV;
	RenderCommand_texCoordFormatNV texCoordFormatNV;
	RenderCommand_edgeFlagFormatNV edgeFlagFormatNV;
	RenderCommand_secondaryColorFormatNV secondaryColorFormatNV;
	RenderCommand_fogCoordFormatNV fogCoordFormatNV;
	RenderCommand_vertexAttribFormatNV vertexAttribFormatNV;
	RenderCommand_vertexAttribIFormatNV vertexAttribIFormatNV;
	RenderCommand_getIntegerui64i_vNV getIntegerui64i_vNV;
	RenderCommand_viewportSwizzleNV viewportSwizzleNV;
	RenderCommand_framebufferTextureMultiviewOVR framebufferTextureMultiviewOVR;

};
