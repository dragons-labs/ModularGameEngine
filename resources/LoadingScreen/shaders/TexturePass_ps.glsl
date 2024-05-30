#version 330

uniform sampler2D RT;

out vec4 fragColour;

in block{
	vec2 uv0;
} inPs;

void main() {
	fragColour = vec4(texture(RT, inPs.uv0).xyz, 1.0);
}
