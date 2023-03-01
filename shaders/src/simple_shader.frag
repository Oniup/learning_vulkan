#version 450

layout (location = 0) in vec3 frag_color;
layout (location = 0) out vec4 frag_out_color;

void main() {
    frag_out_color = vec4(frag_color.xyz, 1.0);
}