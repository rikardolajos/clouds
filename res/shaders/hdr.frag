#version 450

layout(location = 0) out vec4 frag_color;

uniform sampler2D HDR_buffer;

void main() {
	const float gamma = 2.2;

    vec3 hdrColor = texelFetch(HDR_buffer, ivec2(gl_FragCoord.xy), 0).rgb;

	// Reinhard tone mapping
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    // Gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));

    frag_color = vec4(mapped, 1.0);
}