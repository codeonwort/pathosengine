#version 460 core

////////////////////////////////////////////////////////////

#if VERTEX_SHADER
//const vec3[4] vertices = vec3[4](vec3(-1,-1,1), vec3(1,-1,1), vec3(-1,1,1), vec3(1,1,1));

layout (location = 0) in vec3 position;

out vec3 local_position;

layout (location = 0) uniform mat4 transform;

void main() {
    local_position = position;
    gl_Position = transform * vec4(local_position, 1.0);
}
#endif

////////////////////////////////////////////////////////////

/* Layed rendering is cumbersome...
#if GEOMETRY_SHADER
layout (triangles) in;
layout (triangle_strip) out;
layout (max_vertices = 18) out;

const mat3[6] transforms = mat3[6](
    mat3(1,0,0, 0,1,0, 0,0,1),
    mat3(1,0,0, 0,1,0, 0,0,1),
    mat3(1,0,0, 0,1,0, 0,0,1),
    mat3(1,0,0, 0,1,0, 0,0,1),
    mat3(1,0,0, 0,1,0, 0,0,1),
    mat3(1,0,0, 0,1,0, 0,0,1)
);

void main() {
    for(int i = 0; i < 6; i++) {
        for (int j = 0; j < gl_in.length(); j++) {
            gl_Position = transforms[i] * gl_in[j].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    }
}
#endif
*/

////////////////////////////////////////////////////////////

#if FRAGMENT_SHADER
in vec3 local_position;
out vec4 out_color;

layout (binding = 0) uniform sampler2D equirectangularMap;

vec2 CubeToEquirectangular(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= vec2(0.1591, 0.3183); // inverse atan
    uv += 0.5;
    return uv;
}

void main() {
    vec2 uv = CubeToEquirectangular(normalize(local_position));
    vec3 color = texture(equirectangularMap, uv).rgb;
    
    out_color = vec4(color, 1.0);
}
#endif
