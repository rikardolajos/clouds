#version 450

layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 frag_color;

uniform sampler2D HDR_buffer

void main()
{             
    vec3 hdrColor = texture(HDR_buffer, TexCoords).rgb;
    frag_color = vec4(hdrColor, 1.0);
} 