//?#version 460 core
//?#define FP16_CRITERIA 0

#if FP16_CRITERIA == 1

#define A_HALF 1

#elif FP16_CRITERIA == 2

#extension GL_NV_gpu_shader5 : enable
#define A_SKIP_EXT 1
#define A_HALF 1

uint16_t halfBitsToUint16(float16_t v) { return uint16_t(packFloat2x16(f16vec2(v, 0))); }
u16vec2  halfBitsToUint16(f16vec2   v) { return u16vec2(packFloat2x16(f16vec2(v.x, 0)), packFloat2x16(f16vec2(v.y, 0))); }
u16vec3  halfBitsToUint16(f16vec3   v) { return u16vec3(packFloat2x16(f16vec2(v.x, 0)), packFloat2x16(f16vec2(v.y, 0)), packFloat2x16(f16vec2(v.z, 0))); }
u16vec4  halfBitsToUint16(f16vec4   v) { return u16vec4(packFloat2x16(f16vec2(v.x, 0)), packFloat2x16(f16vec2(v.y, 0)), packFloat2x16(f16vec2(v.z, 0)), packFloat2x16(f16vec2(v.w, 0))); }

float16_t uint16BitsToHalf(uint16_t v) { return unpackFloat2x16(uint(v)).x; }
f16vec2   uint16BitsToHalf(u16vec2  v) { return f16vec2(unpackFloat2x16(uint(v.x)).x, unpackFloat2x16(uint(v.y)).x); }
f16vec3   uint16BitsToHalf(u16vec3  v) { return f16vec3(unpackFloat2x16(uint(v.x)).x, unpackFloat2x16(uint(v.y)).x, unpackFloat2x16(uint(v.z)).x); }
f16vec4   uint16BitsToHalf(u16vec4  v) { return f16vec4(unpackFloat2x16(uint(v.x)).x, unpackFloat2x16(uint(v.y)).x, unpackFloat2x16(uint(v.z)).x, unpackFloat2x16(uint(v.w)).x); }

uint32_t packUint2x16(u16vec2 v) { return (uint(v.y) << 16) | uint(v.x); }
u16vec2  unpackUint2x16(uint32_t v) { return u16vec2(v & 0xffff, (v >> 16) & 0xffff); }

#endif // FP16_CRITERIA
