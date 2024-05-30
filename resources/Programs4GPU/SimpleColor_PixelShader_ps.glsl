#version 430 core
#extension GL_ARB_shading_language_420pack: require
layout(std140) uniform;

// input data from previous shader
in block {
	flat uint drawId;
} inPs;

// input buffors for materials:
//  - struct describing material
struct Material {
	vec4 diffuse;
	vec4 param;
};
//  - materials array
layout(std140, binding = 1) uniform MaterialBuf {
	Material m[1024];
} materialArray;
//  - material indexes array
layout(std140, binding = 2) uniform InstanceBuffer {
	uvec4 worldMaterialIdx[4096];
} instance;

// output pixel colour (FRAG_COLOR)
layout(location = 0, index = 0) out vec4 outColour;

// main ...
void main() {
	uint materialId = instance.worldMaterialIdx[inPs.drawId].x;
	Material material = materialArray.m[materialId];
	outColour = material.diffuse;
}
