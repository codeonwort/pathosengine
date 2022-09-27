//? #version 460 core

// #todo: Remove common defines in separate .glsl files and use common.glsl
#define PI         3.14159265359
#define TWO_PI     6.28318530718
#define HALF_PI    1.57079632679489661923

// #todo-material-assembler: Temporarily modify values
// until MATERIAL_IDs in deferred_common.glsl are removed.
// Must match with EMaterialDomain
#define MATERIAL_SHADINGMODEL_UNLIT       1
#define MATERIAL_SHADINGMODEL_DEFAULTLIT  2
#define MATERIAL_SHADINGMODEL_TRANSLUCENT 3
//#define MATERIAL_SHADINGMODEL_SKIN        3
//#define MATERIAL_SHADINGMODEL_HAIR        4
