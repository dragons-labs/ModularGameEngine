#version 430 core
#extension GL_ARB_shading_language_420pack: require
layout(std140) uniform;

mat4 UNPACK_MAT4( samplerBuffer matrixBuf, uint pixelIdx ) {
	vec4 row0 = texelFetch( matrixBuf, int((pixelIdx)  << 2u) );
	vec4 row1 = texelFetch( matrixBuf, int(((pixelIdx) << 2u) + 1u) );
	vec4 row2 = texelFetch( matrixBuf, int(((pixelIdx) << 2u) + 2u) );
	vec4 row3 = texelFetch( matrixBuf, int(((pixelIdx) << 2u) + 3u) );
    return mat4( row0, row1, row2, row3 );
}

// input data (read-only)
in vec4 vertex;
in uint drawId;

// input buffor (for WorldViewProj Matrix)
uniform samplerBuffer worldMatBuf;

// output screen point
out gl_PerVertex {
  vec4  gl_Position;
  float gl_PointSize;
};

// output data for next shader
out block {
	flat uint drawId;
} outVs;


// main ...
void main() {
	mat4 worldViewProj = UNPACK_MAT4( worldMatBuf, drawId );
	gl_Position = vertex * worldViewProj;
	outVs.drawId = drawId;
}
