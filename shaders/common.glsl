//? #version 460 core

// #todo: If you see a term 'radiance' in shaders, probably it means 'luminance'.

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

// BEGIN: Octahedral Normal Vector encoding
float ONVSaturate ( float x ) { return max(0.0, min(1.0, x)); }
vec2 ONVOctWrap( vec2 v ) {
    vec2 w = 1.0 - abs( v.yx );
    if (w.x < 0.0) w.x = -w.x;
    if (w.y < 0.0) w.y = -w.y;
    return w;
}
vec2 ONVEncode( vec3 n ) {
    n /= ( abs( n.x ) + abs( n.y ) + abs( n.z ) );
    n.xy = n.z >= 0.0 ? n.xy : ONVOctWrap( n.xy );
    n.xy = n.xy * 0.5 + 0.5;
    return n.xy;
}
vec3 ONVDecode( vec2 f ) {
    f = f * 2.0 - 1.0;
    // https://twitter.com/Stubbesaurus/status/937994790553227264
    vec3 n = vec3( f.x, f.y, 1.0 - abs( f.x ) - abs( f.y ) );
    float t = ONVSaturate( -n.z );
    n.x += n.x >= 0.0 ? -t : t;
    n.y += n.y >= 0.0 ? -t : t;
    return normalize( n );
}
// END: Octahedral Normal Vector encoding
