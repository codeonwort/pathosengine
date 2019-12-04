void cullFace(
	GLenum mode)
{
	RenderCommand_cullFace* __restrict packet = (RenderCommand_cullFace*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_cullFace::execute);
	packet->mode = mode;
}
void frontFace(
	GLenum mode)
{
	RenderCommand_frontFace* __restrict packet = (RenderCommand_frontFace*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_frontFace::execute);
	packet->mode = mode;
}
void hint(
	GLenum target,
	GLenum mode)
{
	RenderCommand_hint* __restrict packet = (RenderCommand_hint*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_hint::execute);
	packet->target = target;
	packet->mode = mode;
}
void lineWidth(
	GLfloat width)
{
	RenderCommand_lineWidth* __restrict packet = (RenderCommand_lineWidth*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_lineWidth::execute);
	packet->width = width;
}
void pointSize(
	GLfloat size)
{
	RenderCommand_pointSize* __restrict packet = (RenderCommand_pointSize*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pointSize::execute);
	packet->size = size;
}
void polygonMode(
	GLenum face,
	GLenum mode)
{
	RenderCommand_polygonMode* __restrict packet = (RenderCommand_polygonMode*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_polygonMode::execute);
	packet->face = face;
	packet->mode = mode;
}
void scissor(
	GLint x,
	GLint y,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_scissor* __restrict packet = (RenderCommand_scissor*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_scissor::execute);
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->height = height;
}
void texParameterf(
	GLenum target,
	GLenum pname,
	GLfloat param)
{
	RenderCommand_texParameterf* __restrict packet = (RenderCommand_texParameterf*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texParameterf::execute);
	packet->target = target;
	packet->pname = pname;
	packet->param = param;
}
void texParameterfv(
	GLenum target,
	GLenum pname,
	const GLfloat *params)
{
	RenderCommand_texParameterfv* __restrict packet = (RenderCommand_texParameterfv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texParameterfv::execute);
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void texParameteri(
	GLenum target,
	GLenum pname,
	GLint param)
{
	RenderCommand_texParameteri* __restrict packet = (RenderCommand_texParameteri*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texParameteri::execute);
	packet->target = target;
	packet->pname = pname;
	packet->param = param;
}
void texParameteriv(
	GLenum target,
	GLenum pname,
	const GLint *params)
{
	RenderCommand_texParameteriv* __restrict packet = (RenderCommand_texParameteriv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texParameteriv::execute);
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void texImage1D(
	GLenum target,
	GLint level,
	GLint internalformat,
	GLsizei width,
	GLint border,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_texImage1D* __restrict packet = (RenderCommand_texImage1D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texImage1D::execute);
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->border = border;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void texImage2D(
	GLenum target,
	GLint level,
	GLint internalformat,
	GLsizei width,
	GLsizei height,
	GLint border,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_texImage2D* __restrict packet = (RenderCommand_texImage2D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texImage2D::execute);
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->border = border;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void drawBuffer(
	GLenum buf)
{
	RenderCommand_drawBuffer* __restrict packet = (RenderCommand_drawBuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawBuffer::execute);
	packet->buf = buf;
}
void clear(
	GLbitfield mask)
{
	RenderCommand_clear* __restrict packet = (RenderCommand_clear*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clear::execute);
	packet->mask = mask;
}
void clearColor(
	GLfloat red,
	GLfloat green,
	GLfloat blue,
	GLfloat alpha)
{
	RenderCommand_clearColor* __restrict packet = (RenderCommand_clearColor*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clearColor::execute);
	packet->red = red;
	packet->green = green;
	packet->blue = blue;
	packet->alpha = alpha;
}
void clearStencil(
	GLint s)
{
	RenderCommand_clearStencil* __restrict packet = (RenderCommand_clearStencil*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clearStencil::execute);
	packet->s = s;
}
void clearDepth(
	GLdouble depth)
{
	RenderCommand_clearDepth* __restrict packet = (RenderCommand_clearDepth*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clearDepth::execute);
	packet->depth = depth;
}
void stencilMask(
	GLuint mask)
{
	RenderCommand_stencilMask* __restrict packet = (RenderCommand_stencilMask*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_stencilMask::execute);
	packet->mask = mask;
}
void colorMask(
	GLboolean red,
	GLboolean green,
	GLboolean blue,
	GLboolean alpha)
{
	RenderCommand_colorMask* __restrict packet = (RenderCommand_colorMask*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_colorMask::execute);
	packet->red = red;
	packet->green = green;
	packet->blue = blue;
	packet->alpha = alpha;
}
void depthMask(
	GLboolean flag)
{
	RenderCommand_depthMask* __restrict packet = (RenderCommand_depthMask*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_depthMask::execute);
	packet->flag = flag;
}
void disable(
	GLenum cap)
{
	RenderCommand_disable* __restrict packet = (RenderCommand_disable*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_disable::execute);
	packet->cap = cap;
}
void enable(
	GLenum cap)
{
	RenderCommand_enable* __restrict packet = (RenderCommand_enable*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_enable::execute);
	packet->cap = cap;
}
void finish(
	void)
{
	RenderCommand_finish* __restrict packet = (RenderCommand_finish*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_finish::execute);
}
void flush(
	void)
{
	RenderCommand_flush* __restrict packet = (RenderCommand_flush*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_flush::execute);
}
void blendFunc(
	GLenum sfactor,
	GLenum dfactor)
{
	RenderCommand_blendFunc* __restrict packet = (RenderCommand_blendFunc*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_blendFunc::execute);
	packet->sfactor = sfactor;
	packet->dfactor = dfactor;
}
void logicOp(
	GLenum opcode)
{
	RenderCommand_logicOp* __restrict packet = (RenderCommand_logicOp*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_logicOp::execute);
	packet->opcode = opcode;
}
void stencilFunc(
	GLenum func,
	GLint ref,
	GLuint mask)
{
	RenderCommand_stencilFunc* __restrict packet = (RenderCommand_stencilFunc*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_stencilFunc::execute);
	packet->func = func;
	packet->ref = ref;
	packet->mask = mask;
}
void stencilOp(
	GLenum fail,
	GLenum zfail,
	GLenum zpass)
{
	RenderCommand_stencilOp* __restrict packet = (RenderCommand_stencilOp*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_stencilOp::execute);
	packet->fail = fail;
	packet->zfail = zfail;
	packet->zpass = zpass;
}
void depthFunc(
	GLenum func)
{
	RenderCommand_depthFunc* __restrict packet = (RenderCommand_depthFunc*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_depthFunc::execute);
	packet->func = func;
}
void pixelStoref(
	GLenum pname,
	GLfloat param)
{
	RenderCommand_pixelStoref* __restrict packet = (RenderCommand_pixelStoref*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pixelStoref::execute);
	packet->pname = pname;
	packet->param = param;
}
void pixelStorei(
	GLenum pname,
	GLint param)
{
	RenderCommand_pixelStorei* __restrict packet = (RenderCommand_pixelStorei*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pixelStorei::execute);
	packet->pname = pname;
	packet->param = param;
}
void readBuffer(
	GLenum src)
{
	RenderCommand_readBuffer* __restrict packet = (RenderCommand_readBuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_readBuffer::execute);
	packet->src = src;
}
void readPixels(
	GLint x,
	GLint y,
	GLsizei width,
	GLsizei height,
	GLenum format,
	GLenum type,
	void *pixels)
{
	RenderCommand_readPixels* __restrict packet = (RenderCommand_readPixels*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_readPixels::execute);
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->height = height;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void getBooleanv(
	GLenum pname,
	GLboolean *data)
{
	RenderCommand_getBooleanv* __restrict packet = (RenderCommand_getBooleanv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getBooleanv::execute);
	packet->pname = pname;
	packet->data = data;
}
void getDoublev(
	GLenum pname,
	GLdouble *data)
{
	RenderCommand_getDoublev* __restrict packet = (RenderCommand_getDoublev*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getDoublev::execute);
	packet->pname = pname;
	packet->data = data;
}
GLenum getError(
	void)
{
	RenderCommand_getError* __restrict packet = (RenderCommand_getError*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getError::execute);
}
void getFloatv(
	GLenum pname,
	GLfloat *data)
{
	RenderCommand_getFloatv* __restrict packet = (RenderCommand_getFloatv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getFloatv::execute);
	packet->pname = pname;
	packet->data = data;
}
void getIntegerv(
	GLenum pname,
	GLint *data)
{
	RenderCommand_getIntegerv* __restrict packet = (RenderCommand_getIntegerv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getIntegerv::execute);
	packet->pname = pname;
	packet->data = data;
}
const GLubyte* getString(
	GLenum name)
{
	RenderCommand_getString* __restrict packet = (RenderCommand_getString*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getString::execute);
	packet->name = name;
}
void getTexImage(
	GLenum target,
	GLint level,
	GLenum format,
	GLenum type,
	void *pixels)
{
	RenderCommand_getTexImage* __restrict packet = (RenderCommand_getTexImage*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTexImage::execute);
	packet->target = target;
	packet->level = level;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void getTexParameterfv(
	GLenum target,
	GLenum pname,
	GLfloat *params)
{
	RenderCommand_getTexParameterfv* __restrict packet = (RenderCommand_getTexParameterfv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTexParameterfv::execute);
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void getTexParameteriv(
	GLenum target,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getTexParameteriv* __restrict packet = (RenderCommand_getTexParameteriv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTexParameteriv::execute);
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void getTexLevelParameterfv(
	GLenum target,
	GLint level,
	GLenum pname,
	GLfloat *params)
{
	RenderCommand_getTexLevelParameterfv* __restrict packet = (RenderCommand_getTexLevelParameterfv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTexLevelParameterfv::execute);
	packet->target = target;
	packet->level = level;
	packet->pname = pname;
	packet->params = params;
}
void getTexLevelParameteriv(
	GLenum target,
	GLint level,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getTexLevelParameteriv* __restrict packet = (RenderCommand_getTexLevelParameteriv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTexLevelParameteriv::execute);
	packet->target = target;
	packet->level = level;
	packet->pname = pname;
	packet->params = params;
}
GLboolean isEnabled(
	GLenum cap)
{
	RenderCommand_isEnabled* __restrict packet = (RenderCommand_isEnabled*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isEnabled::execute);
	packet->cap = cap;
}
void depthRange(
	GLdouble n,
	GLdouble f)
{
	RenderCommand_depthRange* __restrict packet = (RenderCommand_depthRange*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_depthRange::execute);
	packet->n = n;
	packet->f = f;
}
void viewport(
	GLint x,
	GLint y,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_viewport* __restrict packet = (RenderCommand_viewport*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_viewport::execute);
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->height = height;
}
void drawArrays(
	GLenum mode,
	GLint first,
	GLsizei count)
{
	RenderCommand_drawArrays* __restrict packet = (RenderCommand_drawArrays*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawArrays::execute);
	packet->mode = mode;
	packet->first = first;
	packet->count = count;
}
void drawElements(
	GLenum mode,
	GLsizei count,
	GLenum type,
	const void *indices)
{
	RenderCommand_drawElements* __restrict packet = (RenderCommand_drawElements*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawElements::execute);
	packet->mode = mode;
	packet->count = count;
	packet->type = type;
	packet->indices = indices;
}
void getPointerv(
	GLenum pname,
	void **params)
{
	RenderCommand_getPointerv* __restrict packet = (RenderCommand_getPointerv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPointerv::execute);
	packet->pname = pname;
	packet->params = params;
}
void polygonOffset(
	GLfloat factor,
	GLfloat units)
{
	RenderCommand_polygonOffset* __restrict packet = (RenderCommand_polygonOffset*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_polygonOffset::execute);
	packet->factor = factor;
	packet->units = units;
}
void copyTexImage1D(
	GLenum target,
	GLint level,
	GLenum internalformat,
	GLint x,
	GLint y,
	GLsizei width,
	GLint border)
{
	RenderCommand_copyTexImage1D* __restrict packet = (RenderCommand_copyTexImage1D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyTexImage1D::execute);
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->border = border;
}
void copyTexImage2D(
	GLenum target,
	GLint level,
	GLenum internalformat,
	GLint x,
	GLint y,
	GLsizei width,
	GLsizei height,
	GLint border)
{
	RenderCommand_copyTexImage2D* __restrict packet = (RenderCommand_copyTexImage2D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyTexImage2D::execute);
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->height = height;
	packet->border = border;
}
void copyTexSubImage1D(
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint x,
	GLint y,
	GLsizei width)
{
	RenderCommand_copyTexSubImage1D* __restrict packet = (RenderCommand_copyTexSubImage1D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyTexSubImage1D::execute);
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->x = x;
	packet->y = y;
	packet->width = width;
}
void copyTexSubImage2D(
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint x,
	GLint y,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_copyTexSubImage2D* __restrict packet = (RenderCommand_copyTexSubImage2D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyTexSubImage2D::execute);
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->height = height;
}
void texSubImage1D(
	GLenum target,
	GLint level,
	GLint xoffset,
	GLsizei width,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_texSubImage1D* __restrict packet = (RenderCommand_texSubImage1D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texSubImage1D::execute);
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->width = width;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void texSubImage2D(
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLsizei width,
	GLsizei height,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_texSubImage2D* __restrict packet = (RenderCommand_texSubImage2D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texSubImage2D::execute);
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->width = width;
	packet->height = height;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void bindTexture(
	GLenum target,
	GLuint texture)
{
	RenderCommand_bindTexture* __restrict packet = (RenderCommand_bindTexture*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindTexture::execute);
	packet->target = target;
	packet->texture = texture;
}
void deleteTextures(
	GLsizei n,
	const GLuint *textures)
{
	RenderCommand_deleteTextures* __restrict packet = (RenderCommand_deleteTextures*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_deleteTextures::execute);
	packet->n = n;
	packet->textures = textures;
}
void genTextures(
	GLsizei n,
	GLuint *textures)
{
	RenderCommand_genTextures* __restrict packet = (RenderCommand_genTextures*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_genTextures::execute);
	packet->n = n;
	packet->textures = textures;
}
GLboolean isTexture(
	GLuint texture)
{
	RenderCommand_isTexture* __restrict packet = (RenderCommand_isTexture*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isTexture::execute);
	packet->texture = texture;
}
void drawRangeElements(
	GLenum mode,
	GLuint start,
	GLuint end,
	GLsizei count,
	GLenum type,
	const void *indices)
{
	RenderCommand_drawRangeElements* __restrict packet = (RenderCommand_drawRangeElements*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawRangeElements::execute);
	packet->mode = mode;
	packet->start = start;
	packet->end = end;
	packet->count = count;
	packet->type = type;
	packet->indices = indices;
}
void texImage3D(
	GLenum target,
	GLint level,
	GLint internalformat,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLint border,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_texImage3D* __restrict packet = (RenderCommand_texImage3D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texImage3D::execute);
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->border = border;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void texSubImage3D(
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint zoffset,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_texSubImage3D* __restrict packet = (RenderCommand_texSubImage3D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texSubImage3D::execute);
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->zoffset = zoffset;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void copyTexSubImage3D(
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint zoffset,
	GLint x,
	GLint y,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_copyTexSubImage3D* __restrict packet = (RenderCommand_copyTexSubImage3D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyTexSubImage3D::execute);
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->zoffset = zoffset;
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->height = height;
}
void activeTexture(
	GLenum texture)
{
	RenderCommand_activeTexture* __restrict packet = (RenderCommand_activeTexture*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_activeTexture::execute);
	packet->texture = texture;
}
void sampleCoverage(
	GLfloat value,
	GLboolean invert)
{
	RenderCommand_sampleCoverage* __restrict packet = (RenderCommand_sampleCoverage*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_sampleCoverage::execute);
	packet->value = value;
	packet->invert = invert;
}
void compressedTexImage3D(
	GLenum target,
	GLint level,
	GLenum internalformat,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLint border,
	GLsizei imageSize,
	const void *data)
{
	RenderCommand_compressedTexImage3D* __restrict packet = (RenderCommand_compressedTexImage3D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedTexImage3D::execute);
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->border = border;
	packet->imageSize = imageSize;
	packet->data = data;
}
void compressedTexImage2D(
	GLenum target,
	GLint level,
	GLenum internalformat,
	GLsizei width,
	GLsizei height,
	GLint border,
	GLsizei imageSize,
	const void *data)
{
	RenderCommand_compressedTexImage2D* __restrict packet = (RenderCommand_compressedTexImage2D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedTexImage2D::execute);
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->border = border;
	packet->imageSize = imageSize;
	packet->data = data;
}
void compressedTexImage1D(
	GLenum target,
	GLint level,
	GLenum internalformat,
	GLsizei width,
	GLint border,
	GLsizei imageSize,
	const void *data)
{
	RenderCommand_compressedTexImage1D* __restrict packet = (RenderCommand_compressedTexImage1D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedTexImage1D::execute);
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->border = border;
	packet->imageSize = imageSize;
	packet->data = data;
}
void compressedTexSubImage3D(
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint zoffset,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLenum format,
	GLsizei imageSize,
	const void *data)
{
	RenderCommand_compressedTexSubImage3D* __restrict packet = (RenderCommand_compressedTexSubImage3D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedTexSubImage3D::execute);
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->zoffset = zoffset;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->format = format;
	packet->imageSize = imageSize;
	packet->data = data;
}
void compressedTexSubImage2D(
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLsizei width,
	GLsizei height,
	GLenum format,
	GLsizei imageSize,
	const void *data)
{
	RenderCommand_compressedTexSubImage2D* __restrict packet = (RenderCommand_compressedTexSubImage2D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedTexSubImage2D::execute);
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->width = width;
	packet->height = height;
	packet->format = format;
	packet->imageSize = imageSize;
	packet->data = data;
}
void compressedTexSubImage1D(
	GLenum target,
	GLint level,
	GLint xoffset,
	GLsizei width,
	GLenum format,
	GLsizei imageSize,
	const void *data)
{
	RenderCommand_compressedTexSubImage1D* __restrict packet = (RenderCommand_compressedTexSubImage1D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedTexSubImage1D::execute);
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->width = width;
	packet->format = format;
	packet->imageSize = imageSize;
	packet->data = data;
}
void getCompressedTexImage(
	GLenum target,
	GLint level,
	void *img)
{
	RenderCommand_getCompressedTexImage* __restrict packet = (RenderCommand_getCompressedTexImage*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getCompressedTexImage::execute);
	packet->target = target;
	packet->level = level;
	packet->img = img;
}
void blendFuncSeparate(
	GLenum sfactorRGB,
	GLenum dfactorRGB,
	GLenum sfactorAlpha,
	GLenum dfactorAlpha)
{
	RenderCommand_blendFuncSeparate* __restrict packet = (RenderCommand_blendFuncSeparate*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_blendFuncSeparate::execute);
	packet->sfactorRGB = sfactorRGB;
	packet->dfactorRGB = dfactorRGB;
	packet->sfactorAlpha = sfactorAlpha;
	packet->dfactorAlpha = dfactorAlpha;
}
void multiDrawArrays(
	GLenum mode,
	const GLint *first,
	const GLsizei *count,
	GLsizei drawcount)
{
	RenderCommand_multiDrawArrays* __restrict packet = (RenderCommand_multiDrawArrays*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiDrawArrays::execute);
	packet->mode = mode;
	packet->first = first;
	packet->count = count;
	packet->drawcount = drawcount;
}
void multiDrawElements(
	GLenum mode,
	const GLsizei *count,
	GLenum type,
	const void *const*indices,
	GLsizei drawcount)
{
	RenderCommand_multiDrawElements* __restrict packet = (RenderCommand_multiDrawElements*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiDrawElements::execute);
	packet->mode = mode;
	packet->count = count;
	packet->type = type;
	packet->indices = indices;
	packet->drawcount = drawcount;
}
void pointParameterf(
	GLenum pname,
	GLfloat param)
{
	RenderCommand_pointParameterf* __restrict packet = (RenderCommand_pointParameterf*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pointParameterf::execute);
	packet->pname = pname;
	packet->param = param;
}
void pointParameterfv(
	GLenum pname,
	const GLfloat *params)
{
	RenderCommand_pointParameterfv* __restrict packet = (RenderCommand_pointParameterfv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pointParameterfv::execute);
	packet->pname = pname;
	packet->params = params;
}
void pointParameteri(
	GLenum pname,
	GLint param)
{
	RenderCommand_pointParameteri* __restrict packet = (RenderCommand_pointParameteri*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pointParameteri::execute);
	packet->pname = pname;
	packet->param = param;
}
void pointParameteriv(
	GLenum pname,
	const GLint *params)
{
	RenderCommand_pointParameteriv* __restrict packet = (RenderCommand_pointParameteriv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pointParameteriv::execute);
	packet->pname = pname;
	packet->params = params;
}
void blendColor(
	GLfloat red,
	GLfloat green,
	GLfloat blue,
	GLfloat alpha)
{
	RenderCommand_blendColor* __restrict packet = (RenderCommand_blendColor*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_blendColor::execute);
	packet->red = red;
	packet->green = green;
	packet->blue = blue;
	packet->alpha = alpha;
}
void blendEquation(
	GLenum mode)
{
	RenderCommand_blendEquation* __restrict packet = (RenderCommand_blendEquation*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_blendEquation::execute);
	packet->mode = mode;
}
void genQueries(
	GLsizei n,
	GLuint *ids)
{
	RenderCommand_genQueries* __restrict packet = (RenderCommand_genQueries*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_genQueries::execute);
	packet->n = n;
	packet->ids = ids;
}
void deleteQueries(
	GLsizei n,
	const GLuint *ids)
{
	RenderCommand_deleteQueries* __restrict packet = (RenderCommand_deleteQueries*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_deleteQueries::execute);
	packet->n = n;
	packet->ids = ids;
}
GLboolean isQuery(
	GLuint id)
{
	RenderCommand_isQuery* __restrict packet = (RenderCommand_isQuery*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isQuery::execute);
	packet->id = id;
}
void beginQuery(
	GLenum target,
	GLuint id)
{
	RenderCommand_beginQuery* __restrict packet = (RenderCommand_beginQuery*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_beginQuery::execute);
	packet->target = target;
	packet->id = id;
}
void endQuery(
	GLenum target)
{
	RenderCommand_endQuery* __restrict packet = (RenderCommand_endQuery*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_endQuery::execute);
	packet->target = target;
}
void getQueryiv(
	GLenum target,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getQueryiv* __restrict packet = (RenderCommand_getQueryiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getQueryiv::execute);
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void getQueryObjectiv(
	GLuint id,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getQueryObjectiv* __restrict packet = (RenderCommand_getQueryObjectiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getQueryObjectiv::execute);
	packet->id = id;
	packet->pname = pname;
	packet->params = params;
}
void getQueryObjectuiv(
	GLuint id,
	GLenum pname,
	GLuint *params)
{
	RenderCommand_getQueryObjectuiv* __restrict packet = (RenderCommand_getQueryObjectuiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getQueryObjectuiv::execute);
	packet->id = id;
	packet->pname = pname;
	packet->params = params;
}
void bindBuffer(
	GLenum target,
	GLuint buffer)
{
	RenderCommand_bindBuffer* __restrict packet = (RenderCommand_bindBuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindBuffer::execute);
	packet->target = target;
	packet->buffer = buffer;
}
void deleteBuffers(
	GLsizei n,
	const GLuint *buffers)
{
	RenderCommand_deleteBuffers* __restrict packet = (RenderCommand_deleteBuffers*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_deleteBuffers::execute);
	packet->n = n;
	packet->buffers = buffers;
}
void genBuffers(
	GLsizei n,
	GLuint *buffers)
{
	RenderCommand_genBuffers* __restrict packet = (RenderCommand_genBuffers*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_genBuffers::execute);
	packet->n = n;
	packet->buffers = buffers;
}
GLboolean isBuffer(
	GLuint buffer)
{
	RenderCommand_isBuffer* __restrict packet = (RenderCommand_isBuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isBuffer::execute);
	packet->buffer = buffer;
}
void bufferData(
	GLenum target,
	GLsizeiptr size,
	const void *data,
	GLenum usage)
{
	RenderCommand_bufferData* __restrict packet = (RenderCommand_bufferData*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bufferData::execute);
	packet->target = target;
	packet->size = size;
	packet->data = data;
	packet->usage = usage;
}
void bufferSubData(
	GLenum target,
	GLintptr offset,
	GLsizeiptr size,
	const void *data)
{
	RenderCommand_bufferSubData* __restrict packet = (RenderCommand_bufferSubData*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bufferSubData::execute);
	packet->target = target;
	packet->offset = offset;
	packet->size = size;
	packet->data = data;
}
void getBufferSubData(
	GLenum target,
	GLintptr offset,
	GLsizeiptr size,
	void *data)
{
	RenderCommand_getBufferSubData* __restrict packet = (RenderCommand_getBufferSubData*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getBufferSubData::execute);
	packet->target = target;
	packet->offset = offset;
	packet->size = size;
	packet->data = data;
}
void* mapBuffer(
	GLenum target,
	GLenum access)
{
	RenderCommand_mapBuffer* __restrict packet = (RenderCommand_mapBuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_mapBuffer::execute);
	packet->target = target;
	packet->access = access;
}
GLboolean unmapBuffer(
	GLenum target)
{
	RenderCommand_unmapBuffer* __restrict packet = (RenderCommand_unmapBuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_unmapBuffer::execute);
	packet->target = target;
}
void getBufferParameteriv(
	GLenum target,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getBufferParameteriv* __restrict packet = (RenderCommand_getBufferParameteriv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getBufferParameteriv::execute);
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void getBufferPointerv(
	GLenum target,
	GLenum pname,
	void **params)
{
	RenderCommand_getBufferPointerv* __restrict packet = (RenderCommand_getBufferPointerv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getBufferPointerv::execute);
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void blendEquationSeparate(
	GLenum modeRGB,
	GLenum modeAlpha)
{
	RenderCommand_blendEquationSeparate* __restrict packet = (RenderCommand_blendEquationSeparate*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_blendEquationSeparate::execute);
	packet->modeRGB = modeRGB;
	packet->modeAlpha = modeAlpha;
}
void drawBuffers(
	GLsizei n,
	const GLenum *bufs)
{
	RenderCommand_drawBuffers* __restrict packet = (RenderCommand_drawBuffers*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawBuffers::execute);
	packet->n = n;
	packet->bufs = bufs;
}
void stencilOpSeparate(
	GLenum face,
	GLenum sfail,
	GLenum dpfail,
	GLenum dppass)
{
	RenderCommand_stencilOpSeparate* __restrict packet = (RenderCommand_stencilOpSeparate*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_stencilOpSeparate::execute);
	packet->face = face;
	packet->sfail = sfail;
	packet->dpfail = dpfail;
	packet->dppass = dppass;
}
void stencilFuncSeparate(
	GLenum face,
	GLenum func,
	GLint ref,
	GLuint mask)
{
	RenderCommand_stencilFuncSeparate* __restrict packet = (RenderCommand_stencilFuncSeparate*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_stencilFuncSeparate::execute);
	packet->face = face;
	packet->func = func;
	packet->ref = ref;
	packet->mask = mask;
}
void stencilMaskSeparate(
	GLenum face,
	GLuint mask)
{
	RenderCommand_stencilMaskSeparate* __restrict packet = (RenderCommand_stencilMaskSeparate*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_stencilMaskSeparate::execute);
	packet->face = face;
	packet->mask = mask;
}
void attachShader(
	GLuint program,
	GLuint shader)
{
	RenderCommand_attachShader* __restrict packet = (RenderCommand_attachShader*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_attachShader::execute);
	packet->program = program;
	packet->shader = shader;
}
void bindAttribLocation(
	GLuint program,
	GLuint index,
	const GLchar *name)
{
	RenderCommand_bindAttribLocation* __restrict packet = (RenderCommand_bindAttribLocation*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindAttribLocation::execute);
	packet->program = program;
	packet->index = index;
	packet->name = name;
}
void compileShader(
	GLuint shader)
{
	RenderCommand_compileShader* __restrict packet = (RenderCommand_compileShader*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compileShader::execute);
	packet->shader = shader;
}
GLuint createProgram(
	void)
{
	RenderCommand_createProgram* __restrict packet = (RenderCommand_createProgram*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_createProgram::execute);
}
GLuint createShader(
	GLenum type)
{
	RenderCommand_createShader* __restrict packet = (RenderCommand_createShader*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_createShader::execute);
	packet->type = type;
}
void deleteProgram(
	GLuint program)
{
	RenderCommand_deleteProgram* __restrict packet = (RenderCommand_deleteProgram*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_deleteProgram::execute);
	packet->program = program;
}
void deleteShader(
	GLuint shader)
{
	RenderCommand_deleteShader* __restrict packet = (RenderCommand_deleteShader*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_deleteShader::execute);
	packet->shader = shader;
}
void detachShader(
	GLuint program,
	GLuint shader)
{
	RenderCommand_detachShader* __restrict packet = (RenderCommand_detachShader*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_detachShader::execute);
	packet->program = program;
	packet->shader = shader;
}
void disableVertexAttribArray(
	GLuint index)
{
	RenderCommand_disableVertexAttribArray* __restrict packet = (RenderCommand_disableVertexAttribArray*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_disableVertexAttribArray::execute);
	packet->index = index;
}
void enableVertexAttribArray(
	GLuint index)
{
	RenderCommand_enableVertexAttribArray* __restrict packet = (RenderCommand_enableVertexAttribArray*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_enableVertexAttribArray::execute);
	packet->index = index;
}
void getActiveAttrib(
	GLuint program,
	GLuint index,
	GLsizei bufSize,
	GLsizei *length,
	GLint *size,
	GLenum *type,
	GLchar *name)
{
	RenderCommand_getActiveAttrib* __restrict packet = (RenderCommand_getActiveAttrib*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getActiveAttrib::execute);
	packet->program = program;
	packet->index = index;
	packet->bufSize = bufSize;
	packet->length = length;
	packet->size = size;
	packet->type = type;
	packet->name = name;
}
void getActiveUniform(
	GLuint program,
	GLuint index,
	GLsizei bufSize,
	GLsizei *length,
	GLint *size,
	GLenum *type,
	GLchar *name)
{
	RenderCommand_getActiveUniform* __restrict packet = (RenderCommand_getActiveUniform*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getActiveUniform::execute);
	packet->program = program;
	packet->index = index;
	packet->bufSize = bufSize;
	packet->length = length;
	packet->size = size;
	packet->type = type;
	packet->name = name;
}
void getAttachedShaders(
	GLuint program,
	GLsizei maxCount,
	GLsizei *count,
	GLuint *shaders)
{
	RenderCommand_getAttachedShaders* __restrict packet = (RenderCommand_getAttachedShaders*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getAttachedShaders::execute);
	packet->program = program;
	packet->maxCount = maxCount;
	packet->count = count;
	packet->shaders = shaders;
}
GLint getAttribLocation(
	GLuint program,
	const GLchar *name)
{
	RenderCommand_getAttribLocation* __restrict packet = (RenderCommand_getAttribLocation*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getAttribLocation::execute);
	packet->program = program;
	packet->name = name;
}
void getProgramiv(
	GLuint program,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getProgramiv* __restrict packet = (RenderCommand_getProgramiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getProgramiv::execute);
	packet->program = program;
	packet->pname = pname;
	packet->params = params;
}
void getProgramInfoLog(
	GLuint program,
	GLsizei bufSize,
	GLsizei *length,
	GLchar *infoLog)
{
	RenderCommand_getProgramInfoLog* __restrict packet = (RenderCommand_getProgramInfoLog*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getProgramInfoLog::execute);
	packet->program = program;
	packet->bufSize = bufSize;
	packet->length = length;
	packet->infoLog = infoLog;
}
void getShaderiv(
	GLuint shader,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getShaderiv* __restrict packet = (RenderCommand_getShaderiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getShaderiv::execute);
	packet->shader = shader;
	packet->pname = pname;
	packet->params = params;
}
void getShaderInfoLog(
	GLuint shader,
	GLsizei bufSize,
	GLsizei *length,
	GLchar *infoLog)
{
	RenderCommand_getShaderInfoLog* __restrict packet = (RenderCommand_getShaderInfoLog*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getShaderInfoLog::execute);
	packet->shader = shader;
	packet->bufSize = bufSize;
	packet->length = length;
	packet->infoLog = infoLog;
}
void getShaderSource(
	GLuint shader,
	GLsizei bufSize,
	GLsizei *length,
	GLchar *source)
{
	RenderCommand_getShaderSource* __restrict packet = (RenderCommand_getShaderSource*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getShaderSource::execute);
	packet->shader = shader;
	packet->bufSize = bufSize;
	packet->length = length;
	packet->source = source;
}
GLint getUniformLocation(
	GLuint program,
	const GLchar *name)
{
	RenderCommand_getUniformLocation* __restrict packet = (RenderCommand_getUniformLocation*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getUniformLocation::execute);
	packet->program = program;
	packet->name = name;
}
void getUniformfv(
	GLuint program,
	GLint location,
	GLfloat *params)
{
	RenderCommand_getUniformfv* __restrict packet = (RenderCommand_getUniformfv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getUniformfv::execute);
	packet->program = program;
	packet->location = location;
	packet->params = params;
}
void getUniformiv(
	GLuint program,
	GLint location,
	GLint *params)
{
	RenderCommand_getUniformiv* __restrict packet = (RenderCommand_getUniformiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getUniformiv::execute);
	packet->program = program;
	packet->location = location;
	packet->params = params;
}
void getVertexAttribdv(
	GLuint index,
	GLenum pname,
	GLdouble *params)
{
	RenderCommand_getVertexAttribdv* __restrict packet = (RenderCommand_getVertexAttribdv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getVertexAttribdv::execute);
	packet->index = index;
	packet->pname = pname;
	packet->params = params;
}
void getVertexAttribfv(
	GLuint index,
	GLenum pname,
	GLfloat *params)
{
	RenderCommand_getVertexAttribfv* __restrict packet = (RenderCommand_getVertexAttribfv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getVertexAttribfv::execute);
	packet->index = index;
	packet->pname = pname;
	packet->params = params;
}
void getVertexAttribiv(
	GLuint index,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getVertexAttribiv* __restrict packet = (RenderCommand_getVertexAttribiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getVertexAttribiv::execute);
	packet->index = index;
	packet->pname = pname;
	packet->params = params;
}
void getVertexAttribPointerv(
	GLuint index,
	GLenum pname,
	void **pointer)
{
	RenderCommand_getVertexAttribPointerv* __restrict packet = (RenderCommand_getVertexAttribPointerv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getVertexAttribPointerv::execute);
	packet->index = index;
	packet->pname = pname;
	packet->pointer = pointer;
}
GLboolean isProgram(
	GLuint program)
{
	RenderCommand_isProgram* __restrict packet = (RenderCommand_isProgram*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isProgram::execute);
	packet->program = program;
}
GLboolean isShader(
	GLuint shader)
{
	RenderCommand_isShader* __restrict packet = (RenderCommand_isShader*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isShader::execute);
	packet->shader = shader;
}
void linkProgram(
	GLuint program)
{
	RenderCommand_linkProgram* __restrict packet = (RenderCommand_linkProgram*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_linkProgram::execute);
	packet->program = program;
}
void shaderSource(
	GLuint shader,
	GLsizei count,
	const GLchar *const*string,
	const GLint *length)
{
	RenderCommand_shaderSource* __restrict packet = (RenderCommand_shaderSource*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_shaderSource::execute);
	packet->shader = shader;
	packet->count = count;
	packet->string = string;
	packet->length = length;
}
void useProgram(
	GLuint program)
{
	RenderCommand_useProgram* __restrict packet = (RenderCommand_useProgram*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_useProgram::execute);
	packet->program = program;
}
void uniform1f(
	GLint location,
	GLfloat v0)
{
	RenderCommand_uniform1f* __restrict packet = (RenderCommand_uniform1f*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform1f::execute);
	packet->location = location;
	packet->v0 = v0;
}
void uniform2f(
	GLint location,
	GLfloat v0,
	GLfloat v1)
{
	RenderCommand_uniform2f* __restrict packet = (RenderCommand_uniform2f*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform2f::execute);
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
}
void uniform3f(
	GLint location,
	GLfloat v0,
	GLfloat v1,
	GLfloat v2)
{
	RenderCommand_uniform3f* __restrict packet = (RenderCommand_uniform3f*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform3f::execute);
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
	packet->v2 = v2;
}
void uniform4f(
	GLint location,
	GLfloat v0,
	GLfloat v1,
	GLfloat v2,
	GLfloat v3)
{
	RenderCommand_uniform4f* __restrict packet = (RenderCommand_uniform4f*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform4f::execute);
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
	packet->v2 = v2;
	packet->v3 = v3;
}
void uniform1i(
	GLint location,
	GLint v0)
{
	RenderCommand_uniform1i* __restrict packet = (RenderCommand_uniform1i*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform1i::execute);
	packet->location = location;
	packet->v0 = v0;
}
void uniform2i(
	GLint location,
	GLint v0,
	GLint v1)
{
	RenderCommand_uniform2i* __restrict packet = (RenderCommand_uniform2i*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform2i::execute);
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
}
void uniform3i(
	GLint location,
	GLint v0,
	GLint v1,
	GLint v2)
{
	RenderCommand_uniform3i* __restrict packet = (RenderCommand_uniform3i*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform3i::execute);
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
	packet->v2 = v2;
}
void uniform4i(
	GLint location,
	GLint v0,
	GLint v1,
	GLint v2,
	GLint v3)
{
	RenderCommand_uniform4i* __restrict packet = (RenderCommand_uniform4i*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform4i::execute);
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
	packet->v2 = v2;
	packet->v3 = v3;
}
void uniform1fv(
	GLint location,
	GLsizei count,
	const GLfloat *value)
{
	RenderCommand_uniform1fv* __restrict packet = (RenderCommand_uniform1fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform1fv::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform2fv(
	GLint location,
	GLsizei count,
	const GLfloat *value)
{
	RenderCommand_uniform2fv* __restrict packet = (RenderCommand_uniform2fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform2fv::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform3fv(
	GLint location,
	GLsizei count,
	const GLfloat *value)
{
	RenderCommand_uniform3fv* __restrict packet = (RenderCommand_uniform3fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform3fv::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform4fv(
	GLint location,
	GLsizei count,
	const GLfloat *value)
{
	RenderCommand_uniform4fv* __restrict packet = (RenderCommand_uniform4fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform4fv::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform1iv(
	GLint location,
	GLsizei count,
	const GLint *value)
{
	RenderCommand_uniform1iv* __restrict packet = (RenderCommand_uniform1iv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform1iv::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform2iv(
	GLint location,
	GLsizei count,
	const GLint *value)
{
	RenderCommand_uniform2iv* __restrict packet = (RenderCommand_uniform2iv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform2iv::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform3iv(
	GLint location,
	GLsizei count,
	const GLint *value)
{
	RenderCommand_uniform3iv* __restrict packet = (RenderCommand_uniform3iv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform3iv::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform4iv(
	GLint location,
	GLsizei count,
	const GLint *value)
{
	RenderCommand_uniform4iv* __restrict packet = (RenderCommand_uniform4iv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform4iv::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniformMatrix2fv(
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_uniformMatrix2fv* __restrict packet = (RenderCommand_uniformMatrix2fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformMatrix2fv::execute);
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void uniformMatrix3fv(
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_uniformMatrix3fv* __restrict packet = (RenderCommand_uniformMatrix3fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformMatrix3fv::execute);
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void uniformMatrix4fv(
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_uniformMatrix4fv* __restrict packet = (RenderCommand_uniformMatrix4fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformMatrix4fv::execute);
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void validateProgram(
	GLuint program)
{
	RenderCommand_validateProgram* __restrict packet = (RenderCommand_validateProgram*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_validateProgram::execute);
	packet->program = program;
}
void vertexAttrib1d(
	GLuint index,
	GLdouble x)
{
	RenderCommand_vertexAttrib1d* __restrict packet = (RenderCommand_vertexAttrib1d*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib1d::execute);
	packet->index = index;
	packet->x = x;
}
void vertexAttrib1dv(
	GLuint index,
	const GLdouble *v)
{
	RenderCommand_vertexAttrib1dv* __restrict packet = (RenderCommand_vertexAttrib1dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib1dv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib1f(
	GLuint index,
	GLfloat x)
{
	RenderCommand_vertexAttrib1f* __restrict packet = (RenderCommand_vertexAttrib1f*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib1f::execute);
	packet->index = index;
	packet->x = x;
}
void vertexAttrib1fv(
	GLuint index,
	const GLfloat *v)
{
	RenderCommand_vertexAttrib1fv* __restrict packet = (RenderCommand_vertexAttrib1fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib1fv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib1s(
	GLuint index,
	GLshort x)
{
	RenderCommand_vertexAttrib1s* __restrict packet = (RenderCommand_vertexAttrib1s*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib1s::execute);
	packet->index = index;
	packet->x = x;
}
void vertexAttrib1sv(
	GLuint index,
	const GLshort *v)
{
	RenderCommand_vertexAttrib1sv* __restrict packet = (RenderCommand_vertexAttrib1sv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib1sv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib2d(
	GLuint index,
	GLdouble x,
	GLdouble y)
{
	RenderCommand_vertexAttrib2d* __restrict packet = (RenderCommand_vertexAttrib2d*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib2d::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
}
void vertexAttrib2dv(
	GLuint index,
	const GLdouble *v)
{
	RenderCommand_vertexAttrib2dv* __restrict packet = (RenderCommand_vertexAttrib2dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib2dv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib2f(
	GLuint index,
	GLfloat x,
	GLfloat y)
{
	RenderCommand_vertexAttrib2f* __restrict packet = (RenderCommand_vertexAttrib2f*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib2f::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
}
void vertexAttrib2fv(
	GLuint index,
	const GLfloat *v)
{
	RenderCommand_vertexAttrib2fv* __restrict packet = (RenderCommand_vertexAttrib2fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib2fv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib2s(
	GLuint index,
	GLshort x,
	GLshort y)
{
	RenderCommand_vertexAttrib2s* __restrict packet = (RenderCommand_vertexAttrib2s*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib2s::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
}
void vertexAttrib2sv(
	GLuint index,
	const GLshort *v)
{
	RenderCommand_vertexAttrib2sv* __restrict packet = (RenderCommand_vertexAttrib2sv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib2sv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib3d(
	GLuint index,
	GLdouble x,
	GLdouble y,
	GLdouble z)
{
	RenderCommand_vertexAttrib3d* __restrict packet = (RenderCommand_vertexAttrib3d*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib3d::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void vertexAttrib3dv(
	GLuint index,
	const GLdouble *v)
{
	RenderCommand_vertexAttrib3dv* __restrict packet = (RenderCommand_vertexAttrib3dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib3dv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib3f(
	GLuint index,
	GLfloat x,
	GLfloat y,
	GLfloat z)
{
	RenderCommand_vertexAttrib3f* __restrict packet = (RenderCommand_vertexAttrib3f*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib3f::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void vertexAttrib3fv(
	GLuint index,
	const GLfloat *v)
{
	RenderCommand_vertexAttrib3fv* __restrict packet = (RenderCommand_vertexAttrib3fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib3fv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib3s(
	GLuint index,
	GLshort x,
	GLshort y,
	GLshort z)
{
	RenderCommand_vertexAttrib3s* __restrict packet = (RenderCommand_vertexAttrib3s*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib3s::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void vertexAttrib3sv(
	GLuint index,
	const GLshort *v)
{
	RenderCommand_vertexAttrib3sv* __restrict packet = (RenderCommand_vertexAttrib3sv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib3sv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib4Nbv(
	GLuint index,
	const GLbyte *v)
{
	RenderCommand_vertexAttrib4Nbv* __restrict packet = (RenderCommand_vertexAttrib4Nbv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib4Nbv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib4Niv(
	GLuint index,
	const GLint *v)
{
	RenderCommand_vertexAttrib4Niv* __restrict packet = (RenderCommand_vertexAttrib4Niv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib4Niv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib4Nsv(
	GLuint index,
	const GLshort *v)
{
	RenderCommand_vertexAttrib4Nsv* __restrict packet = (RenderCommand_vertexAttrib4Nsv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib4Nsv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib4Nub(
	GLuint index,
	GLubyte x,
	GLubyte y,
	GLubyte z,
	GLubyte w)
{
	RenderCommand_vertexAttrib4Nub* __restrict packet = (RenderCommand_vertexAttrib4Nub*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib4Nub::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void vertexAttrib4Nubv(
	GLuint index,
	const GLubyte *v)
{
	RenderCommand_vertexAttrib4Nubv* __restrict packet = (RenderCommand_vertexAttrib4Nubv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib4Nubv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib4Nuiv(
	GLuint index,
	const GLuint *v)
{
	RenderCommand_vertexAttrib4Nuiv* __restrict packet = (RenderCommand_vertexAttrib4Nuiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib4Nuiv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib4Nusv(
	GLuint index,
	const GLushort *v)
{
	RenderCommand_vertexAttrib4Nusv* __restrict packet = (RenderCommand_vertexAttrib4Nusv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib4Nusv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib4bv(
	GLuint index,
	const GLbyte *v)
{
	RenderCommand_vertexAttrib4bv* __restrict packet = (RenderCommand_vertexAttrib4bv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib4bv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib4d(
	GLuint index,
	GLdouble x,
	GLdouble y,
	GLdouble z,
	GLdouble w)
{
	RenderCommand_vertexAttrib4d* __restrict packet = (RenderCommand_vertexAttrib4d*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib4d::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void vertexAttrib4dv(
	GLuint index,
	const GLdouble *v)
{
	RenderCommand_vertexAttrib4dv* __restrict packet = (RenderCommand_vertexAttrib4dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib4dv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib4f(
	GLuint index,
	GLfloat x,
	GLfloat y,
	GLfloat z,
	GLfloat w)
{
	RenderCommand_vertexAttrib4f* __restrict packet = (RenderCommand_vertexAttrib4f*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib4f::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void vertexAttrib4fv(
	GLuint index,
	const GLfloat *v)
{
	RenderCommand_vertexAttrib4fv* __restrict packet = (RenderCommand_vertexAttrib4fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib4fv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib4iv(
	GLuint index,
	const GLint *v)
{
	RenderCommand_vertexAttrib4iv* __restrict packet = (RenderCommand_vertexAttrib4iv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib4iv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib4s(
	GLuint index,
	GLshort x,
	GLshort y,
	GLshort z,
	GLshort w)
{
	RenderCommand_vertexAttrib4s* __restrict packet = (RenderCommand_vertexAttrib4s*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib4s::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void vertexAttrib4sv(
	GLuint index,
	const GLshort *v)
{
	RenderCommand_vertexAttrib4sv* __restrict packet = (RenderCommand_vertexAttrib4sv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib4sv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib4ubv(
	GLuint index,
	const GLubyte *v)
{
	RenderCommand_vertexAttrib4ubv* __restrict packet = (RenderCommand_vertexAttrib4ubv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib4ubv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib4uiv(
	GLuint index,
	const GLuint *v)
{
	RenderCommand_vertexAttrib4uiv* __restrict packet = (RenderCommand_vertexAttrib4uiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib4uiv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttrib4usv(
	GLuint index,
	const GLushort *v)
{
	RenderCommand_vertexAttrib4usv* __restrict packet = (RenderCommand_vertexAttrib4usv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttrib4usv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribPointer(
	GLuint index,
	GLint size,
	GLenum type,
	GLboolean normalized,
	GLsizei stride,
	const void *pointer)
{
	RenderCommand_vertexAttribPointer* __restrict packet = (RenderCommand_vertexAttribPointer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribPointer::execute);
	packet->index = index;
	packet->size = size;
	packet->type = type;
	packet->normalized = normalized;
	packet->stride = stride;
	packet->pointer = pointer;
}
void uniformMatrix2x3fv(
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_uniformMatrix2x3fv* __restrict packet = (RenderCommand_uniformMatrix2x3fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformMatrix2x3fv::execute);
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void uniformMatrix3x2fv(
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_uniformMatrix3x2fv* __restrict packet = (RenderCommand_uniformMatrix3x2fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformMatrix3x2fv::execute);
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void uniformMatrix2x4fv(
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_uniformMatrix2x4fv* __restrict packet = (RenderCommand_uniformMatrix2x4fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformMatrix2x4fv::execute);
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void uniformMatrix4x2fv(
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_uniformMatrix4x2fv* __restrict packet = (RenderCommand_uniformMatrix4x2fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformMatrix4x2fv::execute);
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void uniformMatrix3x4fv(
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_uniformMatrix3x4fv* __restrict packet = (RenderCommand_uniformMatrix3x4fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformMatrix3x4fv::execute);
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void uniformMatrix4x3fv(
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_uniformMatrix4x3fv* __restrict packet = (RenderCommand_uniformMatrix4x3fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformMatrix4x3fv::execute);
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void colorMaski(
	GLuint index,
	GLboolean r,
	GLboolean g,
	GLboolean b,
	GLboolean a)
{
	RenderCommand_colorMaski* __restrict packet = (RenderCommand_colorMaski*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_colorMaski::execute);
	packet->index = index;
	packet->r = r;
	packet->g = g;
	packet->b = b;
	packet->a = a;
}
void getBooleani_v(
	GLenum target,
	GLuint index,
	GLboolean *data)
{
	RenderCommand_getBooleani_v* __restrict packet = (RenderCommand_getBooleani_v*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getBooleani_v::execute);
	packet->target = target;
	packet->index = index;
	packet->data = data;
}
void getIntegeri_v(
	GLenum target,
	GLuint index,
	GLint *data)
{
	RenderCommand_getIntegeri_v* __restrict packet = (RenderCommand_getIntegeri_v*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getIntegeri_v::execute);
	packet->target = target;
	packet->index = index;
	packet->data = data;
}
void enablei(
	GLenum target,
	GLuint index)
{
	RenderCommand_enablei* __restrict packet = (RenderCommand_enablei*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_enablei::execute);
	packet->target = target;
	packet->index = index;
}
void disablei(
	GLenum target,
	GLuint index)
{
	RenderCommand_disablei* __restrict packet = (RenderCommand_disablei*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_disablei::execute);
	packet->target = target;
	packet->index = index;
}
GLboolean isEnabledi(
	GLenum target,
	GLuint index)
{
	RenderCommand_isEnabledi* __restrict packet = (RenderCommand_isEnabledi*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isEnabledi::execute);
	packet->target = target;
	packet->index = index;
}
void beginTransformFeedback(
	GLenum primitiveMode)
{
	RenderCommand_beginTransformFeedback* __restrict packet = (RenderCommand_beginTransformFeedback*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_beginTransformFeedback::execute);
	packet->primitiveMode = primitiveMode;
}
void endTransformFeedback(
	void)
{
	RenderCommand_endTransformFeedback* __restrict packet = (RenderCommand_endTransformFeedback*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_endTransformFeedback::execute);
}
void bindBufferRange(
	GLenum target,
	GLuint index,
	GLuint buffer,
	GLintptr offset,
	GLsizeiptr size)
{
	RenderCommand_bindBufferRange* __restrict packet = (RenderCommand_bindBufferRange*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindBufferRange::execute);
	packet->target = target;
	packet->index = index;
	packet->buffer = buffer;
	packet->offset = offset;
	packet->size = size;
}
void bindBufferBase(
	GLenum target,
	GLuint index,
	GLuint buffer)
{
	RenderCommand_bindBufferBase* __restrict packet = (RenderCommand_bindBufferBase*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindBufferBase::execute);
	packet->target = target;
	packet->index = index;
	packet->buffer = buffer;
}
void transformFeedbackVaryings(
	GLuint program,
	GLsizei count,
	const GLchar *const*varyings,
	GLenum bufferMode)
{
	RenderCommand_transformFeedbackVaryings* __restrict packet = (RenderCommand_transformFeedbackVaryings*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_transformFeedbackVaryings::execute);
	packet->program = program;
	packet->count = count;
	packet->varyings = varyings;
	packet->bufferMode = bufferMode;
}
void getTransformFeedbackVarying(
	GLuint program,
	GLuint index,
	GLsizei bufSize,
	GLsizei *length,
	GLsizei *size,
	GLenum *type,
	GLchar *name)
{
	RenderCommand_getTransformFeedbackVarying* __restrict packet = (RenderCommand_getTransformFeedbackVarying*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTransformFeedbackVarying::execute);
	packet->program = program;
	packet->index = index;
	packet->bufSize = bufSize;
	packet->length = length;
	packet->size = size;
	packet->type = type;
	packet->name = name;
}
void clampColor(
	GLenum target,
	GLenum clamp)
{
	RenderCommand_clampColor* __restrict packet = (RenderCommand_clampColor*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clampColor::execute);
	packet->target = target;
	packet->clamp = clamp;
}
void beginConditionalRender(
	GLuint id,
	GLenum mode)
{
	RenderCommand_beginConditionalRender* __restrict packet = (RenderCommand_beginConditionalRender*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_beginConditionalRender::execute);
	packet->id = id;
	packet->mode = mode;
}
void endConditionalRender(
	void)
{
	RenderCommand_endConditionalRender* __restrict packet = (RenderCommand_endConditionalRender*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_endConditionalRender::execute);
}
void vertexAttribIPointer(
	GLuint index,
	GLint size,
	GLenum type,
	GLsizei stride,
	const void *pointer)
{
	RenderCommand_vertexAttribIPointer* __restrict packet = (RenderCommand_vertexAttribIPointer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribIPointer::execute);
	packet->index = index;
	packet->size = size;
	packet->type = type;
	packet->stride = stride;
	packet->pointer = pointer;
}
void getVertexAttribIiv(
	GLuint index,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getVertexAttribIiv* __restrict packet = (RenderCommand_getVertexAttribIiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getVertexAttribIiv::execute);
	packet->index = index;
	packet->pname = pname;
	packet->params = params;
}
void getVertexAttribIuiv(
	GLuint index,
	GLenum pname,
	GLuint *params)
{
	RenderCommand_getVertexAttribIuiv* __restrict packet = (RenderCommand_getVertexAttribIuiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getVertexAttribIuiv::execute);
	packet->index = index;
	packet->pname = pname;
	packet->params = params;
}
void vertexAttribI1i(
	GLuint index,
	GLint x)
{
	RenderCommand_vertexAttribI1i* __restrict packet = (RenderCommand_vertexAttribI1i*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribI1i::execute);
	packet->index = index;
	packet->x = x;
}
void vertexAttribI2i(
	GLuint index,
	GLint x,
	GLint y)
{
	RenderCommand_vertexAttribI2i* __restrict packet = (RenderCommand_vertexAttribI2i*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribI2i::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
}
void vertexAttribI3i(
	GLuint index,
	GLint x,
	GLint y,
	GLint z)
{
	RenderCommand_vertexAttribI3i* __restrict packet = (RenderCommand_vertexAttribI3i*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribI3i::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void vertexAttribI4i(
	GLuint index,
	GLint x,
	GLint y,
	GLint z,
	GLint w)
{
	RenderCommand_vertexAttribI4i* __restrict packet = (RenderCommand_vertexAttribI4i*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribI4i::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void vertexAttribI1ui(
	GLuint index,
	GLuint x)
{
	RenderCommand_vertexAttribI1ui* __restrict packet = (RenderCommand_vertexAttribI1ui*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribI1ui::execute);
	packet->index = index;
	packet->x = x;
}
void vertexAttribI2ui(
	GLuint index,
	GLuint x,
	GLuint y)
{
	RenderCommand_vertexAttribI2ui* __restrict packet = (RenderCommand_vertexAttribI2ui*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribI2ui::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
}
void vertexAttribI3ui(
	GLuint index,
	GLuint x,
	GLuint y,
	GLuint z)
{
	RenderCommand_vertexAttribI3ui* __restrict packet = (RenderCommand_vertexAttribI3ui*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribI3ui::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void vertexAttribI4ui(
	GLuint index,
	GLuint x,
	GLuint y,
	GLuint z,
	GLuint w)
{
	RenderCommand_vertexAttribI4ui* __restrict packet = (RenderCommand_vertexAttribI4ui*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribI4ui::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void vertexAttribI1iv(
	GLuint index,
	const GLint *v)
{
	RenderCommand_vertexAttribI1iv* __restrict packet = (RenderCommand_vertexAttribI1iv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribI1iv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribI2iv(
	GLuint index,
	const GLint *v)
{
	RenderCommand_vertexAttribI2iv* __restrict packet = (RenderCommand_vertexAttribI2iv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribI2iv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribI3iv(
	GLuint index,
	const GLint *v)
{
	RenderCommand_vertexAttribI3iv* __restrict packet = (RenderCommand_vertexAttribI3iv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribI3iv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribI4iv(
	GLuint index,
	const GLint *v)
{
	RenderCommand_vertexAttribI4iv* __restrict packet = (RenderCommand_vertexAttribI4iv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribI4iv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribI1uiv(
	GLuint index,
	const GLuint *v)
{
	RenderCommand_vertexAttribI1uiv* __restrict packet = (RenderCommand_vertexAttribI1uiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribI1uiv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribI2uiv(
	GLuint index,
	const GLuint *v)
{
	RenderCommand_vertexAttribI2uiv* __restrict packet = (RenderCommand_vertexAttribI2uiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribI2uiv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribI3uiv(
	GLuint index,
	const GLuint *v)
{
	RenderCommand_vertexAttribI3uiv* __restrict packet = (RenderCommand_vertexAttribI3uiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribI3uiv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribI4uiv(
	GLuint index,
	const GLuint *v)
{
	RenderCommand_vertexAttribI4uiv* __restrict packet = (RenderCommand_vertexAttribI4uiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribI4uiv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribI4bv(
	GLuint index,
	const GLbyte *v)
{
	RenderCommand_vertexAttribI4bv* __restrict packet = (RenderCommand_vertexAttribI4bv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribI4bv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribI4sv(
	GLuint index,
	const GLshort *v)
{
	RenderCommand_vertexAttribI4sv* __restrict packet = (RenderCommand_vertexAttribI4sv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribI4sv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribI4ubv(
	GLuint index,
	const GLubyte *v)
{
	RenderCommand_vertexAttribI4ubv* __restrict packet = (RenderCommand_vertexAttribI4ubv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribI4ubv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribI4usv(
	GLuint index,
	const GLushort *v)
{
	RenderCommand_vertexAttribI4usv* __restrict packet = (RenderCommand_vertexAttribI4usv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribI4usv::execute);
	packet->index = index;
	packet->v = v;
}
void getUniformuiv(
	GLuint program,
	GLint location,
	GLuint *params)
{
	RenderCommand_getUniformuiv* __restrict packet = (RenderCommand_getUniformuiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getUniformuiv::execute);
	packet->program = program;
	packet->location = location;
	packet->params = params;
}
void bindFragDataLocation(
	GLuint program,
	GLuint color,
	const GLchar *name)
{
	RenderCommand_bindFragDataLocation* __restrict packet = (RenderCommand_bindFragDataLocation*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindFragDataLocation::execute);
	packet->program = program;
	packet->color = color;
	packet->name = name;
}
GLint getFragDataLocation(
	GLuint program,
	const GLchar *name)
{
	RenderCommand_getFragDataLocation* __restrict packet = (RenderCommand_getFragDataLocation*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getFragDataLocation::execute);
	packet->program = program;
	packet->name = name;
}
void uniform1ui(
	GLint location,
	GLuint v0)
{
	RenderCommand_uniform1ui* __restrict packet = (RenderCommand_uniform1ui*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform1ui::execute);
	packet->location = location;
	packet->v0 = v0;
}
void uniform2ui(
	GLint location,
	GLuint v0,
	GLuint v1)
{
	RenderCommand_uniform2ui* __restrict packet = (RenderCommand_uniform2ui*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform2ui::execute);
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
}
void uniform3ui(
	GLint location,
	GLuint v0,
	GLuint v1,
	GLuint v2)
{
	RenderCommand_uniform3ui* __restrict packet = (RenderCommand_uniform3ui*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform3ui::execute);
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
	packet->v2 = v2;
}
void uniform4ui(
	GLint location,
	GLuint v0,
	GLuint v1,
	GLuint v2,
	GLuint v3)
{
	RenderCommand_uniform4ui* __restrict packet = (RenderCommand_uniform4ui*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform4ui::execute);
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
	packet->v2 = v2;
	packet->v3 = v3;
}
void uniform1uiv(
	GLint location,
	GLsizei count,
	const GLuint *value)
{
	RenderCommand_uniform1uiv* __restrict packet = (RenderCommand_uniform1uiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform1uiv::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform2uiv(
	GLint location,
	GLsizei count,
	const GLuint *value)
{
	RenderCommand_uniform2uiv* __restrict packet = (RenderCommand_uniform2uiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform2uiv::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform3uiv(
	GLint location,
	GLsizei count,
	const GLuint *value)
{
	RenderCommand_uniform3uiv* __restrict packet = (RenderCommand_uniform3uiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform3uiv::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform4uiv(
	GLint location,
	GLsizei count,
	const GLuint *value)
{
	RenderCommand_uniform4uiv* __restrict packet = (RenderCommand_uniform4uiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform4uiv::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void texParameterIiv(
	GLenum target,
	GLenum pname,
	const GLint *params)
{
	RenderCommand_texParameterIiv* __restrict packet = (RenderCommand_texParameterIiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texParameterIiv::execute);
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void texParameterIuiv(
	GLenum target,
	GLenum pname,
	const GLuint *params)
{
	RenderCommand_texParameterIuiv* __restrict packet = (RenderCommand_texParameterIuiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texParameterIuiv::execute);
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void getTexParameterIiv(
	GLenum target,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getTexParameterIiv* __restrict packet = (RenderCommand_getTexParameterIiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTexParameterIiv::execute);
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void getTexParameterIuiv(
	GLenum target,
	GLenum pname,
	GLuint *params)
{
	RenderCommand_getTexParameterIuiv* __restrict packet = (RenderCommand_getTexParameterIuiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTexParameterIuiv::execute);
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void clearBufferiv(
	GLenum buffer,
	GLint drawbuffer,
	const GLint *value)
{
	RenderCommand_clearBufferiv* __restrict packet = (RenderCommand_clearBufferiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clearBufferiv::execute);
	packet->buffer = buffer;
	packet->drawbuffer = drawbuffer;
	packet->value = value;
}
void clearBufferuiv(
	GLenum buffer,
	GLint drawbuffer,
	const GLuint *value)
{
	RenderCommand_clearBufferuiv* __restrict packet = (RenderCommand_clearBufferuiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clearBufferuiv::execute);
	packet->buffer = buffer;
	packet->drawbuffer = drawbuffer;
	packet->value = value;
}
void clearBufferfv(
	GLenum buffer,
	GLint drawbuffer,
	const GLfloat *value)
{
	RenderCommand_clearBufferfv* __restrict packet = (RenderCommand_clearBufferfv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clearBufferfv::execute);
	packet->buffer = buffer;
	packet->drawbuffer = drawbuffer;
	packet->value = value;
}
void clearBufferfi(
	GLenum buffer,
	GLint drawbuffer,
	GLfloat depth,
	GLint stencil)
{
	RenderCommand_clearBufferfi* __restrict packet = (RenderCommand_clearBufferfi*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clearBufferfi::execute);
	packet->buffer = buffer;
	packet->drawbuffer = drawbuffer;
	packet->depth = depth;
	packet->stencil = stencil;
}
const GLubyte* getStringi(
	GLenum name,
	GLuint index)
{
	RenderCommand_getStringi* __restrict packet = (RenderCommand_getStringi*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getStringi::execute);
	packet->name = name;
	packet->index = index;
}
GLboolean isRenderbuffer(
	GLuint renderbuffer)
{
	RenderCommand_isRenderbuffer* __restrict packet = (RenderCommand_isRenderbuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isRenderbuffer::execute);
	packet->renderbuffer = renderbuffer;
}
void bindRenderbuffer(
	GLenum target,
	GLuint renderbuffer)
{
	RenderCommand_bindRenderbuffer* __restrict packet = (RenderCommand_bindRenderbuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindRenderbuffer::execute);
	packet->target = target;
	packet->renderbuffer = renderbuffer;
}
void deleteRenderbuffers(
	GLsizei n,
	const GLuint *renderbuffers)
{
	RenderCommand_deleteRenderbuffers* __restrict packet = (RenderCommand_deleteRenderbuffers*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_deleteRenderbuffers::execute);
	packet->n = n;
	packet->renderbuffers = renderbuffers;
}
void genRenderbuffers(
	GLsizei n,
	GLuint *renderbuffers)
{
	RenderCommand_genRenderbuffers* __restrict packet = (RenderCommand_genRenderbuffers*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_genRenderbuffers::execute);
	packet->n = n;
	packet->renderbuffers = renderbuffers;
}
void renderbufferStorage(
	GLenum target,
	GLenum internalformat,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_renderbufferStorage* __restrict packet = (RenderCommand_renderbufferStorage*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_renderbufferStorage::execute);
	packet->target = target;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
}
void getRenderbufferParameteriv(
	GLenum target,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getRenderbufferParameteriv* __restrict packet = (RenderCommand_getRenderbufferParameteriv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getRenderbufferParameteriv::execute);
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
GLboolean isFramebuffer(
	GLuint framebuffer)
{
	RenderCommand_isFramebuffer* __restrict packet = (RenderCommand_isFramebuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isFramebuffer::execute);
	packet->framebuffer = framebuffer;
}
void bindFramebuffer(
	GLenum target,
	GLuint framebuffer)
{
	RenderCommand_bindFramebuffer* __restrict packet = (RenderCommand_bindFramebuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindFramebuffer::execute);
	packet->target = target;
	packet->framebuffer = framebuffer;
}
void deleteFramebuffers(
	GLsizei n,
	const GLuint *framebuffers)
{
	RenderCommand_deleteFramebuffers* __restrict packet = (RenderCommand_deleteFramebuffers*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_deleteFramebuffers::execute);
	packet->n = n;
	packet->framebuffers = framebuffers;
}
void genFramebuffers(
	GLsizei n,
	GLuint *framebuffers)
{
	RenderCommand_genFramebuffers* __restrict packet = (RenderCommand_genFramebuffers*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_genFramebuffers::execute);
	packet->n = n;
	packet->framebuffers = framebuffers;
}
GLenum checkFramebufferStatus(
	GLenum target)
{
	RenderCommand_checkFramebufferStatus* __restrict packet = (RenderCommand_checkFramebufferStatus*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_checkFramebufferStatus::execute);
	packet->target = target;
}
void framebufferTexture1D(
	GLenum target,
	GLenum attachment,
	GLenum textarget,
	GLuint texture,
	GLint level)
{
	RenderCommand_framebufferTexture1D* __restrict packet = (RenderCommand_framebufferTexture1D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_framebufferTexture1D::execute);
	packet->target = target;
	packet->attachment = attachment;
	packet->textarget = textarget;
	packet->texture = texture;
	packet->level = level;
}
void framebufferTexture2D(
	GLenum target,
	GLenum attachment,
	GLenum textarget,
	GLuint texture,
	GLint level)
{
	RenderCommand_framebufferTexture2D* __restrict packet = (RenderCommand_framebufferTexture2D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_framebufferTexture2D::execute);
	packet->target = target;
	packet->attachment = attachment;
	packet->textarget = textarget;
	packet->texture = texture;
	packet->level = level;
}
void framebufferTexture3D(
	GLenum target,
	GLenum attachment,
	GLenum textarget,
	GLuint texture,
	GLint level,
	GLint zoffset)
{
	RenderCommand_framebufferTexture3D* __restrict packet = (RenderCommand_framebufferTexture3D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_framebufferTexture3D::execute);
	packet->target = target;
	packet->attachment = attachment;
	packet->textarget = textarget;
	packet->texture = texture;
	packet->level = level;
	packet->zoffset = zoffset;
}
void framebufferRenderbuffer(
	GLenum target,
	GLenum attachment,
	GLenum renderbuffertarget,
	GLuint renderbuffer)
{
	RenderCommand_framebufferRenderbuffer* __restrict packet = (RenderCommand_framebufferRenderbuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_framebufferRenderbuffer::execute);
	packet->target = target;
	packet->attachment = attachment;
	packet->renderbuffertarget = renderbuffertarget;
	packet->renderbuffer = renderbuffer;
}
void getFramebufferAttachmentParameteriv(
	GLenum target,
	GLenum attachment,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getFramebufferAttachmentParameteriv* __restrict packet = (RenderCommand_getFramebufferAttachmentParameteriv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getFramebufferAttachmentParameteriv::execute);
	packet->target = target;
	packet->attachment = attachment;
	packet->pname = pname;
	packet->params = params;
}
void generateMipmap(
	GLenum target)
{
	RenderCommand_generateMipmap* __restrict packet = (RenderCommand_generateMipmap*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_generateMipmap::execute);
	packet->target = target;
}
void blitFramebuffer(
	GLint srcX0,
	GLint srcY0,
	GLint srcX1,
	GLint srcY1,
	GLint dstX0,
	GLint dstY0,
	GLint dstX1,
	GLint dstY1,
	GLbitfield mask,
	GLenum filter)
{
	RenderCommand_blitFramebuffer* __restrict packet = (RenderCommand_blitFramebuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_blitFramebuffer::execute);
	packet->srcX0 = srcX0;
	packet->srcY0 = srcY0;
	packet->srcX1 = srcX1;
	packet->srcY1 = srcY1;
	packet->dstX0 = dstX0;
	packet->dstY0 = dstY0;
	packet->dstX1 = dstX1;
	packet->dstY1 = dstY1;
	packet->mask = mask;
	packet->filter = filter;
}
void renderbufferStorageMultisample(
	GLenum target,
	GLsizei samples,
	GLenum internalformat,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_renderbufferStorageMultisample* __restrict packet = (RenderCommand_renderbufferStorageMultisample*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_renderbufferStorageMultisample::execute);
	packet->target = target;
	packet->samples = samples;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
}
void framebufferTextureLayer(
	GLenum target,
	GLenum attachment,
	GLuint texture,
	GLint level,
	GLint layer)
{
	RenderCommand_framebufferTextureLayer* __restrict packet = (RenderCommand_framebufferTextureLayer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_framebufferTextureLayer::execute);
	packet->target = target;
	packet->attachment = attachment;
	packet->texture = texture;
	packet->level = level;
	packet->layer = layer;
}
void* mapBufferRange(
	GLenum target,
	GLintptr offset,
	GLsizeiptr length,
	GLbitfield access)
{
	RenderCommand_mapBufferRange* __restrict packet = (RenderCommand_mapBufferRange*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_mapBufferRange::execute);
	packet->target = target;
	packet->offset = offset;
	packet->length = length;
	packet->access = access;
}
void flushMappedBufferRange(
	GLenum target,
	GLintptr offset,
	GLsizeiptr length)
{
	RenderCommand_flushMappedBufferRange* __restrict packet = (RenderCommand_flushMappedBufferRange*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_flushMappedBufferRange::execute);
	packet->target = target;
	packet->offset = offset;
	packet->length = length;
}
void bindVertexArray(
	GLuint array)
{
	RenderCommand_bindVertexArray* __restrict packet = (RenderCommand_bindVertexArray*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindVertexArray::execute);
	packet->array = array;
}
void deleteVertexArrays(
	GLsizei n,
	const GLuint *arrays)
{
	RenderCommand_deleteVertexArrays* __restrict packet = (RenderCommand_deleteVertexArrays*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_deleteVertexArrays::execute);
	packet->n = n;
	packet->arrays = arrays;
}
void genVertexArrays(
	GLsizei n,
	GLuint *arrays)
{
	RenderCommand_genVertexArrays* __restrict packet = (RenderCommand_genVertexArrays*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_genVertexArrays::execute);
	packet->n = n;
	packet->arrays = arrays;
}
GLboolean isVertexArray(
	GLuint array)
{
	RenderCommand_isVertexArray* __restrict packet = (RenderCommand_isVertexArray*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isVertexArray::execute);
	packet->array = array;
}
void drawArraysInstanced(
	GLenum mode,
	GLint first,
	GLsizei count,
	GLsizei instancecount)
{
	RenderCommand_drawArraysInstanced* __restrict packet = (RenderCommand_drawArraysInstanced*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawArraysInstanced::execute);
	packet->mode = mode;
	packet->first = first;
	packet->count = count;
	packet->instancecount = instancecount;
}
void drawElementsInstanced(
	GLenum mode,
	GLsizei count,
	GLenum type,
	const void *indices,
	GLsizei instancecount)
{
	RenderCommand_drawElementsInstanced* __restrict packet = (RenderCommand_drawElementsInstanced*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawElementsInstanced::execute);
	packet->mode = mode;
	packet->count = count;
	packet->type = type;
	packet->indices = indices;
	packet->instancecount = instancecount;
}
void texBuffer(
	GLenum target,
	GLenum internalformat,
	GLuint buffer)
{
	RenderCommand_texBuffer* __restrict packet = (RenderCommand_texBuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texBuffer::execute);
	packet->target = target;
	packet->internalformat = internalformat;
	packet->buffer = buffer;
}
void primitiveRestartIndex(
	GLuint index)
{
	RenderCommand_primitiveRestartIndex* __restrict packet = (RenderCommand_primitiveRestartIndex*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_primitiveRestartIndex::execute);
	packet->index = index;
}
void copyBufferSubData(
	GLenum readTarget,
	GLenum writeTarget,
	GLintptr readOffset,
	GLintptr writeOffset,
	GLsizeiptr size)
{
	RenderCommand_copyBufferSubData* __restrict packet = (RenderCommand_copyBufferSubData*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyBufferSubData::execute);
	packet->readTarget = readTarget;
	packet->writeTarget = writeTarget;
	packet->readOffset = readOffset;
	packet->writeOffset = writeOffset;
	packet->size = size;
}
void getUniformIndices(
	GLuint program,
	GLsizei uniformCount,
	const GLchar *const*uniformNames,
	GLuint *uniformIndices)
{
	RenderCommand_getUniformIndices* __restrict packet = (RenderCommand_getUniformIndices*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getUniformIndices::execute);
	packet->program = program;
	packet->uniformCount = uniformCount;
	packet->uniformNames = uniformNames;
	packet->uniformIndices = uniformIndices;
}
void getActiveUniformsiv(
	GLuint program,
	GLsizei uniformCount,
	const GLuint *uniformIndices,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getActiveUniformsiv* __restrict packet = (RenderCommand_getActiveUniformsiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getActiveUniformsiv::execute);
	packet->program = program;
	packet->uniformCount = uniformCount;
	packet->uniformIndices = uniformIndices;
	packet->pname = pname;
	packet->params = params;
}
void getActiveUniformName(
	GLuint program,
	GLuint uniformIndex,
	GLsizei bufSize,
	GLsizei *length,
	GLchar *uniformName)
{
	RenderCommand_getActiveUniformName* __restrict packet = (RenderCommand_getActiveUniformName*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getActiveUniformName::execute);
	packet->program = program;
	packet->uniformIndex = uniformIndex;
	packet->bufSize = bufSize;
	packet->length = length;
	packet->uniformName = uniformName;
}
GLuint getUniformBlockIndex(
	GLuint program,
	const GLchar *uniformBlockName)
{
	RenderCommand_getUniformBlockIndex* __restrict packet = (RenderCommand_getUniformBlockIndex*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getUniformBlockIndex::execute);
	packet->program = program;
	packet->uniformBlockName = uniformBlockName;
}
void getActiveUniformBlockiv(
	GLuint program,
	GLuint uniformBlockIndex,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getActiveUniformBlockiv* __restrict packet = (RenderCommand_getActiveUniformBlockiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getActiveUniformBlockiv::execute);
	packet->program = program;
	packet->uniformBlockIndex = uniformBlockIndex;
	packet->pname = pname;
	packet->params = params;
}
void getActiveUniformBlockName(
	GLuint program,
	GLuint uniformBlockIndex,
	GLsizei bufSize,
	GLsizei *length,
	GLchar *uniformBlockName)
{
	RenderCommand_getActiveUniformBlockName* __restrict packet = (RenderCommand_getActiveUniformBlockName*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getActiveUniformBlockName::execute);
	packet->program = program;
	packet->uniformBlockIndex = uniformBlockIndex;
	packet->bufSize = bufSize;
	packet->length = length;
	packet->uniformBlockName = uniformBlockName;
}
void uniformBlockBinding(
	GLuint program,
	GLuint uniformBlockIndex,
	GLuint uniformBlockBinding)
{
	RenderCommand_uniformBlockBinding* __restrict packet = (RenderCommand_uniformBlockBinding*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformBlockBinding::execute);
	packet->program = program;
	packet->uniformBlockIndex = uniformBlockIndex;
	packet->uniformBlockBinding = uniformBlockBinding;
}
void drawElementsBaseVertex(
	GLenum mode,
	GLsizei count,
	GLenum type,
	const void *indices,
	GLint basevertex)
{
	RenderCommand_drawElementsBaseVertex* __restrict packet = (RenderCommand_drawElementsBaseVertex*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawElementsBaseVertex::execute);
	packet->mode = mode;
	packet->count = count;
	packet->type = type;
	packet->indices = indices;
	packet->basevertex = basevertex;
}
void drawRangeElementsBaseVertex(
	GLenum mode,
	GLuint start,
	GLuint end,
	GLsizei count,
	GLenum type,
	const void *indices,
	GLint basevertex)
{
	RenderCommand_drawRangeElementsBaseVertex* __restrict packet = (RenderCommand_drawRangeElementsBaseVertex*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawRangeElementsBaseVertex::execute);
	packet->mode = mode;
	packet->start = start;
	packet->end = end;
	packet->count = count;
	packet->type = type;
	packet->indices = indices;
	packet->basevertex = basevertex;
}
void drawElementsInstancedBaseVertex(
	GLenum mode,
	GLsizei count,
	GLenum type,
	const void *indices,
	GLsizei instancecount,
	GLint basevertex)
{
	RenderCommand_drawElementsInstancedBaseVertex* __restrict packet = (RenderCommand_drawElementsInstancedBaseVertex*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawElementsInstancedBaseVertex::execute);
	packet->mode = mode;
	packet->count = count;
	packet->type = type;
	packet->indices = indices;
	packet->instancecount = instancecount;
	packet->basevertex = basevertex;
}
void multiDrawElementsBaseVertex(
	GLenum mode,
	const GLsizei *count,
	GLenum type,
	const void *const*indices,
	GLsizei drawcount,
	const GLint *basevertex)
{
	RenderCommand_multiDrawElementsBaseVertex* __restrict packet = (RenderCommand_multiDrawElementsBaseVertex*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiDrawElementsBaseVertex::execute);
	packet->mode = mode;
	packet->count = count;
	packet->type = type;
	packet->indices = indices;
	packet->drawcount = drawcount;
	packet->basevertex = basevertex;
}
void provokingVertex(
	GLenum mode)
{
	RenderCommand_provokingVertex* __restrict packet = (RenderCommand_provokingVertex*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_provokingVertex::execute);
	packet->mode = mode;
}
GLsync fenceSync(
	GLenum condition,
	GLbitfield flags)
{
	RenderCommand_fenceSync* __restrict packet = (RenderCommand_fenceSync*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_fenceSync::execute);
	packet->condition = condition;
	packet->flags = flags;
}
GLboolean isSync(
	GLsync sync)
{
	RenderCommand_isSync* __restrict packet = (RenderCommand_isSync*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isSync::execute);
	packet->sync = sync;
}
void deleteSync(
	GLsync sync)
{
	RenderCommand_deleteSync* __restrict packet = (RenderCommand_deleteSync*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_deleteSync::execute);
	packet->sync = sync;
}
GLenum clientWaitSync(
	GLsync sync,
	GLbitfield flags,
	GLuint64 timeout)
{
	RenderCommand_clientWaitSync* __restrict packet = (RenderCommand_clientWaitSync*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clientWaitSync::execute);
	packet->sync = sync;
	packet->flags = flags;
	packet->timeout = timeout;
}
void waitSync(
	GLsync sync,
	GLbitfield flags,
	GLuint64 timeout)
{
	RenderCommand_waitSync* __restrict packet = (RenderCommand_waitSync*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_waitSync::execute);
	packet->sync = sync;
	packet->flags = flags;
	packet->timeout = timeout;
}
void getInteger64v(
	GLenum pname,
	GLint64 *data)
{
	RenderCommand_getInteger64v* __restrict packet = (RenderCommand_getInteger64v*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getInteger64v::execute);
	packet->pname = pname;
	packet->data = data;
}
void getSynciv(
	GLsync sync,
	GLenum pname,
	GLsizei bufSize,
	GLsizei *length,
	GLint *values)
{
	RenderCommand_getSynciv* __restrict packet = (RenderCommand_getSynciv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getSynciv::execute);
	packet->sync = sync;
	packet->pname = pname;
	packet->bufSize = bufSize;
	packet->length = length;
	packet->values = values;
}
void getInteger64i_v(
	GLenum target,
	GLuint index,
	GLint64 *data)
{
	RenderCommand_getInteger64i_v* __restrict packet = (RenderCommand_getInteger64i_v*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getInteger64i_v::execute);
	packet->target = target;
	packet->index = index;
	packet->data = data;
}
void getBufferParameteri64v(
	GLenum target,
	GLenum pname,
	GLint64 *params)
{
	RenderCommand_getBufferParameteri64v* __restrict packet = (RenderCommand_getBufferParameteri64v*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getBufferParameteri64v::execute);
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void framebufferTexture(
	GLenum target,
	GLenum attachment,
	GLuint texture,
	GLint level)
{
	RenderCommand_framebufferTexture* __restrict packet = (RenderCommand_framebufferTexture*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_framebufferTexture::execute);
	packet->target = target;
	packet->attachment = attachment;
	packet->texture = texture;
	packet->level = level;
}
void texImage2DMultisample(
	GLenum target,
	GLsizei samples,
	GLenum internalformat,
	GLsizei width,
	GLsizei height,
	GLboolean fixedsamplelocations)
{
	RenderCommand_texImage2DMultisample* __restrict packet = (RenderCommand_texImage2DMultisample*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texImage2DMultisample::execute);
	packet->target = target;
	packet->samples = samples;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->fixedsamplelocations = fixedsamplelocations;
}
void texImage3DMultisample(
	GLenum target,
	GLsizei samples,
	GLenum internalformat,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLboolean fixedsamplelocations)
{
	RenderCommand_texImage3DMultisample* __restrict packet = (RenderCommand_texImage3DMultisample*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texImage3DMultisample::execute);
	packet->target = target;
	packet->samples = samples;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->fixedsamplelocations = fixedsamplelocations;
}
void getMultisamplefv(
	GLenum pname,
	GLuint index,
	GLfloat *val)
{
	RenderCommand_getMultisamplefv* __restrict packet = (RenderCommand_getMultisamplefv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getMultisamplefv::execute);
	packet->pname = pname;
	packet->index = index;
	packet->val = val;
}
void sampleMaski(
	GLuint maskNumber,
	GLbitfield mask)
{
	RenderCommand_sampleMaski* __restrict packet = (RenderCommand_sampleMaski*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_sampleMaski::execute);
	packet->maskNumber = maskNumber;
	packet->mask = mask;
}
void bindFragDataLocationIndexed(
	GLuint program,
	GLuint colorNumber,
	GLuint index,
	const GLchar *name)
{
	RenderCommand_bindFragDataLocationIndexed* __restrict packet = (RenderCommand_bindFragDataLocationIndexed*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindFragDataLocationIndexed::execute);
	packet->program = program;
	packet->colorNumber = colorNumber;
	packet->index = index;
	packet->name = name;
}
GLint getFragDataIndex(
	GLuint program,
	const GLchar *name)
{
	RenderCommand_getFragDataIndex* __restrict packet = (RenderCommand_getFragDataIndex*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getFragDataIndex::execute);
	packet->program = program;
	packet->name = name;
}
void genSamplers(
	GLsizei count,
	GLuint *samplers)
{
	RenderCommand_genSamplers* __restrict packet = (RenderCommand_genSamplers*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_genSamplers::execute);
	packet->count = count;
	packet->samplers = samplers;
}
void deleteSamplers(
	GLsizei count,
	const GLuint *samplers)
{
	RenderCommand_deleteSamplers* __restrict packet = (RenderCommand_deleteSamplers*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_deleteSamplers::execute);
	packet->count = count;
	packet->samplers = samplers;
}
GLboolean isSampler(
	GLuint sampler)
{
	RenderCommand_isSampler* __restrict packet = (RenderCommand_isSampler*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isSampler::execute);
	packet->sampler = sampler;
}
void bindSampler(
	GLuint unit,
	GLuint sampler)
{
	RenderCommand_bindSampler* __restrict packet = (RenderCommand_bindSampler*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindSampler::execute);
	packet->unit = unit;
	packet->sampler = sampler;
}
void samplerParameteri(
	GLuint sampler,
	GLenum pname,
	GLint param)
{
	RenderCommand_samplerParameteri* __restrict packet = (RenderCommand_samplerParameteri*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_samplerParameteri::execute);
	packet->sampler = sampler;
	packet->pname = pname;
	packet->param = param;
}
void samplerParameteriv(
	GLuint sampler,
	GLenum pname,
	const GLint *param)
{
	RenderCommand_samplerParameteriv* __restrict packet = (RenderCommand_samplerParameteriv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_samplerParameteriv::execute);
	packet->sampler = sampler;
	packet->pname = pname;
	packet->param = param;
}
void samplerParameterf(
	GLuint sampler,
	GLenum pname,
	GLfloat param)
{
	RenderCommand_samplerParameterf* __restrict packet = (RenderCommand_samplerParameterf*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_samplerParameterf::execute);
	packet->sampler = sampler;
	packet->pname = pname;
	packet->param = param;
}
void samplerParameterfv(
	GLuint sampler,
	GLenum pname,
	const GLfloat *param)
{
	RenderCommand_samplerParameterfv* __restrict packet = (RenderCommand_samplerParameterfv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_samplerParameterfv::execute);
	packet->sampler = sampler;
	packet->pname = pname;
	packet->param = param;
}
void samplerParameterIiv(
	GLuint sampler,
	GLenum pname,
	const GLint *param)
{
	RenderCommand_samplerParameterIiv* __restrict packet = (RenderCommand_samplerParameterIiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_samplerParameterIiv::execute);
	packet->sampler = sampler;
	packet->pname = pname;
	packet->param = param;
}
void samplerParameterIuiv(
	GLuint sampler,
	GLenum pname,
	const GLuint *param)
{
	RenderCommand_samplerParameterIuiv* __restrict packet = (RenderCommand_samplerParameterIuiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_samplerParameterIuiv::execute);
	packet->sampler = sampler;
	packet->pname = pname;
	packet->param = param;
}
void getSamplerParameteriv(
	GLuint sampler,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getSamplerParameteriv* __restrict packet = (RenderCommand_getSamplerParameteriv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getSamplerParameteriv::execute);
	packet->sampler = sampler;
	packet->pname = pname;
	packet->params = params;
}
void getSamplerParameterIiv(
	GLuint sampler,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getSamplerParameterIiv* __restrict packet = (RenderCommand_getSamplerParameterIiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getSamplerParameterIiv::execute);
	packet->sampler = sampler;
	packet->pname = pname;
	packet->params = params;
}
void getSamplerParameterfv(
	GLuint sampler,
	GLenum pname,
	GLfloat *params)
{
	RenderCommand_getSamplerParameterfv* __restrict packet = (RenderCommand_getSamplerParameterfv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getSamplerParameterfv::execute);
	packet->sampler = sampler;
	packet->pname = pname;
	packet->params = params;
}
void getSamplerParameterIuiv(
	GLuint sampler,
	GLenum pname,
	GLuint *params)
{
	RenderCommand_getSamplerParameterIuiv* __restrict packet = (RenderCommand_getSamplerParameterIuiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getSamplerParameterIuiv::execute);
	packet->sampler = sampler;
	packet->pname = pname;
	packet->params = params;
}
void queryCounter(
	GLuint id,
	GLenum target)
{
	RenderCommand_queryCounter* __restrict packet = (RenderCommand_queryCounter*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_queryCounter::execute);
	packet->id = id;
	packet->target = target;
}
void getQueryObjecti64v(
	GLuint id,
	GLenum pname,
	GLint64 *params)
{
	RenderCommand_getQueryObjecti64v* __restrict packet = (RenderCommand_getQueryObjecti64v*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getQueryObjecti64v::execute);
	packet->id = id;
	packet->pname = pname;
	packet->params = params;
}
void getQueryObjectui64v(
	GLuint id,
	GLenum pname,
	GLuint64 *params)
{
	RenderCommand_getQueryObjectui64v* __restrict packet = (RenderCommand_getQueryObjectui64v*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getQueryObjectui64v::execute);
	packet->id = id;
	packet->pname = pname;
	packet->params = params;
}
void vertexAttribDivisor(
	GLuint index,
	GLuint divisor)
{
	RenderCommand_vertexAttribDivisor* __restrict packet = (RenderCommand_vertexAttribDivisor*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribDivisor::execute);
	packet->index = index;
	packet->divisor = divisor;
}
void vertexAttribP1ui(
	GLuint index,
	GLenum type,
	GLboolean normalized,
	GLuint value)
{
	RenderCommand_vertexAttribP1ui* __restrict packet = (RenderCommand_vertexAttribP1ui*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribP1ui::execute);
	packet->index = index;
	packet->type = type;
	packet->normalized = normalized;
	packet->value = value;
}
void vertexAttribP1uiv(
	GLuint index,
	GLenum type,
	GLboolean normalized,
	const GLuint *value)
{
	RenderCommand_vertexAttribP1uiv* __restrict packet = (RenderCommand_vertexAttribP1uiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribP1uiv::execute);
	packet->index = index;
	packet->type = type;
	packet->normalized = normalized;
	packet->value = value;
}
void vertexAttribP2ui(
	GLuint index,
	GLenum type,
	GLboolean normalized,
	GLuint value)
{
	RenderCommand_vertexAttribP2ui* __restrict packet = (RenderCommand_vertexAttribP2ui*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribP2ui::execute);
	packet->index = index;
	packet->type = type;
	packet->normalized = normalized;
	packet->value = value;
}
void vertexAttribP2uiv(
	GLuint index,
	GLenum type,
	GLboolean normalized,
	const GLuint *value)
{
	RenderCommand_vertexAttribP2uiv* __restrict packet = (RenderCommand_vertexAttribP2uiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribP2uiv::execute);
	packet->index = index;
	packet->type = type;
	packet->normalized = normalized;
	packet->value = value;
}
void vertexAttribP3ui(
	GLuint index,
	GLenum type,
	GLboolean normalized,
	GLuint value)
{
	RenderCommand_vertexAttribP3ui* __restrict packet = (RenderCommand_vertexAttribP3ui*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribP3ui::execute);
	packet->index = index;
	packet->type = type;
	packet->normalized = normalized;
	packet->value = value;
}
void vertexAttribP3uiv(
	GLuint index,
	GLenum type,
	GLboolean normalized,
	const GLuint *value)
{
	RenderCommand_vertexAttribP3uiv* __restrict packet = (RenderCommand_vertexAttribP3uiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribP3uiv::execute);
	packet->index = index;
	packet->type = type;
	packet->normalized = normalized;
	packet->value = value;
}
void vertexAttribP4ui(
	GLuint index,
	GLenum type,
	GLboolean normalized,
	GLuint value)
{
	RenderCommand_vertexAttribP4ui* __restrict packet = (RenderCommand_vertexAttribP4ui*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribP4ui::execute);
	packet->index = index;
	packet->type = type;
	packet->normalized = normalized;
	packet->value = value;
}
void vertexAttribP4uiv(
	GLuint index,
	GLenum type,
	GLboolean normalized,
	const GLuint *value)
{
	RenderCommand_vertexAttribP4uiv* __restrict packet = (RenderCommand_vertexAttribP4uiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribP4uiv::execute);
	packet->index = index;
	packet->type = type;
	packet->normalized = normalized;
	packet->value = value;
}
void minSampleShading(
	GLfloat value)
{
	RenderCommand_minSampleShading* __restrict packet = (RenderCommand_minSampleShading*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_minSampleShading::execute);
	packet->value = value;
}
void blendEquationi(
	GLuint buf,
	GLenum mode)
{
	RenderCommand_blendEquationi* __restrict packet = (RenderCommand_blendEquationi*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_blendEquationi::execute);
	packet->buf = buf;
	packet->mode = mode;
}
void blendEquationSeparatei(
	GLuint buf,
	GLenum modeRGB,
	GLenum modeAlpha)
{
	RenderCommand_blendEquationSeparatei* __restrict packet = (RenderCommand_blendEquationSeparatei*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_blendEquationSeparatei::execute);
	packet->buf = buf;
	packet->modeRGB = modeRGB;
	packet->modeAlpha = modeAlpha;
}
void blendFunci(
	GLuint buf,
	GLenum src,
	GLenum dst)
{
	RenderCommand_blendFunci* __restrict packet = (RenderCommand_blendFunci*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_blendFunci::execute);
	packet->buf = buf;
	packet->src = src;
	packet->dst = dst;
}
void blendFuncSeparatei(
	GLuint buf,
	GLenum srcRGB,
	GLenum dstRGB,
	GLenum srcAlpha,
	GLenum dstAlpha)
{
	RenderCommand_blendFuncSeparatei* __restrict packet = (RenderCommand_blendFuncSeparatei*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_blendFuncSeparatei::execute);
	packet->buf = buf;
	packet->srcRGB = srcRGB;
	packet->dstRGB = dstRGB;
	packet->srcAlpha = srcAlpha;
	packet->dstAlpha = dstAlpha;
}
void drawArraysIndirect(
	GLenum mode,
	const void *indirect)
{
	RenderCommand_drawArraysIndirect* __restrict packet = (RenderCommand_drawArraysIndirect*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawArraysIndirect::execute);
	packet->mode = mode;
	packet->indirect = indirect;
}
void drawElementsIndirect(
	GLenum mode,
	GLenum type,
	const void *indirect)
{
	RenderCommand_drawElementsIndirect* __restrict packet = (RenderCommand_drawElementsIndirect*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawElementsIndirect::execute);
	packet->mode = mode;
	packet->type = type;
	packet->indirect = indirect;
}
void uniform1d(
	GLint location,
	GLdouble x)
{
	RenderCommand_uniform1d* __restrict packet = (RenderCommand_uniform1d*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform1d::execute);
	packet->location = location;
	packet->x = x;
}
void uniform2d(
	GLint location,
	GLdouble x,
	GLdouble y)
{
	RenderCommand_uniform2d* __restrict packet = (RenderCommand_uniform2d*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform2d::execute);
	packet->location = location;
	packet->x = x;
	packet->y = y;
}
void uniform3d(
	GLint location,
	GLdouble x,
	GLdouble y,
	GLdouble z)
{
	RenderCommand_uniform3d* __restrict packet = (RenderCommand_uniform3d*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform3d::execute);
	packet->location = location;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void uniform4d(
	GLint location,
	GLdouble x,
	GLdouble y,
	GLdouble z,
	GLdouble w)
{
	RenderCommand_uniform4d* __restrict packet = (RenderCommand_uniform4d*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform4d::execute);
	packet->location = location;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void uniform1dv(
	GLint location,
	GLsizei count,
	const GLdouble *value)
{
	RenderCommand_uniform1dv* __restrict packet = (RenderCommand_uniform1dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform1dv::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform2dv(
	GLint location,
	GLsizei count,
	const GLdouble *value)
{
	RenderCommand_uniform2dv* __restrict packet = (RenderCommand_uniform2dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform2dv::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform3dv(
	GLint location,
	GLsizei count,
	const GLdouble *value)
{
	RenderCommand_uniform3dv* __restrict packet = (RenderCommand_uniform3dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform3dv::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform4dv(
	GLint location,
	GLsizei count,
	const GLdouble *value)
{
	RenderCommand_uniform4dv* __restrict packet = (RenderCommand_uniform4dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform4dv::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniformMatrix2dv(
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_uniformMatrix2dv* __restrict packet = (RenderCommand_uniformMatrix2dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformMatrix2dv::execute);
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void uniformMatrix3dv(
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_uniformMatrix3dv* __restrict packet = (RenderCommand_uniformMatrix3dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformMatrix3dv::execute);
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void uniformMatrix4dv(
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_uniformMatrix4dv* __restrict packet = (RenderCommand_uniformMatrix4dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformMatrix4dv::execute);
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void uniformMatrix2x3dv(
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_uniformMatrix2x3dv* __restrict packet = (RenderCommand_uniformMatrix2x3dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformMatrix2x3dv::execute);
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void uniformMatrix2x4dv(
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_uniformMatrix2x4dv* __restrict packet = (RenderCommand_uniformMatrix2x4dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformMatrix2x4dv::execute);
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void uniformMatrix3x2dv(
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_uniformMatrix3x2dv* __restrict packet = (RenderCommand_uniformMatrix3x2dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformMatrix3x2dv::execute);
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void uniformMatrix3x4dv(
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_uniformMatrix3x4dv* __restrict packet = (RenderCommand_uniformMatrix3x4dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformMatrix3x4dv::execute);
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void uniformMatrix4x2dv(
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_uniformMatrix4x2dv* __restrict packet = (RenderCommand_uniformMatrix4x2dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformMatrix4x2dv::execute);
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void uniformMatrix4x3dv(
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_uniformMatrix4x3dv* __restrict packet = (RenderCommand_uniformMatrix4x3dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformMatrix4x3dv::execute);
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void getUniformdv(
	GLuint program,
	GLint location,
	GLdouble *params)
{
	RenderCommand_getUniformdv* __restrict packet = (RenderCommand_getUniformdv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getUniformdv::execute);
	packet->program = program;
	packet->location = location;
	packet->params = params;
}
GLint getSubroutineUniformLocation(
	GLuint program,
	GLenum shadertype,
	const GLchar *name)
{
	RenderCommand_getSubroutineUniformLocation* __restrict packet = (RenderCommand_getSubroutineUniformLocation*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getSubroutineUniformLocation::execute);
	packet->program = program;
	packet->shadertype = shadertype;
	packet->name = name;
}
GLuint getSubroutineIndex(
	GLuint program,
	GLenum shadertype,
	const GLchar *name)
{
	RenderCommand_getSubroutineIndex* __restrict packet = (RenderCommand_getSubroutineIndex*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getSubroutineIndex::execute);
	packet->program = program;
	packet->shadertype = shadertype;
	packet->name = name;
}
void getActiveSubroutineUniformiv(
	GLuint program,
	GLenum shadertype,
	GLuint index,
	GLenum pname,
	GLint *values)
{
	RenderCommand_getActiveSubroutineUniformiv* __restrict packet = (RenderCommand_getActiveSubroutineUniformiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getActiveSubroutineUniformiv::execute);
	packet->program = program;
	packet->shadertype = shadertype;
	packet->index = index;
	packet->pname = pname;
	packet->values = values;
}
void getActiveSubroutineUniformName(
	GLuint program,
	GLenum shadertype,
	GLuint index,
	GLsizei bufsize,
	GLsizei *length,
	GLchar *name)
{
	RenderCommand_getActiveSubroutineUniformName* __restrict packet = (RenderCommand_getActiveSubroutineUniformName*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getActiveSubroutineUniformName::execute);
	packet->program = program;
	packet->shadertype = shadertype;
	packet->index = index;
	packet->bufsize = bufsize;
	packet->length = length;
	packet->name = name;
}
void getActiveSubroutineName(
	GLuint program,
	GLenum shadertype,
	GLuint index,
	GLsizei bufsize,
	GLsizei *length,
	GLchar *name)
{
	RenderCommand_getActiveSubroutineName* __restrict packet = (RenderCommand_getActiveSubroutineName*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getActiveSubroutineName::execute);
	packet->program = program;
	packet->shadertype = shadertype;
	packet->index = index;
	packet->bufsize = bufsize;
	packet->length = length;
	packet->name = name;
}
void uniformSubroutinesuiv(
	GLenum shadertype,
	GLsizei count,
	const GLuint *indices)
{
	RenderCommand_uniformSubroutinesuiv* __restrict packet = (RenderCommand_uniformSubroutinesuiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformSubroutinesuiv::execute);
	packet->shadertype = shadertype;
	packet->count = count;
	packet->indices = indices;
}
void getUniformSubroutineuiv(
	GLenum shadertype,
	GLint location,
	GLuint *params)
{
	RenderCommand_getUniformSubroutineuiv* __restrict packet = (RenderCommand_getUniformSubroutineuiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getUniformSubroutineuiv::execute);
	packet->shadertype = shadertype;
	packet->location = location;
	packet->params = params;
}
void getProgramStageiv(
	GLuint program,
	GLenum shadertype,
	GLenum pname,
	GLint *values)
{
	RenderCommand_getProgramStageiv* __restrict packet = (RenderCommand_getProgramStageiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getProgramStageiv::execute);
	packet->program = program;
	packet->shadertype = shadertype;
	packet->pname = pname;
	packet->values = values;
}
void patchParameteri(
	GLenum pname,
	GLint value)
{
	RenderCommand_patchParameteri* __restrict packet = (RenderCommand_patchParameteri*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_patchParameteri::execute);
	packet->pname = pname;
	packet->value = value;
}
void patchParameterfv(
	GLenum pname,
	const GLfloat *values)
{
	RenderCommand_patchParameterfv* __restrict packet = (RenderCommand_patchParameterfv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_patchParameterfv::execute);
	packet->pname = pname;
	packet->values = values;
}
void bindTransformFeedback(
	GLenum target,
	GLuint id)
{
	RenderCommand_bindTransformFeedback* __restrict packet = (RenderCommand_bindTransformFeedback*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindTransformFeedback::execute);
	packet->target = target;
	packet->id = id;
}
void deleteTransformFeedbacks(
	GLsizei n,
	const GLuint *ids)
{
	RenderCommand_deleteTransformFeedbacks* __restrict packet = (RenderCommand_deleteTransformFeedbacks*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_deleteTransformFeedbacks::execute);
	packet->n = n;
	packet->ids = ids;
}
void genTransformFeedbacks(
	GLsizei n,
	GLuint *ids)
{
	RenderCommand_genTransformFeedbacks* __restrict packet = (RenderCommand_genTransformFeedbacks*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_genTransformFeedbacks::execute);
	packet->n = n;
	packet->ids = ids;
}
GLboolean isTransformFeedback(
	GLuint id)
{
	RenderCommand_isTransformFeedback* __restrict packet = (RenderCommand_isTransformFeedback*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isTransformFeedback::execute);
	packet->id = id;
}
void pauseTransformFeedback(
	void)
{
	RenderCommand_pauseTransformFeedback* __restrict packet = (RenderCommand_pauseTransformFeedback*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pauseTransformFeedback::execute);
}
void resumeTransformFeedback(
	void)
{
	RenderCommand_resumeTransformFeedback* __restrict packet = (RenderCommand_resumeTransformFeedback*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_resumeTransformFeedback::execute);
}
void drawTransformFeedback(
	GLenum mode,
	GLuint id)
{
	RenderCommand_drawTransformFeedback* __restrict packet = (RenderCommand_drawTransformFeedback*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawTransformFeedback::execute);
	packet->mode = mode;
	packet->id = id;
}
void drawTransformFeedbackStream(
	GLenum mode,
	GLuint id,
	GLuint stream)
{
	RenderCommand_drawTransformFeedbackStream* __restrict packet = (RenderCommand_drawTransformFeedbackStream*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawTransformFeedbackStream::execute);
	packet->mode = mode;
	packet->id = id;
	packet->stream = stream;
}
void beginQueryIndexed(
	GLenum target,
	GLuint index,
	GLuint id)
{
	RenderCommand_beginQueryIndexed* __restrict packet = (RenderCommand_beginQueryIndexed*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_beginQueryIndexed::execute);
	packet->target = target;
	packet->index = index;
	packet->id = id;
}
void endQueryIndexed(
	GLenum target,
	GLuint index)
{
	RenderCommand_endQueryIndexed* __restrict packet = (RenderCommand_endQueryIndexed*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_endQueryIndexed::execute);
	packet->target = target;
	packet->index = index;
}
void getQueryIndexediv(
	GLenum target,
	GLuint index,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getQueryIndexediv* __restrict packet = (RenderCommand_getQueryIndexediv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getQueryIndexediv::execute);
	packet->target = target;
	packet->index = index;
	packet->pname = pname;
	packet->params = params;
}
void releaseShaderCompiler(
	void)
{
	RenderCommand_releaseShaderCompiler* __restrict packet = (RenderCommand_releaseShaderCompiler*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_releaseShaderCompiler::execute);
}
void shaderBinary(
	GLsizei count,
	const GLuint *shaders,
	GLenum binaryformat,
	const void *binary,
	GLsizei length)
{
	RenderCommand_shaderBinary* __restrict packet = (RenderCommand_shaderBinary*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_shaderBinary::execute);
	packet->count = count;
	packet->shaders = shaders;
	packet->binaryformat = binaryformat;
	packet->binary = binary;
	packet->length = length;
}
void getShaderPrecisionFormat(
	GLenum shadertype,
	GLenum precisiontype,
	GLint *range,
	GLint *precision)
{
	RenderCommand_getShaderPrecisionFormat* __restrict packet = (RenderCommand_getShaderPrecisionFormat*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getShaderPrecisionFormat::execute);
	packet->shadertype = shadertype;
	packet->precisiontype = precisiontype;
	packet->range = range;
	packet->precision = precision;
}
void depthRangef(
	GLfloat n,
	GLfloat f)
{
	RenderCommand_depthRangef* __restrict packet = (RenderCommand_depthRangef*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_depthRangef::execute);
	packet->n = n;
	packet->f = f;
}
void clearDepthf(
	GLfloat d)
{
	RenderCommand_clearDepthf* __restrict packet = (RenderCommand_clearDepthf*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clearDepthf::execute);
	packet->d = d;
}
void getProgramBinary(
	GLuint program,
	GLsizei bufSize,
	GLsizei *length,
	GLenum *binaryFormat,
	void *binary)
{
	RenderCommand_getProgramBinary* __restrict packet = (RenderCommand_getProgramBinary*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getProgramBinary::execute);
	packet->program = program;
	packet->bufSize = bufSize;
	packet->length = length;
	packet->binaryFormat = binaryFormat;
	packet->binary = binary;
}
void programBinary(
	GLuint program,
	GLenum binaryFormat,
	const void *binary,
	GLsizei length)
{
	RenderCommand_programBinary* __restrict packet = (RenderCommand_programBinary*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programBinary::execute);
	packet->program = program;
	packet->binaryFormat = binaryFormat;
	packet->binary = binary;
	packet->length = length;
}
void programParameteri(
	GLuint program,
	GLenum pname,
	GLint value)
{
	RenderCommand_programParameteri* __restrict packet = (RenderCommand_programParameteri*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programParameteri::execute);
	packet->program = program;
	packet->pname = pname;
	packet->value = value;
}
void useProgramStages(
	GLuint pipeline,
	GLbitfield stages,
	GLuint program)
{
	RenderCommand_useProgramStages* __restrict packet = (RenderCommand_useProgramStages*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_useProgramStages::execute);
	packet->pipeline = pipeline;
	packet->stages = stages;
	packet->program = program;
}
void activeShaderProgram(
	GLuint pipeline,
	GLuint program)
{
	RenderCommand_activeShaderProgram* __restrict packet = (RenderCommand_activeShaderProgram*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_activeShaderProgram::execute);
	packet->pipeline = pipeline;
	packet->program = program;
}
GLuint createShaderProgramv(
	GLenum type,
	GLsizei count,
	const GLchar *const*strings)
{
	RenderCommand_createShaderProgramv* __restrict packet = (RenderCommand_createShaderProgramv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_createShaderProgramv::execute);
	packet->type = type;
	packet->count = count;
	packet->strings = strings;
}
void bindProgramPipeline(
	GLuint pipeline)
{
	RenderCommand_bindProgramPipeline* __restrict packet = (RenderCommand_bindProgramPipeline*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindProgramPipeline::execute);
	packet->pipeline = pipeline;
}
void deleteProgramPipelines(
	GLsizei n,
	const GLuint *pipelines)
{
	RenderCommand_deleteProgramPipelines* __restrict packet = (RenderCommand_deleteProgramPipelines*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_deleteProgramPipelines::execute);
	packet->n = n;
	packet->pipelines = pipelines;
}
void genProgramPipelines(
	GLsizei n,
	GLuint *pipelines)
{
	RenderCommand_genProgramPipelines* __restrict packet = (RenderCommand_genProgramPipelines*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_genProgramPipelines::execute);
	packet->n = n;
	packet->pipelines = pipelines;
}
GLboolean isProgramPipeline(
	GLuint pipeline)
{
	RenderCommand_isProgramPipeline* __restrict packet = (RenderCommand_isProgramPipeline*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isProgramPipeline::execute);
	packet->pipeline = pipeline;
}
void getProgramPipelineiv(
	GLuint pipeline,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getProgramPipelineiv* __restrict packet = (RenderCommand_getProgramPipelineiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getProgramPipelineiv::execute);
	packet->pipeline = pipeline;
	packet->pname = pname;
	packet->params = params;
}
void programUniform1i(
	GLuint program,
	GLint location,
	GLint v0)
{
	RenderCommand_programUniform1i* __restrict packet = (RenderCommand_programUniform1i*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1i::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
}
void programUniform1iv(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLint *value)
{
	RenderCommand_programUniform1iv* __restrict packet = (RenderCommand_programUniform1iv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1iv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform1f(
	GLuint program,
	GLint location,
	GLfloat v0)
{
	RenderCommand_programUniform1f* __restrict packet = (RenderCommand_programUniform1f*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1f::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
}
void programUniform1fv(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLfloat *value)
{
	RenderCommand_programUniform1fv* __restrict packet = (RenderCommand_programUniform1fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1fv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform1d(
	GLuint program,
	GLint location,
	GLdouble v0)
{
	RenderCommand_programUniform1d* __restrict packet = (RenderCommand_programUniform1d*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1d::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
}
void programUniform1dv(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLdouble *value)
{
	RenderCommand_programUniform1dv* __restrict packet = (RenderCommand_programUniform1dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1dv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform1ui(
	GLuint program,
	GLint location,
	GLuint v0)
{
	RenderCommand_programUniform1ui* __restrict packet = (RenderCommand_programUniform1ui*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1ui::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
}
void programUniform1uiv(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLuint *value)
{
	RenderCommand_programUniform1uiv* __restrict packet = (RenderCommand_programUniform1uiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1uiv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform2i(
	GLuint program,
	GLint location,
	GLint v0,
	GLint v1)
{
	RenderCommand_programUniform2i* __restrict packet = (RenderCommand_programUniform2i*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2i::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
}
void programUniform2iv(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLint *value)
{
	RenderCommand_programUniform2iv* __restrict packet = (RenderCommand_programUniform2iv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2iv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform2f(
	GLuint program,
	GLint location,
	GLfloat v0,
	GLfloat v1)
{
	RenderCommand_programUniform2f* __restrict packet = (RenderCommand_programUniform2f*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2f::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
}
void programUniform2fv(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLfloat *value)
{
	RenderCommand_programUniform2fv* __restrict packet = (RenderCommand_programUniform2fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2fv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform2d(
	GLuint program,
	GLint location,
	GLdouble v0,
	GLdouble v1)
{
	RenderCommand_programUniform2d* __restrict packet = (RenderCommand_programUniform2d*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2d::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
}
void programUniform2dv(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLdouble *value)
{
	RenderCommand_programUniform2dv* __restrict packet = (RenderCommand_programUniform2dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2dv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform2ui(
	GLuint program,
	GLint location,
	GLuint v0,
	GLuint v1)
{
	RenderCommand_programUniform2ui* __restrict packet = (RenderCommand_programUniform2ui*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2ui::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
}
void programUniform2uiv(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLuint *value)
{
	RenderCommand_programUniform2uiv* __restrict packet = (RenderCommand_programUniform2uiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2uiv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform3i(
	GLuint program,
	GLint location,
	GLint v0,
	GLint v1,
	GLint v2)
{
	RenderCommand_programUniform3i* __restrict packet = (RenderCommand_programUniform3i*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3i::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
	packet->v2 = v2;
}
void programUniform3iv(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLint *value)
{
	RenderCommand_programUniform3iv* __restrict packet = (RenderCommand_programUniform3iv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3iv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform3f(
	GLuint program,
	GLint location,
	GLfloat v0,
	GLfloat v1,
	GLfloat v2)
{
	RenderCommand_programUniform3f* __restrict packet = (RenderCommand_programUniform3f*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3f::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
	packet->v2 = v2;
}
void programUniform3fv(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLfloat *value)
{
	RenderCommand_programUniform3fv* __restrict packet = (RenderCommand_programUniform3fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3fv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform3d(
	GLuint program,
	GLint location,
	GLdouble v0,
	GLdouble v1,
	GLdouble v2)
{
	RenderCommand_programUniform3d* __restrict packet = (RenderCommand_programUniform3d*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3d::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
	packet->v2 = v2;
}
void programUniform3dv(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLdouble *value)
{
	RenderCommand_programUniform3dv* __restrict packet = (RenderCommand_programUniform3dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3dv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform3ui(
	GLuint program,
	GLint location,
	GLuint v0,
	GLuint v1,
	GLuint v2)
{
	RenderCommand_programUniform3ui* __restrict packet = (RenderCommand_programUniform3ui*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3ui::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
	packet->v2 = v2;
}
void programUniform3uiv(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLuint *value)
{
	RenderCommand_programUniform3uiv* __restrict packet = (RenderCommand_programUniform3uiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3uiv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform4i(
	GLuint program,
	GLint location,
	GLint v0,
	GLint v1,
	GLint v2,
	GLint v3)
{
	RenderCommand_programUniform4i* __restrict packet = (RenderCommand_programUniform4i*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4i::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
	packet->v2 = v2;
	packet->v3 = v3;
}
void programUniform4iv(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLint *value)
{
	RenderCommand_programUniform4iv* __restrict packet = (RenderCommand_programUniform4iv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4iv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform4f(
	GLuint program,
	GLint location,
	GLfloat v0,
	GLfloat v1,
	GLfloat v2,
	GLfloat v3)
{
	RenderCommand_programUniform4f* __restrict packet = (RenderCommand_programUniform4f*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4f::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
	packet->v2 = v2;
	packet->v3 = v3;
}
void programUniform4fv(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLfloat *value)
{
	RenderCommand_programUniform4fv* __restrict packet = (RenderCommand_programUniform4fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4fv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform4d(
	GLuint program,
	GLint location,
	GLdouble v0,
	GLdouble v1,
	GLdouble v2,
	GLdouble v3)
{
	RenderCommand_programUniform4d* __restrict packet = (RenderCommand_programUniform4d*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4d::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
	packet->v2 = v2;
	packet->v3 = v3;
}
void programUniform4dv(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLdouble *value)
{
	RenderCommand_programUniform4dv* __restrict packet = (RenderCommand_programUniform4dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4dv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform4ui(
	GLuint program,
	GLint location,
	GLuint v0,
	GLuint v1,
	GLuint v2,
	GLuint v3)
{
	RenderCommand_programUniform4ui* __restrict packet = (RenderCommand_programUniform4ui*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4ui::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
	packet->v2 = v2;
	packet->v3 = v3;
}
void programUniform4uiv(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLuint *value)
{
	RenderCommand_programUniform4uiv* __restrict packet = (RenderCommand_programUniform4uiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4uiv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniformMatrix2fv(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_programUniformMatrix2fv* __restrict packet = (RenderCommand_programUniformMatrix2fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix2fv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix3fv(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_programUniformMatrix3fv* __restrict packet = (RenderCommand_programUniformMatrix3fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix3fv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix4fv(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_programUniformMatrix4fv* __restrict packet = (RenderCommand_programUniformMatrix4fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix4fv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix2dv(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_programUniformMatrix2dv* __restrict packet = (RenderCommand_programUniformMatrix2dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix2dv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix3dv(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_programUniformMatrix3dv* __restrict packet = (RenderCommand_programUniformMatrix3dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix3dv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix4dv(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_programUniformMatrix4dv* __restrict packet = (RenderCommand_programUniformMatrix4dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix4dv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix2x3fv(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_programUniformMatrix2x3fv* __restrict packet = (RenderCommand_programUniformMatrix2x3fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix2x3fv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix3x2fv(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_programUniformMatrix3x2fv* __restrict packet = (RenderCommand_programUniformMatrix3x2fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix3x2fv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix2x4fv(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_programUniformMatrix2x4fv* __restrict packet = (RenderCommand_programUniformMatrix2x4fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix2x4fv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix4x2fv(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_programUniformMatrix4x2fv* __restrict packet = (RenderCommand_programUniformMatrix4x2fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix4x2fv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix3x4fv(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_programUniformMatrix3x4fv* __restrict packet = (RenderCommand_programUniformMatrix3x4fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix3x4fv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix4x3fv(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_programUniformMatrix4x3fv* __restrict packet = (RenderCommand_programUniformMatrix4x3fv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix4x3fv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix2x3dv(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_programUniformMatrix2x3dv* __restrict packet = (RenderCommand_programUniformMatrix2x3dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix2x3dv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix3x2dv(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_programUniformMatrix3x2dv* __restrict packet = (RenderCommand_programUniformMatrix3x2dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix3x2dv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix2x4dv(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_programUniformMatrix2x4dv* __restrict packet = (RenderCommand_programUniformMatrix2x4dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix2x4dv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix4x2dv(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_programUniformMatrix4x2dv* __restrict packet = (RenderCommand_programUniformMatrix4x2dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix4x2dv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix3x4dv(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_programUniformMatrix3x4dv* __restrict packet = (RenderCommand_programUniformMatrix3x4dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix3x4dv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix4x3dv(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_programUniformMatrix4x3dv* __restrict packet = (RenderCommand_programUniformMatrix4x3dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix4x3dv::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void validateProgramPipeline(
	GLuint pipeline)
{
	RenderCommand_validateProgramPipeline* __restrict packet = (RenderCommand_validateProgramPipeline*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_validateProgramPipeline::execute);
	packet->pipeline = pipeline;
}
void getProgramPipelineInfoLog(
	GLuint pipeline,
	GLsizei bufSize,
	GLsizei *length,
	GLchar *infoLog)
{
	RenderCommand_getProgramPipelineInfoLog* __restrict packet = (RenderCommand_getProgramPipelineInfoLog*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getProgramPipelineInfoLog::execute);
	packet->pipeline = pipeline;
	packet->bufSize = bufSize;
	packet->length = length;
	packet->infoLog = infoLog;
}
void vertexAttribL1d(
	GLuint index,
	GLdouble x)
{
	RenderCommand_vertexAttribL1d* __restrict packet = (RenderCommand_vertexAttribL1d*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL1d::execute);
	packet->index = index;
	packet->x = x;
}
void vertexAttribL2d(
	GLuint index,
	GLdouble x,
	GLdouble y)
{
	RenderCommand_vertexAttribL2d* __restrict packet = (RenderCommand_vertexAttribL2d*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL2d::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
}
void vertexAttribL3d(
	GLuint index,
	GLdouble x,
	GLdouble y,
	GLdouble z)
{
	RenderCommand_vertexAttribL3d* __restrict packet = (RenderCommand_vertexAttribL3d*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL3d::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void vertexAttribL4d(
	GLuint index,
	GLdouble x,
	GLdouble y,
	GLdouble z,
	GLdouble w)
{
	RenderCommand_vertexAttribL4d* __restrict packet = (RenderCommand_vertexAttribL4d*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL4d::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void vertexAttribL1dv(
	GLuint index,
	const GLdouble *v)
{
	RenderCommand_vertexAttribL1dv* __restrict packet = (RenderCommand_vertexAttribL1dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL1dv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribL2dv(
	GLuint index,
	const GLdouble *v)
{
	RenderCommand_vertexAttribL2dv* __restrict packet = (RenderCommand_vertexAttribL2dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL2dv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribL3dv(
	GLuint index,
	const GLdouble *v)
{
	RenderCommand_vertexAttribL3dv* __restrict packet = (RenderCommand_vertexAttribL3dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL3dv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribL4dv(
	GLuint index,
	const GLdouble *v)
{
	RenderCommand_vertexAttribL4dv* __restrict packet = (RenderCommand_vertexAttribL4dv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL4dv::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribLPointer(
	GLuint index,
	GLint size,
	GLenum type,
	GLsizei stride,
	const void *pointer)
{
	RenderCommand_vertexAttribLPointer* __restrict packet = (RenderCommand_vertexAttribLPointer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribLPointer::execute);
	packet->index = index;
	packet->size = size;
	packet->type = type;
	packet->stride = stride;
	packet->pointer = pointer;
}
void getVertexAttribLdv(
	GLuint index,
	GLenum pname,
	GLdouble *params)
{
	RenderCommand_getVertexAttribLdv* __restrict packet = (RenderCommand_getVertexAttribLdv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getVertexAttribLdv::execute);
	packet->index = index;
	packet->pname = pname;
	packet->params = params;
}
void viewportArrayv(
	GLuint first,
	GLsizei count,
	const GLfloat *v)
{
	RenderCommand_viewportArrayv* __restrict packet = (RenderCommand_viewportArrayv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_viewportArrayv::execute);
	packet->first = first;
	packet->count = count;
	packet->v = v;
}
void viewportIndexedf(
	GLuint index,
	GLfloat x,
	GLfloat y,
	GLfloat w,
	GLfloat h)
{
	RenderCommand_viewportIndexedf* __restrict packet = (RenderCommand_viewportIndexedf*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_viewportIndexedf::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->w = w;
	packet->h = h;
}
void viewportIndexedfv(
	GLuint index,
	const GLfloat *v)
{
	RenderCommand_viewportIndexedfv* __restrict packet = (RenderCommand_viewportIndexedfv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_viewportIndexedfv::execute);
	packet->index = index;
	packet->v = v;
}
void scissorArrayv(
	GLuint first,
	GLsizei count,
	const GLint *v)
{
	RenderCommand_scissorArrayv* __restrict packet = (RenderCommand_scissorArrayv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_scissorArrayv::execute);
	packet->first = first;
	packet->count = count;
	packet->v = v;
}
void scissorIndexed(
	GLuint index,
	GLint left,
	GLint bottom,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_scissorIndexed* __restrict packet = (RenderCommand_scissorIndexed*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_scissorIndexed::execute);
	packet->index = index;
	packet->left = left;
	packet->bottom = bottom;
	packet->width = width;
	packet->height = height;
}
void scissorIndexedv(
	GLuint index,
	const GLint *v)
{
	RenderCommand_scissorIndexedv* __restrict packet = (RenderCommand_scissorIndexedv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_scissorIndexedv::execute);
	packet->index = index;
	packet->v = v;
}
void depthRangeArrayv(
	GLuint first,
	GLsizei count,
	const GLdouble *v)
{
	RenderCommand_depthRangeArrayv* __restrict packet = (RenderCommand_depthRangeArrayv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_depthRangeArrayv::execute);
	packet->first = first;
	packet->count = count;
	packet->v = v;
}
void depthRangeIndexed(
	GLuint index,
	GLdouble n,
	GLdouble f)
{
	RenderCommand_depthRangeIndexed* __restrict packet = (RenderCommand_depthRangeIndexed*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_depthRangeIndexed::execute);
	packet->index = index;
	packet->n = n;
	packet->f = f;
}
void getFloati_v(
	GLenum target,
	GLuint index,
	GLfloat *data)
{
	RenderCommand_getFloati_v* __restrict packet = (RenderCommand_getFloati_v*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getFloati_v::execute);
	packet->target = target;
	packet->index = index;
	packet->data = data;
}
void getDoublei_v(
	GLenum target,
	GLuint index,
	GLdouble *data)
{
	RenderCommand_getDoublei_v* __restrict packet = (RenderCommand_getDoublei_v*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getDoublei_v::execute);
	packet->target = target;
	packet->index = index;
	packet->data = data;
}
void drawArraysInstancedBaseInstance(
	GLenum mode,
	GLint first,
	GLsizei count,
	GLsizei instancecount,
	GLuint baseinstance)
{
	RenderCommand_drawArraysInstancedBaseInstance* __restrict packet = (RenderCommand_drawArraysInstancedBaseInstance*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawArraysInstancedBaseInstance::execute);
	packet->mode = mode;
	packet->first = first;
	packet->count = count;
	packet->instancecount = instancecount;
	packet->baseinstance = baseinstance;
}
void drawElementsInstancedBaseInstance(
	GLenum mode,
	GLsizei count,
	GLenum type,
	const void *indices,
	GLsizei instancecount,
	GLuint baseinstance)
{
	RenderCommand_drawElementsInstancedBaseInstance* __restrict packet = (RenderCommand_drawElementsInstancedBaseInstance*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawElementsInstancedBaseInstance::execute);
	packet->mode = mode;
	packet->count = count;
	packet->type = type;
	packet->indices = indices;
	packet->instancecount = instancecount;
	packet->baseinstance = baseinstance;
}
void drawElementsInstancedBaseVertexBaseInstance(
	GLenum mode,
	GLsizei count,
	GLenum type,
	const void *indices,
	GLsizei instancecount,
	GLint basevertex,
	GLuint baseinstance)
{
	RenderCommand_drawElementsInstancedBaseVertexBaseInstance* __restrict packet = (RenderCommand_drawElementsInstancedBaseVertexBaseInstance*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawElementsInstancedBaseVertexBaseInstance::execute);
	packet->mode = mode;
	packet->count = count;
	packet->type = type;
	packet->indices = indices;
	packet->instancecount = instancecount;
	packet->basevertex = basevertex;
	packet->baseinstance = baseinstance;
}
void getInternalformativ(
	GLenum target,
	GLenum internalformat,
	GLenum pname,
	GLsizei bufSize,
	GLint *params)
{
	RenderCommand_getInternalformativ* __restrict packet = (RenderCommand_getInternalformativ*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getInternalformativ::execute);
	packet->target = target;
	packet->internalformat = internalformat;
	packet->pname = pname;
	packet->bufSize = bufSize;
	packet->params = params;
}
void getActiveAtomicCounterBufferiv(
	GLuint program,
	GLuint bufferIndex,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getActiveAtomicCounterBufferiv* __restrict packet = (RenderCommand_getActiveAtomicCounterBufferiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getActiveAtomicCounterBufferiv::execute);
	packet->program = program;
	packet->bufferIndex = bufferIndex;
	packet->pname = pname;
	packet->params = params;
}
void bindImageTexture(
	GLuint unit,
	GLuint texture,
	GLint level,
	GLboolean layered,
	GLint layer,
	GLenum access,
	GLenum format)
{
	RenderCommand_bindImageTexture* __restrict packet = (RenderCommand_bindImageTexture*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindImageTexture::execute);
	packet->unit = unit;
	packet->texture = texture;
	packet->level = level;
	packet->layered = layered;
	packet->layer = layer;
	packet->access = access;
	packet->format = format;
}
void memoryBarrier(
	GLbitfield barriers)
{
	RenderCommand_memoryBarrier* __restrict packet = (RenderCommand_memoryBarrier*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_memoryBarrier::execute);
	packet->barriers = barriers;
}
void texStorage1D(
	GLenum target,
	GLsizei levels,
	GLenum internalformat,
	GLsizei width)
{
	RenderCommand_texStorage1D* __restrict packet = (RenderCommand_texStorage1D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texStorage1D::execute);
	packet->target = target;
	packet->levels = levels;
	packet->internalformat = internalformat;
	packet->width = width;
}
void texStorage2D(
	GLenum target,
	GLsizei levels,
	GLenum internalformat,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_texStorage2D* __restrict packet = (RenderCommand_texStorage2D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texStorage2D::execute);
	packet->target = target;
	packet->levels = levels;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
}
void texStorage3D(
	GLenum target,
	GLsizei levels,
	GLenum internalformat,
	GLsizei width,
	GLsizei height,
	GLsizei depth)
{
	RenderCommand_texStorage3D* __restrict packet = (RenderCommand_texStorage3D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texStorage3D::execute);
	packet->target = target;
	packet->levels = levels;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
}
void drawTransformFeedbackInstanced(
	GLenum mode,
	GLuint id,
	GLsizei instancecount)
{
	RenderCommand_drawTransformFeedbackInstanced* __restrict packet = (RenderCommand_drawTransformFeedbackInstanced*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawTransformFeedbackInstanced::execute);
	packet->mode = mode;
	packet->id = id;
	packet->instancecount = instancecount;
}
void drawTransformFeedbackStreamInstanced(
	GLenum mode,
	GLuint id,
	GLuint stream,
	GLsizei instancecount)
{
	RenderCommand_drawTransformFeedbackStreamInstanced* __restrict packet = (RenderCommand_drawTransformFeedbackStreamInstanced*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawTransformFeedbackStreamInstanced::execute);
	packet->mode = mode;
	packet->id = id;
	packet->stream = stream;
	packet->instancecount = instancecount;
}
void clearBufferData(
	GLenum target,
	GLenum internalformat,
	GLenum format,
	GLenum type,
	const void *data)
{
	RenderCommand_clearBufferData* __restrict packet = (RenderCommand_clearBufferData*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clearBufferData::execute);
	packet->target = target;
	packet->internalformat = internalformat;
	packet->format = format;
	packet->type = type;
	packet->data = data;
}
void clearBufferSubData(
	GLenum target,
	GLenum internalformat,
	GLintptr offset,
	GLsizeiptr size,
	GLenum format,
	GLenum type,
	const void *data)
{
	RenderCommand_clearBufferSubData* __restrict packet = (RenderCommand_clearBufferSubData*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clearBufferSubData::execute);
	packet->target = target;
	packet->internalformat = internalformat;
	packet->offset = offset;
	packet->size = size;
	packet->format = format;
	packet->type = type;
	packet->data = data;
}
void dispatchCompute(
	GLuint num_groups_x,
	GLuint num_groups_y,
	GLuint num_groups_z)
{
	RenderCommand_dispatchCompute* __restrict packet = (RenderCommand_dispatchCompute*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_dispatchCompute::execute);
	packet->num_groups_x = num_groups_x;
	packet->num_groups_y = num_groups_y;
	packet->num_groups_z = num_groups_z;
}
void dispatchComputeIndirect(
	GLintptr indirect)
{
	RenderCommand_dispatchComputeIndirect* __restrict packet = (RenderCommand_dispatchComputeIndirect*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_dispatchComputeIndirect::execute);
	packet->indirect = indirect;
}
void copyImageSubData(
	GLuint srcName,
	GLenum srcTarget,
	GLint srcLevel,
	GLint srcX,
	GLint srcY,
	GLint srcZ,
	GLuint dstName,
	GLenum dstTarget,
	GLint dstLevel,
	GLint dstX,
	GLint dstY,
	GLint dstZ,
	GLsizei srcWidth,
	GLsizei srcHeight,
	GLsizei srcDepth)
{
	RenderCommand_copyImageSubData* __restrict packet = (RenderCommand_copyImageSubData*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyImageSubData::execute);
	packet->srcName = srcName;
	packet->srcTarget = srcTarget;
	packet->srcLevel = srcLevel;
	packet->srcX = srcX;
	packet->srcY = srcY;
	packet->srcZ = srcZ;
	packet->dstName = dstName;
	packet->dstTarget = dstTarget;
	packet->dstLevel = dstLevel;
	packet->dstX = dstX;
	packet->dstY = dstY;
	packet->dstZ = dstZ;
	packet->srcWidth = srcWidth;
	packet->srcHeight = srcHeight;
	packet->srcDepth = srcDepth;
}
void framebufferParameteri(
	GLenum target,
	GLenum pname,
	GLint param)
{
	RenderCommand_framebufferParameteri* __restrict packet = (RenderCommand_framebufferParameteri*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_framebufferParameteri::execute);
	packet->target = target;
	packet->pname = pname;
	packet->param = param;
}
void getFramebufferParameteriv(
	GLenum target,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getFramebufferParameteriv* __restrict packet = (RenderCommand_getFramebufferParameteriv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getFramebufferParameteriv::execute);
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void getInternalformati64v(
	GLenum target,
	GLenum internalformat,
	GLenum pname,
	GLsizei bufSize,
	GLint64 *params)
{
	RenderCommand_getInternalformati64v* __restrict packet = (RenderCommand_getInternalformati64v*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getInternalformati64v::execute);
	packet->target = target;
	packet->internalformat = internalformat;
	packet->pname = pname;
	packet->bufSize = bufSize;
	packet->params = params;
}
void invalidateTexSubImage(
	GLuint texture,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint zoffset,
	GLsizei width,
	GLsizei height,
	GLsizei depth)
{
	RenderCommand_invalidateTexSubImage* __restrict packet = (RenderCommand_invalidateTexSubImage*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_invalidateTexSubImage::execute);
	packet->texture = texture;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->zoffset = zoffset;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
}
void invalidateTexImage(
	GLuint texture,
	GLint level)
{
	RenderCommand_invalidateTexImage* __restrict packet = (RenderCommand_invalidateTexImage*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_invalidateTexImage::execute);
	packet->texture = texture;
	packet->level = level;
}
void invalidateBufferSubData(
	GLuint buffer,
	GLintptr offset,
	GLsizeiptr length)
{
	RenderCommand_invalidateBufferSubData* __restrict packet = (RenderCommand_invalidateBufferSubData*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_invalidateBufferSubData::execute);
	packet->buffer = buffer;
	packet->offset = offset;
	packet->length = length;
}
void invalidateBufferData(
	GLuint buffer)
{
	RenderCommand_invalidateBufferData* __restrict packet = (RenderCommand_invalidateBufferData*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_invalidateBufferData::execute);
	packet->buffer = buffer;
}
void invalidateFramebuffer(
	GLenum target,
	GLsizei numAttachments,
	const GLenum *attachments)
{
	RenderCommand_invalidateFramebuffer* __restrict packet = (RenderCommand_invalidateFramebuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_invalidateFramebuffer::execute);
	packet->target = target;
	packet->numAttachments = numAttachments;
	packet->attachments = attachments;
}
void invalidateSubFramebuffer(
	GLenum target,
	GLsizei numAttachments,
	const GLenum *attachments,
	GLint x,
	GLint y,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_invalidateSubFramebuffer* __restrict packet = (RenderCommand_invalidateSubFramebuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_invalidateSubFramebuffer::execute);
	packet->target = target;
	packet->numAttachments = numAttachments;
	packet->attachments = attachments;
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->height = height;
}
void multiDrawArraysIndirect(
	GLenum mode,
	const void *indirect,
	GLsizei drawcount,
	GLsizei stride)
{
	RenderCommand_multiDrawArraysIndirect* __restrict packet = (RenderCommand_multiDrawArraysIndirect*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiDrawArraysIndirect::execute);
	packet->mode = mode;
	packet->indirect = indirect;
	packet->drawcount = drawcount;
	packet->stride = stride;
}
void multiDrawElementsIndirect(
	GLenum mode,
	GLenum type,
	const void *indirect,
	GLsizei drawcount,
	GLsizei stride)
{
	RenderCommand_multiDrawElementsIndirect* __restrict packet = (RenderCommand_multiDrawElementsIndirect*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiDrawElementsIndirect::execute);
	packet->mode = mode;
	packet->type = type;
	packet->indirect = indirect;
	packet->drawcount = drawcount;
	packet->stride = stride;
}
void getProgramInterfaceiv(
	GLuint program,
	GLenum programInterface,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getProgramInterfaceiv* __restrict packet = (RenderCommand_getProgramInterfaceiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getProgramInterfaceiv::execute);
	packet->program = program;
	packet->programInterface = programInterface;
	packet->pname = pname;
	packet->params = params;
}
GLuint getProgramResourceIndex(
	GLuint program,
	GLenum programInterface,
	const GLchar *name)
{
	RenderCommand_getProgramResourceIndex* __restrict packet = (RenderCommand_getProgramResourceIndex*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getProgramResourceIndex::execute);
	packet->program = program;
	packet->programInterface = programInterface;
	packet->name = name;
}
void getProgramResourceName(
	GLuint program,
	GLenum programInterface,
	GLuint index,
	GLsizei bufSize,
	GLsizei *length,
	GLchar *name)
{
	RenderCommand_getProgramResourceName* __restrict packet = (RenderCommand_getProgramResourceName*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getProgramResourceName::execute);
	packet->program = program;
	packet->programInterface = programInterface;
	packet->index = index;
	packet->bufSize = bufSize;
	packet->length = length;
	packet->name = name;
}
void getProgramResourceiv(
	GLuint program,
	GLenum programInterface,
	GLuint index,
	GLsizei propCount,
	const GLenum *props,
	GLsizei bufSize,
	GLsizei *length,
	GLint *params)
{
	RenderCommand_getProgramResourceiv* __restrict packet = (RenderCommand_getProgramResourceiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getProgramResourceiv::execute);
	packet->program = program;
	packet->programInterface = programInterface;
	packet->index = index;
	packet->propCount = propCount;
	packet->props = props;
	packet->bufSize = bufSize;
	packet->length = length;
	packet->params = params;
}
GLint getProgramResourceLocation(
	GLuint program,
	GLenum programInterface,
	const GLchar *name)
{
	RenderCommand_getProgramResourceLocation* __restrict packet = (RenderCommand_getProgramResourceLocation*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getProgramResourceLocation::execute);
	packet->program = program;
	packet->programInterface = programInterface;
	packet->name = name;
}
GLint getProgramResourceLocationIndex(
	GLuint program,
	GLenum programInterface,
	const GLchar *name)
{
	RenderCommand_getProgramResourceLocationIndex* __restrict packet = (RenderCommand_getProgramResourceLocationIndex*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getProgramResourceLocationIndex::execute);
	packet->program = program;
	packet->programInterface = programInterface;
	packet->name = name;
}
void shaderStorageBlockBinding(
	GLuint program,
	GLuint storageBlockIndex,
	GLuint storageBlockBinding)
{
	RenderCommand_shaderStorageBlockBinding* __restrict packet = (RenderCommand_shaderStorageBlockBinding*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_shaderStorageBlockBinding::execute);
	packet->program = program;
	packet->storageBlockIndex = storageBlockIndex;
	packet->storageBlockBinding = storageBlockBinding;
}
void texBufferRange(
	GLenum target,
	GLenum internalformat,
	GLuint buffer,
	GLintptr offset,
	GLsizeiptr size)
{
	RenderCommand_texBufferRange* __restrict packet = (RenderCommand_texBufferRange*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texBufferRange::execute);
	packet->target = target;
	packet->internalformat = internalformat;
	packet->buffer = buffer;
	packet->offset = offset;
	packet->size = size;
}
void texStorage2DMultisample(
	GLenum target,
	GLsizei samples,
	GLenum internalformat,
	GLsizei width,
	GLsizei height,
	GLboolean fixedsamplelocations)
{
	RenderCommand_texStorage2DMultisample* __restrict packet = (RenderCommand_texStorage2DMultisample*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texStorage2DMultisample::execute);
	packet->target = target;
	packet->samples = samples;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->fixedsamplelocations = fixedsamplelocations;
}
void texStorage3DMultisample(
	GLenum target,
	GLsizei samples,
	GLenum internalformat,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLboolean fixedsamplelocations)
{
	RenderCommand_texStorage3DMultisample* __restrict packet = (RenderCommand_texStorage3DMultisample*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texStorage3DMultisample::execute);
	packet->target = target;
	packet->samples = samples;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->fixedsamplelocations = fixedsamplelocations;
}
void textureView(
	GLuint texture,
	GLenum target,
	GLuint origtexture,
	GLenum internalformat,
	GLuint minlevel,
	GLuint numlevels,
	GLuint minlayer,
	GLuint numlayers)
{
	RenderCommand_textureView* __restrict packet = (RenderCommand_textureView*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureView::execute);
	packet->texture = texture;
	packet->target = target;
	packet->origtexture = origtexture;
	packet->internalformat = internalformat;
	packet->minlevel = minlevel;
	packet->numlevels = numlevels;
	packet->minlayer = minlayer;
	packet->numlayers = numlayers;
}
void bindVertexBuffer(
	GLuint bindingindex,
	GLuint buffer,
	GLintptr offset,
	GLsizei stride)
{
	RenderCommand_bindVertexBuffer* __restrict packet = (RenderCommand_bindVertexBuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindVertexBuffer::execute);
	packet->bindingindex = bindingindex;
	packet->buffer = buffer;
	packet->offset = offset;
	packet->stride = stride;
}
void vertexAttribFormat(
	GLuint attribindex,
	GLint size,
	GLenum type,
	GLboolean normalized,
	GLuint relativeoffset)
{
	RenderCommand_vertexAttribFormat* __restrict packet = (RenderCommand_vertexAttribFormat*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribFormat::execute);
	packet->attribindex = attribindex;
	packet->size = size;
	packet->type = type;
	packet->normalized = normalized;
	packet->relativeoffset = relativeoffset;
}
void vertexAttribIFormat(
	GLuint attribindex,
	GLint size,
	GLenum type,
	GLuint relativeoffset)
{
	RenderCommand_vertexAttribIFormat* __restrict packet = (RenderCommand_vertexAttribIFormat*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribIFormat::execute);
	packet->attribindex = attribindex;
	packet->size = size;
	packet->type = type;
	packet->relativeoffset = relativeoffset;
}
void vertexAttribLFormat(
	GLuint attribindex,
	GLint size,
	GLenum type,
	GLuint relativeoffset)
{
	RenderCommand_vertexAttribLFormat* __restrict packet = (RenderCommand_vertexAttribLFormat*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribLFormat::execute);
	packet->attribindex = attribindex;
	packet->size = size;
	packet->type = type;
	packet->relativeoffset = relativeoffset;
}
void vertexAttribBinding(
	GLuint attribindex,
	GLuint bindingindex)
{
	RenderCommand_vertexAttribBinding* __restrict packet = (RenderCommand_vertexAttribBinding*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribBinding::execute);
	packet->attribindex = attribindex;
	packet->bindingindex = bindingindex;
}
void vertexBindingDivisor(
	GLuint bindingindex,
	GLuint divisor)
{
	RenderCommand_vertexBindingDivisor* __restrict packet = (RenderCommand_vertexBindingDivisor*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexBindingDivisor::execute);
	packet->bindingindex = bindingindex;
	packet->divisor = divisor;
}
void debugMessageControl(
	GLenum source,
	GLenum type,
	GLenum severity,
	GLsizei count,
	const GLuint *ids,
	GLboolean enabled)
{
	RenderCommand_debugMessageControl* __restrict packet = (RenderCommand_debugMessageControl*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_debugMessageControl::execute);
	packet->source = source;
	packet->type = type;
	packet->severity = severity;
	packet->count = count;
	packet->ids = ids;
	packet->enabled = enabled;
}
void debugMessageInsert(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *buf)
{
	RenderCommand_debugMessageInsert* __restrict packet = (RenderCommand_debugMessageInsert*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_debugMessageInsert::execute);
	packet->source = source;
	packet->type = type;
	packet->id = id;
	packet->severity = severity;
	packet->length = length;
	packet->buf = buf;
}
void debugMessageCallback(
	GLDEBUGPROC callback,
	const void *userParam)
{
	RenderCommand_debugMessageCallback* __restrict packet = (RenderCommand_debugMessageCallback*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_debugMessageCallback::execute);
	packet->callback = callback;
	packet->userParam = userParam;
}
GLuint getDebugMessageLog(
	GLuint count,
	GLsizei bufSize,
	GLenum *sources,
	GLenum *types,
	GLuint *ids,
	GLenum *severities,
	GLsizei *lengths,
	GLchar *messageLog)
{
	RenderCommand_getDebugMessageLog* __restrict packet = (RenderCommand_getDebugMessageLog*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getDebugMessageLog::execute);
	packet->count = count;
	packet->bufSize = bufSize;
	packet->sources = sources;
	packet->types = types;
	packet->ids = ids;
	packet->severities = severities;
	packet->lengths = lengths;
	packet->messageLog = messageLog;
}
void pushDebugGroup(
	GLenum source,
	GLuint id,
	GLsizei length,
	const GLchar *message)
{
	RenderCommand_pushDebugGroup* __restrict packet = (RenderCommand_pushDebugGroup*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pushDebugGroup::execute);
	packet->source = source;
	packet->id = id;
	packet->length = length;
	packet->message = message;
}
void popDebugGroup(
	void)
{
	RenderCommand_popDebugGroup* __restrict packet = (RenderCommand_popDebugGroup*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_popDebugGroup::execute);
}
void objectLabel(
	GLenum identifier,
	GLuint name,
	GLsizei length,
	const GLchar *label)
{
	RenderCommand_objectLabel* __restrict packet = (RenderCommand_objectLabel*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_objectLabel::execute);
	packet->identifier = identifier;
	packet->name = name;
	packet->length = length;
	packet->label = label;
}
void getObjectLabel(
	GLenum identifier,
	GLuint name,
	GLsizei bufSize,
	GLsizei *length,
	GLchar *label)
{
	RenderCommand_getObjectLabel* __restrict packet = (RenderCommand_getObjectLabel*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getObjectLabel::execute);
	packet->identifier = identifier;
	packet->name = name;
	packet->bufSize = bufSize;
	packet->length = length;
	packet->label = label;
}
void objectPtrLabel(
	const void *ptr,
	GLsizei length,
	const GLchar *label)
{
	RenderCommand_objectPtrLabel* __restrict packet = (RenderCommand_objectPtrLabel*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_objectPtrLabel::execute);
	packet->ptr = ptr;
	packet->length = length;
	packet->label = label;
}
void getObjectPtrLabel(
	const void *ptr,
	GLsizei bufSize,
	GLsizei *length,
	GLchar *label)
{
	RenderCommand_getObjectPtrLabel* __restrict packet = (RenderCommand_getObjectPtrLabel*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getObjectPtrLabel::execute);
	packet->ptr = ptr;
	packet->bufSize = bufSize;
	packet->length = length;
	packet->label = label;
}
void bufferStorage(
	GLenum target,
	GLsizeiptr size,
	const void *data,
	GLbitfield flags)
{
	RenderCommand_bufferStorage* __restrict packet = (RenderCommand_bufferStorage*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bufferStorage::execute);
	packet->target = target;
	packet->size = size;
	packet->data = data;
	packet->flags = flags;
}
void clearTexImage(
	GLuint texture,
	GLint level,
	GLenum format,
	GLenum type,
	const void *data)
{
	RenderCommand_clearTexImage* __restrict packet = (RenderCommand_clearTexImage*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clearTexImage::execute);
	packet->texture = texture;
	packet->level = level;
	packet->format = format;
	packet->type = type;
	packet->data = data;
}
void clearTexSubImage(
	GLuint texture,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint zoffset,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLenum format,
	GLenum type,
	const void *data)
{
	RenderCommand_clearTexSubImage* __restrict packet = (RenderCommand_clearTexSubImage*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clearTexSubImage::execute);
	packet->texture = texture;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->zoffset = zoffset;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->format = format;
	packet->type = type;
	packet->data = data;
}
void bindBuffersBase(
	GLenum target,
	GLuint first,
	GLsizei count,
	const GLuint *buffers)
{
	RenderCommand_bindBuffersBase* __restrict packet = (RenderCommand_bindBuffersBase*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindBuffersBase::execute);
	packet->target = target;
	packet->first = first;
	packet->count = count;
	packet->buffers = buffers;
}
void bindBuffersRange(
	GLenum target,
	GLuint first,
	GLsizei count,
	const GLuint *buffers,
	const GLintptr *offsets,
	const GLsizeiptr *sizes)
{
	RenderCommand_bindBuffersRange* __restrict packet = (RenderCommand_bindBuffersRange*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindBuffersRange::execute);
	packet->target = target;
	packet->first = first;
	packet->count = count;
	packet->buffers = buffers;
	packet->offsets = offsets;
	packet->sizes = sizes;
}
void bindTextures(
	GLuint first,
	GLsizei count,
	const GLuint *textures)
{
	RenderCommand_bindTextures* __restrict packet = (RenderCommand_bindTextures*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindTextures::execute);
	packet->first = first;
	packet->count = count;
	packet->textures = textures;
}
void bindSamplers(
	GLuint first,
	GLsizei count,
	const GLuint *samplers)
{
	RenderCommand_bindSamplers* __restrict packet = (RenderCommand_bindSamplers*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindSamplers::execute);
	packet->first = first;
	packet->count = count;
	packet->samplers = samplers;
}
void bindImageTextures(
	GLuint first,
	GLsizei count,
	const GLuint *textures)
{
	RenderCommand_bindImageTextures* __restrict packet = (RenderCommand_bindImageTextures*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindImageTextures::execute);
	packet->first = first;
	packet->count = count;
	packet->textures = textures;
}
void bindVertexBuffers(
	GLuint first,
	GLsizei count,
	const GLuint *buffers,
	const GLintptr *offsets,
	const GLsizei *strides)
{
	RenderCommand_bindVertexBuffers* __restrict packet = (RenderCommand_bindVertexBuffers*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindVertexBuffers::execute);
	packet->first = first;
	packet->count = count;
	packet->buffers = buffers;
	packet->offsets = offsets;
	packet->strides = strides;
}
void clipControl(
	GLenum origin,
	GLenum depth)
{
	RenderCommand_clipControl* __restrict packet = (RenderCommand_clipControl*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clipControl::execute);
	packet->origin = origin;
	packet->depth = depth;
}
void createTransformFeedbacks(
	GLsizei n,
	GLuint *ids)
{
	RenderCommand_createTransformFeedbacks* __restrict packet = (RenderCommand_createTransformFeedbacks*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_createTransformFeedbacks::execute);
	packet->n = n;
	packet->ids = ids;
}
void transformFeedbackBufferBase(
	GLuint xfb,
	GLuint index,
	GLuint buffer)
{
	RenderCommand_transformFeedbackBufferBase* __restrict packet = (RenderCommand_transformFeedbackBufferBase*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_transformFeedbackBufferBase::execute);
	packet->xfb = xfb;
	packet->index = index;
	packet->buffer = buffer;
}
void transformFeedbackBufferRange(
	GLuint xfb,
	GLuint index,
	GLuint buffer,
	GLintptr offset,
	GLsizeiptr size)
{
	RenderCommand_transformFeedbackBufferRange* __restrict packet = (RenderCommand_transformFeedbackBufferRange*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_transformFeedbackBufferRange::execute);
	packet->xfb = xfb;
	packet->index = index;
	packet->buffer = buffer;
	packet->offset = offset;
	packet->size = size;
}
void getTransformFeedbackiv(
	GLuint xfb,
	GLenum pname,
	GLint *param)
{
	RenderCommand_getTransformFeedbackiv* __restrict packet = (RenderCommand_getTransformFeedbackiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTransformFeedbackiv::execute);
	packet->xfb = xfb;
	packet->pname = pname;
	packet->param = param;
}
void getTransformFeedbacki_v(
	GLuint xfb,
	GLenum pname,
	GLuint index,
	GLint *param)
{
	RenderCommand_getTransformFeedbacki_v* __restrict packet = (RenderCommand_getTransformFeedbacki_v*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTransformFeedbacki_v::execute);
	packet->xfb = xfb;
	packet->pname = pname;
	packet->index = index;
	packet->param = param;
}
void getTransformFeedbacki64_v(
	GLuint xfb,
	GLenum pname,
	GLuint index,
	GLint64 *param)
{
	RenderCommand_getTransformFeedbacki64_v* __restrict packet = (RenderCommand_getTransformFeedbacki64_v*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTransformFeedbacki64_v::execute);
	packet->xfb = xfb;
	packet->pname = pname;
	packet->index = index;
	packet->param = param;
}
void createBuffers(
	GLsizei n,
	GLuint *buffers)
{
	RenderCommand_createBuffers* __restrict packet = (RenderCommand_createBuffers*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_createBuffers::execute);
	packet->n = n;
	packet->buffers = buffers;
}
void namedBufferStorage(
	GLuint buffer,
	GLsizeiptr size,
	const void *data,
	GLbitfield flags)
{
	RenderCommand_namedBufferStorage* __restrict packet = (RenderCommand_namedBufferStorage*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedBufferStorage::execute);
	packet->buffer = buffer;
	packet->size = size;
	packet->data = data;
	packet->flags = flags;
}
void namedBufferData(
	GLuint buffer,
	GLsizeiptr size,
	const void *data,
	GLenum usage)
{
	RenderCommand_namedBufferData* __restrict packet = (RenderCommand_namedBufferData*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedBufferData::execute);
	packet->buffer = buffer;
	packet->size = size;
	packet->data = data;
	packet->usage = usage;
}
void namedBufferSubData(
	GLuint buffer,
	GLintptr offset,
	GLsizeiptr size,
	const void *data)
{
	RenderCommand_namedBufferSubData* __restrict packet = (RenderCommand_namedBufferSubData*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedBufferSubData::execute);
	packet->buffer = buffer;
	packet->offset = offset;
	packet->size = size;
	packet->data = data;
}
void copyNamedBufferSubData(
	GLuint readBuffer,
	GLuint writeBuffer,
	GLintptr readOffset,
	GLintptr writeOffset,
	GLsizeiptr size)
{
	RenderCommand_copyNamedBufferSubData* __restrict packet = (RenderCommand_copyNamedBufferSubData*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyNamedBufferSubData::execute);
	packet->readBuffer = readBuffer;
	packet->writeBuffer = writeBuffer;
	packet->readOffset = readOffset;
	packet->writeOffset = writeOffset;
	packet->size = size;
}
void clearNamedBufferData(
	GLuint buffer,
	GLenum internalformat,
	GLenum format,
	GLenum type,
	const void *data)
{
	RenderCommand_clearNamedBufferData* __restrict packet = (RenderCommand_clearNamedBufferData*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clearNamedBufferData::execute);
	packet->buffer = buffer;
	packet->internalformat = internalformat;
	packet->format = format;
	packet->type = type;
	packet->data = data;
}
void clearNamedBufferSubData(
	GLuint buffer,
	GLenum internalformat,
	GLintptr offset,
	GLsizeiptr size,
	GLenum format,
	GLenum type,
	const void *data)
{
	RenderCommand_clearNamedBufferSubData* __restrict packet = (RenderCommand_clearNamedBufferSubData*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clearNamedBufferSubData::execute);
	packet->buffer = buffer;
	packet->internalformat = internalformat;
	packet->offset = offset;
	packet->size = size;
	packet->format = format;
	packet->type = type;
	packet->data = data;
}
void* mapNamedBuffer(
	GLuint buffer,
	GLenum access)
{
	RenderCommand_mapNamedBuffer* __restrict packet = (RenderCommand_mapNamedBuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_mapNamedBuffer::execute);
	packet->buffer = buffer;
	packet->access = access;
}
void* mapNamedBufferRange(
	GLuint buffer,
	GLintptr offset,
	GLsizeiptr length,
	GLbitfield access)
{
	RenderCommand_mapNamedBufferRange* __restrict packet = (RenderCommand_mapNamedBufferRange*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_mapNamedBufferRange::execute);
	packet->buffer = buffer;
	packet->offset = offset;
	packet->length = length;
	packet->access = access;
}
GLboolean unmapNamedBuffer(
	GLuint buffer)
{
	RenderCommand_unmapNamedBuffer* __restrict packet = (RenderCommand_unmapNamedBuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_unmapNamedBuffer::execute);
	packet->buffer = buffer;
}
void flushMappedNamedBufferRange(
	GLuint buffer,
	GLintptr offset,
	GLsizeiptr length)
{
	RenderCommand_flushMappedNamedBufferRange* __restrict packet = (RenderCommand_flushMappedNamedBufferRange*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_flushMappedNamedBufferRange::execute);
	packet->buffer = buffer;
	packet->offset = offset;
	packet->length = length;
}
void getNamedBufferParameteriv(
	GLuint buffer,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getNamedBufferParameteriv* __restrict packet = (RenderCommand_getNamedBufferParameteriv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedBufferParameteriv::execute);
	packet->buffer = buffer;
	packet->pname = pname;
	packet->params = params;
}
void getNamedBufferParameteri64v(
	GLuint buffer,
	GLenum pname,
	GLint64 *params)
{
	RenderCommand_getNamedBufferParameteri64v* __restrict packet = (RenderCommand_getNamedBufferParameteri64v*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedBufferParameteri64v::execute);
	packet->buffer = buffer;
	packet->pname = pname;
	packet->params = params;
}
void getNamedBufferPointerv(
	GLuint buffer,
	GLenum pname,
	void **params)
{
	RenderCommand_getNamedBufferPointerv* __restrict packet = (RenderCommand_getNamedBufferPointerv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedBufferPointerv::execute);
	packet->buffer = buffer;
	packet->pname = pname;
	packet->params = params;
}
void getNamedBufferSubData(
	GLuint buffer,
	GLintptr offset,
	GLsizeiptr size,
	void *data)
{
	RenderCommand_getNamedBufferSubData* __restrict packet = (RenderCommand_getNamedBufferSubData*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedBufferSubData::execute);
	packet->buffer = buffer;
	packet->offset = offset;
	packet->size = size;
	packet->data = data;
}
void createFramebuffers(
	GLsizei n,
	GLuint *framebuffers)
{
	RenderCommand_createFramebuffers* __restrict packet = (RenderCommand_createFramebuffers*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_createFramebuffers::execute);
	packet->n = n;
	packet->framebuffers = framebuffers;
}
void namedFramebufferRenderbuffer(
	GLuint framebuffer,
	GLenum attachment,
	GLenum renderbuffertarget,
	GLuint renderbuffer)
{
	RenderCommand_namedFramebufferRenderbuffer* __restrict packet = (RenderCommand_namedFramebufferRenderbuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedFramebufferRenderbuffer::execute);
	packet->framebuffer = framebuffer;
	packet->attachment = attachment;
	packet->renderbuffertarget = renderbuffertarget;
	packet->renderbuffer = renderbuffer;
}
void namedFramebufferParameteri(
	GLuint framebuffer,
	GLenum pname,
	GLint param)
{
	RenderCommand_namedFramebufferParameteri* __restrict packet = (RenderCommand_namedFramebufferParameteri*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedFramebufferParameteri::execute);
	packet->framebuffer = framebuffer;
	packet->pname = pname;
	packet->param = param;
}
void namedFramebufferTexture(
	GLuint framebuffer,
	GLenum attachment,
	GLuint texture,
	GLint level)
{
	RenderCommand_namedFramebufferTexture* __restrict packet = (RenderCommand_namedFramebufferTexture*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedFramebufferTexture::execute);
	packet->framebuffer = framebuffer;
	packet->attachment = attachment;
	packet->texture = texture;
	packet->level = level;
}
void namedFramebufferTextureLayer(
	GLuint framebuffer,
	GLenum attachment,
	GLuint texture,
	GLint level,
	GLint layer)
{
	RenderCommand_namedFramebufferTextureLayer* __restrict packet = (RenderCommand_namedFramebufferTextureLayer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedFramebufferTextureLayer::execute);
	packet->framebuffer = framebuffer;
	packet->attachment = attachment;
	packet->texture = texture;
	packet->level = level;
	packet->layer = layer;
}
void namedFramebufferDrawBuffer(
	GLuint framebuffer,
	GLenum buf)
{
	RenderCommand_namedFramebufferDrawBuffer* __restrict packet = (RenderCommand_namedFramebufferDrawBuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedFramebufferDrawBuffer::execute);
	packet->framebuffer = framebuffer;
	packet->buf = buf;
}
void namedFramebufferDrawBuffers(
	GLuint framebuffer,
	GLsizei n,
	const GLenum *bufs)
{
	RenderCommand_namedFramebufferDrawBuffers* __restrict packet = (RenderCommand_namedFramebufferDrawBuffers*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedFramebufferDrawBuffers::execute);
	packet->framebuffer = framebuffer;
	packet->n = n;
	packet->bufs = bufs;
}
void namedFramebufferReadBuffer(
	GLuint framebuffer,
	GLenum src)
{
	RenderCommand_namedFramebufferReadBuffer* __restrict packet = (RenderCommand_namedFramebufferReadBuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedFramebufferReadBuffer::execute);
	packet->framebuffer = framebuffer;
	packet->src = src;
}
void invalidateNamedFramebufferData(
	GLuint framebuffer,
	GLsizei numAttachments,
	const GLenum *attachments)
{
	RenderCommand_invalidateNamedFramebufferData* __restrict packet = (RenderCommand_invalidateNamedFramebufferData*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_invalidateNamedFramebufferData::execute);
	packet->framebuffer = framebuffer;
	packet->numAttachments = numAttachments;
	packet->attachments = attachments;
}
void invalidateNamedFramebufferSubData(
	GLuint framebuffer,
	GLsizei numAttachments,
	const GLenum *attachments,
	GLint x,
	GLint y,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_invalidateNamedFramebufferSubData* __restrict packet = (RenderCommand_invalidateNamedFramebufferSubData*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_invalidateNamedFramebufferSubData::execute);
	packet->framebuffer = framebuffer;
	packet->numAttachments = numAttachments;
	packet->attachments = attachments;
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->height = height;
}
void clearNamedFramebufferiv(
	GLuint framebuffer,
	GLenum buffer,
	GLint drawbuffer,
	const GLint *value)
{
	RenderCommand_clearNamedFramebufferiv* __restrict packet = (RenderCommand_clearNamedFramebufferiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clearNamedFramebufferiv::execute);
	packet->framebuffer = framebuffer;
	packet->buffer = buffer;
	packet->drawbuffer = drawbuffer;
	packet->value = value;
}
void clearNamedFramebufferuiv(
	GLuint framebuffer,
	GLenum buffer,
	GLint drawbuffer,
	const GLuint *value)
{
	RenderCommand_clearNamedFramebufferuiv* __restrict packet = (RenderCommand_clearNamedFramebufferuiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clearNamedFramebufferuiv::execute);
	packet->framebuffer = framebuffer;
	packet->buffer = buffer;
	packet->drawbuffer = drawbuffer;
	packet->value = value;
}
void clearNamedFramebufferfv(
	GLuint framebuffer,
	GLenum buffer,
	GLint drawbuffer,
	const GLfloat *value)
{
	RenderCommand_clearNamedFramebufferfv* __restrict packet = (RenderCommand_clearNamedFramebufferfv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clearNamedFramebufferfv::execute);
	packet->framebuffer = framebuffer;
	packet->buffer = buffer;
	packet->drawbuffer = drawbuffer;
	packet->value = value;
}
void clearNamedFramebufferfi(
	GLuint framebuffer,
	GLenum buffer,
	GLint drawbuffer,
	GLfloat depth,
	GLint stencil)
{
	RenderCommand_clearNamedFramebufferfi* __restrict packet = (RenderCommand_clearNamedFramebufferfi*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clearNamedFramebufferfi::execute);
	packet->framebuffer = framebuffer;
	packet->buffer = buffer;
	packet->drawbuffer = drawbuffer;
	packet->depth = depth;
	packet->stencil = stencil;
}
void blitNamedFramebuffer(
	GLuint readFramebuffer,
	GLuint drawFramebuffer,
	GLint srcX0,
	GLint srcY0,
	GLint srcX1,
	GLint srcY1,
	GLint dstX0,
	GLint dstY0,
	GLint dstX1,
	GLint dstY1,
	GLbitfield mask,
	GLenum filter)
{
	RenderCommand_blitNamedFramebuffer* __restrict packet = (RenderCommand_blitNamedFramebuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_blitNamedFramebuffer::execute);
	packet->readFramebuffer = readFramebuffer;
	packet->drawFramebuffer = drawFramebuffer;
	packet->srcX0 = srcX0;
	packet->srcY0 = srcY0;
	packet->srcX1 = srcX1;
	packet->srcY1 = srcY1;
	packet->dstX0 = dstX0;
	packet->dstY0 = dstY0;
	packet->dstX1 = dstX1;
	packet->dstY1 = dstY1;
	packet->mask = mask;
	packet->filter = filter;
}
GLenum checkNamedFramebufferStatus(
	GLuint framebuffer,
	GLenum target)
{
	RenderCommand_checkNamedFramebufferStatus* __restrict packet = (RenderCommand_checkNamedFramebufferStatus*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_checkNamedFramebufferStatus::execute);
	packet->framebuffer = framebuffer;
	packet->target = target;
}
void getNamedFramebufferParameteriv(
	GLuint framebuffer,
	GLenum pname,
	GLint *param)
{
	RenderCommand_getNamedFramebufferParameteriv* __restrict packet = (RenderCommand_getNamedFramebufferParameteriv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedFramebufferParameteriv::execute);
	packet->framebuffer = framebuffer;
	packet->pname = pname;
	packet->param = param;
}
void getNamedFramebufferAttachmentParameteriv(
	GLuint framebuffer,
	GLenum attachment,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getNamedFramebufferAttachmentParameteriv* __restrict packet = (RenderCommand_getNamedFramebufferAttachmentParameteriv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedFramebufferAttachmentParameteriv::execute);
	packet->framebuffer = framebuffer;
	packet->attachment = attachment;
	packet->pname = pname;
	packet->params = params;
}
void createRenderbuffers(
	GLsizei n,
	GLuint *renderbuffers)
{
	RenderCommand_createRenderbuffers* __restrict packet = (RenderCommand_createRenderbuffers*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_createRenderbuffers::execute);
	packet->n = n;
	packet->renderbuffers = renderbuffers;
}
void namedRenderbufferStorage(
	GLuint renderbuffer,
	GLenum internalformat,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_namedRenderbufferStorage* __restrict packet = (RenderCommand_namedRenderbufferStorage*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedRenderbufferStorage::execute);
	packet->renderbuffer = renderbuffer;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
}
void namedRenderbufferStorageMultisample(
	GLuint renderbuffer,
	GLsizei samples,
	GLenum internalformat,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_namedRenderbufferStorageMultisample* __restrict packet = (RenderCommand_namedRenderbufferStorageMultisample*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedRenderbufferStorageMultisample::execute);
	packet->renderbuffer = renderbuffer;
	packet->samples = samples;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
}
void getNamedRenderbufferParameteriv(
	GLuint renderbuffer,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getNamedRenderbufferParameteriv* __restrict packet = (RenderCommand_getNamedRenderbufferParameteriv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedRenderbufferParameteriv::execute);
	packet->renderbuffer = renderbuffer;
	packet->pname = pname;
	packet->params = params;
}
void createTextures(
	GLenum target,
	GLsizei n,
	GLuint *textures)
{
	RenderCommand_createTextures* __restrict packet = (RenderCommand_createTextures*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_createTextures::execute);
	packet->target = target;
	packet->n = n;
	packet->textures = textures;
}
void textureBuffer(
	GLuint texture,
	GLenum internalformat,
	GLuint buffer)
{
	RenderCommand_textureBuffer* __restrict packet = (RenderCommand_textureBuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureBuffer::execute);
	packet->texture = texture;
	packet->internalformat = internalformat;
	packet->buffer = buffer;
}
void textureBufferRange(
	GLuint texture,
	GLenum internalformat,
	GLuint buffer,
	GLintptr offset,
	GLsizeiptr size)
{
	RenderCommand_textureBufferRange* __restrict packet = (RenderCommand_textureBufferRange*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureBufferRange::execute);
	packet->texture = texture;
	packet->internalformat = internalformat;
	packet->buffer = buffer;
	packet->offset = offset;
	packet->size = size;
}
void textureStorage1D(
	GLuint texture,
	GLsizei levels,
	GLenum internalformat,
	GLsizei width)
{
	RenderCommand_textureStorage1D* __restrict packet = (RenderCommand_textureStorage1D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureStorage1D::execute);
	packet->texture = texture;
	packet->levels = levels;
	packet->internalformat = internalformat;
	packet->width = width;
}
void textureStorage2D(
	GLuint texture,
	GLsizei levels,
	GLenum internalformat,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_textureStorage2D* __restrict packet = (RenderCommand_textureStorage2D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureStorage2D::execute);
	packet->texture = texture;
	packet->levels = levels;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
}
void textureStorage3D(
	GLuint texture,
	GLsizei levels,
	GLenum internalformat,
	GLsizei width,
	GLsizei height,
	GLsizei depth)
{
	RenderCommand_textureStorage3D* __restrict packet = (RenderCommand_textureStorage3D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureStorage3D::execute);
	packet->texture = texture;
	packet->levels = levels;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
}
void textureStorage2DMultisample(
	GLuint texture,
	GLsizei samples,
	GLenum internalformat,
	GLsizei width,
	GLsizei height,
	GLboolean fixedsamplelocations)
{
	RenderCommand_textureStorage2DMultisample* __restrict packet = (RenderCommand_textureStorage2DMultisample*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureStorage2DMultisample::execute);
	packet->texture = texture;
	packet->samples = samples;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->fixedsamplelocations = fixedsamplelocations;
}
void textureStorage3DMultisample(
	GLuint texture,
	GLsizei samples,
	GLenum internalformat,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLboolean fixedsamplelocations)
{
	RenderCommand_textureStorage3DMultisample* __restrict packet = (RenderCommand_textureStorage3DMultisample*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureStorage3DMultisample::execute);
	packet->texture = texture;
	packet->samples = samples;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->fixedsamplelocations = fixedsamplelocations;
}
void textureSubImage1D(
	GLuint texture,
	GLint level,
	GLint xoffset,
	GLsizei width,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_textureSubImage1D* __restrict packet = (RenderCommand_textureSubImage1D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureSubImage1D::execute);
	packet->texture = texture;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->width = width;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void textureSubImage2D(
	GLuint texture,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLsizei width,
	GLsizei height,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_textureSubImage2D* __restrict packet = (RenderCommand_textureSubImage2D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureSubImage2D::execute);
	packet->texture = texture;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->width = width;
	packet->height = height;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void textureSubImage3D(
	GLuint texture,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint zoffset,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_textureSubImage3D* __restrict packet = (RenderCommand_textureSubImage3D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureSubImage3D::execute);
	packet->texture = texture;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->zoffset = zoffset;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void compressedTextureSubImage1D(
	GLuint texture,
	GLint level,
	GLint xoffset,
	GLsizei width,
	GLenum format,
	GLsizei imageSize,
	const void *data)
{
	RenderCommand_compressedTextureSubImage1D* __restrict packet = (RenderCommand_compressedTextureSubImage1D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedTextureSubImage1D::execute);
	packet->texture = texture;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->width = width;
	packet->format = format;
	packet->imageSize = imageSize;
	packet->data = data;
}
void compressedTextureSubImage2D(
	GLuint texture,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLsizei width,
	GLsizei height,
	GLenum format,
	GLsizei imageSize,
	const void *data)
{
	RenderCommand_compressedTextureSubImage2D* __restrict packet = (RenderCommand_compressedTextureSubImage2D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedTextureSubImage2D::execute);
	packet->texture = texture;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->width = width;
	packet->height = height;
	packet->format = format;
	packet->imageSize = imageSize;
	packet->data = data;
}
void compressedTextureSubImage3D(
	GLuint texture,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint zoffset,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLenum format,
	GLsizei imageSize,
	const void *data)
{
	RenderCommand_compressedTextureSubImage3D* __restrict packet = (RenderCommand_compressedTextureSubImage3D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedTextureSubImage3D::execute);
	packet->texture = texture;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->zoffset = zoffset;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->format = format;
	packet->imageSize = imageSize;
	packet->data = data;
}
void copyTextureSubImage1D(
	GLuint texture,
	GLint level,
	GLint xoffset,
	GLint x,
	GLint y,
	GLsizei width)
{
	RenderCommand_copyTextureSubImage1D* __restrict packet = (RenderCommand_copyTextureSubImage1D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyTextureSubImage1D::execute);
	packet->texture = texture;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->x = x;
	packet->y = y;
	packet->width = width;
}
void copyTextureSubImage2D(
	GLuint texture,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint x,
	GLint y,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_copyTextureSubImage2D* __restrict packet = (RenderCommand_copyTextureSubImage2D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyTextureSubImage2D::execute);
	packet->texture = texture;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->height = height;
}
void copyTextureSubImage3D(
	GLuint texture,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint zoffset,
	GLint x,
	GLint y,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_copyTextureSubImage3D* __restrict packet = (RenderCommand_copyTextureSubImage3D*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyTextureSubImage3D::execute);
	packet->texture = texture;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->zoffset = zoffset;
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->height = height;
}
void textureParameterf(
	GLuint texture,
	GLenum pname,
	GLfloat param)
{
	RenderCommand_textureParameterf* __restrict packet = (RenderCommand_textureParameterf*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureParameterf::execute);
	packet->texture = texture;
	packet->pname = pname;
	packet->param = param;
}
void textureParameterfv(
	GLuint texture,
	GLenum pname,
	const GLfloat *param)
{
	RenderCommand_textureParameterfv* __restrict packet = (RenderCommand_textureParameterfv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureParameterfv::execute);
	packet->texture = texture;
	packet->pname = pname;
	packet->param = param;
}
void textureParameteri(
	GLuint texture,
	GLenum pname,
	GLint param)
{
	RenderCommand_textureParameteri* __restrict packet = (RenderCommand_textureParameteri*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureParameteri::execute);
	packet->texture = texture;
	packet->pname = pname;
	packet->param = param;
}
void textureParameterIiv(
	GLuint texture,
	GLenum pname,
	const GLint *params)
{
	RenderCommand_textureParameterIiv* __restrict packet = (RenderCommand_textureParameterIiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureParameterIiv::execute);
	packet->texture = texture;
	packet->pname = pname;
	packet->params = params;
}
void textureParameterIuiv(
	GLuint texture,
	GLenum pname,
	const GLuint *params)
{
	RenderCommand_textureParameterIuiv* __restrict packet = (RenderCommand_textureParameterIuiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureParameterIuiv::execute);
	packet->texture = texture;
	packet->pname = pname;
	packet->params = params;
}
void textureParameteriv(
	GLuint texture,
	GLenum pname,
	const GLint *param)
{
	RenderCommand_textureParameteriv* __restrict packet = (RenderCommand_textureParameteriv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureParameteriv::execute);
	packet->texture = texture;
	packet->pname = pname;
	packet->param = param;
}
void generateTextureMipmap(
	GLuint texture)
{
	RenderCommand_generateTextureMipmap* __restrict packet = (RenderCommand_generateTextureMipmap*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_generateTextureMipmap::execute);
	packet->texture = texture;
}
void bindTextureUnit(
	GLuint unit,
	GLuint texture)
{
	RenderCommand_bindTextureUnit* __restrict packet = (RenderCommand_bindTextureUnit*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindTextureUnit::execute);
	packet->unit = unit;
	packet->texture = texture;
}
void getTextureImage(
	GLuint texture,
	GLint level,
	GLenum format,
	GLenum type,
	GLsizei bufSize,
	void *pixels)
{
	RenderCommand_getTextureImage* __restrict packet = (RenderCommand_getTextureImage*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTextureImage::execute);
	packet->texture = texture;
	packet->level = level;
	packet->format = format;
	packet->type = type;
	packet->bufSize = bufSize;
	packet->pixels = pixels;
}
void getCompressedTextureImage(
	GLuint texture,
	GLint level,
	GLsizei bufSize,
	void *pixels)
{
	RenderCommand_getCompressedTextureImage* __restrict packet = (RenderCommand_getCompressedTextureImage*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getCompressedTextureImage::execute);
	packet->texture = texture;
	packet->level = level;
	packet->bufSize = bufSize;
	packet->pixels = pixels;
}
void getTextureLevelParameterfv(
	GLuint texture,
	GLint level,
	GLenum pname,
	GLfloat *params)
{
	RenderCommand_getTextureLevelParameterfv* __restrict packet = (RenderCommand_getTextureLevelParameterfv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTextureLevelParameterfv::execute);
	packet->texture = texture;
	packet->level = level;
	packet->pname = pname;
	packet->params = params;
}
void getTextureLevelParameteriv(
	GLuint texture,
	GLint level,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getTextureLevelParameteriv* __restrict packet = (RenderCommand_getTextureLevelParameteriv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTextureLevelParameteriv::execute);
	packet->texture = texture;
	packet->level = level;
	packet->pname = pname;
	packet->params = params;
}
void getTextureParameterfv(
	GLuint texture,
	GLenum pname,
	GLfloat *params)
{
	RenderCommand_getTextureParameterfv* __restrict packet = (RenderCommand_getTextureParameterfv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTextureParameterfv::execute);
	packet->texture = texture;
	packet->pname = pname;
	packet->params = params;
}
void getTextureParameterIiv(
	GLuint texture,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getTextureParameterIiv* __restrict packet = (RenderCommand_getTextureParameterIiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTextureParameterIiv::execute);
	packet->texture = texture;
	packet->pname = pname;
	packet->params = params;
}
void getTextureParameterIuiv(
	GLuint texture,
	GLenum pname,
	GLuint *params)
{
	RenderCommand_getTextureParameterIuiv* __restrict packet = (RenderCommand_getTextureParameterIuiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTextureParameterIuiv::execute);
	packet->texture = texture;
	packet->pname = pname;
	packet->params = params;
}
void getTextureParameteriv(
	GLuint texture,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getTextureParameteriv* __restrict packet = (RenderCommand_getTextureParameteriv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTextureParameteriv::execute);
	packet->texture = texture;
	packet->pname = pname;
	packet->params = params;
}
void createVertexArrays(
	GLsizei n,
	GLuint *arrays)
{
	RenderCommand_createVertexArrays* __restrict packet = (RenderCommand_createVertexArrays*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_createVertexArrays::execute);
	packet->n = n;
	packet->arrays = arrays;
}
void disableVertexArrayAttrib(
	GLuint vaobj,
	GLuint index)
{
	RenderCommand_disableVertexArrayAttrib* __restrict packet = (RenderCommand_disableVertexArrayAttrib*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_disableVertexArrayAttrib::execute);
	packet->vaobj = vaobj;
	packet->index = index;
}
void enableVertexArrayAttrib(
	GLuint vaobj,
	GLuint index)
{
	RenderCommand_enableVertexArrayAttrib* __restrict packet = (RenderCommand_enableVertexArrayAttrib*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_enableVertexArrayAttrib::execute);
	packet->vaobj = vaobj;
	packet->index = index;
}
void vertexArrayElementBuffer(
	GLuint vaobj,
	GLuint buffer)
{
	RenderCommand_vertexArrayElementBuffer* __restrict packet = (RenderCommand_vertexArrayElementBuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayElementBuffer::execute);
	packet->vaobj = vaobj;
	packet->buffer = buffer;
}
void vertexArrayVertexBuffer(
	GLuint vaobj,
	GLuint bindingindex,
	GLuint buffer,
	GLintptr offset,
	GLsizei stride)
{
	RenderCommand_vertexArrayVertexBuffer* __restrict packet = (RenderCommand_vertexArrayVertexBuffer*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayVertexBuffer::execute);
	packet->vaobj = vaobj;
	packet->bindingindex = bindingindex;
	packet->buffer = buffer;
	packet->offset = offset;
	packet->stride = stride;
}
void vertexArrayVertexBuffers(
	GLuint vaobj,
	GLuint first,
	GLsizei count,
	const GLuint *buffers,
	const GLintptr *offsets,
	const GLsizei *strides)
{
	RenderCommand_vertexArrayVertexBuffers* __restrict packet = (RenderCommand_vertexArrayVertexBuffers*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayVertexBuffers::execute);
	packet->vaobj = vaobj;
	packet->first = first;
	packet->count = count;
	packet->buffers = buffers;
	packet->offsets = offsets;
	packet->strides = strides;
}
void vertexArrayAttribBinding(
	GLuint vaobj,
	GLuint attribindex,
	GLuint bindingindex)
{
	RenderCommand_vertexArrayAttribBinding* __restrict packet = (RenderCommand_vertexArrayAttribBinding*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayAttribBinding::execute);
	packet->vaobj = vaobj;
	packet->attribindex = attribindex;
	packet->bindingindex = bindingindex;
}
void vertexArrayAttribFormat(
	GLuint vaobj,
	GLuint attribindex,
	GLint size,
	GLenum type,
	GLboolean normalized,
	GLuint relativeoffset)
{
	RenderCommand_vertexArrayAttribFormat* __restrict packet = (RenderCommand_vertexArrayAttribFormat*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayAttribFormat::execute);
	packet->vaobj = vaobj;
	packet->attribindex = attribindex;
	packet->size = size;
	packet->type = type;
	packet->normalized = normalized;
	packet->relativeoffset = relativeoffset;
}
void vertexArrayAttribIFormat(
	GLuint vaobj,
	GLuint attribindex,
	GLint size,
	GLenum type,
	GLuint relativeoffset)
{
	RenderCommand_vertexArrayAttribIFormat* __restrict packet = (RenderCommand_vertexArrayAttribIFormat*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayAttribIFormat::execute);
	packet->vaobj = vaobj;
	packet->attribindex = attribindex;
	packet->size = size;
	packet->type = type;
	packet->relativeoffset = relativeoffset;
}
void vertexArrayAttribLFormat(
	GLuint vaobj,
	GLuint attribindex,
	GLint size,
	GLenum type,
	GLuint relativeoffset)
{
	RenderCommand_vertexArrayAttribLFormat* __restrict packet = (RenderCommand_vertexArrayAttribLFormat*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayAttribLFormat::execute);
	packet->vaobj = vaobj;
	packet->attribindex = attribindex;
	packet->size = size;
	packet->type = type;
	packet->relativeoffset = relativeoffset;
}
void vertexArrayBindingDivisor(
	GLuint vaobj,
	GLuint bindingindex,
	GLuint divisor)
{
	RenderCommand_vertexArrayBindingDivisor* __restrict packet = (RenderCommand_vertexArrayBindingDivisor*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayBindingDivisor::execute);
	packet->vaobj = vaobj;
	packet->bindingindex = bindingindex;
	packet->divisor = divisor;
}
void getVertexArrayiv(
	GLuint vaobj,
	GLenum pname,
	GLint *param)
{
	RenderCommand_getVertexArrayiv* __restrict packet = (RenderCommand_getVertexArrayiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getVertexArrayiv::execute);
	packet->vaobj = vaobj;
	packet->pname = pname;
	packet->param = param;
}
void getVertexArrayIndexediv(
	GLuint vaobj,
	GLuint index,
	GLenum pname,
	GLint *param)
{
	RenderCommand_getVertexArrayIndexediv* __restrict packet = (RenderCommand_getVertexArrayIndexediv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getVertexArrayIndexediv::execute);
	packet->vaobj = vaobj;
	packet->index = index;
	packet->pname = pname;
	packet->param = param;
}
void getVertexArrayIndexed64iv(
	GLuint vaobj,
	GLuint index,
	GLenum pname,
	GLint64 *param)
{
	RenderCommand_getVertexArrayIndexed64iv* __restrict packet = (RenderCommand_getVertexArrayIndexed64iv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getVertexArrayIndexed64iv::execute);
	packet->vaobj = vaobj;
	packet->index = index;
	packet->pname = pname;
	packet->param = param;
}
void createSamplers(
	GLsizei n,
	GLuint *samplers)
{
	RenderCommand_createSamplers* __restrict packet = (RenderCommand_createSamplers*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_createSamplers::execute);
	packet->n = n;
	packet->samplers = samplers;
}
void createProgramPipelines(
	GLsizei n,
	GLuint *pipelines)
{
	RenderCommand_createProgramPipelines* __restrict packet = (RenderCommand_createProgramPipelines*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_createProgramPipelines::execute);
	packet->n = n;
	packet->pipelines = pipelines;
}
void createQueries(
	GLenum target,
	GLsizei n,
	GLuint *ids)
{
	RenderCommand_createQueries* __restrict packet = (RenderCommand_createQueries*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_createQueries::execute);
	packet->target = target;
	packet->n = n;
	packet->ids = ids;
}
void getQueryBufferObjecti64v(
	GLuint id,
	GLuint buffer,
	GLenum pname,
	GLintptr offset)
{
	RenderCommand_getQueryBufferObjecti64v* __restrict packet = (RenderCommand_getQueryBufferObjecti64v*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getQueryBufferObjecti64v::execute);
	packet->id = id;
	packet->buffer = buffer;
	packet->pname = pname;
	packet->offset = offset;
}
void getQueryBufferObjectiv(
	GLuint id,
	GLuint buffer,
	GLenum pname,
	GLintptr offset)
{
	RenderCommand_getQueryBufferObjectiv* __restrict packet = (RenderCommand_getQueryBufferObjectiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getQueryBufferObjectiv::execute);
	packet->id = id;
	packet->buffer = buffer;
	packet->pname = pname;
	packet->offset = offset;
}
void getQueryBufferObjectui64v(
	GLuint id,
	GLuint buffer,
	GLenum pname,
	GLintptr offset)
{
	RenderCommand_getQueryBufferObjectui64v* __restrict packet = (RenderCommand_getQueryBufferObjectui64v*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getQueryBufferObjectui64v::execute);
	packet->id = id;
	packet->buffer = buffer;
	packet->pname = pname;
	packet->offset = offset;
}
void getQueryBufferObjectuiv(
	GLuint id,
	GLuint buffer,
	GLenum pname,
	GLintptr offset)
{
	RenderCommand_getQueryBufferObjectuiv* __restrict packet = (RenderCommand_getQueryBufferObjectuiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getQueryBufferObjectuiv::execute);
	packet->id = id;
	packet->buffer = buffer;
	packet->pname = pname;
	packet->offset = offset;
}
void memoryBarrierByRegion(
	GLbitfield barriers)
{
	RenderCommand_memoryBarrierByRegion* __restrict packet = (RenderCommand_memoryBarrierByRegion*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_memoryBarrierByRegion::execute);
	packet->barriers = barriers;
}
void getTextureSubImage(
	GLuint texture,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint zoffset,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLenum format,
	GLenum type,
	GLsizei bufSize,
	void *pixels)
{
	RenderCommand_getTextureSubImage* __restrict packet = (RenderCommand_getTextureSubImage*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTextureSubImage::execute);
	packet->texture = texture;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->zoffset = zoffset;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->format = format;
	packet->type = type;
	packet->bufSize = bufSize;
	packet->pixels = pixels;
}
void getCompressedTextureSubImage(
	GLuint texture,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint zoffset,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLsizei bufSize,
	void *pixels)
{
	RenderCommand_getCompressedTextureSubImage* __restrict packet = (RenderCommand_getCompressedTextureSubImage*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getCompressedTextureSubImage::execute);
	packet->texture = texture;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->zoffset = zoffset;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->bufSize = bufSize;
	packet->pixels = pixels;
}
GLenum getGraphicsResetStatus(
	void)
{
	RenderCommand_getGraphicsResetStatus* __restrict packet = (RenderCommand_getGraphicsResetStatus*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getGraphicsResetStatus::execute);
}
void getnCompressedTexImage(
	GLenum target,
	GLint lod,
	GLsizei bufSize,
	void *pixels)
{
	RenderCommand_getnCompressedTexImage* __restrict packet = (RenderCommand_getnCompressedTexImage*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getnCompressedTexImage::execute);
	packet->target = target;
	packet->lod = lod;
	packet->bufSize = bufSize;
	packet->pixels = pixels;
}
void getnTexImage(
	GLenum target,
	GLint level,
	GLenum format,
	GLenum type,
	GLsizei bufSize,
	void *pixels)
{
	RenderCommand_getnTexImage* __restrict packet = (RenderCommand_getnTexImage*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getnTexImage::execute);
	packet->target = target;
	packet->level = level;
	packet->format = format;
	packet->type = type;
	packet->bufSize = bufSize;
	packet->pixels = pixels;
}
void getnUniformdv(
	GLuint program,
	GLint location,
	GLsizei bufSize,
	GLdouble *params)
{
	RenderCommand_getnUniformdv* __restrict packet = (RenderCommand_getnUniformdv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getnUniformdv::execute);
	packet->program = program;
	packet->location = location;
	packet->bufSize = bufSize;
	packet->params = params;
}
void getnUniformfv(
	GLuint program,
	GLint location,
	GLsizei bufSize,
	GLfloat *params)
{
	RenderCommand_getnUniformfv* __restrict packet = (RenderCommand_getnUniformfv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getnUniformfv::execute);
	packet->program = program;
	packet->location = location;
	packet->bufSize = bufSize;
	packet->params = params;
}
void getnUniformiv(
	GLuint program,
	GLint location,
	GLsizei bufSize,
	GLint *params)
{
	RenderCommand_getnUniformiv* __restrict packet = (RenderCommand_getnUniformiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getnUniformiv::execute);
	packet->program = program;
	packet->location = location;
	packet->bufSize = bufSize;
	packet->params = params;
}
void getnUniformuiv(
	GLuint program,
	GLint location,
	GLsizei bufSize,
	GLuint *params)
{
	RenderCommand_getnUniformuiv* __restrict packet = (RenderCommand_getnUniformuiv*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getnUniformuiv::execute);
	packet->program = program;
	packet->location = location;
	packet->bufSize = bufSize;
	packet->params = params;
}
void readnPixels(
	GLint x,
	GLint y,
	GLsizei width,
	GLsizei height,
	GLenum format,
	GLenum type,
	GLsizei bufSize,
	void *data)
{
	RenderCommand_readnPixels* __restrict packet = (RenderCommand_readnPixels*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_readnPixels::execute);
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->height = height;
	packet->format = format;
	packet->type = type;
	packet->bufSize = bufSize;
	packet->data = data;
}
void textureBarrier(
	void)
{
	RenderCommand_textureBarrier* __restrict packet = (RenderCommand_textureBarrier*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureBarrier::execute);
}
void specializeShader(
	GLuint shader,
	const GLchar *pEntryPoint,
	GLuint numSpecializationConstants,
	const GLuint *pConstantIndex,
	const GLuint *pConstantValue)
{
	RenderCommand_specializeShader* __restrict packet = (RenderCommand_specializeShader*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_specializeShader::execute);
	packet->shader = shader;
	packet->pEntryPoint = pEntryPoint;
	packet->numSpecializationConstants = numSpecializationConstants;
	packet->pConstantIndex = pConstantIndex;
	packet->pConstantValue = pConstantValue;
}
void multiDrawArraysIndirectCount(
	GLenum mode,
	const void *indirect,
	GLintptr drawcount,
	GLsizei maxdrawcount,
	GLsizei stride)
{
	RenderCommand_multiDrawArraysIndirectCount* __restrict packet = (RenderCommand_multiDrawArraysIndirectCount*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiDrawArraysIndirectCount::execute);
	packet->mode = mode;
	packet->indirect = indirect;
	packet->drawcount = drawcount;
	packet->maxdrawcount = maxdrawcount;
	packet->stride = stride;
}
void multiDrawElementsIndirectCount(
	GLenum mode,
	GLenum type,
	const void *indirect,
	GLintptr drawcount,
	GLsizei maxdrawcount,
	GLsizei stride)
{
	RenderCommand_multiDrawElementsIndirectCount* __restrict packet = (RenderCommand_multiDrawElementsIndirectCount*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiDrawElementsIndirectCount::execute);
	packet->mode = mode;
	packet->type = type;
	packet->indirect = indirect;
	packet->drawcount = drawcount;
	packet->maxdrawcount = maxdrawcount;
	packet->stride = stride;
}
void polygonOffsetClamp(
	GLfloat factor,
	GLfloat units,
	GLfloat clamp)
{
	RenderCommand_polygonOffsetClamp* __restrict packet = (RenderCommand_polygonOffsetClamp*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_polygonOffsetClamp::execute);
	packet->factor = factor;
	packet->units = units;
	packet->clamp = clamp;
}
void primitiveBoundingBoxARB(
	GLfloat minX,
	GLfloat minY,
	GLfloat minZ,
	GLfloat minW,
	GLfloat maxX,
	GLfloat maxY,
	GLfloat maxZ,
	GLfloat maxW)
{
	RenderCommand_primitiveBoundingBoxARB* __restrict packet = (RenderCommand_primitiveBoundingBoxARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_primitiveBoundingBoxARB::execute);
	packet->minX = minX;
	packet->minY = minY;
	packet->minZ = minZ;
	packet->minW = minW;
	packet->maxX = maxX;
	packet->maxY = maxY;
	packet->maxZ = maxZ;
	packet->maxW = maxW;
}
GLuint64 getTextureHandleARB(
	GLuint texture)
{
	RenderCommand_getTextureHandleARB* __restrict packet = (RenderCommand_getTextureHandleARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTextureHandleARB::execute);
	packet->texture = texture;
}
GLuint64 getTextureSamplerHandleARB(
	GLuint texture,
	GLuint sampler)
{
	RenderCommand_getTextureSamplerHandleARB* __restrict packet = (RenderCommand_getTextureSamplerHandleARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTextureSamplerHandleARB::execute);
	packet->texture = texture;
	packet->sampler = sampler;
}
void makeTextureHandleResidentARB(
	GLuint64 handle)
{
	RenderCommand_makeTextureHandleResidentARB* __restrict packet = (RenderCommand_makeTextureHandleResidentARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_makeTextureHandleResidentARB::execute);
	packet->handle = handle;
}
void makeTextureHandleNonResidentARB(
	GLuint64 handle)
{
	RenderCommand_makeTextureHandleNonResidentARB* __restrict packet = (RenderCommand_makeTextureHandleNonResidentARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_makeTextureHandleNonResidentARB::execute);
	packet->handle = handle;
}
GLuint64 getImageHandleARB(
	GLuint texture,
	GLint level,
	GLboolean layered,
	GLint layer,
	GLenum format)
{
	RenderCommand_getImageHandleARB* __restrict packet = (RenderCommand_getImageHandleARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getImageHandleARB::execute);
	packet->texture = texture;
	packet->level = level;
	packet->layered = layered;
	packet->layer = layer;
	packet->format = format;
}
void makeImageHandleResidentARB(
	GLuint64 handle,
	GLenum access)
{
	RenderCommand_makeImageHandleResidentARB* __restrict packet = (RenderCommand_makeImageHandleResidentARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_makeImageHandleResidentARB::execute);
	packet->handle = handle;
	packet->access = access;
}
void makeImageHandleNonResidentARB(
	GLuint64 handle)
{
	RenderCommand_makeImageHandleNonResidentARB* __restrict packet = (RenderCommand_makeImageHandleNonResidentARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_makeImageHandleNonResidentARB::execute);
	packet->handle = handle;
}
void uniformHandleui64ARB(
	GLint location,
	GLuint64 value)
{
	RenderCommand_uniformHandleui64ARB* __restrict packet = (RenderCommand_uniformHandleui64ARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformHandleui64ARB::execute);
	packet->location = location;
	packet->value = value;
}
void uniformHandleui64vARB(
	GLint location,
	GLsizei count,
	const GLuint64 *value)
{
	RenderCommand_uniformHandleui64vARB* __restrict packet = (RenderCommand_uniformHandleui64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformHandleui64vARB::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniformHandleui64ARB(
	GLuint program,
	GLint location,
	GLuint64 value)
{
	RenderCommand_programUniformHandleui64ARB* __restrict packet = (RenderCommand_programUniformHandleui64ARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformHandleui64ARB::execute);
	packet->program = program;
	packet->location = location;
	packet->value = value;
}
void programUniformHandleui64vARB(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLuint64 *values)
{
	RenderCommand_programUniformHandleui64vARB* __restrict packet = (RenderCommand_programUniformHandleui64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformHandleui64vARB::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->values = values;
}
GLboolean isTextureHandleResidentARB(
	GLuint64 handle)
{
	RenderCommand_isTextureHandleResidentARB* __restrict packet = (RenderCommand_isTextureHandleResidentARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isTextureHandleResidentARB::execute);
	packet->handle = handle;
}
GLboolean isImageHandleResidentARB(
	GLuint64 handle)
{
	RenderCommand_isImageHandleResidentARB* __restrict packet = (RenderCommand_isImageHandleResidentARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isImageHandleResidentARB::execute);
	packet->handle = handle;
}
void vertexAttribL1ui64ARB(
	GLuint index,
	GLuint64EXT x)
{
	RenderCommand_vertexAttribL1ui64ARB* __restrict packet = (RenderCommand_vertexAttribL1ui64ARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL1ui64ARB::execute);
	packet->index = index;
	packet->x = x;
}
void vertexAttribL1ui64vARB(
	GLuint index,
	const GLuint64EXT *v)
{
	RenderCommand_vertexAttribL1ui64vARB* __restrict packet = (RenderCommand_vertexAttribL1ui64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL1ui64vARB::execute);
	packet->index = index;
	packet->v = v;
}
void getVertexAttribLui64vARB(
	GLuint index,
	GLenum pname,
	GLuint64EXT *params)
{
	RenderCommand_getVertexAttribLui64vARB* __restrict packet = (RenderCommand_getVertexAttribLui64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getVertexAttribLui64vARB::execute);
	packet->index = index;
	packet->pname = pname;
	packet->params = params;
}
GLsync createSyncFromCLeventARB(
	struct _cl_context *context,
	struct _cl_event *event,
	GLbitfield flags)
{
	RenderCommand_createSyncFromCLeventARB* __restrict packet = (RenderCommand_createSyncFromCLeventARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_createSyncFromCLeventARB::execute);
	packet->context = context;
	packet->event = event;
	packet->flags = flags;
}
void dispatchComputeGroupSizeARB(
	GLuint num_groups_x,
	GLuint num_groups_y,
	GLuint num_groups_z,
	GLuint group_size_x,
	GLuint group_size_y,
	GLuint group_size_z)
{
	RenderCommand_dispatchComputeGroupSizeARB* __restrict packet = (RenderCommand_dispatchComputeGroupSizeARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_dispatchComputeGroupSizeARB::execute);
	packet->num_groups_x = num_groups_x;
	packet->num_groups_y = num_groups_y;
	packet->num_groups_z = num_groups_z;
	packet->group_size_x = group_size_x;
	packet->group_size_y = group_size_y;
	packet->group_size_z = group_size_z;
}
void debugMessageControlARB(
	GLenum source,
	GLenum type,
	GLenum severity,
	GLsizei count,
	const GLuint *ids,
	GLboolean enabled)
{
	RenderCommand_debugMessageControlARB* __restrict packet = (RenderCommand_debugMessageControlARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_debugMessageControlARB::execute);
	packet->source = source;
	packet->type = type;
	packet->severity = severity;
	packet->count = count;
	packet->ids = ids;
	packet->enabled = enabled;
}
void debugMessageInsertARB(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *buf)
{
	RenderCommand_debugMessageInsertARB* __restrict packet = (RenderCommand_debugMessageInsertARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_debugMessageInsertARB::execute);
	packet->source = source;
	packet->type = type;
	packet->id = id;
	packet->severity = severity;
	packet->length = length;
	packet->buf = buf;
}
void debugMessageCallbackARB(
	GLDEBUGPROCARB callback,
	const void *userParam)
{
	RenderCommand_debugMessageCallbackARB* __restrict packet = (RenderCommand_debugMessageCallbackARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_debugMessageCallbackARB::execute);
	packet->callback = callback;
	packet->userParam = userParam;
}
GLuint getDebugMessageLogARB(
	GLuint count,
	GLsizei bufSize,
	GLenum *sources,
	GLenum *types,
	GLuint *ids,
	GLenum *severities,
	GLsizei *lengths,
	GLchar *messageLog)
{
	RenderCommand_getDebugMessageLogARB* __restrict packet = (RenderCommand_getDebugMessageLogARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getDebugMessageLogARB::execute);
	packet->count = count;
	packet->bufSize = bufSize;
	packet->sources = sources;
	packet->types = types;
	packet->ids = ids;
	packet->severities = severities;
	packet->lengths = lengths;
	packet->messageLog = messageLog;
}
void blendEquationiARB(
	GLuint buf,
	GLenum mode)
{
	RenderCommand_blendEquationiARB* __restrict packet = (RenderCommand_blendEquationiARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_blendEquationiARB::execute);
	packet->buf = buf;
	packet->mode = mode;
}
void blendEquationSeparateiARB(
	GLuint buf,
	GLenum modeRGB,
	GLenum modeAlpha)
{
	RenderCommand_blendEquationSeparateiARB* __restrict packet = (RenderCommand_blendEquationSeparateiARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_blendEquationSeparateiARB::execute);
	packet->buf = buf;
	packet->modeRGB = modeRGB;
	packet->modeAlpha = modeAlpha;
}
void blendFunciARB(
	GLuint buf,
	GLenum src,
	GLenum dst)
{
	RenderCommand_blendFunciARB* __restrict packet = (RenderCommand_blendFunciARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_blendFunciARB::execute);
	packet->buf = buf;
	packet->src = src;
	packet->dst = dst;
}
void blendFuncSeparateiARB(
	GLuint buf,
	GLenum srcRGB,
	GLenum dstRGB,
	GLenum srcAlpha,
	GLenum dstAlpha)
{
	RenderCommand_blendFuncSeparateiARB* __restrict packet = (RenderCommand_blendFuncSeparateiARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_blendFuncSeparateiARB::execute);
	packet->buf = buf;
	packet->srcRGB = srcRGB;
	packet->dstRGB = dstRGB;
	packet->srcAlpha = srcAlpha;
	packet->dstAlpha = dstAlpha;
}
void drawArraysInstancedARB(
	GLenum mode,
	GLint first,
	GLsizei count,
	GLsizei primcount)
{
	RenderCommand_drawArraysInstancedARB* __restrict packet = (RenderCommand_drawArraysInstancedARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawArraysInstancedARB::execute);
	packet->mode = mode;
	packet->first = first;
	packet->count = count;
	packet->primcount = primcount;
}
void drawElementsInstancedARB(
	GLenum mode,
	GLsizei count,
	GLenum type,
	const void *indices,
	GLsizei primcount)
{
	RenderCommand_drawElementsInstancedARB* __restrict packet = (RenderCommand_drawElementsInstancedARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawElementsInstancedARB::execute);
	packet->mode = mode;
	packet->count = count;
	packet->type = type;
	packet->indices = indices;
	packet->primcount = primcount;
}
void programParameteriARB(
	GLuint program,
	GLenum pname,
	GLint value)
{
	RenderCommand_programParameteriARB* __restrict packet = (RenderCommand_programParameteriARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programParameteriARB::execute);
	packet->program = program;
	packet->pname = pname;
	packet->value = value;
}
void framebufferTextureARB(
	GLenum target,
	GLenum attachment,
	GLuint texture,
	GLint level)
{
	RenderCommand_framebufferTextureARB* __restrict packet = (RenderCommand_framebufferTextureARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_framebufferTextureARB::execute);
	packet->target = target;
	packet->attachment = attachment;
	packet->texture = texture;
	packet->level = level;
}
void framebufferTextureLayerARB(
	GLenum target,
	GLenum attachment,
	GLuint texture,
	GLint level,
	GLint layer)
{
	RenderCommand_framebufferTextureLayerARB* __restrict packet = (RenderCommand_framebufferTextureLayerARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_framebufferTextureLayerARB::execute);
	packet->target = target;
	packet->attachment = attachment;
	packet->texture = texture;
	packet->level = level;
	packet->layer = layer;
}
void framebufferTextureFaceARB(
	GLenum target,
	GLenum attachment,
	GLuint texture,
	GLint level,
	GLenum face)
{
	RenderCommand_framebufferTextureFaceARB* __restrict packet = (RenderCommand_framebufferTextureFaceARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_framebufferTextureFaceARB::execute);
	packet->target = target;
	packet->attachment = attachment;
	packet->texture = texture;
	packet->level = level;
	packet->face = face;
}
void specializeShaderARB(
	GLuint shader,
	const GLchar *pEntryPoint,
	GLuint numSpecializationConstants,
	const GLuint *pConstantIndex,
	const GLuint *pConstantValue)
{
	RenderCommand_specializeShaderARB* __restrict packet = (RenderCommand_specializeShaderARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_specializeShaderARB::execute);
	packet->shader = shader;
	packet->pEntryPoint = pEntryPoint;
	packet->numSpecializationConstants = numSpecializationConstants;
	packet->pConstantIndex = pConstantIndex;
	packet->pConstantValue = pConstantValue;
}
void uniform1i64ARB(
	GLint location,
	GLint64 x)
{
	RenderCommand_uniform1i64ARB* __restrict packet = (RenderCommand_uniform1i64ARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform1i64ARB::execute);
	packet->location = location;
	packet->x = x;
}
void uniform2i64ARB(
	GLint location,
	GLint64 x,
	GLint64 y)
{
	RenderCommand_uniform2i64ARB* __restrict packet = (RenderCommand_uniform2i64ARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform2i64ARB::execute);
	packet->location = location;
	packet->x = x;
	packet->y = y;
}
void uniform3i64ARB(
	GLint location,
	GLint64 x,
	GLint64 y,
	GLint64 z)
{
	RenderCommand_uniform3i64ARB* __restrict packet = (RenderCommand_uniform3i64ARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform3i64ARB::execute);
	packet->location = location;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void uniform4i64ARB(
	GLint location,
	GLint64 x,
	GLint64 y,
	GLint64 z,
	GLint64 w)
{
	RenderCommand_uniform4i64ARB* __restrict packet = (RenderCommand_uniform4i64ARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform4i64ARB::execute);
	packet->location = location;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void uniform1i64vARB(
	GLint location,
	GLsizei count,
	const GLint64 *value)
{
	RenderCommand_uniform1i64vARB* __restrict packet = (RenderCommand_uniform1i64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform1i64vARB::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform2i64vARB(
	GLint location,
	GLsizei count,
	const GLint64 *value)
{
	RenderCommand_uniform2i64vARB* __restrict packet = (RenderCommand_uniform2i64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform2i64vARB::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform3i64vARB(
	GLint location,
	GLsizei count,
	const GLint64 *value)
{
	RenderCommand_uniform3i64vARB* __restrict packet = (RenderCommand_uniform3i64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform3i64vARB::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform4i64vARB(
	GLint location,
	GLsizei count,
	const GLint64 *value)
{
	RenderCommand_uniform4i64vARB* __restrict packet = (RenderCommand_uniform4i64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform4i64vARB::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform1ui64ARB(
	GLint location,
	GLuint64 x)
{
	RenderCommand_uniform1ui64ARB* __restrict packet = (RenderCommand_uniform1ui64ARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform1ui64ARB::execute);
	packet->location = location;
	packet->x = x;
}
void uniform2ui64ARB(
	GLint location,
	GLuint64 x,
	GLuint64 y)
{
	RenderCommand_uniform2ui64ARB* __restrict packet = (RenderCommand_uniform2ui64ARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform2ui64ARB::execute);
	packet->location = location;
	packet->x = x;
	packet->y = y;
}
void uniform3ui64ARB(
	GLint location,
	GLuint64 x,
	GLuint64 y,
	GLuint64 z)
{
	RenderCommand_uniform3ui64ARB* __restrict packet = (RenderCommand_uniform3ui64ARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform3ui64ARB::execute);
	packet->location = location;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void uniform4ui64ARB(
	GLint location,
	GLuint64 x,
	GLuint64 y,
	GLuint64 z,
	GLuint64 w)
{
	RenderCommand_uniform4ui64ARB* __restrict packet = (RenderCommand_uniform4ui64ARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform4ui64ARB::execute);
	packet->location = location;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void uniform1ui64vARB(
	GLint location,
	GLsizei count,
	const GLuint64 *value)
{
	RenderCommand_uniform1ui64vARB* __restrict packet = (RenderCommand_uniform1ui64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform1ui64vARB::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform2ui64vARB(
	GLint location,
	GLsizei count,
	const GLuint64 *value)
{
	RenderCommand_uniform2ui64vARB* __restrict packet = (RenderCommand_uniform2ui64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform2ui64vARB::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform3ui64vARB(
	GLint location,
	GLsizei count,
	const GLuint64 *value)
{
	RenderCommand_uniform3ui64vARB* __restrict packet = (RenderCommand_uniform3ui64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform3ui64vARB::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform4ui64vARB(
	GLint location,
	GLsizei count,
	const GLuint64 *value)
{
	RenderCommand_uniform4ui64vARB* __restrict packet = (RenderCommand_uniform4ui64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform4ui64vARB::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void getUniformi64vARB(
	GLuint program,
	GLint location,
	GLint64 *params)
{
	RenderCommand_getUniformi64vARB* __restrict packet = (RenderCommand_getUniformi64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getUniformi64vARB::execute);
	packet->program = program;
	packet->location = location;
	packet->params = params;
}
void getUniformui64vARB(
	GLuint program,
	GLint location,
	GLuint64 *params)
{
	RenderCommand_getUniformui64vARB* __restrict packet = (RenderCommand_getUniformui64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getUniformui64vARB::execute);
	packet->program = program;
	packet->location = location;
	packet->params = params;
}
void getnUniformi64vARB(
	GLuint program,
	GLint location,
	GLsizei bufSize,
	GLint64 *params)
{
	RenderCommand_getnUniformi64vARB* __restrict packet = (RenderCommand_getnUniformi64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getnUniformi64vARB::execute);
	packet->program = program;
	packet->location = location;
	packet->bufSize = bufSize;
	packet->params = params;
}
void getnUniformui64vARB(
	GLuint program,
	GLint location,
	GLsizei bufSize,
	GLuint64 *params)
{
	RenderCommand_getnUniformui64vARB* __restrict packet = (RenderCommand_getnUniformui64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getnUniformui64vARB::execute);
	packet->program = program;
	packet->location = location;
	packet->bufSize = bufSize;
	packet->params = params;
}
void programUniform1i64ARB(
	GLuint program,
	GLint location,
	GLint64 x)
{
	RenderCommand_programUniform1i64ARB* __restrict packet = (RenderCommand_programUniform1i64ARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1i64ARB::execute);
	packet->program = program;
	packet->location = location;
	packet->x = x;
}
void programUniform2i64ARB(
	GLuint program,
	GLint location,
	GLint64 x,
	GLint64 y)
{
	RenderCommand_programUniform2i64ARB* __restrict packet = (RenderCommand_programUniform2i64ARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2i64ARB::execute);
	packet->program = program;
	packet->location = location;
	packet->x = x;
	packet->y = y;
}
void programUniform3i64ARB(
	GLuint program,
	GLint location,
	GLint64 x,
	GLint64 y,
	GLint64 z)
{
	RenderCommand_programUniform3i64ARB* __restrict packet = (RenderCommand_programUniform3i64ARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3i64ARB::execute);
	packet->program = program;
	packet->location = location;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void programUniform4i64ARB(
	GLuint program,
	GLint location,
	GLint64 x,
	GLint64 y,
	GLint64 z,
	GLint64 w)
{
	RenderCommand_programUniform4i64ARB* __restrict packet = (RenderCommand_programUniform4i64ARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4i64ARB::execute);
	packet->program = program;
	packet->location = location;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void programUniform1i64vARB(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLint64 *value)
{
	RenderCommand_programUniform1i64vARB* __restrict packet = (RenderCommand_programUniform1i64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1i64vARB::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform2i64vARB(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLint64 *value)
{
	RenderCommand_programUniform2i64vARB* __restrict packet = (RenderCommand_programUniform2i64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2i64vARB::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform3i64vARB(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLint64 *value)
{
	RenderCommand_programUniform3i64vARB* __restrict packet = (RenderCommand_programUniform3i64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3i64vARB::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform4i64vARB(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLint64 *value)
{
	RenderCommand_programUniform4i64vARB* __restrict packet = (RenderCommand_programUniform4i64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4i64vARB::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform1ui64ARB(
	GLuint program,
	GLint location,
	GLuint64 x)
{
	RenderCommand_programUniform1ui64ARB* __restrict packet = (RenderCommand_programUniform1ui64ARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1ui64ARB::execute);
	packet->program = program;
	packet->location = location;
	packet->x = x;
}
void programUniform2ui64ARB(
	GLuint program,
	GLint location,
	GLuint64 x,
	GLuint64 y)
{
	RenderCommand_programUniform2ui64ARB* __restrict packet = (RenderCommand_programUniform2ui64ARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2ui64ARB::execute);
	packet->program = program;
	packet->location = location;
	packet->x = x;
	packet->y = y;
}
void programUniform3ui64ARB(
	GLuint program,
	GLint location,
	GLuint64 x,
	GLuint64 y,
	GLuint64 z)
{
	RenderCommand_programUniform3ui64ARB* __restrict packet = (RenderCommand_programUniform3ui64ARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3ui64ARB::execute);
	packet->program = program;
	packet->location = location;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void programUniform4ui64ARB(
	GLuint program,
	GLint location,
	GLuint64 x,
	GLuint64 y,
	GLuint64 z,
	GLuint64 w)
{
	RenderCommand_programUniform4ui64ARB* __restrict packet = (RenderCommand_programUniform4ui64ARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4ui64ARB::execute);
	packet->program = program;
	packet->location = location;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void programUniform1ui64vARB(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLuint64 *value)
{
	RenderCommand_programUniform1ui64vARB* __restrict packet = (RenderCommand_programUniform1ui64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1ui64vARB::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform2ui64vARB(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLuint64 *value)
{
	RenderCommand_programUniform2ui64vARB* __restrict packet = (RenderCommand_programUniform2ui64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2ui64vARB::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform3ui64vARB(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLuint64 *value)
{
	RenderCommand_programUniform3ui64vARB* __restrict packet = (RenderCommand_programUniform3ui64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3ui64vARB::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform4ui64vARB(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLuint64 *value)
{
	RenderCommand_programUniform4ui64vARB* __restrict packet = (RenderCommand_programUniform4ui64vARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4ui64vARB::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void multiDrawArraysIndirectCountARB(
	GLenum mode,
	const void *indirect,
	GLintptr drawcount,
	GLsizei maxdrawcount,
	GLsizei stride)
{
	RenderCommand_multiDrawArraysIndirectCountARB* __restrict packet = (RenderCommand_multiDrawArraysIndirectCountARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiDrawArraysIndirectCountARB::execute);
	packet->mode = mode;
	packet->indirect = indirect;
	packet->drawcount = drawcount;
	packet->maxdrawcount = maxdrawcount;
	packet->stride = stride;
}
void multiDrawElementsIndirectCountARB(
	GLenum mode,
	GLenum type,
	const void *indirect,
	GLintptr drawcount,
	GLsizei maxdrawcount,
	GLsizei stride)
{
	RenderCommand_multiDrawElementsIndirectCountARB* __restrict packet = (RenderCommand_multiDrawElementsIndirectCountARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiDrawElementsIndirectCountARB::execute);
	packet->mode = mode;
	packet->type = type;
	packet->indirect = indirect;
	packet->drawcount = drawcount;
	packet->maxdrawcount = maxdrawcount;
	packet->stride = stride;
}
void vertexAttribDivisorARB(
	GLuint index,
	GLuint divisor)
{
	RenderCommand_vertexAttribDivisorARB* __restrict packet = (RenderCommand_vertexAttribDivisorARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribDivisorARB::execute);
	packet->index = index;
	packet->divisor = divisor;
}
void maxShaderCompilerThreadsARB(
	GLuint count)
{
	RenderCommand_maxShaderCompilerThreadsARB* __restrict packet = (RenderCommand_maxShaderCompilerThreadsARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_maxShaderCompilerThreadsARB::execute);
	packet->count = count;
}
GLenum getGraphicsResetStatusARB(
	void)
{
	RenderCommand_getGraphicsResetStatusARB* __restrict packet = (RenderCommand_getGraphicsResetStatusARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getGraphicsResetStatusARB::execute);
}
void getnTexImageARB(
	GLenum target,
	GLint level,
	GLenum format,
	GLenum type,
	GLsizei bufSize,
	void *img)
{
	RenderCommand_getnTexImageARB* __restrict packet = (RenderCommand_getnTexImageARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getnTexImageARB::execute);
	packet->target = target;
	packet->level = level;
	packet->format = format;
	packet->type = type;
	packet->bufSize = bufSize;
	packet->img = img;
}
void readnPixelsARB(
	GLint x,
	GLint y,
	GLsizei width,
	GLsizei height,
	GLenum format,
	GLenum type,
	GLsizei bufSize,
	void *data)
{
	RenderCommand_readnPixelsARB* __restrict packet = (RenderCommand_readnPixelsARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_readnPixelsARB::execute);
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->height = height;
	packet->format = format;
	packet->type = type;
	packet->bufSize = bufSize;
	packet->data = data;
}
void getnCompressedTexImageARB(
	GLenum target,
	GLint lod,
	GLsizei bufSize,
	void *img)
{
	RenderCommand_getnCompressedTexImageARB* __restrict packet = (RenderCommand_getnCompressedTexImageARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getnCompressedTexImageARB::execute);
	packet->target = target;
	packet->lod = lod;
	packet->bufSize = bufSize;
	packet->img = img;
}
void getnUniformfvARB(
	GLuint program,
	GLint location,
	GLsizei bufSize,
	GLfloat *params)
{
	RenderCommand_getnUniformfvARB* __restrict packet = (RenderCommand_getnUniformfvARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getnUniformfvARB::execute);
	packet->program = program;
	packet->location = location;
	packet->bufSize = bufSize;
	packet->params = params;
}
void getnUniformivARB(
	GLuint program,
	GLint location,
	GLsizei bufSize,
	GLint *params)
{
	RenderCommand_getnUniformivARB* __restrict packet = (RenderCommand_getnUniformivARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getnUniformivARB::execute);
	packet->program = program;
	packet->location = location;
	packet->bufSize = bufSize;
	packet->params = params;
}
void getnUniformuivARB(
	GLuint program,
	GLint location,
	GLsizei bufSize,
	GLuint *params)
{
	RenderCommand_getnUniformuivARB* __restrict packet = (RenderCommand_getnUniformuivARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getnUniformuivARB::execute);
	packet->program = program;
	packet->location = location;
	packet->bufSize = bufSize;
	packet->params = params;
}
void getnUniformdvARB(
	GLuint program,
	GLint location,
	GLsizei bufSize,
	GLdouble *params)
{
	RenderCommand_getnUniformdvARB* __restrict packet = (RenderCommand_getnUniformdvARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getnUniformdvARB::execute);
	packet->program = program;
	packet->location = location;
	packet->bufSize = bufSize;
	packet->params = params;
}
void framebufferSampleLocationsfvARB(
	GLenum target,
	GLuint start,
	GLsizei count,
	const GLfloat *v)
{
	RenderCommand_framebufferSampleLocationsfvARB* __restrict packet = (RenderCommand_framebufferSampleLocationsfvARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_framebufferSampleLocationsfvARB::execute);
	packet->target = target;
	packet->start = start;
	packet->count = count;
	packet->v = v;
}
void namedFramebufferSampleLocationsfvARB(
	GLuint framebuffer,
	GLuint start,
	GLsizei count,
	const GLfloat *v)
{
	RenderCommand_namedFramebufferSampleLocationsfvARB* __restrict packet = (RenderCommand_namedFramebufferSampleLocationsfvARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedFramebufferSampleLocationsfvARB::execute);
	packet->framebuffer = framebuffer;
	packet->start = start;
	packet->count = count;
	packet->v = v;
}
void evaluateDepthValuesARB(
	void)
{
	RenderCommand_evaluateDepthValuesARB* __restrict packet = (RenderCommand_evaluateDepthValuesARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_evaluateDepthValuesARB::execute);
}
void minSampleShadingARB(
	GLfloat value)
{
	RenderCommand_minSampleShadingARB* __restrict packet = (RenderCommand_minSampleShadingARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_minSampleShadingARB::execute);
	packet->value = value;
}
void namedStringARB(
	GLenum type,
	GLint namelen,
	const GLchar *name,
	GLint stringlen,
	const GLchar *string)
{
	RenderCommand_namedStringARB* __restrict packet = (RenderCommand_namedStringARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedStringARB::execute);
	packet->type = type;
	packet->namelen = namelen;
	packet->name = name;
	packet->stringlen = stringlen;
	packet->string = string;
}
void deleteNamedStringARB(
	GLint namelen,
	const GLchar *name)
{
	RenderCommand_deleteNamedStringARB* __restrict packet = (RenderCommand_deleteNamedStringARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_deleteNamedStringARB::execute);
	packet->namelen = namelen;
	packet->name = name;
}
void compileShaderIncludeARB(
	GLuint shader,
	GLsizei count,
	const GLchar *const*path,
	const GLint *length)
{
	RenderCommand_compileShaderIncludeARB* __restrict packet = (RenderCommand_compileShaderIncludeARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compileShaderIncludeARB::execute);
	packet->shader = shader;
	packet->count = count;
	packet->path = path;
	packet->length = length;
}
GLboolean isNamedStringARB(
	GLint namelen,
	const GLchar *name)
{
	RenderCommand_isNamedStringARB* __restrict packet = (RenderCommand_isNamedStringARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isNamedStringARB::execute);
	packet->namelen = namelen;
	packet->name = name;
}
void getNamedStringARB(
	GLint namelen,
	const GLchar *name,
	GLsizei bufSize,
	GLint *stringlen,
	GLchar *string)
{
	RenderCommand_getNamedStringARB* __restrict packet = (RenderCommand_getNamedStringARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedStringARB::execute);
	packet->namelen = namelen;
	packet->name = name;
	packet->bufSize = bufSize;
	packet->stringlen = stringlen;
	packet->string = string;
}
void getNamedStringivARB(
	GLint namelen,
	const GLchar *name,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getNamedStringivARB* __restrict packet = (RenderCommand_getNamedStringivARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedStringivARB::execute);
	packet->namelen = namelen;
	packet->name = name;
	packet->pname = pname;
	packet->params = params;
}
void bufferPageCommitmentARB(
	GLenum target,
	GLintptr offset,
	GLsizeiptr size,
	GLboolean commit)
{
	RenderCommand_bufferPageCommitmentARB* __restrict packet = (RenderCommand_bufferPageCommitmentARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bufferPageCommitmentARB::execute);
	packet->target = target;
	packet->offset = offset;
	packet->size = size;
	packet->commit = commit;
}
void namedBufferPageCommitmentEXT(
	GLuint buffer,
	GLintptr offset,
	GLsizeiptr size,
	GLboolean commit)
{
	RenderCommand_namedBufferPageCommitmentEXT* __restrict packet = (RenderCommand_namedBufferPageCommitmentEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedBufferPageCommitmentEXT::execute);
	packet->buffer = buffer;
	packet->offset = offset;
	packet->size = size;
	packet->commit = commit;
}
void namedBufferPageCommitmentARB(
	GLuint buffer,
	GLintptr offset,
	GLsizeiptr size,
	GLboolean commit)
{
	RenderCommand_namedBufferPageCommitmentARB* __restrict packet = (RenderCommand_namedBufferPageCommitmentARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedBufferPageCommitmentARB::execute);
	packet->buffer = buffer;
	packet->offset = offset;
	packet->size = size;
	packet->commit = commit;
}
void texPageCommitmentARB(
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint zoffset,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLboolean commit)
{
	RenderCommand_texPageCommitmentARB* __restrict packet = (RenderCommand_texPageCommitmentARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texPageCommitmentARB::execute);
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->zoffset = zoffset;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->commit = commit;
}
void texBufferARB(
	GLenum target,
	GLenum internalformat,
	GLuint buffer)
{
	RenderCommand_texBufferARB* __restrict packet = (RenderCommand_texBufferARB*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texBufferARB::execute);
	packet->target = target;
	packet->internalformat = internalformat;
	packet->buffer = buffer;
}
void blendBarrierKHR(
	void)
{
	RenderCommand_blendBarrierKHR* __restrict packet = (RenderCommand_blendBarrierKHR*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_blendBarrierKHR::execute);
}
void maxShaderCompilerThreadsKHR(
	GLuint count)
{
	RenderCommand_maxShaderCompilerThreadsKHR* __restrict packet = (RenderCommand_maxShaderCompilerThreadsKHR*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_maxShaderCompilerThreadsKHR::execute);
	packet->count = count;
}
void renderbufferStorageMultisampleAdvancedAMD(
	GLenum target,
	GLsizei samples,
	GLsizei storageSamples,
	GLenum internalformat,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_renderbufferStorageMultisampleAdvancedAMD* __restrict packet = (RenderCommand_renderbufferStorageMultisampleAdvancedAMD*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_renderbufferStorageMultisampleAdvancedAMD::execute);
	packet->target = target;
	packet->samples = samples;
	packet->storageSamples = storageSamples;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
}
void namedRenderbufferStorageMultisampleAdvancedAMD(
	GLuint renderbuffer,
	GLsizei samples,
	GLsizei storageSamples,
	GLenum internalformat,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_namedRenderbufferStorageMultisampleAdvancedAMD* __restrict packet = (RenderCommand_namedRenderbufferStorageMultisampleAdvancedAMD*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedRenderbufferStorageMultisampleAdvancedAMD::execute);
	packet->renderbuffer = renderbuffer;
	packet->samples = samples;
	packet->storageSamples = storageSamples;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
}
void getPerfMonitorGroupsAMD(
	GLint *numGroups,
	GLsizei groupsSize,
	GLuint *groups)
{
	RenderCommand_getPerfMonitorGroupsAMD* __restrict packet = (RenderCommand_getPerfMonitorGroupsAMD*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPerfMonitorGroupsAMD::execute);
	packet->numGroups = numGroups;
	packet->groupsSize = groupsSize;
	packet->groups = groups;
}
void getPerfMonitorCountersAMD(
	GLuint group,
	GLint *numCounters,
	GLint *maxActiveCounters,
	GLsizei counterSize,
	GLuint *counters)
{
	RenderCommand_getPerfMonitorCountersAMD* __restrict packet = (RenderCommand_getPerfMonitorCountersAMD*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPerfMonitorCountersAMD::execute);
	packet->group = group;
	packet->numCounters = numCounters;
	packet->maxActiveCounters = maxActiveCounters;
	packet->counterSize = counterSize;
	packet->counters = counters;
}
void getPerfMonitorGroupStringAMD(
	GLuint group,
	GLsizei bufSize,
	GLsizei *length,
	GLchar *groupString)
{
	RenderCommand_getPerfMonitorGroupStringAMD* __restrict packet = (RenderCommand_getPerfMonitorGroupStringAMD*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPerfMonitorGroupStringAMD::execute);
	packet->group = group;
	packet->bufSize = bufSize;
	packet->length = length;
	packet->groupString = groupString;
}
void getPerfMonitorCounterStringAMD(
	GLuint group,
	GLuint counter,
	GLsizei bufSize,
	GLsizei *length,
	GLchar *counterString)
{
	RenderCommand_getPerfMonitorCounterStringAMD* __restrict packet = (RenderCommand_getPerfMonitorCounterStringAMD*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPerfMonitorCounterStringAMD::execute);
	packet->group = group;
	packet->counter = counter;
	packet->bufSize = bufSize;
	packet->length = length;
	packet->counterString = counterString;
}
void getPerfMonitorCounterInfoAMD(
	GLuint group,
	GLuint counter,
	GLenum pname,
	void *data)
{
	RenderCommand_getPerfMonitorCounterInfoAMD* __restrict packet = (RenderCommand_getPerfMonitorCounterInfoAMD*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPerfMonitorCounterInfoAMD::execute);
	packet->group = group;
	packet->counter = counter;
	packet->pname = pname;
	packet->data = data;
}
void genPerfMonitorsAMD(
	GLsizei n,
	GLuint *monitors)
{
	RenderCommand_genPerfMonitorsAMD* __restrict packet = (RenderCommand_genPerfMonitorsAMD*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_genPerfMonitorsAMD::execute);
	packet->n = n;
	packet->monitors = monitors;
}
void deletePerfMonitorsAMD(
	GLsizei n,
	GLuint *monitors)
{
	RenderCommand_deletePerfMonitorsAMD* __restrict packet = (RenderCommand_deletePerfMonitorsAMD*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_deletePerfMonitorsAMD::execute);
	packet->n = n;
	packet->monitors = monitors;
}
void selectPerfMonitorCountersAMD(
	GLuint monitor,
	GLboolean enable,
	GLuint group,
	GLint numCounters,
	GLuint *counterList)
{
	RenderCommand_selectPerfMonitorCountersAMD* __restrict packet = (RenderCommand_selectPerfMonitorCountersAMD*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_selectPerfMonitorCountersAMD::execute);
	packet->monitor = monitor;
	packet->enable = enable;
	packet->group = group;
	packet->numCounters = numCounters;
	packet->counterList = counterList;
}
void beginPerfMonitorAMD(
	GLuint monitor)
{
	RenderCommand_beginPerfMonitorAMD* __restrict packet = (RenderCommand_beginPerfMonitorAMD*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_beginPerfMonitorAMD::execute);
	packet->monitor = monitor;
}
void endPerfMonitorAMD(
	GLuint monitor)
{
	RenderCommand_endPerfMonitorAMD* __restrict packet = (RenderCommand_endPerfMonitorAMD*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_endPerfMonitorAMD::execute);
	packet->monitor = monitor;
}
void getPerfMonitorCounterDataAMD(
	GLuint monitor,
	GLenum pname,
	GLsizei dataSize,
	GLuint *data,
	GLint *bytesWritten)
{
	RenderCommand_getPerfMonitorCounterDataAMD* __restrict packet = (RenderCommand_getPerfMonitorCounterDataAMD*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPerfMonitorCounterDataAMD::execute);
	packet->monitor = monitor;
	packet->pname = pname;
	packet->dataSize = dataSize;
	packet->data = data;
	packet->bytesWritten = bytesWritten;
}
void eGLImageTargetTexStorageEXT(
	GLenum target,
	GLeglImageOES image,
	const GLint* attrib_list)
{
	RenderCommand_eGLImageTargetTexStorageEXT* __restrict packet = (RenderCommand_eGLImageTargetTexStorageEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_eGLImageTargetTexStorageEXT::execute);
	packet->target = target;
	packet->image = image;
	packet->attrib_list = attrib_list;
}
void eGLImageTargetTextureStorageEXT(
	GLuint texture,
	GLeglImageOES image,
	const GLint* attrib_list)
{
	RenderCommand_eGLImageTargetTextureStorageEXT* __restrict packet = (RenderCommand_eGLImageTargetTextureStorageEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_eGLImageTargetTextureStorageEXT::execute);
	packet->texture = texture;
	packet->image = image;
	packet->attrib_list = attrib_list;
}
void labelObjectEXT(
	GLenum type,
	GLuint object,
	GLsizei length,
	const GLchar *label)
{
	RenderCommand_labelObjectEXT* __restrict packet = (RenderCommand_labelObjectEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_labelObjectEXT::execute);
	packet->type = type;
	packet->object = object;
	packet->length = length;
	packet->label = label;
}
void getObjectLabelEXT(
	GLenum type,
	GLuint object,
	GLsizei bufSize,
	GLsizei *length,
	GLchar *label)
{
	RenderCommand_getObjectLabelEXT* __restrict packet = (RenderCommand_getObjectLabelEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getObjectLabelEXT::execute);
	packet->type = type;
	packet->object = object;
	packet->bufSize = bufSize;
	packet->length = length;
	packet->label = label;
}
void insertEventMarkerEXT(
	GLsizei length,
	const GLchar *marker)
{
	RenderCommand_insertEventMarkerEXT* __restrict packet = (RenderCommand_insertEventMarkerEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_insertEventMarkerEXT::execute);
	packet->length = length;
	packet->marker = marker;
}
void pushGroupMarkerEXT(
	GLsizei length,
	const GLchar *marker)
{
	RenderCommand_pushGroupMarkerEXT* __restrict packet = (RenderCommand_pushGroupMarkerEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pushGroupMarkerEXT::execute);
	packet->length = length;
	packet->marker = marker;
}
void popGroupMarkerEXT(
	void)
{
	RenderCommand_popGroupMarkerEXT* __restrict packet = (RenderCommand_popGroupMarkerEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_popGroupMarkerEXT::execute);
}
void matrixLoadfEXT(
	GLenum mode,
	const GLfloat *m)
{
	RenderCommand_matrixLoadfEXT* __restrict packet = (RenderCommand_matrixLoadfEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixLoadfEXT::execute);
	packet->mode = mode;
	packet->m = m;
}
void matrixLoaddEXT(
	GLenum mode,
	const GLdouble *m)
{
	RenderCommand_matrixLoaddEXT* __restrict packet = (RenderCommand_matrixLoaddEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixLoaddEXT::execute);
	packet->mode = mode;
	packet->m = m;
}
void matrixMultfEXT(
	GLenum mode,
	const GLfloat *m)
{
	RenderCommand_matrixMultfEXT* __restrict packet = (RenderCommand_matrixMultfEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixMultfEXT::execute);
	packet->mode = mode;
	packet->m = m;
}
void matrixMultdEXT(
	GLenum mode,
	const GLdouble *m)
{
	RenderCommand_matrixMultdEXT* __restrict packet = (RenderCommand_matrixMultdEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixMultdEXT::execute);
	packet->mode = mode;
	packet->m = m;
}
void matrixLoadIdentityEXT(
	GLenum mode)
{
	RenderCommand_matrixLoadIdentityEXT* __restrict packet = (RenderCommand_matrixLoadIdentityEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixLoadIdentityEXT::execute);
	packet->mode = mode;
}
void matrixRotatefEXT(
	GLenum mode,
	GLfloat angle,
	GLfloat x,
	GLfloat y,
	GLfloat z)
{
	RenderCommand_matrixRotatefEXT* __restrict packet = (RenderCommand_matrixRotatefEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixRotatefEXT::execute);
	packet->mode = mode;
	packet->angle = angle;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void matrixRotatedEXT(
	GLenum mode,
	GLdouble angle,
	GLdouble x,
	GLdouble y,
	GLdouble z)
{
	RenderCommand_matrixRotatedEXT* __restrict packet = (RenderCommand_matrixRotatedEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixRotatedEXT::execute);
	packet->mode = mode;
	packet->angle = angle;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void matrixScalefEXT(
	GLenum mode,
	GLfloat x,
	GLfloat y,
	GLfloat z)
{
	RenderCommand_matrixScalefEXT* __restrict packet = (RenderCommand_matrixScalefEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixScalefEXT::execute);
	packet->mode = mode;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void matrixScaledEXT(
	GLenum mode,
	GLdouble x,
	GLdouble y,
	GLdouble z)
{
	RenderCommand_matrixScaledEXT* __restrict packet = (RenderCommand_matrixScaledEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixScaledEXT::execute);
	packet->mode = mode;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void matrixTranslatefEXT(
	GLenum mode,
	GLfloat x,
	GLfloat y,
	GLfloat z)
{
	RenderCommand_matrixTranslatefEXT* __restrict packet = (RenderCommand_matrixTranslatefEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixTranslatefEXT::execute);
	packet->mode = mode;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void matrixTranslatedEXT(
	GLenum mode,
	GLdouble x,
	GLdouble y,
	GLdouble z)
{
	RenderCommand_matrixTranslatedEXT* __restrict packet = (RenderCommand_matrixTranslatedEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixTranslatedEXT::execute);
	packet->mode = mode;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void matrixFrustumEXT(
	GLenum mode,
	GLdouble left,
	GLdouble right,
	GLdouble bottom,
	GLdouble top,
	GLdouble zNear,
	GLdouble zFar)
{
	RenderCommand_matrixFrustumEXT* __restrict packet = (RenderCommand_matrixFrustumEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixFrustumEXT::execute);
	packet->mode = mode;
	packet->left = left;
	packet->right = right;
	packet->bottom = bottom;
	packet->top = top;
	packet->zNear = zNear;
	packet->zFar = zFar;
}
void matrixOrthoEXT(
	GLenum mode,
	GLdouble left,
	GLdouble right,
	GLdouble bottom,
	GLdouble top,
	GLdouble zNear,
	GLdouble zFar)
{
	RenderCommand_matrixOrthoEXT* __restrict packet = (RenderCommand_matrixOrthoEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixOrthoEXT::execute);
	packet->mode = mode;
	packet->left = left;
	packet->right = right;
	packet->bottom = bottom;
	packet->top = top;
	packet->zNear = zNear;
	packet->zFar = zFar;
}
void matrixPopEXT(
	GLenum mode)
{
	RenderCommand_matrixPopEXT* __restrict packet = (RenderCommand_matrixPopEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixPopEXT::execute);
	packet->mode = mode;
}
void matrixPushEXT(
	GLenum mode)
{
	RenderCommand_matrixPushEXT* __restrict packet = (RenderCommand_matrixPushEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixPushEXT::execute);
	packet->mode = mode;
}
void clientAttribDefaultEXT(
	GLbitfield mask)
{
	RenderCommand_clientAttribDefaultEXT* __restrict packet = (RenderCommand_clientAttribDefaultEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clientAttribDefaultEXT::execute);
	packet->mask = mask;
}
void pushClientAttribDefaultEXT(
	GLbitfield mask)
{
	RenderCommand_pushClientAttribDefaultEXT* __restrict packet = (RenderCommand_pushClientAttribDefaultEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pushClientAttribDefaultEXT::execute);
	packet->mask = mask;
}
void textureParameterfEXT(
	GLuint texture,
	GLenum target,
	GLenum pname,
	GLfloat param)
{
	RenderCommand_textureParameterfEXT* __restrict packet = (RenderCommand_textureParameterfEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureParameterfEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->pname = pname;
	packet->param = param;
}
void textureParameterfvEXT(
	GLuint texture,
	GLenum target,
	GLenum pname,
	const GLfloat *params)
{
	RenderCommand_textureParameterfvEXT* __restrict packet = (RenderCommand_textureParameterfvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureParameterfvEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void textureParameteriEXT(
	GLuint texture,
	GLenum target,
	GLenum pname,
	GLint param)
{
	RenderCommand_textureParameteriEXT* __restrict packet = (RenderCommand_textureParameteriEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureParameteriEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->pname = pname;
	packet->param = param;
}
void textureParameterivEXT(
	GLuint texture,
	GLenum target,
	GLenum pname,
	const GLint *params)
{
	RenderCommand_textureParameterivEXT* __restrict packet = (RenderCommand_textureParameterivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureParameterivEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void textureImage1DEXT(
	GLuint texture,
	GLenum target,
	GLint level,
	GLint internalformat,
	GLsizei width,
	GLint border,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_textureImage1DEXT* __restrict packet = (RenderCommand_textureImage1DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureImage1DEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->border = border;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void textureImage2DEXT(
	GLuint texture,
	GLenum target,
	GLint level,
	GLint internalformat,
	GLsizei width,
	GLsizei height,
	GLint border,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_textureImage2DEXT* __restrict packet = (RenderCommand_textureImage2DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureImage2DEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->border = border;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void textureSubImage1DEXT(
	GLuint texture,
	GLenum target,
	GLint level,
	GLint xoffset,
	GLsizei width,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_textureSubImage1DEXT* __restrict packet = (RenderCommand_textureSubImage1DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureSubImage1DEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->width = width;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void textureSubImage2DEXT(
	GLuint texture,
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLsizei width,
	GLsizei height,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_textureSubImage2DEXT* __restrict packet = (RenderCommand_textureSubImage2DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureSubImage2DEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->width = width;
	packet->height = height;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void copyTextureImage1DEXT(
	GLuint texture,
	GLenum target,
	GLint level,
	GLenum internalformat,
	GLint x,
	GLint y,
	GLsizei width,
	GLint border)
{
	RenderCommand_copyTextureImage1DEXT* __restrict packet = (RenderCommand_copyTextureImage1DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyTextureImage1DEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->border = border;
}
void copyTextureImage2DEXT(
	GLuint texture,
	GLenum target,
	GLint level,
	GLenum internalformat,
	GLint x,
	GLint y,
	GLsizei width,
	GLsizei height,
	GLint border)
{
	RenderCommand_copyTextureImage2DEXT* __restrict packet = (RenderCommand_copyTextureImage2DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyTextureImage2DEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->height = height;
	packet->border = border;
}
void copyTextureSubImage1DEXT(
	GLuint texture,
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint x,
	GLint y,
	GLsizei width)
{
	RenderCommand_copyTextureSubImage1DEXT* __restrict packet = (RenderCommand_copyTextureSubImage1DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyTextureSubImage1DEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->x = x;
	packet->y = y;
	packet->width = width;
}
void copyTextureSubImage2DEXT(
	GLuint texture,
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint x,
	GLint y,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_copyTextureSubImage2DEXT* __restrict packet = (RenderCommand_copyTextureSubImage2DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyTextureSubImage2DEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->height = height;
}
void getTextureImageEXT(
	GLuint texture,
	GLenum target,
	GLint level,
	GLenum format,
	GLenum type,
	void *pixels)
{
	RenderCommand_getTextureImageEXT* __restrict packet = (RenderCommand_getTextureImageEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTextureImageEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->level = level;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void getTextureParameterfvEXT(
	GLuint texture,
	GLenum target,
	GLenum pname,
	GLfloat *params)
{
	RenderCommand_getTextureParameterfvEXT* __restrict packet = (RenderCommand_getTextureParameterfvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTextureParameterfvEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void getTextureParameterivEXT(
	GLuint texture,
	GLenum target,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getTextureParameterivEXT* __restrict packet = (RenderCommand_getTextureParameterivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTextureParameterivEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void getTextureLevelParameterfvEXT(
	GLuint texture,
	GLenum target,
	GLint level,
	GLenum pname,
	GLfloat *params)
{
	RenderCommand_getTextureLevelParameterfvEXT* __restrict packet = (RenderCommand_getTextureLevelParameterfvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTextureLevelParameterfvEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->level = level;
	packet->pname = pname;
	packet->params = params;
}
void getTextureLevelParameterivEXT(
	GLuint texture,
	GLenum target,
	GLint level,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getTextureLevelParameterivEXT* __restrict packet = (RenderCommand_getTextureLevelParameterivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTextureLevelParameterivEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->level = level;
	packet->pname = pname;
	packet->params = params;
}
void textureImage3DEXT(
	GLuint texture,
	GLenum target,
	GLint level,
	GLint internalformat,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLint border,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_textureImage3DEXT* __restrict packet = (RenderCommand_textureImage3DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureImage3DEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->border = border;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void textureSubImage3DEXT(
	GLuint texture,
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint zoffset,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_textureSubImage3DEXT* __restrict packet = (RenderCommand_textureSubImage3DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureSubImage3DEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->zoffset = zoffset;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void copyTextureSubImage3DEXT(
	GLuint texture,
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint zoffset,
	GLint x,
	GLint y,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_copyTextureSubImage3DEXT* __restrict packet = (RenderCommand_copyTextureSubImage3DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyTextureSubImage3DEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->zoffset = zoffset;
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->height = height;
}
void bindMultiTextureEXT(
	GLenum texunit,
	GLenum target,
	GLuint texture)
{
	RenderCommand_bindMultiTextureEXT* __restrict packet = (RenderCommand_bindMultiTextureEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindMultiTextureEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->texture = texture;
}
void multiTexCoordPointerEXT(
	GLenum texunit,
	GLint size,
	GLenum type,
	GLsizei stride,
	const void *pointer)
{
	RenderCommand_multiTexCoordPointerEXT* __restrict packet = (RenderCommand_multiTexCoordPointerEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexCoordPointerEXT::execute);
	packet->texunit = texunit;
	packet->size = size;
	packet->type = type;
	packet->stride = stride;
	packet->pointer = pointer;
}
void multiTexEnvfEXT(
	GLenum texunit,
	GLenum target,
	GLenum pname,
	GLfloat param)
{
	RenderCommand_multiTexEnvfEXT* __restrict packet = (RenderCommand_multiTexEnvfEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexEnvfEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->pname = pname;
	packet->param = param;
}
void multiTexEnvfvEXT(
	GLenum texunit,
	GLenum target,
	GLenum pname,
	const GLfloat *params)
{
	RenderCommand_multiTexEnvfvEXT* __restrict packet = (RenderCommand_multiTexEnvfvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexEnvfvEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void multiTexEnviEXT(
	GLenum texunit,
	GLenum target,
	GLenum pname,
	GLint param)
{
	RenderCommand_multiTexEnviEXT* __restrict packet = (RenderCommand_multiTexEnviEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexEnviEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->pname = pname;
	packet->param = param;
}
void multiTexEnvivEXT(
	GLenum texunit,
	GLenum target,
	GLenum pname,
	const GLint *params)
{
	RenderCommand_multiTexEnvivEXT* __restrict packet = (RenderCommand_multiTexEnvivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexEnvivEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void multiTexGendEXT(
	GLenum texunit,
	GLenum coord,
	GLenum pname,
	GLdouble param)
{
	RenderCommand_multiTexGendEXT* __restrict packet = (RenderCommand_multiTexGendEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexGendEXT::execute);
	packet->texunit = texunit;
	packet->coord = coord;
	packet->pname = pname;
	packet->param = param;
}
void multiTexGendvEXT(
	GLenum texunit,
	GLenum coord,
	GLenum pname,
	const GLdouble *params)
{
	RenderCommand_multiTexGendvEXT* __restrict packet = (RenderCommand_multiTexGendvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexGendvEXT::execute);
	packet->texunit = texunit;
	packet->coord = coord;
	packet->pname = pname;
	packet->params = params;
}
void multiTexGenfEXT(
	GLenum texunit,
	GLenum coord,
	GLenum pname,
	GLfloat param)
{
	RenderCommand_multiTexGenfEXT* __restrict packet = (RenderCommand_multiTexGenfEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexGenfEXT::execute);
	packet->texunit = texunit;
	packet->coord = coord;
	packet->pname = pname;
	packet->param = param;
}
void multiTexGenfvEXT(
	GLenum texunit,
	GLenum coord,
	GLenum pname,
	const GLfloat *params)
{
	RenderCommand_multiTexGenfvEXT* __restrict packet = (RenderCommand_multiTexGenfvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexGenfvEXT::execute);
	packet->texunit = texunit;
	packet->coord = coord;
	packet->pname = pname;
	packet->params = params;
}
void multiTexGeniEXT(
	GLenum texunit,
	GLenum coord,
	GLenum pname,
	GLint param)
{
	RenderCommand_multiTexGeniEXT* __restrict packet = (RenderCommand_multiTexGeniEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexGeniEXT::execute);
	packet->texunit = texunit;
	packet->coord = coord;
	packet->pname = pname;
	packet->param = param;
}
void multiTexGenivEXT(
	GLenum texunit,
	GLenum coord,
	GLenum pname,
	const GLint *params)
{
	RenderCommand_multiTexGenivEXT* __restrict packet = (RenderCommand_multiTexGenivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexGenivEXT::execute);
	packet->texunit = texunit;
	packet->coord = coord;
	packet->pname = pname;
	packet->params = params;
}
void getMultiTexEnvfvEXT(
	GLenum texunit,
	GLenum target,
	GLenum pname,
	GLfloat *params)
{
	RenderCommand_getMultiTexEnvfvEXT* __restrict packet = (RenderCommand_getMultiTexEnvfvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getMultiTexEnvfvEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void getMultiTexEnvivEXT(
	GLenum texunit,
	GLenum target,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getMultiTexEnvivEXT* __restrict packet = (RenderCommand_getMultiTexEnvivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getMultiTexEnvivEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void getMultiTexGendvEXT(
	GLenum texunit,
	GLenum coord,
	GLenum pname,
	GLdouble *params)
{
	RenderCommand_getMultiTexGendvEXT* __restrict packet = (RenderCommand_getMultiTexGendvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getMultiTexGendvEXT::execute);
	packet->texunit = texunit;
	packet->coord = coord;
	packet->pname = pname;
	packet->params = params;
}
void getMultiTexGenfvEXT(
	GLenum texunit,
	GLenum coord,
	GLenum pname,
	GLfloat *params)
{
	RenderCommand_getMultiTexGenfvEXT* __restrict packet = (RenderCommand_getMultiTexGenfvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getMultiTexGenfvEXT::execute);
	packet->texunit = texunit;
	packet->coord = coord;
	packet->pname = pname;
	packet->params = params;
}
void getMultiTexGenivEXT(
	GLenum texunit,
	GLenum coord,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getMultiTexGenivEXT* __restrict packet = (RenderCommand_getMultiTexGenivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getMultiTexGenivEXT::execute);
	packet->texunit = texunit;
	packet->coord = coord;
	packet->pname = pname;
	packet->params = params;
}
void multiTexParameteriEXT(
	GLenum texunit,
	GLenum target,
	GLenum pname,
	GLint param)
{
	RenderCommand_multiTexParameteriEXT* __restrict packet = (RenderCommand_multiTexParameteriEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexParameteriEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->pname = pname;
	packet->param = param;
}
void multiTexParameterivEXT(
	GLenum texunit,
	GLenum target,
	GLenum pname,
	const GLint *params)
{
	RenderCommand_multiTexParameterivEXT* __restrict packet = (RenderCommand_multiTexParameterivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexParameterivEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void multiTexParameterfEXT(
	GLenum texunit,
	GLenum target,
	GLenum pname,
	GLfloat param)
{
	RenderCommand_multiTexParameterfEXT* __restrict packet = (RenderCommand_multiTexParameterfEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexParameterfEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->pname = pname;
	packet->param = param;
}
void multiTexParameterfvEXT(
	GLenum texunit,
	GLenum target,
	GLenum pname,
	const GLfloat *params)
{
	RenderCommand_multiTexParameterfvEXT* __restrict packet = (RenderCommand_multiTexParameterfvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexParameterfvEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void multiTexImage1DEXT(
	GLenum texunit,
	GLenum target,
	GLint level,
	GLint internalformat,
	GLsizei width,
	GLint border,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_multiTexImage1DEXT* __restrict packet = (RenderCommand_multiTexImage1DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexImage1DEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->border = border;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void multiTexImage2DEXT(
	GLenum texunit,
	GLenum target,
	GLint level,
	GLint internalformat,
	GLsizei width,
	GLsizei height,
	GLint border,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_multiTexImage2DEXT* __restrict packet = (RenderCommand_multiTexImage2DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexImage2DEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->border = border;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void multiTexSubImage1DEXT(
	GLenum texunit,
	GLenum target,
	GLint level,
	GLint xoffset,
	GLsizei width,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_multiTexSubImage1DEXT* __restrict packet = (RenderCommand_multiTexSubImage1DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexSubImage1DEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->width = width;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void multiTexSubImage2DEXT(
	GLenum texunit,
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLsizei width,
	GLsizei height,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_multiTexSubImage2DEXT* __restrict packet = (RenderCommand_multiTexSubImage2DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexSubImage2DEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->width = width;
	packet->height = height;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void copyMultiTexImage1DEXT(
	GLenum texunit,
	GLenum target,
	GLint level,
	GLenum internalformat,
	GLint x,
	GLint y,
	GLsizei width,
	GLint border)
{
	RenderCommand_copyMultiTexImage1DEXT* __restrict packet = (RenderCommand_copyMultiTexImage1DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyMultiTexImage1DEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->border = border;
}
void copyMultiTexImage2DEXT(
	GLenum texunit,
	GLenum target,
	GLint level,
	GLenum internalformat,
	GLint x,
	GLint y,
	GLsizei width,
	GLsizei height,
	GLint border)
{
	RenderCommand_copyMultiTexImage2DEXT* __restrict packet = (RenderCommand_copyMultiTexImage2DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyMultiTexImage2DEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->height = height;
	packet->border = border;
}
void copyMultiTexSubImage1DEXT(
	GLenum texunit,
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint x,
	GLint y,
	GLsizei width)
{
	RenderCommand_copyMultiTexSubImage1DEXT* __restrict packet = (RenderCommand_copyMultiTexSubImage1DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyMultiTexSubImage1DEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->x = x;
	packet->y = y;
	packet->width = width;
}
void copyMultiTexSubImage2DEXT(
	GLenum texunit,
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint x,
	GLint y,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_copyMultiTexSubImage2DEXT* __restrict packet = (RenderCommand_copyMultiTexSubImage2DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyMultiTexSubImage2DEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->height = height;
}
void getMultiTexImageEXT(
	GLenum texunit,
	GLenum target,
	GLint level,
	GLenum format,
	GLenum type,
	void *pixels)
{
	RenderCommand_getMultiTexImageEXT* __restrict packet = (RenderCommand_getMultiTexImageEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getMultiTexImageEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->level = level;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void getMultiTexParameterfvEXT(
	GLenum texunit,
	GLenum target,
	GLenum pname,
	GLfloat *params)
{
	RenderCommand_getMultiTexParameterfvEXT* __restrict packet = (RenderCommand_getMultiTexParameterfvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getMultiTexParameterfvEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void getMultiTexParameterivEXT(
	GLenum texunit,
	GLenum target,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getMultiTexParameterivEXT* __restrict packet = (RenderCommand_getMultiTexParameterivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getMultiTexParameterivEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void getMultiTexLevelParameterfvEXT(
	GLenum texunit,
	GLenum target,
	GLint level,
	GLenum pname,
	GLfloat *params)
{
	RenderCommand_getMultiTexLevelParameterfvEXT* __restrict packet = (RenderCommand_getMultiTexLevelParameterfvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getMultiTexLevelParameterfvEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->level = level;
	packet->pname = pname;
	packet->params = params;
}
void getMultiTexLevelParameterivEXT(
	GLenum texunit,
	GLenum target,
	GLint level,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getMultiTexLevelParameterivEXT* __restrict packet = (RenderCommand_getMultiTexLevelParameterivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getMultiTexLevelParameterivEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->level = level;
	packet->pname = pname;
	packet->params = params;
}
void multiTexImage3DEXT(
	GLenum texunit,
	GLenum target,
	GLint level,
	GLint internalformat,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLint border,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_multiTexImage3DEXT* __restrict packet = (RenderCommand_multiTexImage3DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexImage3DEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->border = border;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void multiTexSubImage3DEXT(
	GLenum texunit,
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint zoffset,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLenum format,
	GLenum type,
	const void *pixels)
{
	RenderCommand_multiTexSubImage3DEXT* __restrict packet = (RenderCommand_multiTexSubImage3DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexSubImage3DEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->zoffset = zoffset;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->format = format;
	packet->type = type;
	packet->pixels = pixels;
}
void copyMultiTexSubImage3DEXT(
	GLenum texunit,
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint zoffset,
	GLint x,
	GLint y,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_copyMultiTexSubImage3DEXT* __restrict packet = (RenderCommand_copyMultiTexSubImage3DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyMultiTexSubImage3DEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->zoffset = zoffset;
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->height = height;
}
void enableClientStateIndexedEXT(
	GLenum array,
	GLuint index)
{
	RenderCommand_enableClientStateIndexedEXT* __restrict packet = (RenderCommand_enableClientStateIndexedEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_enableClientStateIndexedEXT::execute);
	packet->array = array;
	packet->index = index;
}
void disableClientStateIndexedEXT(
	GLenum array,
	GLuint index)
{
	RenderCommand_disableClientStateIndexedEXT* __restrict packet = (RenderCommand_disableClientStateIndexedEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_disableClientStateIndexedEXT::execute);
	packet->array = array;
	packet->index = index;
}
void getFloatIndexedvEXT(
	GLenum target,
	GLuint index,
	GLfloat *data)
{
	RenderCommand_getFloatIndexedvEXT* __restrict packet = (RenderCommand_getFloatIndexedvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getFloatIndexedvEXT::execute);
	packet->target = target;
	packet->index = index;
	packet->data = data;
}
void getDoubleIndexedvEXT(
	GLenum target,
	GLuint index,
	GLdouble *data)
{
	RenderCommand_getDoubleIndexedvEXT* __restrict packet = (RenderCommand_getDoubleIndexedvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getDoubleIndexedvEXT::execute);
	packet->target = target;
	packet->index = index;
	packet->data = data;
}
void getPointerIndexedvEXT(
	GLenum target,
	GLuint index,
	void **data)
{
	RenderCommand_getPointerIndexedvEXT* __restrict packet = (RenderCommand_getPointerIndexedvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPointerIndexedvEXT::execute);
	packet->target = target;
	packet->index = index;
	packet->data = data;
}
void enableIndexedEXT(
	GLenum target,
	GLuint index)
{
	RenderCommand_enableIndexedEXT* __restrict packet = (RenderCommand_enableIndexedEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_enableIndexedEXT::execute);
	packet->target = target;
	packet->index = index;
}
void disableIndexedEXT(
	GLenum target,
	GLuint index)
{
	RenderCommand_disableIndexedEXT* __restrict packet = (RenderCommand_disableIndexedEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_disableIndexedEXT::execute);
	packet->target = target;
	packet->index = index;
}
GLboolean isEnabledIndexedEXT(
	GLenum target,
	GLuint index)
{
	RenderCommand_isEnabledIndexedEXT* __restrict packet = (RenderCommand_isEnabledIndexedEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isEnabledIndexedEXT::execute);
	packet->target = target;
	packet->index = index;
}
void getIntegerIndexedvEXT(
	GLenum target,
	GLuint index,
	GLint *data)
{
	RenderCommand_getIntegerIndexedvEXT* __restrict packet = (RenderCommand_getIntegerIndexedvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getIntegerIndexedvEXT::execute);
	packet->target = target;
	packet->index = index;
	packet->data = data;
}
void getBooleanIndexedvEXT(
	GLenum target,
	GLuint index,
	GLboolean *data)
{
	RenderCommand_getBooleanIndexedvEXT* __restrict packet = (RenderCommand_getBooleanIndexedvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getBooleanIndexedvEXT::execute);
	packet->target = target;
	packet->index = index;
	packet->data = data;
}
void compressedTextureImage3DEXT(
	GLuint texture,
	GLenum target,
	GLint level,
	GLenum internalformat,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLint border,
	GLsizei imageSize,
	const void *bits)
{
	RenderCommand_compressedTextureImage3DEXT* __restrict packet = (RenderCommand_compressedTextureImage3DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedTextureImage3DEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->border = border;
	packet->imageSize = imageSize;
	packet->bits = bits;
}
void compressedTextureImage2DEXT(
	GLuint texture,
	GLenum target,
	GLint level,
	GLenum internalformat,
	GLsizei width,
	GLsizei height,
	GLint border,
	GLsizei imageSize,
	const void *bits)
{
	RenderCommand_compressedTextureImage2DEXT* __restrict packet = (RenderCommand_compressedTextureImage2DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedTextureImage2DEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->border = border;
	packet->imageSize = imageSize;
	packet->bits = bits;
}
void compressedTextureImage1DEXT(
	GLuint texture,
	GLenum target,
	GLint level,
	GLenum internalformat,
	GLsizei width,
	GLint border,
	GLsizei imageSize,
	const void *bits)
{
	RenderCommand_compressedTextureImage1DEXT* __restrict packet = (RenderCommand_compressedTextureImage1DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedTextureImage1DEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->border = border;
	packet->imageSize = imageSize;
	packet->bits = bits;
}
void compressedTextureSubImage3DEXT(
	GLuint texture,
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint zoffset,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLenum format,
	GLsizei imageSize,
	const void *bits)
{
	RenderCommand_compressedTextureSubImage3DEXT* __restrict packet = (RenderCommand_compressedTextureSubImage3DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedTextureSubImage3DEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->zoffset = zoffset;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->format = format;
	packet->imageSize = imageSize;
	packet->bits = bits;
}
void compressedTextureSubImage2DEXT(
	GLuint texture,
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLsizei width,
	GLsizei height,
	GLenum format,
	GLsizei imageSize,
	const void *bits)
{
	RenderCommand_compressedTextureSubImage2DEXT* __restrict packet = (RenderCommand_compressedTextureSubImage2DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedTextureSubImage2DEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->width = width;
	packet->height = height;
	packet->format = format;
	packet->imageSize = imageSize;
	packet->bits = bits;
}
void compressedTextureSubImage1DEXT(
	GLuint texture,
	GLenum target,
	GLint level,
	GLint xoffset,
	GLsizei width,
	GLenum format,
	GLsizei imageSize,
	const void *bits)
{
	RenderCommand_compressedTextureSubImage1DEXT* __restrict packet = (RenderCommand_compressedTextureSubImage1DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedTextureSubImage1DEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->width = width;
	packet->format = format;
	packet->imageSize = imageSize;
	packet->bits = bits;
}
void getCompressedTextureImageEXT(
	GLuint texture,
	GLenum target,
	GLint lod,
	void *img)
{
	RenderCommand_getCompressedTextureImageEXT* __restrict packet = (RenderCommand_getCompressedTextureImageEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getCompressedTextureImageEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->lod = lod;
	packet->img = img;
}
void compressedMultiTexImage3DEXT(
	GLenum texunit,
	GLenum target,
	GLint level,
	GLenum internalformat,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLint border,
	GLsizei imageSize,
	const void *bits)
{
	RenderCommand_compressedMultiTexImage3DEXT* __restrict packet = (RenderCommand_compressedMultiTexImage3DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedMultiTexImage3DEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->border = border;
	packet->imageSize = imageSize;
	packet->bits = bits;
}
void compressedMultiTexImage2DEXT(
	GLenum texunit,
	GLenum target,
	GLint level,
	GLenum internalformat,
	GLsizei width,
	GLsizei height,
	GLint border,
	GLsizei imageSize,
	const void *bits)
{
	RenderCommand_compressedMultiTexImage2DEXT* __restrict packet = (RenderCommand_compressedMultiTexImage2DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedMultiTexImage2DEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->border = border;
	packet->imageSize = imageSize;
	packet->bits = bits;
}
void compressedMultiTexImage1DEXT(
	GLenum texunit,
	GLenum target,
	GLint level,
	GLenum internalformat,
	GLsizei width,
	GLint border,
	GLsizei imageSize,
	const void *bits)
{
	RenderCommand_compressedMultiTexImage1DEXT* __restrict packet = (RenderCommand_compressedMultiTexImage1DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedMultiTexImage1DEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->level = level;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->border = border;
	packet->imageSize = imageSize;
	packet->bits = bits;
}
void compressedMultiTexSubImage3DEXT(
	GLenum texunit,
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint zoffset,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLenum format,
	GLsizei imageSize,
	const void *bits)
{
	RenderCommand_compressedMultiTexSubImage3DEXT* __restrict packet = (RenderCommand_compressedMultiTexSubImage3DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedMultiTexSubImage3DEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->zoffset = zoffset;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->format = format;
	packet->imageSize = imageSize;
	packet->bits = bits;
}
void compressedMultiTexSubImage2DEXT(
	GLenum texunit,
	GLenum target,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLsizei width,
	GLsizei height,
	GLenum format,
	GLsizei imageSize,
	const void *bits)
{
	RenderCommand_compressedMultiTexSubImage2DEXT* __restrict packet = (RenderCommand_compressedMultiTexSubImage2DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedMultiTexSubImage2DEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->width = width;
	packet->height = height;
	packet->format = format;
	packet->imageSize = imageSize;
	packet->bits = bits;
}
void compressedMultiTexSubImage1DEXT(
	GLenum texunit,
	GLenum target,
	GLint level,
	GLint xoffset,
	GLsizei width,
	GLenum format,
	GLsizei imageSize,
	const void *bits)
{
	RenderCommand_compressedMultiTexSubImage1DEXT* __restrict packet = (RenderCommand_compressedMultiTexSubImage1DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compressedMultiTexSubImage1DEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->width = width;
	packet->format = format;
	packet->imageSize = imageSize;
	packet->bits = bits;
}
void getCompressedMultiTexImageEXT(
	GLenum texunit,
	GLenum target,
	GLint lod,
	void *img)
{
	RenderCommand_getCompressedMultiTexImageEXT* __restrict packet = (RenderCommand_getCompressedMultiTexImageEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getCompressedMultiTexImageEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->lod = lod;
	packet->img = img;
}
void matrixLoadTransposefEXT(
	GLenum mode,
	const GLfloat *m)
{
	RenderCommand_matrixLoadTransposefEXT* __restrict packet = (RenderCommand_matrixLoadTransposefEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixLoadTransposefEXT::execute);
	packet->mode = mode;
	packet->m = m;
}
void matrixLoadTransposedEXT(
	GLenum mode,
	const GLdouble *m)
{
	RenderCommand_matrixLoadTransposedEXT* __restrict packet = (RenderCommand_matrixLoadTransposedEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixLoadTransposedEXT::execute);
	packet->mode = mode;
	packet->m = m;
}
void matrixMultTransposefEXT(
	GLenum mode,
	const GLfloat *m)
{
	RenderCommand_matrixMultTransposefEXT* __restrict packet = (RenderCommand_matrixMultTransposefEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixMultTransposefEXT::execute);
	packet->mode = mode;
	packet->m = m;
}
void matrixMultTransposedEXT(
	GLenum mode,
	const GLdouble *m)
{
	RenderCommand_matrixMultTransposedEXT* __restrict packet = (RenderCommand_matrixMultTransposedEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixMultTransposedEXT::execute);
	packet->mode = mode;
	packet->m = m;
}
void namedBufferDataEXT(
	GLuint buffer,
	GLsizeiptr size,
	const void *data,
	GLenum usage)
{
	RenderCommand_namedBufferDataEXT* __restrict packet = (RenderCommand_namedBufferDataEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedBufferDataEXT::execute);
	packet->buffer = buffer;
	packet->size = size;
	packet->data = data;
	packet->usage = usage;
}
void namedBufferSubDataEXT(
	GLuint buffer,
	GLintptr offset,
	GLsizeiptr size,
	const void *data)
{
	RenderCommand_namedBufferSubDataEXT* __restrict packet = (RenderCommand_namedBufferSubDataEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedBufferSubDataEXT::execute);
	packet->buffer = buffer;
	packet->offset = offset;
	packet->size = size;
	packet->data = data;
}
void* mapNamedBufferEXT(
	GLuint buffer,
	GLenum access)
{
	RenderCommand_mapNamedBufferEXT* __restrict packet = (RenderCommand_mapNamedBufferEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_mapNamedBufferEXT::execute);
	packet->buffer = buffer;
	packet->access = access;
}
GLboolean unmapNamedBufferEXT(
	GLuint buffer)
{
	RenderCommand_unmapNamedBufferEXT* __restrict packet = (RenderCommand_unmapNamedBufferEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_unmapNamedBufferEXT::execute);
	packet->buffer = buffer;
}
void getNamedBufferParameterivEXT(
	GLuint buffer,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getNamedBufferParameterivEXT* __restrict packet = (RenderCommand_getNamedBufferParameterivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedBufferParameterivEXT::execute);
	packet->buffer = buffer;
	packet->pname = pname;
	packet->params = params;
}
void getNamedBufferPointervEXT(
	GLuint buffer,
	GLenum pname,
	void **params)
{
	RenderCommand_getNamedBufferPointervEXT* __restrict packet = (RenderCommand_getNamedBufferPointervEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedBufferPointervEXT::execute);
	packet->buffer = buffer;
	packet->pname = pname;
	packet->params = params;
}
void getNamedBufferSubDataEXT(
	GLuint buffer,
	GLintptr offset,
	GLsizeiptr size,
	void *data)
{
	RenderCommand_getNamedBufferSubDataEXT* __restrict packet = (RenderCommand_getNamedBufferSubDataEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedBufferSubDataEXT::execute);
	packet->buffer = buffer;
	packet->offset = offset;
	packet->size = size;
	packet->data = data;
}
void programUniform1fEXT(
	GLuint program,
	GLint location,
	GLfloat v0)
{
	RenderCommand_programUniform1fEXT* __restrict packet = (RenderCommand_programUniform1fEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1fEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
}
void programUniform2fEXT(
	GLuint program,
	GLint location,
	GLfloat v0,
	GLfloat v1)
{
	RenderCommand_programUniform2fEXT* __restrict packet = (RenderCommand_programUniform2fEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2fEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
}
void programUniform3fEXT(
	GLuint program,
	GLint location,
	GLfloat v0,
	GLfloat v1,
	GLfloat v2)
{
	RenderCommand_programUniform3fEXT* __restrict packet = (RenderCommand_programUniform3fEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3fEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
	packet->v2 = v2;
}
void programUniform4fEXT(
	GLuint program,
	GLint location,
	GLfloat v0,
	GLfloat v1,
	GLfloat v2,
	GLfloat v3)
{
	RenderCommand_programUniform4fEXT* __restrict packet = (RenderCommand_programUniform4fEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4fEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
	packet->v2 = v2;
	packet->v3 = v3;
}
void programUniform1iEXT(
	GLuint program,
	GLint location,
	GLint v0)
{
	RenderCommand_programUniform1iEXT* __restrict packet = (RenderCommand_programUniform1iEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1iEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
}
void programUniform2iEXT(
	GLuint program,
	GLint location,
	GLint v0,
	GLint v1)
{
	RenderCommand_programUniform2iEXT* __restrict packet = (RenderCommand_programUniform2iEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2iEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
}
void programUniform3iEXT(
	GLuint program,
	GLint location,
	GLint v0,
	GLint v1,
	GLint v2)
{
	RenderCommand_programUniform3iEXT* __restrict packet = (RenderCommand_programUniform3iEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3iEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
	packet->v2 = v2;
}
void programUniform4iEXT(
	GLuint program,
	GLint location,
	GLint v0,
	GLint v1,
	GLint v2,
	GLint v3)
{
	RenderCommand_programUniform4iEXT* __restrict packet = (RenderCommand_programUniform4iEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4iEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
	packet->v2 = v2;
	packet->v3 = v3;
}
void programUniform1fvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLfloat *value)
{
	RenderCommand_programUniform1fvEXT* __restrict packet = (RenderCommand_programUniform1fvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1fvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform2fvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLfloat *value)
{
	RenderCommand_programUniform2fvEXT* __restrict packet = (RenderCommand_programUniform2fvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2fvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform3fvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLfloat *value)
{
	RenderCommand_programUniform3fvEXT* __restrict packet = (RenderCommand_programUniform3fvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3fvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform4fvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLfloat *value)
{
	RenderCommand_programUniform4fvEXT* __restrict packet = (RenderCommand_programUniform4fvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4fvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform1ivEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLint *value)
{
	RenderCommand_programUniform1ivEXT* __restrict packet = (RenderCommand_programUniform1ivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1ivEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform2ivEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLint *value)
{
	RenderCommand_programUniform2ivEXT* __restrict packet = (RenderCommand_programUniform2ivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2ivEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform3ivEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLint *value)
{
	RenderCommand_programUniform3ivEXT* __restrict packet = (RenderCommand_programUniform3ivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3ivEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform4ivEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLint *value)
{
	RenderCommand_programUniform4ivEXT* __restrict packet = (RenderCommand_programUniform4ivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4ivEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniformMatrix2fvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_programUniformMatrix2fvEXT* __restrict packet = (RenderCommand_programUniformMatrix2fvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix2fvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix3fvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_programUniformMatrix3fvEXT* __restrict packet = (RenderCommand_programUniformMatrix3fvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix3fvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix4fvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_programUniformMatrix4fvEXT* __restrict packet = (RenderCommand_programUniformMatrix4fvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix4fvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix2x3fvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_programUniformMatrix2x3fvEXT* __restrict packet = (RenderCommand_programUniformMatrix2x3fvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix2x3fvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix3x2fvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_programUniformMatrix3x2fvEXT* __restrict packet = (RenderCommand_programUniformMatrix3x2fvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix3x2fvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix2x4fvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_programUniformMatrix2x4fvEXT* __restrict packet = (RenderCommand_programUniformMatrix2x4fvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix2x4fvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix4x2fvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_programUniformMatrix4x2fvEXT* __restrict packet = (RenderCommand_programUniformMatrix4x2fvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix4x2fvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix3x4fvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_programUniformMatrix3x4fvEXT* __restrict packet = (RenderCommand_programUniformMatrix3x4fvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix3x4fvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix4x3fvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLfloat *value)
{
	RenderCommand_programUniformMatrix4x3fvEXT* __restrict packet = (RenderCommand_programUniformMatrix4x3fvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix4x3fvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void textureBufferEXT(
	GLuint texture,
	GLenum target,
	GLenum internalformat,
	GLuint buffer)
{
	RenderCommand_textureBufferEXT* __restrict packet = (RenderCommand_textureBufferEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureBufferEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->internalformat = internalformat;
	packet->buffer = buffer;
}
void multiTexBufferEXT(
	GLenum texunit,
	GLenum target,
	GLenum internalformat,
	GLuint buffer)
{
	RenderCommand_multiTexBufferEXT* __restrict packet = (RenderCommand_multiTexBufferEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexBufferEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->internalformat = internalformat;
	packet->buffer = buffer;
}
void textureParameterIivEXT(
	GLuint texture,
	GLenum target,
	GLenum pname,
	const GLint *params)
{
	RenderCommand_textureParameterIivEXT* __restrict packet = (RenderCommand_textureParameterIivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureParameterIivEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void textureParameterIuivEXT(
	GLuint texture,
	GLenum target,
	GLenum pname,
	const GLuint *params)
{
	RenderCommand_textureParameterIuivEXT* __restrict packet = (RenderCommand_textureParameterIuivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureParameterIuivEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void getTextureParameterIivEXT(
	GLuint texture,
	GLenum target,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getTextureParameterIivEXT* __restrict packet = (RenderCommand_getTextureParameterIivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTextureParameterIivEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void getTextureParameterIuivEXT(
	GLuint texture,
	GLenum target,
	GLenum pname,
	GLuint *params)
{
	RenderCommand_getTextureParameterIuivEXT* __restrict packet = (RenderCommand_getTextureParameterIuivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTextureParameterIuivEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void multiTexParameterIivEXT(
	GLenum texunit,
	GLenum target,
	GLenum pname,
	const GLint *params)
{
	RenderCommand_multiTexParameterIivEXT* __restrict packet = (RenderCommand_multiTexParameterIivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexParameterIivEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void multiTexParameterIuivEXT(
	GLenum texunit,
	GLenum target,
	GLenum pname,
	const GLuint *params)
{
	RenderCommand_multiTexParameterIuivEXT* __restrict packet = (RenderCommand_multiTexParameterIuivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexParameterIuivEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void getMultiTexParameterIivEXT(
	GLenum texunit,
	GLenum target,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getMultiTexParameterIivEXT* __restrict packet = (RenderCommand_getMultiTexParameterIivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getMultiTexParameterIivEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void getMultiTexParameterIuivEXT(
	GLenum texunit,
	GLenum target,
	GLenum pname,
	GLuint *params)
{
	RenderCommand_getMultiTexParameterIuivEXT* __restrict packet = (RenderCommand_getMultiTexParameterIuivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getMultiTexParameterIuivEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void programUniform1uiEXT(
	GLuint program,
	GLint location,
	GLuint v0)
{
	RenderCommand_programUniform1uiEXT* __restrict packet = (RenderCommand_programUniform1uiEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1uiEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
}
void programUniform2uiEXT(
	GLuint program,
	GLint location,
	GLuint v0,
	GLuint v1)
{
	RenderCommand_programUniform2uiEXT* __restrict packet = (RenderCommand_programUniform2uiEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2uiEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
}
void programUniform3uiEXT(
	GLuint program,
	GLint location,
	GLuint v0,
	GLuint v1,
	GLuint v2)
{
	RenderCommand_programUniform3uiEXT* __restrict packet = (RenderCommand_programUniform3uiEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3uiEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
	packet->v2 = v2;
}
void programUniform4uiEXT(
	GLuint program,
	GLint location,
	GLuint v0,
	GLuint v1,
	GLuint v2,
	GLuint v3)
{
	RenderCommand_programUniform4uiEXT* __restrict packet = (RenderCommand_programUniform4uiEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4uiEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->v0 = v0;
	packet->v1 = v1;
	packet->v2 = v2;
	packet->v3 = v3;
}
void programUniform1uivEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLuint *value)
{
	RenderCommand_programUniform1uivEXT* __restrict packet = (RenderCommand_programUniform1uivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1uivEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform2uivEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLuint *value)
{
	RenderCommand_programUniform2uivEXT* __restrict packet = (RenderCommand_programUniform2uivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2uivEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform3uivEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLuint *value)
{
	RenderCommand_programUniform3uivEXT* __restrict packet = (RenderCommand_programUniform3uivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3uivEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform4uivEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLuint *value)
{
	RenderCommand_programUniform4uivEXT* __restrict packet = (RenderCommand_programUniform4uivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4uivEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void namedProgramLocalParameters4fvEXT(
	GLuint program,
	GLenum target,
	GLuint index,
	GLsizei count,
	const GLfloat *params)
{
	RenderCommand_namedProgramLocalParameters4fvEXT* __restrict packet = (RenderCommand_namedProgramLocalParameters4fvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedProgramLocalParameters4fvEXT::execute);
	packet->program = program;
	packet->target = target;
	packet->index = index;
	packet->count = count;
	packet->params = params;
}
void namedProgramLocalParameterI4iEXT(
	GLuint program,
	GLenum target,
	GLuint index,
	GLint x,
	GLint y,
	GLint z,
	GLint w)
{
	RenderCommand_namedProgramLocalParameterI4iEXT* __restrict packet = (RenderCommand_namedProgramLocalParameterI4iEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedProgramLocalParameterI4iEXT::execute);
	packet->program = program;
	packet->target = target;
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void namedProgramLocalParameterI4ivEXT(
	GLuint program,
	GLenum target,
	GLuint index,
	const GLint *params)
{
	RenderCommand_namedProgramLocalParameterI4ivEXT* __restrict packet = (RenderCommand_namedProgramLocalParameterI4ivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedProgramLocalParameterI4ivEXT::execute);
	packet->program = program;
	packet->target = target;
	packet->index = index;
	packet->params = params;
}
void namedProgramLocalParametersI4ivEXT(
	GLuint program,
	GLenum target,
	GLuint index,
	GLsizei count,
	const GLint *params)
{
	RenderCommand_namedProgramLocalParametersI4ivEXT* __restrict packet = (RenderCommand_namedProgramLocalParametersI4ivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedProgramLocalParametersI4ivEXT::execute);
	packet->program = program;
	packet->target = target;
	packet->index = index;
	packet->count = count;
	packet->params = params;
}
void namedProgramLocalParameterI4uiEXT(
	GLuint program,
	GLenum target,
	GLuint index,
	GLuint x,
	GLuint y,
	GLuint z,
	GLuint w)
{
	RenderCommand_namedProgramLocalParameterI4uiEXT* __restrict packet = (RenderCommand_namedProgramLocalParameterI4uiEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedProgramLocalParameterI4uiEXT::execute);
	packet->program = program;
	packet->target = target;
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void namedProgramLocalParameterI4uivEXT(
	GLuint program,
	GLenum target,
	GLuint index,
	const GLuint *params)
{
	RenderCommand_namedProgramLocalParameterI4uivEXT* __restrict packet = (RenderCommand_namedProgramLocalParameterI4uivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedProgramLocalParameterI4uivEXT::execute);
	packet->program = program;
	packet->target = target;
	packet->index = index;
	packet->params = params;
}
void namedProgramLocalParametersI4uivEXT(
	GLuint program,
	GLenum target,
	GLuint index,
	GLsizei count,
	const GLuint *params)
{
	RenderCommand_namedProgramLocalParametersI4uivEXT* __restrict packet = (RenderCommand_namedProgramLocalParametersI4uivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedProgramLocalParametersI4uivEXT::execute);
	packet->program = program;
	packet->target = target;
	packet->index = index;
	packet->count = count;
	packet->params = params;
}
void getNamedProgramLocalParameterIivEXT(
	GLuint program,
	GLenum target,
	GLuint index,
	GLint *params)
{
	RenderCommand_getNamedProgramLocalParameterIivEXT* __restrict packet = (RenderCommand_getNamedProgramLocalParameterIivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedProgramLocalParameterIivEXT::execute);
	packet->program = program;
	packet->target = target;
	packet->index = index;
	packet->params = params;
}
void getNamedProgramLocalParameterIuivEXT(
	GLuint program,
	GLenum target,
	GLuint index,
	GLuint *params)
{
	RenderCommand_getNamedProgramLocalParameterIuivEXT* __restrict packet = (RenderCommand_getNamedProgramLocalParameterIuivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedProgramLocalParameterIuivEXT::execute);
	packet->program = program;
	packet->target = target;
	packet->index = index;
	packet->params = params;
}
void enableClientStateiEXT(
	GLenum array,
	GLuint index)
{
	RenderCommand_enableClientStateiEXT* __restrict packet = (RenderCommand_enableClientStateiEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_enableClientStateiEXT::execute);
	packet->array = array;
	packet->index = index;
}
void disableClientStateiEXT(
	GLenum array,
	GLuint index)
{
	RenderCommand_disableClientStateiEXT* __restrict packet = (RenderCommand_disableClientStateiEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_disableClientStateiEXT::execute);
	packet->array = array;
	packet->index = index;
}
void getFloati_vEXT(
	GLenum pname,
	GLuint index,
	GLfloat *params)
{
	RenderCommand_getFloati_vEXT* __restrict packet = (RenderCommand_getFloati_vEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getFloati_vEXT::execute);
	packet->pname = pname;
	packet->index = index;
	packet->params = params;
}
void getDoublei_vEXT(
	GLenum pname,
	GLuint index,
	GLdouble *params)
{
	RenderCommand_getDoublei_vEXT* __restrict packet = (RenderCommand_getDoublei_vEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getDoublei_vEXT::execute);
	packet->pname = pname;
	packet->index = index;
	packet->params = params;
}
void getPointeri_vEXT(
	GLenum pname,
	GLuint index,
	void **params)
{
	RenderCommand_getPointeri_vEXT* __restrict packet = (RenderCommand_getPointeri_vEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPointeri_vEXT::execute);
	packet->pname = pname;
	packet->index = index;
	packet->params = params;
}
void namedProgramStringEXT(
	GLuint program,
	GLenum target,
	GLenum format,
	GLsizei len,
	const void *string)
{
	RenderCommand_namedProgramStringEXT* __restrict packet = (RenderCommand_namedProgramStringEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedProgramStringEXT::execute);
	packet->program = program;
	packet->target = target;
	packet->format = format;
	packet->len = len;
	packet->string = string;
}
void namedProgramLocalParameter4dEXT(
	GLuint program,
	GLenum target,
	GLuint index,
	GLdouble x,
	GLdouble y,
	GLdouble z,
	GLdouble w)
{
	RenderCommand_namedProgramLocalParameter4dEXT* __restrict packet = (RenderCommand_namedProgramLocalParameter4dEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedProgramLocalParameter4dEXT::execute);
	packet->program = program;
	packet->target = target;
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void namedProgramLocalParameter4dvEXT(
	GLuint program,
	GLenum target,
	GLuint index,
	const GLdouble *params)
{
	RenderCommand_namedProgramLocalParameter4dvEXT* __restrict packet = (RenderCommand_namedProgramLocalParameter4dvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedProgramLocalParameter4dvEXT::execute);
	packet->program = program;
	packet->target = target;
	packet->index = index;
	packet->params = params;
}
void namedProgramLocalParameter4fEXT(
	GLuint program,
	GLenum target,
	GLuint index,
	GLfloat x,
	GLfloat y,
	GLfloat z,
	GLfloat w)
{
	RenderCommand_namedProgramLocalParameter4fEXT* __restrict packet = (RenderCommand_namedProgramLocalParameter4fEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedProgramLocalParameter4fEXT::execute);
	packet->program = program;
	packet->target = target;
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void namedProgramLocalParameter4fvEXT(
	GLuint program,
	GLenum target,
	GLuint index,
	const GLfloat *params)
{
	RenderCommand_namedProgramLocalParameter4fvEXT* __restrict packet = (RenderCommand_namedProgramLocalParameter4fvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedProgramLocalParameter4fvEXT::execute);
	packet->program = program;
	packet->target = target;
	packet->index = index;
	packet->params = params;
}
void getNamedProgramLocalParameterdvEXT(
	GLuint program,
	GLenum target,
	GLuint index,
	GLdouble *params)
{
	RenderCommand_getNamedProgramLocalParameterdvEXT* __restrict packet = (RenderCommand_getNamedProgramLocalParameterdvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedProgramLocalParameterdvEXT::execute);
	packet->program = program;
	packet->target = target;
	packet->index = index;
	packet->params = params;
}
void getNamedProgramLocalParameterfvEXT(
	GLuint program,
	GLenum target,
	GLuint index,
	GLfloat *params)
{
	RenderCommand_getNamedProgramLocalParameterfvEXT* __restrict packet = (RenderCommand_getNamedProgramLocalParameterfvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedProgramLocalParameterfvEXT::execute);
	packet->program = program;
	packet->target = target;
	packet->index = index;
	packet->params = params;
}
void getNamedProgramivEXT(
	GLuint program,
	GLenum target,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getNamedProgramivEXT* __restrict packet = (RenderCommand_getNamedProgramivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedProgramivEXT::execute);
	packet->program = program;
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void getNamedProgramStringEXT(
	GLuint program,
	GLenum target,
	GLenum pname,
	void *string)
{
	RenderCommand_getNamedProgramStringEXT* __restrict packet = (RenderCommand_getNamedProgramStringEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedProgramStringEXT::execute);
	packet->program = program;
	packet->target = target;
	packet->pname = pname;
	packet->string = string;
}
void namedRenderbufferStorageEXT(
	GLuint renderbuffer,
	GLenum internalformat,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_namedRenderbufferStorageEXT* __restrict packet = (RenderCommand_namedRenderbufferStorageEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedRenderbufferStorageEXT::execute);
	packet->renderbuffer = renderbuffer;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
}
void getNamedRenderbufferParameterivEXT(
	GLuint renderbuffer,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getNamedRenderbufferParameterivEXT* __restrict packet = (RenderCommand_getNamedRenderbufferParameterivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedRenderbufferParameterivEXT::execute);
	packet->renderbuffer = renderbuffer;
	packet->pname = pname;
	packet->params = params;
}
void namedRenderbufferStorageMultisampleEXT(
	GLuint renderbuffer,
	GLsizei samples,
	GLenum internalformat,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_namedRenderbufferStorageMultisampleEXT* __restrict packet = (RenderCommand_namedRenderbufferStorageMultisampleEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedRenderbufferStorageMultisampleEXT::execute);
	packet->renderbuffer = renderbuffer;
	packet->samples = samples;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
}
void namedRenderbufferStorageMultisampleCoverageEXT(
	GLuint renderbuffer,
	GLsizei coverageSamples,
	GLsizei colorSamples,
	GLenum internalformat,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_namedRenderbufferStorageMultisampleCoverageEXT* __restrict packet = (RenderCommand_namedRenderbufferStorageMultisampleCoverageEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedRenderbufferStorageMultisampleCoverageEXT::execute);
	packet->renderbuffer = renderbuffer;
	packet->coverageSamples = coverageSamples;
	packet->colorSamples = colorSamples;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
}
GLenum checkNamedFramebufferStatusEXT(
	GLuint framebuffer,
	GLenum target)
{
	RenderCommand_checkNamedFramebufferStatusEXT* __restrict packet = (RenderCommand_checkNamedFramebufferStatusEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_checkNamedFramebufferStatusEXT::execute);
	packet->framebuffer = framebuffer;
	packet->target = target;
}
void namedFramebufferTexture1DEXT(
	GLuint framebuffer,
	GLenum attachment,
	GLenum textarget,
	GLuint texture,
	GLint level)
{
	RenderCommand_namedFramebufferTexture1DEXT* __restrict packet = (RenderCommand_namedFramebufferTexture1DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedFramebufferTexture1DEXT::execute);
	packet->framebuffer = framebuffer;
	packet->attachment = attachment;
	packet->textarget = textarget;
	packet->texture = texture;
	packet->level = level;
}
void namedFramebufferTexture2DEXT(
	GLuint framebuffer,
	GLenum attachment,
	GLenum textarget,
	GLuint texture,
	GLint level)
{
	RenderCommand_namedFramebufferTexture2DEXT* __restrict packet = (RenderCommand_namedFramebufferTexture2DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedFramebufferTexture2DEXT::execute);
	packet->framebuffer = framebuffer;
	packet->attachment = attachment;
	packet->textarget = textarget;
	packet->texture = texture;
	packet->level = level;
}
void namedFramebufferTexture3DEXT(
	GLuint framebuffer,
	GLenum attachment,
	GLenum textarget,
	GLuint texture,
	GLint level,
	GLint zoffset)
{
	RenderCommand_namedFramebufferTexture3DEXT* __restrict packet = (RenderCommand_namedFramebufferTexture3DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedFramebufferTexture3DEXT::execute);
	packet->framebuffer = framebuffer;
	packet->attachment = attachment;
	packet->textarget = textarget;
	packet->texture = texture;
	packet->level = level;
	packet->zoffset = zoffset;
}
void namedFramebufferRenderbufferEXT(
	GLuint framebuffer,
	GLenum attachment,
	GLenum renderbuffertarget,
	GLuint renderbuffer)
{
	RenderCommand_namedFramebufferRenderbufferEXT* __restrict packet = (RenderCommand_namedFramebufferRenderbufferEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedFramebufferRenderbufferEXT::execute);
	packet->framebuffer = framebuffer;
	packet->attachment = attachment;
	packet->renderbuffertarget = renderbuffertarget;
	packet->renderbuffer = renderbuffer;
}
void getNamedFramebufferAttachmentParameterivEXT(
	GLuint framebuffer,
	GLenum attachment,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getNamedFramebufferAttachmentParameterivEXT* __restrict packet = (RenderCommand_getNamedFramebufferAttachmentParameterivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedFramebufferAttachmentParameterivEXT::execute);
	packet->framebuffer = framebuffer;
	packet->attachment = attachment;
	packet->pname = pname;
	packet->params = params;
}
void generateTextureMipmapEXT(
	GLuint texture,
	GLenum target)
{
	RenderCommand_generateTextureMipmapEXT* __restrict packet = (RenderCommand_generateTextureMipmapEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_generateTextureMipmapEXT::execute);
	packet->texture = texture;
	packet->target = target;
}
void generateMultiTexMipmapEXT(
	GLenum texunit,
	GLenum target)
{
	RenderCommand_generateMultiTexMipmapEXT* __restrict packet = (RenderCommand_generateMultiTexMipmapEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_generateMultiTexMipmapEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
}
void framebufferDrawBufferEXT(
	GLuint framebuffer,
	GLenum mode)
{
	RenderCommand_framebufferDrawBufferEXT* __restrict packet = (RenderCommand_framebufferDrawBufferEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_framebufferDrawBufferEXT::execute);
	packet->framebuffer = framebuffer;
	packet->mode = mode;
}
void framebufferDrawBuffersEXT(
	GLuint framebuffer,
	GLsizei n,
	const GLenum *bufs)
{
	RenderCommand_framebufferDrawBuffersEXT* __restrict packet = (RenderCommand_framebufferDrawBuffersEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_framebufferDrawBuffersEXT::execute);
	packet->framebuffer = framebuffer;
	packet->n = n;
	packet->bufs = bufs;
}
void framebufferReadBufferEXT(
	GLuint framebuffer,
	GLenum mode)
{
	RenderCommand_framebufferReadBufferEXT* __restrict packet = (RenderCommand_framebufferReadBufferEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_framebufferReadBufferEXT::execute);
	packet->framebuffer = framebuffer;
	packet->mode = mode;
}
void getFramebufferParameterivEXT(
	GLuint framebuffer,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getFramebufferParameterivEXT* __restrict packet = (RenderCommand_getFramebufferParameterivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getFramebufferParameterivEXT::execute);
	packet->framebuffer = framebuffer;
	packet->pname = pname;
	packet->params = params;
}
void namedCopyBufferSubDataEXT(
	GLuint readBuffer,
	GLuint writeBuffer,
	GLintptr readOffset,
	GLintptr writeOffset,
	GLsizeiptr size)
{
	RenderCommand_namedCopyBufferSubDataEXT* __restrict packet = (RenderCommand_namedCopyBufferSubDataEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedCopyBufferSubDataEXT::execute);
	packet->readBuffer = readBuffer;
	packet->writeBuffer = writeBuffer;
	packet->readOffset = readOffset;
	packet->writeOffset = writeOffset;
	packet->size = size;
}
void namedFramebufferTextureEXT(
	GLuint framebuffer,
	GLenum attachment,
	GLuint texture,
	GLint level)
{
	RenderCommand_namedFramebufferTextureEXT* __restrict packet = (RenderCommand_namedFramebufferTextureEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedFramebufferTextureEXT::execute);
	packet->framebuffer = framebuffer;
	packet->attachment = attachment;
	packet->texture = texture;
	packet->level = level;
}
void namedFramebufferTextureLayerEXT(
	GLuint framebuffer,
	GLenum attachment,
	GLuint texture,
	GLint level,
	GLint layer)
{
	RenderCommand_namedFramebufferTextureLayerEXT* __restrict packet = (RenderCommand_namedFramebufferTextureLayerEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedFramebufferTextureLayerEXT::execute);
	packet->framebuffer = framebuffer;
	packet->attachment = attachment;
	packet->texture = texture;
	packet->level = level;
	packet->layer = layer;
}
void namedFramebufferTextureFaceEXT(
	GLuint framebuffer,
	GLenum attachment,
	GLuint texture,
	GLint level,
	GLenum face)
{
	RenderCommand_namedFramebufferTextureFaceEXT* __restrict packet = (RenderCommand_namedFramebufferTextureFaceEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedFramebufferTextureFaceEXT::execute);
	packet->framebuffer = framebuffer;
	packet->attachment = attachment;
	packet->texture = texture;
	packet->level = level;
	packet->face = face;
}
void textureRenderbufferEXT(
	GLuint texture,
	GLenum target,
	GLuint renderbuffer)
{
	RenderCommand_textureRenderbufferEXT* __restrict packet = (RenderCommand_textureRenderbufferEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureRenderbufferEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->renderbuffer = renderbuffer;
}
void multiTexRenderbufferEXT(
	GLenum texunit,
	GLenum target,
	GLuint renderbuffer)
{
	RenderCommand_multiTexRenderbufferEXT* __restrict packet = (RenderCommand_multiTexRenderbufferEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiTexRenderbufferEXT::execute);
	packet->texunit = texunit;
	packet->target = target;
	packet->renderbuffer = renderbuffer;
}
void vertexArrayVertexOffsetEXT(
	GLuint vaobj,
	GLuint buffer,
	GLint size,
	GLenum type,
	GLsizei stride,
	GLintptr offset)
{
	RenderCommand_vertexArrayVertexOffsetEXT* __restrict packet = (RenderCommand_vertexArrayVertexOffsetEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayVertexOffsetEXT::execute);
	packet->vaobj = vaobj;
	packet->buffer = buffer;
	packet->size = size;
	packet->type = type;
	packet->stride = stride;
	packet->offset = offset;
}
void vertexArrayColorOffsetEXT(
	GLuint vaobj,
	GLuint buffer,
	GLint size,
	GLenum type,
	GLsizei stride,
	GLintptr offset)
{
	RenderCommand_vertexArrayColorOffsetEXT* __restrict packet = (RenderCommand_vertexArrayColorOffsetEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayColorOffsetEXT::execute);
	packet->vaobj = vaobj;
	packet->buffer = buffer;
	packet->size = size;
	packet->type = type;
	packet->stride = stride;
	packet->offset = offset;
}
void vertexArrayEdgeFlagOffsetEXT(
	GLuint vaobj,
	GLuint buffer,
	GLsizei stride,
	GLintptr offset)
{
	RenderCommand_vertexArrayEdgeFlagOffsetEXT* __restrict packet = (RenderCommand_vertexArrayEdgeFlagOffsetEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayEdgeFlagOffsetEXT::execute);
	packet->vaobj = vaobj;
	packet->buffer = buffer;
	packet->stride = stride;
	packet->offset = offset;
}
void vertexArrayIndexOffsetEXT(
	GLuint vaobj,
	GLuint buffer,
	GLenum type,
	GLsizei stride,
	GLintptr offset)
{
	RenderCommand_vertexArrayIndexOffsetEXT* __restrict packet = (RenderCommand_vertexArrayIndexOffsetEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayIndexOffsetEXT::execute);
	packet->vaobj = vaobj;
	packet->buffer = buffer;
	packet->type = type;
	packet->stride = stride;
	packet->offset = offset;
}
void vertexArrayNormalOffsetEXT(
	GLuint vaobj,
	GLuint buffer,
	GLenum type,
	GLsizei stride,
	GLintptr offset)
{
	RenderCommand_vertexArrayNormalOffsetEXT* __restrict packet = (RenderCommand_vertexArrayNormalOffsetEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayNormalOffsetEXT::execute);
	packet->vaobj = vaobj;
	packet->buffer = buffer;
	packet->type = type;
	packet->stride = stride;
	packet->offset = offset;
}
void vertexArrayTexCoordOffsetEXT(
	GLuint vaobj,
	GLuint buffer,
	GLint size,
	GLenum type,
	GLsizei stride,
	GLintptr offset)
{
	RenderCommand_vertexArrayTexCoordOffsetEXT* __restrict packet = (RenderCommand_vertexArrayTexCoordOffsetEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayTexCoordOffsetEXT::execute);
	packet->vaobj = vaobj;
	packet->buffer = buffer;
	packet->size = size;
	packet->type = type;
	packet->stride = stride;
	packet->offset = offset;
}
void vertexArrayMultiTexCoordOffsetEXT(
	GLuint vaobj,
	GLuint buffer,
	GLenum texunit,
	GLint size,
	GLenum type,
	GLsizei stride,
	GLintptr offset)
{
	RenderCommand_vertexArrayMultiTexCoordOffsetEXT* __restrict packet = (RenderCommand_vertexArrayMultiTexCoordOffsetEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayMultiTexCoordOffsetEXT::execute);
	packet->vaobj = vaobj;
	packet->buffer = buffer;
	packet->texunit = texunit;
	packet->size = size;
	packet->type = type;
	packet->stride = stride;
	packet->offset = offset;
}
void vertexArrayFogCoordOffsetEXT(
	GLuint vaobj,
	GLuint buffer,
	GLenum type,
	GLsizei stride,
	GLintptr offset)
{
	RenderCommand_vertexArrayFogCoordOffsetEXT* __restrict packet = (RenderCommand_vertexArrayFogCoordOffsetEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayFogCoordOffsetEXT::execute);
	packet->vaobj = vaobj;
	packet->buffer = buffer;
	packet->type = type;
	packet->stride = stride;
	packet->offset = offset;
}
void vertexArraySecondaryColorOffsetEXT(
	GLuint vaobj,
	GLuint buffer,
	GLint size,
	GLenum type,
	GLsizei stride,
	GLintptr offset)
{
	RenderCommand_vertexArraySecondaryColorOffsetEXT* __restrict packet = (RenderCommand_vertexArraySecondaryColorOffsetEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArraySecondaryColorOffsetEXT::execute);
	packet->vaobj = vaobj;
	packet->buffer = buffer;
	packet->size = size;
	packet->type = type;
	packet->stride = stride;
	packet->offset = offset;
}
void vertexArrayVertexAttribOffsetEXT(
	GLuint vaobj,
	GLuint buffer,
	GLuint index,
	GLint size,
	GLenum type,
	GLboolean normalized,
	GLsizei stride,
	GLintptr offset)
{
	RenderCommand_vertexArrayVertexAttribOffsetEXT* __restrict packet = (RenderCommand_vertexArrayVertexAttribOffsetEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayVertexAttribOffsetEXT::execute);
	packet->vaobj = vaobj;
	packet->buffer = buffer;
	packet->index = index;
	packet->size = size;
	packet->type = type;
	packet->normalized = normalized;
	packet->stride = stride;
	packet->offset = offset;
}
void vertexArrayVertexAttribIOffsetEXT(
	GLuint vaobj,
	GLuint buffer,
	GLuint index,
	GLint size,
	GLenum type,
	GLsizei stride,
	GLintptr offset)
{
	RenderCommand_vertexArrayVertexAttribIOffsetEXT* __restrict packet = (RenderCommand_vertexArrayVertexAttribIOffsetEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayVertexAttribIOffsetEXT::execute);
	packet->vaobj = vaobj;
	packet->buffer = buffer;
	packet->index = index;
	packet->size = size;
	packet->type = type;
	packet->stride = stride;
	packet->offset = offset;
}
void enableVertexArrayEXT(
	GLuint vaobj,
	GLenum array)
{
	RenderCommand_enableVertexArrayEXT* __restrict packet = (RenderCommand_enableVertexArrayEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_enableVertexArrayEXT::execute);
	packet->vaobj = vaobj;
	packet->array = array;
}
void disableVertexArrayEXT(
	GLuint vaobj,
	GLenum array)
{
	RenderCommand_disableVertexArrayEXT* __restrict packet = (RenderCommand_disableVertexArrayEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_disableVertexArrayEXT::execute);
	packet->vaobj = vaobj;
	packet->array = array;
}
void enableVertexArrayAttribEXT(
	GLuint vaobj,
	GLuint index)
{
	RenderCommand_enableVertexArrayAttribEXT* __restrict packet = (RenderCommand_enableVertexArrayAttribEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_enableVertexArrayAttribEXT::execute);
	packet->vaobj = vaobj;
	packet->index = index;
}
void disableVertexArrayAttribEXT(
	GLuint vaobj,
	GLuint index)
{
	RenderCommand_disableVertexArrayAttribEXT* __restrict packet = (RenderCommand_disableVertexArrayAttribEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_disableVertexArrayAttribEXT::execute);
	packet->vaobj = vaobj;
	packet->index = index;
}
void getVertexArrayIntegervEXT(
	GLuint vaobj,
	GLenum pname,
	GLint *param)
{
	RenderCommand_getVertexArrayIntegervEXT* __restrict packet = (RenderCommand_getVertexArrayIntegervEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getVertexArrayIntegervEXT::execute);
	packet->vaobj = vaobj;
	packet->pname = pname;
	packet->param = param;
}
void getVertexArrayPointervEXT(
	GLuint vaobj,
	GLenum pname,
	void **param)
{
	RenderCommand_getVertexArrayPointervEXT* __restrict packet = (RenderCommand_getVertexArrayPointervEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getVertexArrayPointervEXT::execute);
	packet->vaobj = vaobj;
	packet->pname = pname;
	packet->param = param;
}
void getVertexArrayIntegeri_vEXT(
	GLuint vaobj,
	GLuint index,
	GLenum pname,
	GLint *param)
{
	RenderCommand_getVertexArrayIntegeri_vEXT* __restrict packet = (RenderCommand_getVertexArrayIntegeri_vEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getVertexArrayIntegeri_vEXT::execute);
	packet->vaobj = vaobj;
	packet->index = index;
	packet->pname = pname;
	packet->param = param;
}
void getVertexArrayPointeri_vEXT(
	GLuint vaobj,
	GLuint index,
	GLenum pname,
	void **param)
{
	RenderCommand_getVertexArrayPointeri_vEXT* __restrict packet = (RenderCommand_getVertexArrayPointeri_vEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getVertexArrayPointeri_vEXT::execute);
	packet->vaobj = vaobj;
	packet->index = index;
	packet->pname = pname;
	packet->param = param;
}
void* mapNamedBufferRangeEXT(
	GLuint buffer,
	GLintptr offset,
	GLsizeiptr length,
	GLbitfield access)
{
	RenderCommand_mapNamedBufferRangeEXT* __restrict packet = (RenderCommand_mapNamedBufferRangeEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_mapNamedBufferRangeEXT::execute);
	packet->buffer = buffer;
	packet->offset = offset;
	packet->length = length;
	packet->access = access;
}
void flushMappedNamedBufferRangeEXT(
	GLuint buffer,
	GLintptr offset,
	GLsizeiptr length)
{
	RenderCommand_flushMappedNamedBufferRangeEXT* __restrict packet = (RenderCommand_flushMappedNamedBufferRangeEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_flushMappedNamedBufferRangeEXT::execute);
	packet->buffer = buffer;
	packet->offset = offset;
	packet->length = length;
}
void namedBufferStorageEXT(
	GLuint buffer,
	GLsizeiptr size,
	const void *data,
	GLbitfield flags)
{
	RenderCommand_namedBufferStorageEXT* __restrict packet = (RenderCommand_namedBufferStorageEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedBufferStorageEXT::execute);
	packet->buffer = buffer;
	packet->size = size;
	packet->data = data;
	packet->flags = flags;
}
void clearNamedBufferDataEXT(
	GLuint buffer,
	GLenum internalformat,
	GLenum format,
	GLenum type,
	const void *data)
{
	RenderCommand_clearNamedBufferDataEXT* __restrict packet = (RenderCommand_clearNamedBufferDataEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clearNamedBufferDataEXT::execute);
	packet->buffer = buffer;
	packet->internalformat = internalformat;
	packet->format = format;
	packet->type = type;
	packet->data = data;
}
void clearNamedBufferSubDataEXT(
	GLuint buffer,
	GLenum internalformat,
	GLsizeiptr offset,
	GLsizeiptr size,
	GLenum format,
	GLenum type,
	const void *data)
{
	RenderCommand_clearNamedBufferSubDataEXT* __restrict packet = (RenderCommand_clearNamedBufferSubDataEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_clearNamedBufferSubDataEXT::execute);
	packet->buffer = buffer;
	packet->internalformat = internalformat;
	packet->offset = offset;
	packet->size = size;
	packet->format = format;
	packet->type = type;
	packet->data = data;
}
void namedFramebufferParameteriEXT(
	GLuint framebuffer,
	GLenum pname,
	GLint param)
{
	RenderCommand_namedFramebufferParameteriEXT* __restrict packet = (RenderCommand_namedFramebufferParameteriEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedFramebufferParameteriEXT::execute);
	packet->framebuffer = framebuffer;
	packet->pname = pname;
	packet->param = param;
}
void getNamedFramebufferParameterivEXT(
	GLuint framebuffer,
	GLenum pname,
	GLint *params)
{
	RenderCommand_getNamedFramebufferParameterivEXT* __restrict packet = (RenderCommand_getNamedFramebufferParameterivEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedFramebufferParameterivEXT::execute);
	packet->framebuffer = framebuffer;
	packet->pname = pname;
	packet->params = params;
}
void programUniform1dEXT(
	GLuint program,
	GLint location,
	GLdouble x)
{
	RenderCommand_programUniform1dEXT* __restrict packet = (RenderCommand_programUniform1dEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1dEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->x = x;
}
void programUniform2dEXT(
	GLuint program,
	GLint location,
	GLdouble x,
	GLdouble y)
{
	RenderCommand_programUniform2dEXT* __restrict packet = (RenderCommand_programUniform2dEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2dEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->x = x;
	packet->y = y;
}
void programUniform3dEXT(
	GLuint program,
	GLint location,
	GLdouble x,
	GLdouble y,
	GLdouble z)
{
	RenderCommand_programUniform3dEXT* __restrict packet = (RenderCommand_programUniform3dEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3dEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void programUniform4dEXT(
	GLuint program,
	GLint location,
	GLdouble x,
	GLdouble y,
	GLdouble z,
	GLdouble w)
{
	RenderCommand_programUniform4dEXT* __restrict packet = (RenderCommand_programUniform4dEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4dEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void programUniform1dvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLdouble *value)
{
	RenderCommand_programUniform1dvEXT* __restrict packet = (RenderCommand_programUniform1dvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1dvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform2dvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLdouble *value)
{
	RenderCommand_programUniform2dvEXT* __restrict packet = (RenderCommand_programUniform2dvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2dvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform3dvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLdouble *value)
{
	RenderCommand_programUniform3dvEXT* __restrict packet = (RenderCommand_programUniform3dvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3dvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform4dvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLdouble *value)
{
	RenderCommand_programUniform4dvEXT* __restrict packet = (RenderCommand_programUniform4dvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4dvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniformMatrix2dvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_programUniformMatrix2dvEXT* __restrict packet = (RenderCommand_programUniformMatrix2dvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix2dvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix3dvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_programUniformMatrix3dvEXT* __restrict packet = (RenderCommand_programUniformMatrix3dvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix3dvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix4dvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_programUniformMatrix4dvEXT* __restrict packet = (RenderCommand_programUniformMatrix4dvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix4dvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix2x3dvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_programUniformMatrix2x3dvEXT* __restrict packet = (RenderCommand_programUniformMatrix2x3dvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix2x3dvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix2x4dvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_programUniformMatrix2x4dvEXT* __restrict packet = (RenderCommand_programUniformMatrix2x4dvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix2x4dvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix3x2dvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_programUniformMatrix3x2dvEXT* __restrict packet = (RenderCommand_programUniformMatrix3x2dvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix3x2dvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix3x4dvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_programUniformMatrix3x4dvEXT* __restrict packet = (RenderCommand_programUniformMatrix3x4dvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix3x4dvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix4x2dvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_programUniformMatrix4x2dvEXT* __restrict packet = (RenderCommand_programUniformMatrix4x2dvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix4x2dvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void programUniformMatrix4x3dvEXT(
	GLuint program,
	GLint location,
	GLsizei count,
	GLboolean transpose,
	const GLdouble *value)
{
	RenderCommand_programUniformMatrix4x3dvEXT* __restrict packet = (RenderCommand_programUniformMatrix4x3dvEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformMatrix4x3dvEXT::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->transpose = transpose;
	packet->value = value;
}
void textureBufferRangeEXT(
	GLuint texture,
	GLenum target,
	GLenum internalformat,
	GLuint buffer,
	GLintptr offset,
	GLsizeiptr size)
{
	RenderCommand_textureBufferRangeEXT* __restrict packet = (RenderCommand_textureBufferRangeEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureBufferRangeEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->internalformat = internalformat;
	packet->buffer = buffer;
	packet->offset = offset;
	packet->size = size;
}
void textureStorage1DEXT(
	GLuint texture,
	GLenum target,
	GLsizei levels,
	GLenum internalformat,
	GLsizei width)
{
	RenderCommand_textureStorage1DEXT* __restrict packet = (RenderCommand_textureStorage1DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureStorage1DEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->levels = levels;
	packet->internalformat = internalformat;
	packet->width = width;
}
void textureStorage2DEXT(
	GLuint texture,
	GLenum target,
	GLsizei levels,
	GLenum internalformat,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_textureStorage2DEXT* __restrict packet = (RenderCommand_textureStorage2DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureStorage2DEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->levels = levels;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
}
void textureStorage3DEXT(
	GLuint texture,
	GLenum target,
	GLsizei levels,
	GLenum internalformat,
	GLsizei width,
	GLsizei height,
	GLsizei depth)
{
	RenderCommand_textureStorage3DEXT* __restrict packet = (RenderCommand_textureStorage3DEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureStorage3DEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->levels = levels;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
}
void textureStorage2DMultisampleEXT(
	GLuint texture,
	GLenum target,
	GLsizei samples,
	GLenum internalformat,
	GLsizei width,
	GLsizei height,
	GLboolean fixedsamplelocations)
{
	RenderCommand_textureStorage2DMultisampleEXT* __restrict packet = (RenderCommand_textureStorage2DMultisampleEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureStorage2DMultisampleEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->samples = samples;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->fixedsamplelocations = fixedsamplelocations;
}
void textureStorage3DMultisampleEXT(
	GLuint texture,
	GLenum target,
	GLsizei samples,
	GLenum internalformat,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLboolean fixedsamplelocations)
{
	RenderCommand_textureStorage3DMultisampleEXT* __restrict packet = (RenderCommand_textureStorage3DMultisampleEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureStorage3DMultisampleEXT::execute);
	packet->texture = texture;
	packet->target = target;
	packet->samples = samples;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->fixedsamplelocations = fixedsamplelocations;
}
void vertexArrayBindVertexBufferEXT(
	GLuint vaobj,
	GLuint bindingindex,
	GLuint buffer,
	GLintptr offset,
	GLsizei stride)
{
	RenderCommand_vertexArrayBindVertexBufferEXT* __restrict packet = (RenderCommand_vertexArrayBindVertexBufferEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayBindVertexBufferEXT::execute);
	packet->vaobj = vaobj;
	packet->bindingindex = bindingindex;
	packet->buffer = buffer;
	packet->offset = offset;
	packet->stride = stride;
}
void vertexArrayVertexAttribFormatEXT(
	GLuint vaobj,
	GLuint attribindex,
	GLint size,
	GLenum type,
	GLboolean normalized,
	GLuint relativeoffset)
{
	RenderCommand_vertexArrayVertexAttribFormatEXT* __restrict packet = (RenderCommand_vertexArrayVertexAttribFormatEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayVertexAttribFormatEXT::execute);
	packet->vaobj = vaobj;
	packet->attribindex = attribindex;
	packet->size = size;
	packet->type = type;
	packet->normalized = normalized;
	packet->relativeoffset = relativeoffset;
}
void vertexArrayVertexAttribIFormatEXT(
	GLuint vaobj,
	GLuint attribindex,
	GLint size,
	GLenum type,
	GLuint relativeoffset)
{
	RenderCommand_vertexArrayVertexAttribIFormatEXT* __restrict packet = (RenderCommand_vertexArrayVertexAttribIFormatEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayVertexAttribIFormatEXT::execute);
	packet->vaobj = vaobj;
	packet->attribindex = attribindex;
	packet->size = size;
	packet->type = type;
	packet->relativeoffset = relativeoffset;
}
void vertexArrayVertexAttribLFormatEXT(
	GLuint vaobj,
	GLuint attribindex,
	GLint size,
	GLenum type,
	GLuint relativeoffset)
{
	RenderCommand_vertexArrayVertexAttribLFormatEXT* __restrict packet = (RenderCommand_vertexArrayVertexAttribLFormatEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayVertexAttribLFormatEXT::execute);
	packet->vaobj = vaobj;
	packet->attribindex = attribindex;
	packet->size = size;
	packet->type = type;
	packet->relativeoffset = relativeoffset;
}
void vertexArrayVertexAttribBindingEXT(
	GLuint vaobj,
	GLuint attribindex,
	GLuint bindingindex)
{
	RenderCommand_vertexArrayVertexAttribBindingEXT* __restrict packet = (RenderCommand_vertexArrayVertexAttribBindingEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayVertexAttribBindingEXT::execute);
	packet->vaobj = vaobj;
	packet->attribindex = attribindex;
	packet->bindingindex = bindingindex;
}
void vertexArrayVertexBindingDivisorEXT(
	GLuint vaobj,
	GLuint bindingindex,
	GLuint divisor)
{
	RenderCommand_vertexArrayVertexBindingDivisorEXT* __restrict packet = (RenderCommand_vertexArrayVertexBindingDivisorEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayVertexBindingDivisorEXT::execute);
	packet->vaobj = vaobj;
	packet->bindingindex = bindingindex;
	packet->divisor = divisor;
}
void vertexArrayVertexAttribLOffsetEXT(
	GLuint vaobj,
	GLuint buffer,
	GLuint index,
	GLint size,
	GLenum type,
	GLsizei stride,
	GLintptr offset)
{
	RenderCommand_vertexArrayVertexAttribLOffsetEXT* __restrict packet = (RenderCommand_vertexArrayVertexAttribLOffsetEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayVertexAttribLOffsetEXT::execute);
	packet->vaobj = vaobj;
	packet->buffer = buffer;
	packet->index = index;
	packet->size = size;
	packet->type = type;
	packet->stride = stride;
	packet->offset = offset;
}
void texturePageCommitmentEXT(
	GLuint texture,
	GLint level,
	GLint xoffset,
	GLint yoffset,
	GLint zoffset,
	GLsizei width,
	GLsizei height,
	GLsizei depth,
	GLboolean commit)
{
	RenderCommand_texturePageCommitmentEXT* __restrict packet = (RenderCommand_texturePageCommitmentEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texturePageCommitmentEXT::execute);
	packet->texture = texture;
	packet->level = level;
	packet->xoffset = xoffset;
	packet->yoffset = yoffset;
	packet->zoffset = zoffset;
	packet->width = width;
	packet->height = height;
	packet->depth = depth;
	packet->commit = commit;
}
void vertexArrayVertexAttribDivisorEXT(
	GLuint vaobj,
	GLuint index,
	GLuint divisor)
{
	RenderCommand_vertexArrayVertexAttribDivisorEXT* __restrict packet = (RenderCommand_vertexArrayVertexAttribDivisorEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexArrayVertexAttribDivisorEXT::execute);
	packet->vaobj = vaobj;
	packet->index = index;
	packet->divisor = divisor;
}
void drawArraysInstancedEXT(
	GLenum mode,
	GLint start,
	GLsizei count,
	GLsizei primcount)
{
	RenderCommand_drawArraysInstancedEXT* __restrict packet = (RenderCommand_drawArraysInstancedEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawArraysInstancedEXT::execute);
	packet->mode = mode;
	packet->start = start;
	packet->count = count;
	packet->primcount = primcount;
}
void drawElementsInstancedEXT(
	GLenum mode,
	GLsizei count,
	GLenum type,
	const void *indices,
	GLsizei primcount)
{
	RenderCommand_drawElementsInstancedEXT* __restrict packet = (RenderCommand_drawElementsInstancedEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawElementsInstancedEXT::execute);
	packet->mode = mode;
	packet->count = count;
	packet->type = type;
	packet->indices = indices;
	packet->primcount = primcount;
}
void polygonOffsetClampEXT(
	GLfloat factor,
	GLfloat units,
	GLfloat clamp)
{
	RenderCommand_polygonOffsetClampEXT* __restrict packet = (RenderCommand_polygonOffsetClampEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_polygonOffsetClampEXT::execute);
	packet->factor = factor;
	packet->units = units;
	packet->clamp = clamp;
}
void rasterSamplesEXT(
	GLuint samples,
	GLboolean fixedsamplelocations)
{
	RenderCommand_rasterSamplesEXT* __restrict packet = (RenderCommand_rasterSamplesEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_rasterSamplesEXT::execute);
	packet->samples = samples;
	packet->fixedsamplelocations = fixedsamplelocations;
}
void useShaderProgramEXT(
	GLenum type,
	GLuint program)
{
	RenderCommand_useShaderProgramEXT* __restrict packet = (RenderCommand_useShaderProgramEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_useShaderProgramEXT::execute);
	packet->type = type;
	packet->program = program;
}
void activeProgramEXT(
	GLuint program)
{
	RenderCommand_activeProgramEXT* __restrict packet = (RenderCommand_activeProgramEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_activeProgramEXT::execute);
	packet->program = program;
}
GLuint createShaderProgramEXT(
	GLenum type,
	const GLchar *string)
{
	RenderCommand_createShaderProgramEXT* __restrict packet = (RenderCommand_createShaderProgramEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_createShaderProgramEXT::execute);
	packet->type = type;
	packet->string = string;
}
void framebufferFetchBarrierEXT(
	void)
{
	RenderCommand_framebufferFetchBarrierEXT* __restrict packet = (RenderCommand_framebufferFetchBarrierEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_framebufferFetchBarrierEXT::execute);
}
void windowRectanglesEXT(
	GLenum mode,
	GLsizei count,
	const GLint *box)
{
	RenderCommand_windowRectanglesEXT* __restrict packet = (RenderCommand_windowRectanglesEXT*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_windowRectanglesEXT::execute);
	packet->mode = mode;
	packet->count = count;
	packet->box = box;
}
void applyFramebufferAttachmentCMAAINTEL(
	void)
{
	RenderCommand_applyFramebufferAttachmentCMAAINTEL* __restrict packet = (RenderCommand_applyFramebufferAttachmentCMAAINTEL*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_applyFramebufferAttachmentCMAAINTEL::execute);
}
void beginPerfQueryINTEL(
	GLuint queryHandle)
{
	RenderCommand_beginPerfQueryINTEL* __restrict packet = (RenderCommand_beginPerfQueryINTEL*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_beginPerfQueryINTEL::execute);
	packet->queryHandle = queryHandle;
}
void createPerfQueryINTEL(
	GLuint queryId,
	GLuint *queryHandle)
{
	RenderCommand_createPerfQueryINTEL* __restrict packet = (RenderCommand_createPerfQueryINTEL*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_createPerfQueryINTEL::execute);
	packet->queryId = queryId;
	packet->queryHandle = queryHandle;
}
void deletePerfQueryINTEL(
	GLuint queryHandle)
{
	RenderCommand_deletePerfQueryINTEL* __restrict packet = (RenderCommand_deletePerfQueryINTEL*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_deletePerfQueryINTEL::execute);
	packet->queryHandle = queryHandle;
}
void endPerfQueryINTEL(
	GLuint queryHandle)
{
	RenderCommand_endPerfQueryINTEL* __restrict packet = (RenderCommand_endPerfQueryINTEL*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_endPerfQueryINTEL::execute);
	packet->queryHandle = queryHandle;
}
void getFirstPerfQueryIdINTEL(
	GLuint *queryId)
{
	RenderCommand_getFirstPerfQueryIdINTEL* __restrict packet = (RenderCommand_getFirstPerfQueryIdINTEL*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getFirstPerfQueryIdINTEL::execute);
	packet->queryId = queryId;
}
void getNextPerfQueryIdINTEL(
	GLuint queryId,
	GLuint *nextQueryId)
{
	RenderCommand_getNextPerfQueryIdINTEL* __restrict packet = (RenderCommand_getNextPerfQueryIdINTEL*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNextPerfQueryIdINTEL::execute);
	packet->queryId = queryId;
	packet->nextQueryId = nextQueryId;
}
void getPerfCounterInfoINTEL(
	GLuint queryId,
	GLuint counterId,
	GLuint counterNameLength,
	GLchar *counterName,
	GLuint counterDescLength,
	GLchar *counterDesc,
	GLuint *counterOffset,
	GLuint *counterDataSize,
	GLuint *counterTypeEnum,
	GLuint *counterDataTypeEnum,
	GLuint64 *rawCounterMaxValue)
{
	RenderCommand_getPerfCounterInfoINTEL* __restrict packet = (RenderCommand_getPerfCounterInfoINTEL*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPerfCounterInfoINTEL::execute);
	packet->queryId = queryId;
	packet->counterId = counterId;
	packet->counterNameLength = counterNameLength;
	packet->counterName = counterName;
	packet->counterDescLength = counterDescLength;
	packet->counterDesc = counterDesc;
	packet->counterOffset = counterOffset;
	packet->counterDataSize = counterDataSize;
	packet->counterTypeEnum = counterTypeEnum;
	packet->counterDataTypeEnum = counterDataTypeEnum;
	packet->rawCounterMaxValue = rawCounterMaxValue;
}
void getPerfQueryDataINTEL(
	GLuint queryHandle,
	GLuint flags,
	GLsizei dataSize,
	void *data,
	GLuint *bytesWritten)
{
	RenderCommand_getPerfQueryDataINTEL* __restrict packet = (RenderCommand_getPerfQueryDataINTEL*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPerfQueryDataINTEL::execute);
	packet->queryHandle = queryHandle;
	packet->flags = flags;
	packet->dataSize = dataSize;
	packet->data = data;
	packet->bytesWritten = bytesWritten;
}
void getPerfQueryIdByNameINTEL(
	GLchar *queryName,
	GLuint *queryId)
{
	RenderCommand_getPerfQueryIdByNameINTEL* __restrict packet = (RenderCommand_getPerfQueryIdByNameINTEL*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPerfQueryIdByNameINTEL::execute);
	packet->queryName = queryName;
	packet->queryId = queryId;
}
void getPerfQueryInfoINTEL(
	GLuint queryId,
	GLuint queryNameLength,
	GLchar *queryName,
	GLuint *dataSize,
	GLuint *noCounters,
	GLuint *noInstances,
	GLuint *capsMask)
{
	RenderCommand_getPerfQueryInfoINTEL* __restrict packet = (RenderCommand_getPerfQueryInfoINTEL*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPerfQueryInfoINTEL::execute);
	packet->queryId = queryId;
	packet->queryNameLength = queryNameLength;
	packet->queryName = queryName;
	packet->dataSize = dataSize;
	packet->noCounters = noCounters;
	packet->noInstances = noInstances;
	packet->capsMask = capsMask;
}
void multiDrawArraysIndirectBindlessNV(
	GLenum mode,
	const void *indirect,
	GLsizei drawCount,
	GLsizei stride,
	GLint vertexBufferCount)
{
	RenderCommand_multiDrawArraysIndirectBindlessNV* __restrict packet = (RenderCommand_multiDrawArraysIndirectBindlessNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiDrawArraysIndirectBindlessNV::execute);
	packet->mode = mode;
	packet->indirect = indirect;
	packet->drawCount = drawCount;
	packet->stride = stride;
	packet->vertexBufferCount = vertexBufferCount;
}
void multiDrawElementsIndirectBindlessNV(
	GLenum mode,
	GLenum type,
	const void *indirect,
	GLsizei drawCount,
	GLsizei stride,
	GLint vertexBufferCount)
{
	RenderCommand_multiDrawElementsIndirectBindlessNV* __restrict packet = (RenderCommand_multiDrawElementsIndirectBindlessNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiDrawElementsIndirectBindlessNV::execute);
	packet->mode = mode;
	packet->type = type;
	packet->indirect = indirect;
	packet->drawCount = drawCount;
	packet->stride = stride;
	packet->vertexBufferCount = vertexBufferCount;
}
void multiDrawArraysIndirectBindlessCountNV(
	GLenum mode,
	const void *indirect,
	GLsizei drawCount,
	GLsizei maxDrawCount,
	GLsizei stride,
	GLint vertexBufferCount)
{
	RenderCommand_multiDrawArraysIndirectBindlessCountNV* __restrict packet = (RenderCommand_multiDrawArraysIndirectBindlessCountNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiDrawArraysIndirectBindlessCountNV::execute);
	packet->mode = mode;
	packet->indirect = indirect;
	packet->drawCount = drawCount;
	packet->maxDrawCount = maxDrawCount;
	packet->stride = stride;
	packet->vertexBufferCount = vertexBufferCount;
}
void multiDrawElementsIndirectBindlessCountNV(
	GLenum mode,
	GLenum type,
	const void *indirect,
	GLsizei drawCount,
	GLsizei maxDrawCount,
	GLsizei stride,
	GLint vertexBufferCount)
{
	RenderCommand_multiDrawElementsIndirectBindlessCountNV* __restrict packet = (RenderCommand_multiDrawElementsIndirectBindlessCountNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiDrawElementsIndirectBindlessCountNV::execute);
	packet->mode = mode;
	packet->type = type;
	packet->indirect = indirect;
	packet->drawCount = drawCount;
	packet->maxDrawCount = maxDrawCount;
	packet->stride = stride;
	packet->vertexBufferCount = vertexBufferCount;
}
GLuint64 getTextureHandleNV(
	GLuint texture)
{
	RenderCommand_getTextureHandleNV* __restrict packet = (RenderCommand_getTextureHandleNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTextureHandleNV::execute);
	packet->texture = texture;
}
GLuint64 getTextureSamplerHandleNV(
	GLuint texture,
	GLuint sampler)
{
	RenderCommand_getTextureSamplerHandleNV* __restrict packet = (RenderCommand_getTextureSamplerHandleNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getTextureSamplerHandleNV::execute);
	packet->texture = texture;
	packet->sampler = sampler;
}
void makeTextureHandleResidentNV(
	GLuint64 handle)
{
	RenderCommand_makeTextureHandleResidentNV* __restrict packet = (RenderCommand_makeTextureHandleResidentNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_makeTextureHandleResidentNV::execute);
	packet->handle = handle;
}
void makeTextureHandleNonResidentNV(
	GLuint64 handle)
{
	RenderCommand_makeTextureHandleNonResidentNV* __restrict packet = (RenderCommand_makeTextureHandleNonResidentNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_makeTextureHandleNonResidentNV::execute);
	packet->handle = handle;
}
GLuint64 getImageHandleNV(
	GLuint texture,
	GLint level,
	GLboolean layered,
	GLint layer,
	GLenum format)
{
	RenderCommand_getImageHandleNV* __restrict packet = (RenderCommand_getImageHandleNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getImageHandleNV::execute);
	packet->texture = texture;
	packet->level = level;
	packet->layered = layered;
	packet->layer = layer;
	packet->format = format;
}
void makeImageHandleResidentNV(
	GLuint64 handle,
	GLenum access)
{
	RenderCommand_makeImageHandleResidentNV* __restrict packet = (RenderCommand_makeImageHandleResidentNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_makeImageHandleResidentNV::execute);
	packet->handle = handle;
	packet->access = access;
}
void makeImageHandleNonResidentNV(
	GLuint64 handle)
{
	RenderCommand_makeImageHandleNonResidentNV* __restrict packet = (RenderCommand_makeImageHandleNonResidentNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_makeImageHandleNonResidentNV::execute);
	packet->handle = handle;
}
void uniformHandleui64NV(
	GLint location,
	GLuint64 value)
{
	RenderCommand_uniformHandleui64NV* __restrict packet = (RenderCommand_uniformHandleui64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformHandleui64NV::execute);
	packet->location = location;
	packet->value = value;
}
void uniformHandleui64vNV(
	GLint location,
	GLsizei count,
	const GLuint64 *value)
{
	RenderCommand_uniformHandleui64vNV* __restrict packet = (RenderCommand_uniformHandleui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformHandleui64vNV::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniformHandleui64NV(
	GLuint program,
	GLint location,
	GLuint64 value)
{
	RenderCommand_programUniformHandleui64NV* __restrict packet = (RenderCommand_programUniformHandleui64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformHandleui64NV::execute);
	packet->program = program;
	packet->location = location;
	packet->value = value;
}
void programUniformHandleui64vNV(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLuint64 *values)
{
	RenderCommand_programUniformHandleui64vNV* __restrict packet = (RenderCommand_programUniformHandleui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformHandleui64vNV::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->values = values;
}
GLboolean isTextureHandleResidentNV(
	GLuint64 handle)
{
	RenderCommand_isTextureHandleResidentNV* __restrict packet = (RenderCommand_isTextureHandleResidentNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isTextureHandleResidentNV::execute);
	packet->handle = handle;
}
GLboolean isImageHandleResidentNV(
	GLuint64 handle)
{
	RenderCommand_isImageHandleResidentNV* __restrict packet = (RenderCommand_isImageHandleResidentNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isImageHandleResidentNV::execute);
	packet->handle = handle;
}
void blendParameteriNV(
	GLenum pname,
	GLint value)
{
	RenderCommand_blendParameteriNV* __restrict packet = (RenderCommand_blendParameteriNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_blendParameteriNV::execute);
	packet->pname = pname;
	packet->value = value;
}
void blendBarrierNV(
	void)
{
	RenderCommand_blendBarrierNV* __restrict packet = (RenderCommand_blendBarrierNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_blendBarrierNV::execute);
}
void viewportPositionWScaleNV(
	GLuint index,
	GLfloat xcoeff,
	GLfloat ycoeff)
{
	RenderCommand_viewportPositionWScaleNV* __restrict packet = (RenderCommand_viewportPositionWScaleNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_viewportPositionWScaleNV::execute);
	packet->index = index;
	packet->xcoeff = xcoeff;
	packet->ycoeff = ycoeff;
}
void createStatesNV(
	GLsizei n,
	GLuint *states)
{
	RenderCommand_createStatesNV* __restrict packet = (RenderCommand_createStatesNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_createStatesNV::execute);
	packet->n = n;
	packet->states = states;
}
void deleteStatesNV(
	GLsizei n,
	const GLuint *states)
{
	RenderCommand_deleteStatesNV* __restrict packet = (RenderCommand_deleteStatesNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_deleteStatesNV::execute);
	packet->n = n;
	packet->states = states;
}
GLboolean isStateNV(
	GLuint state)
{
	RenderCommand_isStateNV* __restrict packet = (RenderCommand_isStateNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isStateNV::execute);
	packet->state = state;
}
void stateCaptureNV(
	GLuint state,
	GLenum mode)
{
	RenderCommand_stateCaptureNV* __restrict packet = (RenderCommand_stateCaptureNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_stateCaptureNV::execute);
	packet->state = state;
	packet->mode = mode;
}
GLuint getCommandHeaderNV(
	GLenum tokenID,
	GLuint size)
{
	RenderCommand_getCommandHeaderNV* __restrict packet = (RenderCommand_getCommandHeaderNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getCommandHeaderNV::execute);
	packet->tokenID = tokenID;
	packet->size = size;
}
GLushort getStageIndexNV(
	GLenum shadertype)
{
	RenderCommand_getStageIndexNV* __restrict packet = (RenderCommand_getStageIndexNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getStageIndexNV::execute);
	packet->shadertype = shadertype;
}
void drawCommandsNV(
	GLenum primitiveMode,
	GLuint buffer,
	const GLintptr *indirects,
	const GLsizei *sizes,
	GLuint count)
{
	RenderCommand_drawCommandsNV* __restrict packet = (RenderCommand_drawCommandsNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawCommandsNV::execute);
	packet->primitiveMode = primitiveMode;
	packet->buffer = buffer;
	packet->indirects = indirects;
	packet->sizes = sizes;
	packet->count = count;
}
void drawCommandsAddressNV(
	GLenum primitiveMode,
	const GLuint64 *indirects,
	const GLsizei *sizes,
	GLuint count)
{
	RenderCommand_drawCommandsAddressNV* __restrict packet = (RenderCommand_drawCommandsAddressNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawCommandsAddressNV::execute);
	packet->primitiveMode = primitiveMode;
	packet->indirects = indirects;
	packet->sizes = sizes;
	packet->count = count;
}
void drawCommandsStatesNV(
	GLuint buffer,
	const GLintptr *indirects,
	const GLsizei *sizes,
	const GLuint *states,
	const GLuint *fbos,
	GLuint count)
{
	RenderCommand_drawCommandsStatesNV* __restrict packet = (RenderCommand_drawCommandsStatesNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawCommandsStatesNV::execute);
	packet->buffer = buffer;
	packet->indirects = indirects;
	packet->sizes = sizes;
	packet->states = states;
	packet->fbos = fbos;
	packet->count = count;
}
void drawCommandsStatesAddressNV(
	const GLuint64 *indirects,
	const GLsizei *sizes,
	const GLuint *states,
	const GLuint *fbos,
	GLuint count)
{
	RenderCommand_drawCommandsStatesAddressNV* __restrict packet = (RenderCommand_drawCommandsStatesAddressNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawCommandsStatesAddressNV::execute);
	packet->indirects = indirects;
	packet->sizes = sizes;
	packet->states = states;
	packet->fbos = fbos;
	packet->count = count;
}
void createCommandListsNV(
	GLsizei n,
	GLuint *lists)
{
	RenderCommand_createCommandListsNV* __restrict packet = (RenderCommand_createCommandListsNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_createCommandListsNV::execute);
	packet->n = n;
	packet->lists = lists;
}
void deleteCommandListsNV(
	GLsizei n,
	const GLuint *lists)
{
	RenderCommand_deleteCommandListsNV* __restrict packet = (RenderCommand_deleteCommandListsNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_deleteCommandListsNV::execute);
	packet->n = n;
	packet->lists = lists;
}
GLboolean isCommandListNV(
	GLuint list)
{
	RenderCommand_isCommandListNV* __restrict packet = (RenderCommand_isCommandListNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isCommandListNV::execute);
	packet->list = list;
}
void listDrawCommandsStatesClientNV(
	GLuint list,
	GLuint segment,
	const void **indirects,
	const GLsizei *sizes,
	const GLuint *states,
	const GLuint *fbos,
	GLuint count)
{
	RenderCommand_listDrawCommandsStatesClientNV* __restrict packet = (RenderCommand_listDrawCommandsStatesClientNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_listDrawCommandsStatesClientNV::execute);
	packet->list = list;
	packet->segment = segment;
	packet->indirects = indirects;
	packet->sizes = sizes;
	packet->states = states;
	packet->fbos = fbos;
	packet->count = count;
}
void commandListSegmentsNV(
	GLuint list,
	GLuint segments)
{
	RenderCommand_commandListSegmentsNV* __restrict packet = (RenderCommand_commandListSegmentsNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_commandListSegmentsNV::execute);
	packet->list = list;
	packet->segments = segments;
}
void compileCommandListNV(
	GLuint list)
{
	RenderCommand_compileCommandListNV* __restrict packet = (RenderCommand_compileCommandListNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_compileCommandListNV::execute);
	packet->list = list;
}
void callCommandListNV(
	GLuint list)
{
	RenderCommand_callCommandListNV* __restrict packet = (RenderCommand_callCommandListNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_callCommandListNV::execute);
	packet->list = list;
}
void beginConditionalRenderNV(
	GLuint id,
	GLenum mode)
{
	RenderCommand_beginConditionalRenderNV* __restrict packet = (RenderCommand_beginConditionalRenderNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_beginConditionalRenderNV::execute);
	packet->id = id;
	packet->mode = mode;
}
void endConditionalRenderNV(
	void)
{
	RenderCommand_endConditionalRenderNV* __restrict packet = (RenderCommand_endConditionalRenderNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_endConditionalRenderNV::execute);
}
void subpixelPrecisionBiasNV(
	GLuint xbits,
	GLuint ybits)
{
	RenderCommand_subpixelPrecisionBiasNV* __restrict packet = (RenderCommand_subpixelPrecisionBiasNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_subpixelPrecisionBiasNV::execute);
	packet->xbits = xbits;
	packet->ybits = ybits;
}
void conservativeRasterParameterfNV(
	GLenum pname,
	GLfloat value)
{
	RenderCommand_conservativeRasterParameterfNV* __restrict packet = (RenderCommand_conservativeRasterParameterfNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_conservativeRasterParameterfNV::execute);
	packet->pname = pname;
	packet->value = value;
}
void conservativeRasterParameteriNV(
	GLenum pname,
	GLint param)
{
	RenderCommand_conservativeRasterParameteriNV* __restrict packet = (RenderCommand_conservativeRasterParameteriNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_conservativeRasterParameteriNV::execute);
	packet->pname = pname;
	packet->param = param;
}
void drawVkImageNV(
	GLuint64 vkImage,
	GLuint sampler,
	GLfloat x0,
	GLfloat y0,
	GLfloat x1,
	GLfloat y1,
	GLfloat z,
	GLfloat s0,
	GLfloat t0,
	GLfloat s1,
	GLfloat t1)
{
	RenderCommand_drawVkImageNV* __restrict packet = (RenderCommand_drawVkImageNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawVkImageNV::execute);
	packet->vkImage = vkImage;
	packet->sampler = sampler;
	packet->x0 = x0;
	packet->y0 = y0;
	packet->x1 = x1;
	packet->y1 = y1;
	packet->z = z;
	packet->s0 = s0;
	packet->t0 = t0;
	packet->s1 = s1;
	packet->t1 = t1;
}
GLVULKANPROCNV getVkProcAddrNV(
	const GLchar *name)
{
	RenderCommand_getVkProcAddrNV* __restrict packet = (RenderCommand_getVkProcAddrNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getVkProcAddrNV::execute);
	packet->name = name;
}
void waitVkSemaphoreNV(
	GLuint64 vkSemaphore)
{
	RenderCommand_waitVkSemaphoreNV* __restrict packet = (RenderCommand_waitVkSemaphoreNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_waitVkSemaphoreNV::execute);
	packet->vkSemaphore = vkSemaphore;
}
void signalVkSemaphoreNV(
	GLuint64 vkSemaphore)
{
	RenderCommand_signalVkSemaphoreNV* __restrict packet = (RenderCommand_signalVkSemaphoreNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_signalVkSemaphoreNV::execute);
	packet->vkSemaphore = vkSemaphore;
}
void signalVkFenceNV(
	GLuint64 vkFence)
{
	RenderCommand_signalVkFenceNV* __restrict packet = (RenderCommand_signalVkFenceNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_signalVkFenceNV::execute);
	packet->vkFence = vkFence;
}
void fragmentCoverageColorNV(
	GLuint color)
{
	RenderCommand_fragmentCoverageColorNV* __restrict packet = (RenderCommand_fragmentCoverageColorNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_fragmentCoverageColorNV::execute);
	packet->color = color;
}
void coverageModulationTableNV(
	GLsizei n,
	const GLfloat *v)
{
	RenderCommand_coverageModulationTableNV* __restrict packet = (RenderCommand_coverageModulationTableNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_coverageModulationTableNV::execute);
	packet->n = n;
	packet->v = v;
}
void getCoverageModulationTableNV(
	GLsizei bufsize,
	GLfloat *v)
{
	RenderCommand_getCoverageModulationTableNV* __restrict packet = (RenderCommand_getCoverageModulationTableNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getCoverageModulationTableNV::execute);
	packet->bufsize = bufsize;
	packet->v = v;
}
void coverageModulationNV(
	GLenum components)
{
	RenderCommand_coverageModulationNV* __restrict packet = (RenderCommand_coverageModulationNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_coverageModulationNV::execute);
	packet->components = components;
}
void renderbufferStorageMultisampleCoverageNV(
	GLenum target,
	GLsizei coverageSamples,
	GLsizei colorSamples,
	GLenum internalformat,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_renderbufferStorageMultisampleCoverageNV* __restrict packet = (RenderCommand_renderbufferStorageMultisampleCoverageNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_renderbufferStorageMultisampleCoverageNV::execute);
	packet->target = target;
	packet->coverageSamples = coverageSamples;
	packet->colorSamples = colorSamples;
	packet->internalformat = internalformat;
	packet->width = width;
	packet->height = height;
}
void uniform1i64NV(
	GLint location,
	GLint64EXT x)
{
	RenderCommand_uniform1i64NV* __restrict packet = (RenderCommand_uniform1i64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform1i64NV::execute);
	packet->location = location;
	packet->x = x;
}
void uniform2i64NV(
	GLint location,
	GLint64EXT x,
	GLint64EXT y)
{
	RenderCommand_uniform2i64NV* __restrict packet = (RenderCommand_uniform2i64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform2i64NV::execute);
	packet->location = location;
	packet->x = x;
	packet->y = y;
}
void uniform3i64NV(
	GLint location,
	GLint64EXT x,
	GLint64EXT y,
	GLint64EXT z)
{
	RenderCommand_uniform3i64NV* __restrict packet = (RenderCommand_uniform3i64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform3i64NV::execute);
	packet->location = location;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void uniform4i64NV(
	GLint location,
	GLint64EXT x,
	GLint64EXT y,
	GLint64EXT z,
	GLint64EXT w)
{
	RenderCommand_uniform4i64NV* __restrict packet = (RenderCommand_uniform4i64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform4i64NV::execute);
	packet->location = location;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void uniform1i64vNV(
	GLint location,
	GLsizei count,
	const GLint64EXT *value)
{
	RenderCommand_uniform1i64vNV* __restrict packet = (RenderCommand_uniform1i64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform1i64vNV::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform2i64vNV(
	GLint location,
	GLsizei count,
	const GLint64EXT *value)
{
	RenderCommand_uniform2i64vNV* __restrict packet = (RenderCommand_uniform2i64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform2i64vNV::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform3i64vNV(
	GLint location,
	GLsizei count,
	const GLint64EXT *value)
{
	RenderCommand_uniform3i64vNV* __restrict packet = (RenderCommand_uniform3i64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform3i64vNV::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform4i64vNV(
	GLint location,
	GLsizei count,
	const GLint64EXT *value)
{
	RenderCommand_uniform4i64vNV* __restrict packet = (RenderCommand_uniform4i64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform4i64vNV::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform1ui64NV(
	GLint location,
	GLuint64EXT x)
{
	RenderCommand_uniform1ui64NV* __restrict packet = (RenderCommand_uniform1ui64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform1ui64NV::execute);
	packet->location = location;
	packet->x = x;
}
void uniform2ui64NV(
	GLint location,
	GLuint64EXT x,
	GLuint64EXT y)
{
	RenderCommand_uniform2ui64NV* __restrict packet = (RenderCommand_uniform2ui64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform2ui64NV::execute);
	packet->location = location;
	packet->x = x;
	packet->y = y;
}
void uniform3ui64NV(
	GLint location,
	GLuint64EXT x,
	GLuint64EXT y,
	GLuint64EXT z)
{
	RenderCommand_uniform3ui64NV* __restrict packet = (RenderCommand_uniform3ui64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform3ui64NV::execute);
	packet->location = location;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void uniform4ui64NV(
	GLint location,
	GLuint64EXT x,
	GLuint64EXT y,
	GLuint64EXT z,
	GLuint64EXT w)
{
	RenderCommand_uniform4ui64NV* __restrict packet = (RenderCommand_uniform4ui64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform4ui64NV::execute);
	packet->location = location;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void uniform1ui64vNV(
	GLint location,
	GLsizei count,
	const GLuint64EXT *value)
{
	RenderCommand_uniform1ui64vNV* __restrict packet = (RenderCommand_uniform1ui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform1ui64vNV::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform2ui64vNV(
	GLint location,
	GLsizei count,
	const GLuint64EXT *value)
{
	RenderCommand_uniform2ui64vNV* __restrict packet = (RenderCommand_uniform2ui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform2ui64vNV::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform3ui64vNV(
	GLint location,
	GLsizei count,
	const GLuint64EXT *value)
{
	RenderCommand_uniform3ui64vNV* __restrict packet = (RenderCommand_uniform3ui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform3ui64vNV::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void uniform4ui64vNV(
	GLint location,
	GLsizei count,
	const GLuint64EXT *value)
{
	RenderCommand_uniform4ui64vNV* __restrict packet = (RenderCommand_uniform4ui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniform4ui64vNV::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void getUniformi64vNV(
	GLuint program,
	GLint location,
	GLint64EXT *params)
{
	RenderCommand_getUniformi64vNV* __restrict packet = (RenderCommand_getUniformi64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getUniformi64vNV::execute);
	packet->program = program;
	packet->location = location;
	packet->params = params;
}
void programUniform1i64NV(
	GLuint program,
	GLint location,
	GLint64EXT x)
{
	RenderCommand_programUniform1i64NV* __restrict packet = (RenderCommand_programUniform1i64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1i64NV::execute);
	packet->program = program;
	packet->location = location;
	packet->x = x;
}
void programUniform2i64NV(
	GLuint program,
	GLint location,
	GLint64EXT x,
	GLint64EXT y)
{
	RenderCommand_programUniform2i64NV* __restrict packet = (RenderCommand_programUniform2i64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2i64NV::execute);
	packet->program = program;
	packet->location = location;
	packet->x = x;
	packet->y = y;
}
void programUniform3i64NV(
	GLuint program,
	GLint location,
	GLint64EXT x,
	GLint64EXT y,
	GLint64EXT z)
{
	RenderCommand_programUniform3i64NV* __restrict packet = (RenderCommand_programUniform3i64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3i64NV::execute);
	packet->program = program;
	packet->location = location;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void programUniform4i64NV(
	GLuint program,
	GLint location,
	GLint64EXT x,
	GLint64EXT y,
	GLint64EXT z,
	GLint64EXT w)
{
	RenderCommand_programUniform4i64NV* __restrict packet = (RenderCommand_programUniform4i64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4i64NV::execute);
	packet->program = program;
	packet->location = location;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void programUniform1i64vNV(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLint64EXT *value)
{
	RenderCommand_programUniform1i64vNV* __restrict packet = (RenderCommand_programUniform1i64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1i64vNV::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform2i64vNV(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLint64EXT *value)
{
	RenderCommand_programUniform2i64vNV* __restrict packet = (RenderCommand_programUniform2i64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2i64vNV::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform3i64vNV(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLint64EXT *value)
{
	RenderCommand_programUniform3i64vNV* __restrict packet = (RenderCommand_programUniform3i64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3i64vNV::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform4i64vNV(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLint64EXT *value)
{
	RenderCommand_programUniform4i64vNV* __restrict packet = (RenderCommand_programUniform4i64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4i64vNV::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform1ui64NV(
	GLuint program,
	GLint location,
	GLuint64EXT x)
{
	RenderCommand_programUniform1ui64NV* __restrict packet = (RenderCommand_programUniform1ui64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1ui64NV::execute);
	packet->program = program;
	packet->location = location;
	packet->x = x;
}
void programUniform2ui64NV(
	GLuint program,
	GLint location,
	GLuint64EXT x,
	GLuint64EXT y)
{
	RenderCommand_programUniform2ui64NV* __restrict packet = (RenderCommand_programUniform2ui64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2ui64NV::execute);
	packet->program = program;
	packet->location = location;
	packet->x = x;
	packet->y = y;
}
void programUniform3ui64NV(
	GLuint program,
	GLint location,
	GLuint64EXT x,
	GLuint64EXT y,
	GLuint64EXT z)
{
	RenderCommand_programUniform3ui64NV* __restrict packet = (RenderCommand_programUniform3ui64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3ui64NV::execute);
	packet->program = program;
	packet->location = location;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void programUniform4ui64NV(
	GLuint program,
	GLint location,
	GLuint64EXT x,
	GLuint64EXT y,
	GLuint64EXT z,
	GLuint64EXT w)
{
	RenderCommand_programUniform4ui64NV* __restrict packet = (RenderCommand_programUniform4ui64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4ui64NV::execute);
	packet->program = program;
	packet->location = location;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void programUniform1ui64vNV(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLuint64EXT *value)
{
	RenderCommand_programUniform1ui64vNV* __restrict packet = (RenderCommand_programUniform1ui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform1ui64vNV::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform2ui64vNV(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLuint64EXT *value)
{
	RenderCommand_programUniform2ui64vNV* __restrict packet = (RenderCommand_programUniform2ui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform2ui64vNV::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform3ui64vNV(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLuint64EXT *value)
{
	RenderCommand_programUniform3ui64vNV* __restrict packet = (RenderCommand_programUniform3ui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform3ui64vNV::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void programUniform4ui64vNV(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLuint64EXT *value)
{
	RenderCommand_programUniform4ui64vNV* __restrict packet = (RenderCommand_programUniform4ui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniform4ui64vNV::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void getInternalformatSampleivNV(
	GLenum target,
	GLenum internalformat,
	GLsizei samples,
	GLenum pname,
	GLsizei bufSize,
	GLint *params)
{
	RenderCommand_getInternalformatSampleivNV* __restrict packet = (RenderCommand_getInternalformatSampleivNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getInternalformatSampleivNV::execute);
	packet->target = target;
	packet->internalformat = internalformat;
	packet->samples = samples;
	packet->pname = pname;
	packet->bufSize = bufSize;
	packet->params = params;
}
void getMemoryObjectDetachedResourcesuivNV(
	GLuint memory,
	GLenum pname,
	GLint first,
	GLsizei count,
	GLuint *params)
{
	RenderCommand_getMemoryObjectDetachedResourcesuivNV* __restrict packet = (RenderCommand_getMemoryObjectDetachedResourcesuivNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getMemoryObjectDetachedResourcesuivNV::execute);
	packet->memory = memory;
	packet->pname = pname;
	packet->first = first;
	packet->count = count;
	packet->params = params;
}
void resetMemoryObjectParameterNV(
	GLuint memory,
	GLenum pname)
{
	RenderCommand_resetMemoryObjectParameterNV* __restrict packet = (RenderCommand_resetMemoryObjectParameterNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_resetMemoryObjectParameterNV::execute);
	packet->memory = memory;
	packet->pname = pname;
}
void texAttachMemoryNV(
	GLenum target,
	GLuint memory,
	GLuint64 offset)
{
	RenderCommand_texAttachMemoryNV* __restrict packet = (RenderCommand_texAttachMemoryNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texAttachMemoryNV::execute);
	packet->target = target;
	packet->memory = memory;
	packet->offset = offset;
}
void bufferAttachMemoryNV(
	GLenum target,
	GLuint memory,
	GLuint64 offset)
{
	RenderCommand_bufferAttachMemoryNV* __restrict packet = (RenderCommand_bufferAttachMemoryNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bufferAttachMemoryNV::execute);
	packet->target = target;
	packet->memory = memory;
	packet->offset = offset;
}
void textureAttachMemoryNV(
	GLuint texture,
	GLuint memory,
	GLuint64 offset)
{
	RenderCommand_textureAttachMemoryNV* __restrict packet = (RenderCommand_textureAttachMemoryNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureAttachMemoryNV::execute);
	packet->texture = texture;
	packet->memory = memory;
	packet->offset = offset;
}
void namedBufferAttachMemoryNV(
	GLuint buffer,
	GLuint memory,
	GLuint64 offset)
{
	RenderCommand_namedBufferAttachMemoryNV* __restrict packet = (RenderCommand_namedBufferAttachMemoryNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedBufferAttachMemoryNV::execute);
	packet->buffer = buffer;
	packet->memory = memory;
	packet->offset = offset;
}
void drawMeshTasksNV(
	GLuint first,
	GLuint count)
{
	RenderCommand_drawMeshTasksNV* __restrict packet = (RenderCommand_drawMeshTasksNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawMeshTasksNV::execute);
	packet->first = first;
	packet->count = count;
}
void drawMeshTasksIndirectNV(
	GLintptr indirect)
{
	RenderCommand_drawMeshTasksIndirectNV* __restrict packet = (RenderCommand_drawMeshTasksIndirectNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_drawMeshTasksIndirectNV::execute);
	packet->indirect = indirect;
}
void multiDrawMeshTasksIndirectNV(
	GLintptr indirect,
	GLsizei drawcount,
	GLsizei stride)
{
	RenderCommand_multiDrawMeshTasksIndirectNV* __restrict packet = (RenderCommand_multiDrawMeshTasksIndirectNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiDrawMeshTasksIndirectNV::execute);
	packet->indirect = indirect;
	packet->drawcount = drawcount;
	packet->stride = stride;
}
void multiDrawMeshTasksIndirectCountNV(
	GLintptr indirect,
	GLintptr drawcount,
	GLsizei maxdrawcount,
	GLsizei stride)
{
	RenderCommand_multiDrawMeshTasksIndirectCountNV* __restrict packet = (RenderCommand_multiDrawMeshTasksIndirectCountNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_multiDrawMeshTasksIndirectCountNV::execute);
	packet->indirect = indirect;
	packet->drawcount = drawcount;
	packet->maxdrawcount = maxdrawcount;
	packet->stride = stride;
}
GLuint genPathsNV(
	GLsizei range)
{
	RenderCommand_genPathsNV* __restrict packet = (RenderCommand_genPathsNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_genPathsNV::execute);
	packet->range = range;
}
void deletePathsNV(
	GLuint path,
	GLsizei range)
{
	RenderCommand_deletePathsNV* __restrict packet = (RenderCommand_deletePathsNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_deletePathsNV::execute);
	packet->path = path;
	packet->range = range;
}
GLboolean isPathNV(
	GLuint path)
{
	RenderCommand_isPathNV* __restrict packet = (RenderCommand_isPathNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isPathNV::execute);
	packet->path = path;
}
void pathCommandsNV(
	GLuint path,
	GLsizei numCommands,
	const GLubyte *commands,
	GLsizei numCoords,
	GLenum coordType,
	const void *coords)
{
	RenderCommand_pathCommandsNV* __restrict packet = (RenderCommand_pathCommandsNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pathCommandsNV::execute);
	packet->path = path;
	packet->numCommands = numCommands;
	packet->commands = commands;
	packet->numCoords = numCoords;
	packet->coordType = coordType;
	packet->coords = coords;
}
void pathCoordsNV(
	GLuint path,
	GLsizei numCoords,
	GLenum coordType,
	const void *coords)
{
	RenderCommand_pathCoordsNV* __restrict packet = (RenderCommand_pathCoordsNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pathCoordsNV::execute);
	packet->path = path;
	packet->numCoords = numCoords;
	packet->coordType = coordType;
	packet->coords = coords;
}
void pathSubCommandsNV(
	GLuint path,
	GLsizei commandStart,
	GLsizei commandsToDelete,
	GLsizei numCommands,
	const GLubyte *commands,
	GLsizei numCoords,
	GLenum coordType,
	const void *coords)
{
	RenderCommand_pathSubCommandsNV* __restrict packet = (RenderCommand_pathSubCommandsNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pathSubCommandsNV::execute);
	packet->path = path;
	packet->commandStart = commandStart;
	packet->commandsToDelete = commandsToDelete;
	packet->numCommands = numCommands;
	packet->commands = commands;
	packet->numCoords = numCoords;
	packet->coordType = coordType;
	packet->coords = coords;
}
void pathSubCoordsNV(
	GLuint path,
	GLsizei coordStart,
	GLsizei numCoords,
	GLenum coordType,
	const void *coords)
{
	RenderCommand_pathSubCoordsNV* __restrict packet = (RenderCommand_pathSubCoordsNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pathSubCoordsNV::execute);
	packet->path = path;
	packet->coordStart = coordStart;
	packet->numCoords = numCoords;
	packet->coordType = coordType;
	packet->coords = coords;
}
void pathStringNV(
	GLuint path,
	GLenum format,
	GLsizei length,
	const void *pathString)
{
	RenderCommand_pathStringNV* __restrict packet = (RenderCommand_pathStringNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pathStringNV::execute);
	packet->path = path;
	packet->format = format;
	packet->length = length;
	packet->pathString = pathString;
}
void pathGlyphsNV(
	GLuint firstPathName,
	GLenum fontTarget,
	const void *fontName,
	GLbitfield fontStyle,
	GLsizei numGlyphs,
	GLenum type,
	const void *charcodes,
	GLenum handleMissingGlyphs,
	GLuint pathParameterTemplate,
	GLfloat emScale)
{
	RenderCommand_pathGlyphsNV* __restrict packet = (RenderCommand_pathGlyphsNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pathGlyphsNV::execute);
	packet->firstPathName = firstPathName;
	packet->fontTarget = fontTarget;
	packet->fontName = fontName;
	packet->fontStyle = fontStyle;
	packet->numGlyphs = numGlyphs;
	packet->type = type;
	packet->charcodes = charcodes;
	packet->handleMissingGlyphs = handleMissingGlyphs;
	packet->pathParameterTemplate = pathParameterTemplate;
	packet->emScale = emScale;
}
void pathGlyphRangeNV(
	GLuint firstPathName,
	GLenum fontTarget,
	const void *fontName,
	GLbitfield fontStyle,
	GLuint firstGlyph,
	GLsizei numGlyphs,
	GLenum handleMissingGlyphs,
	GLuint pathParameterTemplate,
	GLfloat emScale)
{
	RenderCommand_pathGlyphRangeNV* __restrict packet = (RenderCommand_pathGlyphRangeNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pathGlyphRangeNV::execute);
	packet->firstPathName = firstPathName;
	packet->fontTarget = fontTarget;
	packet->fontName = fontName;
	packet->fontStyle = fontStyle;
	packet->firstGlyph = firstGlyph;
	packet->numGlyphs = numGlyphs;
	packet->handleMissingGlyphs = handleMissingGlyphs;
	packet->pathParameterTemplate = pathParameterTemplate;
	packet->emScale = emScale;
}
void weightPathsNV(
	GLuint resultPath,
	GLsizei numPaths,
	const GLuint *paths,
	const GLfloat *weights)
{
	RenderCommand_weightPathsNV* __restrict packet = (RenderCommand_weightPathsNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_weightPathsNV::execute);
	packet->resultPath = resultPath;
	packet->numPaths = numPaths;
	packet->paths = paths;
	packet->weights = weights;
}
void copyPathNV(
	GLuint resultPath,
	GLuint srcPath)
{
	RenderCommand_copyPathNV* __restrict packet = (RenderCommand_copyPathNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_copyPathNV::execute);
	packet->resultPath = resultPath;
	packet->srcPath = srcPath;
}
void interpolatePathsNV(
	GLuint resultPath,
	GLuint pathA,
	GLuint pathB,
	GLfloat weight)
{
	RenderCommand_interpolatePathsNV* __restrict packet = (RenderCommand_interpolatePathsNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_interpolatePathsNV::execute);
	packet->resultPath = resultPath;
	packet->pathA = pathA;
	packet->pathB = pathB;
	packet->weight = weight;
}
void transformPathNV(
	GLuint resultPath,
	GLuint srcPath,
	GLenum transformType,
	const GLfloat *transformValues)
{
	RenderCommand_transformPathNV* __restrict packet = (RenderCommand_transformPathNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_transformPathNV::execute);
	packet->resultPath = resultPath;
	packet->srcPath = srcPath;
	packet->transformType = transformType;
	packet->transformValues = transformValues;
}
void pathParameterivNV(
	GLuint path,
	GLenum pname,
	const GLint *value)
{
	RenderCommand_pathParameterivNV* __restrict packet = (RenderCommand_pathParameterivNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pathParameterivNV::execute);
	packet->path = path;
	packet->pname = pname;
	packet->value = value;
}
void pathParameteriNV(
	GLuint path,
	GLenum pname,
	GLint value)
{
	RenderCommand_pathParameteriNV* __restrict packet = (RenderCommand_pathParameteriNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pathParameteriNV::execute);
	packet->path = path;
	packet->pname = pname;
	packet->value = value;
}
void pathParameterfvNV(
	GLuint path,
	GLenum pname,
	const GLfloat *value)
{
	RenderCommand_pathParameterfvNV* __restrict packet = (RenderCommand_pathParameterfvNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pathParameterfvNV::execute);
	packet->path = path;
	packet->pname = pname;
	packet->value = value;
}
void pathParameterfNV(
	GLuint path,
	GLenum pname,
	GLfloat value)
{
	RenderCommand_pathParameterfNV* __restrict packet = (RenderCommand_pathParameterfNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pathParameterfNV::execute);
	packet->path = path;
	packet->pname = pname;
	packet->value = value;
}
void pathDashArrayNV(
	GLuint path,
	GLsizei dashCount,
	const GLfloat *dashArray)
{
	RenderCommand_pathDashArrayNV* __restrict packet = (RenderCommand_pathDashArrayNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pathDashArrayNV::execute);
	packet->path = path;
	packet->dashCount = dashCount;
	packet->dashArray = dashArray;
}
void pathStencilFuncNV(
	GLenum func,
	GLint ref,
	GLuint mask)
{
	RenderCommand_pathStencilFuncNV* __restrict packet = (RenderCommand_pathStencilFuncNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pathStencilFuncNV::execute);
	packet->func = func;
	packet->ref = ref;
	packet->mask = mask;
}
void pathStencilDepthOffsetNV(
	GLfloat factor,
	GLfloat units)
{
	RenderCommand_pathStencilDepthOffsetNV* __restrict packet = (RenderCommand_pathStencilDepthOffsetNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pathStencilDepthOffsetNV::execute);
	packet->factor = factor;
	packet->units = units;
}
void stencilFillPathNV(
	GLuint path,
	GLenum fillMode,
	GLuint mask)
{
	RenderCommand_stencilFillPathNV* __restrict packet = (RenderCommand_stencilFillPathNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_stencilFillPathNV::execute);
	packet->path = path;
	packet->fillMode = fillMode;
	packet->mask = mask;
}
void stencilStrokePathNV(
	GLuint path,
	GLint reference,
	GLuint mask)
{
	RenderCommand_stencilStrokePathNV* __restrict packet = (RenderCommand_stencilStrokePathNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_stencilStrokePathNV::execute);
	packet->path = path;
	packet->reference = reference;
	packet->mask = mask;
}
void stencilFillPathInstancedNV(
	GLsizei numPaths,
	GLenum pathNameType,
	const void *paths,
	GLuint pathBase,
	GLenum fillMode,
	GLuint mask,
	GLenum transformType,
	const GLfloat *transformValues)
{
	RenderCommand_stencilFillPathInstancedNV* __restrict packet = (RenderCommand_stencilFillPathInstancedNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_stencilFillPathInstancedNV::execute);
	packet->numPaths = numPaths;
	packet->pathNameType = pathNameType;
	packet->paths = paths;
	packet->pathBase = pathBase;
	packet->fillMode = fillMode;
	packet->mask = mask;
	packet->transformType = transformType;
	packet->transformValues = transformValues;
}
void stencilStrokePathInstancedNV(
	GLsizei numPaths,
	GLenum pathNameType,
	const void *paths,
	GLuint pathBase,
	GLint reference,
	GLuint mask,
	GLenum transformType,
	const GLfloat *transformValues)
{
	RenderCommand_stencilStrokePathInstancedNV* __restrict packet = (RenderCommand_stencilStrokePathInstancedNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_stencilStrokePathInstancedNV::execute);
	packet->numPaths = numPaths;
	packet->pathNameType = pathNameType;
	packet->paths = paths;
	packet->pathBase = pathBase;
	packet->reference = reference;
	packet->mask = mask;
	packet->transformType = transformType;
	packet->transformValues = transformValues;
}
void pathCoverDepthFuncNV(
	GLenum func)
{
	RenderCommand_pathCoverDepthFuncNV* __restrict packet = (RenderCommand_pathCoverDepthFuncNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pathCoverDepthFuncNV::execute);
	packet->func = func;
}
void coverFillPathNV(
	GLuint path,
	GLenum coverMode)
{
	RenderCommand_coverFillPathNV* __restrict packet = (RenderCommand_coverFillPathNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_coverFillPathNV::execute);
	packet->path = path;
	packet->coverMode = coverMode;
}
void coverStrokePathNV(
	GLuint path,
	GLenum coverMode)
{
	RenderCommand_coverStrokePathNV* __restrict packet = (RenderCommand_coverStrokePathNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_coverStrokePathNV::execute);
	packet->path = path;
	packet->coverMode = coverMode;
}
void coverFillPathInstancedNV(
	GLsizei numPaths,
	GLenum pathNameType,
	const void *paths,
	GLuint pathBase,
	GLenum coverMode,
	GLenum transformType,
	const GLfloat *transformValues)
{
	RenderCommand_coverFillPathInstancedNV* __restrict packet = (RenderCommand_coverFillPathInstancedNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_coverFillPathInstancedNV::execute);
	packet->numPaths = numPaths;
	packet->pathNameType = pathNameType;
	packet->paths = paths;
	packet->pathBase = pathBase;
	packet->coverMode = coverMode;
	packet->transformType = transformType;
	packet->transformValues = transformValues;
}
void coverStrokePathInstancedNV(
	GLsizei numPaths,
	GLenum pathNameType,
	const void *paths,
	GLuint pathBase,
	GLenum coverMode,
	GLenum transformType,
	const GLfloat *transformValues)
{
	RenderCommand_coverStrokePathInstancedNV* __restrict packet = (RenderCommand_coverStrokePathInstancedNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_coverStrokePathInstancedNV::execute);
	packet->numPaths = numPaths;
	packet->pathNameType = pathNameType;
	packet->paths = paths;
	packet->pathBase = pathBase;
	packet->coverMode = coverMode;
	packet->transformType = transformType;
	packet->transformValues = transformValues;
}
void getPathParameterivNV(
	GLuint path,
	GLenum pname,
	GLint *value)
{
	RenderCommand_getPathParameterivNV* __restrict packet = (RenderCommand_getPathParameterivNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPathParameterivNV::execute);
	packet->path = path;
	packet->pname = pname;
	packet->value = value;
}
void getPathParameterfvNV(
	GLuint path,
	GLenum pname,
	GLfloat *value)
{
	RenderCommand_getPathParameterfvNV* __restrict packet = (RenderCommand_getPathParameterfvNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPathParameterfvNV::execute);
	packet->path = path;
	packet->pname = pname;
	packet->value = value;
}
void getPathCommandsNV(
	GLuint path,
	GLubyte *commands)
{
	RenderCommand_getPathCommandsNV* __restrict packet = (RenderCommand_getPathCommandsNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPathCommandsNV::execute);
	packet->path = path;
	packet->commands = commands;
}
void getPathCoordsNV(
	GLuint path,
	GLfloat *coords)
{
	RenderCommand_getPathCoordsNV* __restrict packet = (RenderCommand_getPathCoordsNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPathCoordsNV::execute);
	packet->path = path;
	packet->coords = coords;
}
void getPathDashArrayNV(
	GLuint path,
	GLfloat *dashArray)
{
	RenderCommand_getPathDashArrayNV* __restrict packet = (RenderCommand_getPathDashArrayNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPathDashArrayNV::execute);
	packet->path = path;
	packet->dashArray = dashArray;
}
void getPathMetricsNV(
	GLbitfield metricQueryMask,
	GLsizei numPaths,
	GLenum pathNameType,
	const void *paths,
	GLuint pathBase,
	GLsizei stride,
	GLfloat *metrics)
{
	RenderCommand_getPathMetricsNV* __restrict packet = (RenderCommand_getPathMetricsNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPathMetricsNV::execute);
	packet->metricQueryMask = metricQueryMask;
	packet->numPaths = numPaths;
	packet->pathNameType = pathNameType;
	packet->paths = paths;
	packet->pathBase = pathBase;
	packet->stride = stride;
	packet->metrics = metrics;
}
void getPathMetricRangeNV(
	GLbitfield metricQueryMask,
	GLuint firstPathName,
	GLsizei numPaths,
	GLsizei stride,
	GLfloat *metrics)
{
	RenderCommand_getPathMetricRangeNV* __restrict packet = (RenderCommand_getPathMetricRangeNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPathMetricRangeNV::execute);
	packet->metricQueryMask = metricQueryMask;
	packet->firstPathName = firstPathName;
	packet->numPaths = numPaths;
	packet->stride = stride;
	packet->metrics = metrics;
}
void getPathSpacingNV(
	GLenum pathListMode,
	GLsizei numPaths,
	GLenum pathNameType,
	const void *paths,
	GLuint pathBase,
	GLfloat advanceScale,
	GLfloat kerningScale,
	GLenum transformType,
	GLfloat *returnedSpacing)
{
	RenderCommand_getPathSpacingNV* __restrict packet = (RenderCommand_getPathSpacingNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPathSpacingNV::execute);
	packet->pathListMode = pathListMode;
	packet->numPaths = numPaths;
	packet->pathNameType = pathNameType;
	packet->paths = paths;
	packet->pathBase = pathBase;
	packet->advanceScale = advanceScale;
	packet->kerningScale = kerningScale;
	packet->transformType = transformType;
	packet->returnedSpacing = returnedSpacing;
}
GLboolean isPointInFillPathNV(
	GLuint path,
	GLuint mask,
	GLfloat x,
	GLfloat y)
{
	RenderCommand_isPointInFillPathNV* __restrict packet = (RenderCommand_isPointInFillPathNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isPointInFillPathNV::execute);
	packet->path = path;
	packet->mask = mask;
	packet->x = x;
	packet->y = y;
}
GLboolean isPointInStrokePathNV(
	GLuint path,
	GLfloat x,
	GLfloat y)
{
	RenderCommand_isPointInStrokePathNV* __restrict packet = (RenderCommand_isPointInStrokePathNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isPointInStrokePathNV::execute);
	packet->path = path;
	packet->x = x;
	packet->y = y;
}
GLfloat getPathLengthNV(
	GLuint path,
	GLsizei startSegment,
	GLsizei numSegments)
{
	RenderCommand_getPathLengthNV* __restrict packet = (RenderCommand_getPathLengthNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getPathLengthNV::execute);
	packet->path = path;
	packet->startSegment = startSegment;
	packet->numSegments = numSegments;
}
GLboolean pointAlongPathNV(
	GLuint path,
	GLsizei startSegment,
	GLsizei numSegments,
	GLfloat distance,
	GLfloat *x,
	GLfloat *y,
	GLfloat *tangentX,
	GLfloat *tangentY)
{
	RenderCommand_pointAlongPathNV* __restrict packet = (RenderCommand_pointAlongPathNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pointAlongPathNV::execute);
	packet->path = path;
	packet->startSegment = startSegment;
	packet->numSegments = numSegments;
	packet->distance = distance;
	packet->x = x;
	packet->y = y;
	packet->tangentX = tangentX;
	packet->tangentY = tangentY;
}
void matrixLoad3x2fNV(
	GLenum matrixMode,
	const GLfloat *m)
{
	RenderCommand_matrixLoad3x2fNV* __restrict packet = (RenderCommand_matrixLoad3x2fNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixLoad3x2fNV::execute);
	packet->matrixMode = matrixMode;
	packet->m = m;
}
void matrixLoad3x3fNV(
	GLenum matrixMode,
	const GLfloat *m)
{
	RenderCommand_matrixLoad3x3fNV* __restrict packet = (RenderCommand_matrixLoad3x3fNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixLoad3x3fNV::execute);
	packet->matrixMode = matrixMode;
	packet->m = m;
}
void matrixLoadTranspose3x3fNV(
	GLenum matrixMode,
	const GLfloat *m)
{
	RenderCommand_matrixLoadTranspose3x3fNV* __restrict packet = (RenderCommand_matrixLoadTranspose3x3fNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixLoadTranspose3x3fNV::execute);
	packet->matrixMode = matrixMode;
	packet->m = m;
}
void matrixMult3x2fNV(
	GLenum matrixMode,
	const GLfloat *m)
{
	RenderCommand_matrixMult3x2fNV* __restrict packet = (RenderCommand_matrixMult3x2fNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixMult3x2fNV::execute);
	packet->matrixMode = matrixMode;
	packet->m = m;
}
void matrixMult3x3fNV(
	GLenum matrixMode,
	const GLfloat *m)
{
	RenderCommand_matrixMult3x3fNV* __restrict packet = (RenderCommand_matrixMult3x3fNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixMult3x3fNV::execute);
	packet->matrixMode = matrixMode;
	packet->m = m;
}
void matrixMultTranspose3x3fNV(
	GLenum matrixMode,
	const GLfloat *m)
{
	RenderCommand_matrixMultTranspose3x3fNV* __restrict packet = (RenderCommand_matrixMultTranspose3x3fNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_matrixMultTranspose3x3fNV::execute);
	packet->matrixMode = matrixMode;
	packet->m = m;
}
void stencilThenCoverFillPathNV(
	GLuint path,
	GLenum fillMode,
	GLuint mask,
	GLenum coverMode)
{
	RenderCommand_stencilThenCoverFillPathNV* __restrict packet = (RenderCommand_stencilThenCoverFillPathNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_stencilThenCoverFillPathNV::execute);
	packet->path = path;
	packet->fillMode = fillMode;
	packet->mask = mask;
	packet->coverMode = coverMode;
}
void stencilThenCoverStrokePathNV(
	GLuint path,
	GLint reference,
	GLuint mask,
	GLenum coverMode)
{
	RenderCommand_stencilThenCoverStrokePathNV* __restrict packet = (RenderCommand_stencilThenCoverStrokePathNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_stencilThenCoverStrokePathNV::execute);
	packet->path = path;
	packet->reference = reference;
	packet->mask = mask;
	packet->coverMode = coverMode;
}
void stencilThenCoverFillPathInstancedNV(
	GLsizei numPaths,
	GLenum pathNameType,
	const void *paths,
	GLuint pathBase,
	GLenum fillMode,
	GLuint mask,
	GLenum coverMode,
	GLenum transformType,
	const GLfloat *transformValues)
{
	RenderCommand_stencilThenCoverFillPathInstancedNV* __restrict packet = (RenderCommand_stencilThenCoverFillPathInstancedNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_stencilThenCoverFillPathInstancedNV::execute);
	packet->numPaths = numPaths;
	packet->pathNameType = pathNameType;
	packet->paths = paths;
	packet->pathBase = pathBase;
	packet->fillMode = fillMode;
	packet->mask = mask;
	packet->coverMode = coverMode;
	packet->transformType = transformType;
	packet->transformValues = transformValues;
}
void stencilThenCoverStrokePathInstancedNV(
	GLsizei numPaths,
	GLenum pathNameType,
	const void *paths,
	GLuint pathBase,
	GLint reference,
	GLuint mask,
	GLenum coverMode,
	GLenum transformType,
	const GLfloat *transformValues)
{
	RenderCommand_stencilThenCoverStrokePathInstancedNV* __restrict packet = (RenderCommand_stencilThenCoverStrokePathInstancedNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_stencilThenCoverStrokePathInstancedNV::execute);
	packet->numPaths = numPaths;
	packet->pathNameType = pathNameType;
	packet->paths = paths;
	packet->pathBase = pathBase;
	packet->reference = reference;
	packet->mask = mask;
	packet->coverMode = coverMode;
	packet->transformType = transformType;
	packet->transformValues = transformValues;
}
GLenum pathGlyphIndexRangeNV(
	GLenum fontTarget,
	const void *fontName,
	GLbitfield fontStyle,
	GLuint pathParameterTemplate,
	GLfloat emScale,
	GLuint baseAndCount[2])
{
	RenderCommand_pathGlyphIndexRangeNV* __restrict packet = (RenderCommand_pathGlyphIndexRangeNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pathGlyphIndexRangeNV::execute);
	packet->fontTarget = fontTarget;
	packet->fontName = fontName;
	packet->fontStyle = fontStyle;
	packet->pathParameterTemplate = pathParameterTemplate;
	packet->emScale = emScale;
	packet->baseAndCount[0] = baseAndCount[0];
	packet->baseAndCount[1] = baseAndCount[1];
}
GLenum pathGlyphIndexArrayNV(
	GLuint firstPathName,
	GLenum fontTarget,
	const void *fontName,
	GLbitfield fontStyle,
	GLuint firstGlyphIndex,
	GLsizei numGlyphs,
	GLuint pathParameterTemplate,
	GLfloat emScale)
{
	RenderCommand_pathGlyphIndexArrayNV* __restrict packet = (RenderCommand_pathGlyphIndexArrayNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pathGlyphIndexArrayNV::execute);
	packet->firstPathName = firstPathName;
	packet->fontTarget = fontTarget;
	packet->fontName = fontName;
	packet->fontStyle = fontStyle;
	packet->firstGlyphIndex = firstGlyphIndex;
	packet->numGlyphs = numGlyphs;
	packet->pathParameterTemplate = pathParameterTemplate;
	packet->emScale = emScale;
}
GLenum pathMemoryGlyphIndexArrayNV(
	GLuint firstPathName,
	GLenum fontTarget,
	GLsizeiptr fontSize,
	const void *fontData,
	GLsizei faceIndex,
	GLuint firstGlyphIndex,
	GLsizei numGlyphs,
	GLuint pathParameterTemplate,
	GLfloat emScale)
{
	RenderCommand_pathMemoryGlyphIndexArrayNV* __restrict packet = (RenderCommand_pathMemoryGlyphIndexArrayNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_pathMemoryGlyphIndexArrayNV::execute);
	packet->firstPathName = firstPathName;
	packet->fontTarget = fontTarget;
	packet->fontSize = fontSize;
	packet->fontData = fontData;
	packet->faceIndex = faceIndex;
	packet->firstGlyphIndex = firstGlyphIndex;
	packet->numGlyphs = numGlyphs;
	packet->pathParameterTemplate = pathParameterTemplate;
	packet->emScale = emScale;
}
void programPathFragmentInputGenNV(
	GLuint program,
	GLint location,
	GLenum genMode,
	GLint components,
	const GLfloat *coeffs)
{
	RenderCommand_programPathFragmentInputGenNV* __restrict packet = (RenderCommand_programPathFragmentInputGenNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programPathFragmentInputGenNV::execute);
	packet->program = program;
	packet->location = location;
	packet->genMode = genMode;
	packet->components = components;
	packet->coeffs = coeffs;
}
void getProgramResourcefvNV(
	GLuint program,
	GLenum programInterface,
	GLuint index,
	GLsizei propCount,
	const GLenum *props,
	GLsizei bufSize,
	GLsizei *length,
	GLfloat *params)
{
	RenderCommand_getProgramResourcefvNV* __restrict packet = (RenderCommand_getProgramResourcefvNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getProgramResourcefvNV::execute);
	packet->program = program;
	packet->programInterface = programInterface;
	packet->index = index;
	packet->propCount = propCount;
	packet->props = props;
	packet->bufSize = bufSize;
	packet->length = length;
	packet->params = params;
}
void framebufferSampleLocationsfvNV(
	GLenum target,
	GLuint start,
	GLsizei count,
	const GLfloat *v)
{
	RenderCommand_framebufferSampleLocationsfvNV* __restrict packet = (RenderCommand_framebufferSampleLocationsfvNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_framebufferSampleLocationsfvNV::execute);
	packet->target = target;
	packet->start = start;
	packet->count = count;
	packet->v = v;
}
void namedFramebufferSampleLocationsfvNV(
	GLuint framebuffer,
	GLuint start,
	GLsizei count,
	const GLfloat *v)
{
	RenderCommand_namedFramebufferSampleLocationsfvNV* __restrict packet = (RenderCommand_namedFramebufferSampleLocationsfvNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_namedFramebufferSampleLocationsfvNV::execute);
	packet->framebuffer = framebuffer;
	packet->start = start;
	packet->count = count;
	packet->v = v;
}
void resolveDepthValuesNV(
	void)
{
	RenderCommand_resolveDepthValuesNV* __restrict packet = (RenderCommand_resolveDepthValuesNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_resolveDepthValuesNV::execute);
}
void scissorExclusiveNV(
	GLint x,
	GLint y,
	GLsizei width,
	GLsizei height)
{
	RenderCommand_scissorExclusiveNV* __restrict packet = (RenderCommand_scissorExclusiveNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_scissorExclusiveNV::execute);
	packet->x = x;
	packet->y = y;
	packet->width = width;
	packet->height = height;
}
void scissorExclusiveArrayvNV(
	GLuint first,
	GLsizei count,
	const GLint *v)
{
	RenderCommand_scissorExclusiveArrayvNV* __restrict packet = (RenderCommand_scissorExclusiveArrayvNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_scissorExclusiveArrayvNV::execute);
	packet->first = first;
	packet->count = count;
	packet->v = v;
}
void makeBufferResidentNV(
	GLenum target,
	GLenum access)
{
	RenderCommand_makeBufferResidentNV* __restrict packet = (RenderCommand_makeBufferResidentNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_makeBufferResidentNV::execute);
	packet->target = target;
	packet->access = access;
}
void makeBufferNonResidentNV(
	GLenum target)
{
	RenderCommand_makeBufferNonResidentNV* __restrict packet = (RenderCommand_makeBufferNonResidentNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_makeBufferNonResidentNV::execute);
	packet->target = target;
}
GLboolean isBufferResidentNV(
	GLenum target)
{
	RenderCommand_isBufferResidentNV* __restrict packet = (RenderCommand_isBufferResidentNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isBufferResidentNV::execute);
	packet->target = target;
}
void makeNamedBufferResidentNV(
	GLuint buffer,
	GLenum access)
{
	RenderCommand_makeNamedBufferResidentNV* __restrict packet = (RenderCommand_makeNamedBufferResidentNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_makeNamedBufferResidentNV::execute);
	packet->buffer = buffer;
	packet->access = access;
}
void makeNamedBufferNonResidentNV(
	GLuint buffer)
{
	RenderCommand_makeNamedBufferNonResidentNV* __restrict packet = (RenderCommand_makeNamedBufferNonResidentNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_makeNamedBufferNonResidentNV::execute);
	packet->buffer = buffer;
}
GLboolean isNamedBufferResidentNV(
	GLuint buffer)
{
	RenderCommand_isNamedBufferResidentNV* __restrict packet = (RenderCommand_isNamedBufferResidentNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_isNamedBufferResidentNV::execute);
	packet->buffer = buffer;
}
void getBufferParameterui64vNV(
	GLenum target,
	GLenum pname,
	GLuint64EXT *params)
{
	RenderCommand_getBufferParameterui64vNV* __restrict packet = (RenderCommand_getBufferParameterui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getBufferParameterui64vNV::execute);
	packet->target = target;
	packet->pname = pname;
	packet->params = params;
}
void getNamedBufferParameterui64vNV(
	GLuint buffer,
	GLenum pname,
	GLuint64EXT *params)
{
	RenderCommand_getNamedBufferParameterui64vNV* __restrict packet = (RenderCommand_getNamedBufferParameterui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getNamedBufferParameterui64vNV::execute);
	packet->buffer = buffer;
	packet->pname = pname;
	packet->params = params;
}
void getIntegerui64vNV(
	GLenum value,
	GLuint64EXT *result)
{
	RenderCommand_getIntegerui64vNV* __restrict packet = (RenderCommand_getIntegerui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getIntegerui64vNV::execute);
	packet->value = value;
	packet->result = result;
}
void uniformui64NV(
	GLint location,
	GLuint64EXT value)
{
	RenderCommand_uniformui64NV* __restrict packet = (RenderCommand_uniformui64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformui64NV::execute);
	packet->location = location;
	packet->value = value;
}
void uniformui64vNV(
	GLint location,
	GLsizei count,
	const GLuint64EXT *value)
{
	RenderCommand_uniformui64vNV* __restrict packet = (RenderCommand_uniformui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_uniformui64vNV::execute);
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void getUniformui64vNV(
	GLuint program,
	GLint location,
	GLuint64EXT *params)
{
	RenderCommand_getUniformui64vNV* __restrict packet = (RenderCommand_getUniformui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getUniformui64vNV::execute);
	packet->program = program;
	packet->location = location;
	packet->params = params;
}
void programUniformui64NV(
	GLuint program,
	GLint location,
	GLuint64EXT value)
{
	RenderCommand_programUniformui64NV* __restrict packet = (RenderCommand_programUniformui64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformui64NV::execute);
	packet->program = program;
	packet->location = location;
	packet->value = value;
}
void programUniformui64vNV(
	GLuint program,
	GLint location,
	GLsizei count,
	const GLuint64EXT *value)
{
	RenderCommand_programUniformui64vNV* __restrict packet = (RenderCommand_programUniformui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_programUniformui64vNV::execute);
	packet->program = program;
	packet->location = location;
	packet->count = count;
	packet->value = value;
}
void bindShadingRateImageNV(
	GLuint texture)
{
	RenderCommand_bindShadingRateImageNV* __restrict packet = (RenderCommand_bindShadingRateImageNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bindShadingRateImageNV::execute);
	packet->texture = texture;
}
void getShadingRateImagePaletteNV(
	GLuint viewport,
	GLuint entry,
	GLenum *rate)
{
	RenderCommand_getShadingRateImagePaletteNV* __restrict packet = (RenderCommand_getShadingRateImagePaletteNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getShadingRateImagePaletteNV::execute);
	packet->viewport = viewport;
	packet->entry = entry;
	packet->rate = rate;
}
void getShadingRateSampleLocationivNV(
	GLenum rate,
	GLuint samples,
	GLuint index,
	GLint *location)
{
	RenderCommand_getShadingRateSampleLocationivNV* __restrict packet = (RenderCommand_getShadingRateSampleLocationivNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getShadingRateSampleLocationivNV::execute);
	packet->rate = rate;
	packet->samples = samples;
	packet->index = index;
	packet->location = location;
}
void shadingRateImageBarrierNV(
	GLboolean synchronize)
{
	RenderCommand_shadingRateImageBarrierNV* __restrict packet = (RenderCommand_shadingRateImageBarrierNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_shadingRateImageBarrierNV::execute);
	packet->synchronize = synchronize;
}
void shadingRateImagePaletteNV(
	GLuint viewport,
	GLuint first,
	GLsizei count,
	const GLenum *rates)
{
	RenderCommand_shadingRateImagePaletteNV* __restrict packet = (RenderCommand_shadingRateImagePaletteNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_shadingRateImagePaletteNV::execute);
	packet->viewport = viewport;
	packet->first = first;
	packet->count = count;
	packet->rates = rates;
}
void shadingRateSampleOrderNV(
	GLenum order)
{
	RenderCommand_shadingRateSampleOrderNV* __restrict packet = (RenderCommand_shadingRateSampleOrderNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_shadingRateSampleOrderNV::execute);
	packet->order = order;
}
void shadingRateSampleOrderCustomNV(
	GLenum rate,
	GLuint samples,
	const GLint *locations)
{
	RenderCommand_shadingRateSampleOrderCustomNV* __restrict packet = (RenderCommand_shadingRateSampleOrderCustomNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_shadingRateSampleOrderCustomNV::execute);
	packet->rate = rate;
	packet->samples = samples;
	packet->locations = locations;
}
void textureBarrierNV(
	void)
{
	RenderCommand_textureBarrierNV* __restrict packet = (RenderCommand_textureBarrierNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_textureBarrierNV::execute);
}
void vertexAttribL1i64NV(
	GLuint index,
	GLint64EXT x)
{
	RenderCommand_vertexAttribL1i64NV* __restrict packet = (RenderCommand_vertexAttribL1i64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL1i64NV::execute);
	packet->index = index;
	packet->x = x;
}
void vertexAttribL2i64NV(
	GLuint index,
	GLint64EXT x,
	GLint64EXT y)
{
	RenderCommand_vertexAttribL2i64NV* __restrict packet = (RenderCommand_vertexAttribL2i64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL2i64NV::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
}
void vertexAttribL3i64NV(
	GLuint index,
	GLint64EXT x,
	GLint64EXT y,
	GLint64EXT z)
{
	RenderCommand_vertexAttribL3i64NV* __restrict packet = (RenderCommand_vertexAttribL3i64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL3i64NV::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void vertexAttribL4i64NV(
	GLuint index,
	GLint64EXT x,
	GLint64EXT y,
	GLint64EXT z,
	GLint64EXT w)
{
	RenderCommand_vertexAttribL4i64NV* __restrict packet = (RenderCommand_vertexAttribL4i64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL4i64NV::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void vertexAttribL1i64vNV(
	GLuint index,
	const GLint64EXT *v)
{
	RenderCommand_vertexAttribL1i64vNV* __restrict packet = (RenderCommand_vertexAttribL1i64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL1i64vNV::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribL2i64vNV(
	GLuint index,
	const GLint64EXT *v)
{
	RenderCommand_vertexAttribL2i64vNV* __restrict packet = (RenderCommand_vertexAttribL2i64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL2i64vNV::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribL3i64vNV(
	GLuint index,
	const GLint64EXT *v)
{
	RenderCommand_vertexAttribL3i64vNV* __restrict packet = (RenderCommand_vertexAttribL3i64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL3i64vNV::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribL4i64vNV(
	GLuint index,
	const GLint64EXT *v)
{
	RenderCommand_vertexAttribL4i64vNV* __restrict packet = (RenderCommand_vertexAttribL4i64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL4i64vNV::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribL1ui64NV(
	GLuint index,
	GLuint64EXT x)
{
	RenderCommand_vertexAttribL1ui64NV* __restrict packet = (RenderCommand_vertexAttribL1ui64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL1ui64NV::execute);
	packet->index = index;
	packet->x = x;
}
void vertexAttribL2ui64NV(
	GLuint index,
	GLuint64EXT x,
	GLuint64EXT y)
{
	RenderCommand_vertexAttribL2ui64NV* __restrict packet = (RenderCommand_vertexAttribL2ui64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL2ui64NV::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
}
void vertexAttribL3ui64NV(
	GLuint index,
	GLuint64EXT x,
	GLuint64EXT y,
	GLuint64EXT z)
{
	RenderCommand_vertexAttribL3ui64NV* __restrict packet = (RenderCommand_vertexAttribL3ui64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL3ui64NV::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
}
void vertexAttribL4ui64NV(
	GLuint index,
	GLuint64EXT x,
	GLuint64EXT y,
	GLuint64EXT z,
	GLuint64EXT w)
{
	RenderCommand_vertexAttribL4ui64NV* __restrict packet = (RenderCommand_vertexAttribL4ui64NV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL4ui64NV::execute);
	packet->index = index;
	packet->x = x;
	packet->y = y;
	packet->z = z;
	packet->w = w;
}
void vertexAttribL1ui64vNV(
	GLuint index,
	const GLuint64EXT *v)
{
	RenderCommand_vertexAttribL1ui64vNV* __restrict packet = (RenderCommand_vertexAttribL1ui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL1ui64vNV::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribL2ui64vNV(
	GLuint index,
	const GLuint64EXT *v)
{
	RenderCommand_vertexAttribL2ui64vNV* __restrict packet = (RenderCommand_vertexAttribL2ui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL2ui64vNV::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribL3ui64vNV(
	GLuint index,
	const GLuint64EXT *v)
{
	RenderCommand_vertexAttribL3ui64vNV* __restrict packet = (RenderCommand_vertexAttribL3ui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL3ui64vNV::execute);
	packet->index = index;
	packet->v = v;
}
void vertexAttribL4ui64vNV(
	GLuint index,
	const GLuint64EXT *v)
{
	RenderCommand_vertexAttribL4ui64vNV* __restrict packet = (RenderCommand_vertexAttribL4ui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribL4ui64vNV::execute);
	packet->index = index;
	packet->v = v;
}
void getVertexAttribLi64vNV(
	GLuint index,
	GLenum pname,
	GLint64EXT *params)
{
	RenderCommand_getVertexAttribLi64vNV* __restrict packet = (RenderCommand_getVertexAttribLi64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getVertexAttribLi64vNV::execute);
	packet->index = index;
	packet->pname = pname;
	packet->params = params;
}
void getVertexAttribLui64vNV(
	GLuint index,
	GLenum pname,
	GLuint64EXT *params)
{
	RenderCommand_getVertexAttribLui64vNV* __restrict packet = (RenderCommand_getVertexAttribLui64vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getVertexAttribLui64vNV::execute);
	packet->index = index;
	packet->pname = pname;
	packet->params = params;
}
void vertexAttribLFormatNV(
	GLuint index,
	GLint size,
	GLenum type,
	GLsizei stride)
{
	RenderCommand_vertexAttribLFormatNV* __restrict packet = (RenderCommand_vertexAttribLFormatNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribLFormatNV::execute);
	packet->index = index;
	packet->size = size;
	packet->type = type;
	packet->stride = stride;
}
void bufferAddressRangeNV(
	GLenum pname,
	GLuint index,
	GLuint64EXT address,
	GLsizeiptr length)
{
	RenderCommand_bufferAddressRangeNV* __restrict packet = (RenderCommand_bufferAddressRangeNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_bufferAddressRangeNV::execute);
	packet->pname = pname;
	packet->index = index;
	packet->address = address;
	packet->length = length;
}
void vertexFormatNV(
	GLint size,
	GLenum type,
	GLsizei stride)
{
	RenderCommand_vertexFormatNV* __restrict packet = (RenderCommand_vertexFormatNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexFormatNV::execute);
	packet->size = size;
	packet->type = type;
	packet->stride = stride;
}
void normalFormatNV(
	GLenum type,
	GLsizei stride)
{
	RenderCommand_normalFormatNV* __restrict packet = (RenderCommand_normalFormatNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_normalFormatNV::execute);
	packet->type = type;
	packet->stride = stride;
}
void colorFormatNV(
	GLint size,
	GLenum type,
	GLsizei stride)
{
	RenderCommand_colorFormatNV* __restrict packet = (RenderCommand_colorFormatNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_colorFormatNV::execute);
	packet->size = size;
	packet->type = type;
	packet->stride = stride;
}
void indexFormatNV(
	GLenum type,
	GLsizei stride)
{
	RenderCommand_indexFormatNV* __restrict packet = (RenderCommand_indexFormatNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_indexFormatNV::execute);
	packet->type = type;
	packet->stride = stride;
}
void texCoordFormatNV(
	GLint size,
	GLenum type,
	GLsizei stride)
{
	RenderCommand_texCoordFormatNV* __restrict packet = (RenderCommand_texCoordFormatNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_texCoordFormatNV::execute);
	packet->size = size;
	packet->type = type;
	packet->stride = stride;
}
void edgeFlagFormatNV(
	GLsizei stride)
{
	RenderCommand_edgeFlagFormatNV* __restrict packet = (RenderCommand_edgeFlagFormatNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_edgeFlagFormatNV::execute);
	packet->stride = stride;
}
void secondaryColorFormatNV(
	GLint size,
	GLenum type,
	GLsizei stride)
{
	RenderCommand_secondaryColorFormatNV* __restrict packet = (RenderCommand_secondaryColorFormatNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_secondaryColorFormatNV::execute);
	packet->size = size;
	packet->type = type;
	packet->stride = stride;
}
void fogCoordFormatNV(
	GLenum type,
	GLsizei stride)
{
	RenderCommand_fogCoordFormatNV* __restrict packet = (RenderCommand_fogCoordFormatNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_fogCoordFormatNV::execute);
	packet->type = type;
	packet->stride = stride;
}
void vertexAttribFormatNV(
	GLuint index,
	GLint size,
	GLenum type,
	GLboolean normalized,
	GLsizei stride)
{
	RenderCommand_vertexAttribFormatNV* __restrict packet = (RenderCommand_vertexAttribFormatNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribFormatNV::execute);
	packet->index = index;
	packet->size = size;
	packet->type = type;
	packet->normalized = normalized;
	packet->stride = stride;
}
void vertexAttribIFormatNV(
	GLuint index,
	GLint size,
	GLenum type,
	GLsizei stride)
{
	RenderCommand_vertexAttribIFormatNV* __restrict packet = (RenderCommand_vertexAttribIFormatNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_vertexAttribIFormatNV::execute);
	packet->index = index;
	packet->size = size;
	packet->type = type;
	packet->stride = stride;
}
void getIntegerui64i_vNV(
	GLenum value,
	GLuint index,
	GLuint64EXT *result)
{
	RenderCommand_getIntegerui64i_vNV* __restrict packet = (RenderCommand_getIntegerui64i_vNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_getIntegerui64i_vNV::execute);
	packet->value = value;
	packet->index = index;
	packet->result = result;
}
void viewportSwizzleNV(
	GLuint index,
	GLenum swizzlex,
	GLenum swizzley,
	GLenum swizzlez,
	GLenum swizzlew)
{
	RenderCommand_viewportSwizzleNV* __restrict packet = (RenderCommand_viewportSwizzleNV*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_viewportSwizzleNV::execute);
	packet->index = index;
	packet->swizzlex = swizzlex;
	packet->swizzley = swizzley;
	packet->swizzlez = swizzlez;
	packet->swizzlew = swizzlew;
}
void framebufferTextureMultiviewOVR(
	GLenum target,
	GLenum attachment,
	GLuint texture,
	GLint level,
	GLint baseViewIndex,
	GLsizei numViews)
{
	RenderCommand_framebufferTextureMultiviewOVR* __restrict packet = (RenderCommand_framebufferTextureMultiviewOVR*)getNextPacket();
	packet->pfn_execute = PFN_EXECUTE(RenderCommand_framebufferTextureMultiviewOVR::execute);
	packet->target = target;
	packet->attachment = attachment;
	packet->texture = texture;
	packet->level = level;
	packet->baseViewIndex = baseViewIndex;
	packet->numViews = numViews;
}
