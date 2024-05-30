#version 430 core
#extension GL_ARB_shading_language_420pack: require

layout(lines) in;
layout(triangle_strip, max_vertices = 6) out;

// input data from previous shader
in block {
	flat uint drawId;
} inGs[];

in gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
} gl_in[];

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
};

out block {
	flat uint drawId;
} outGs;

vec2 get2D(vec4 p) {
	return vec2(p.xy/p.w);
}

// main ...
void main() {
	vec2 inPoint[2];
	int i;
	
	for (i = 0; i<2; ++i) {
		inPoint[i] = get2D(gl_in[i].gl_Position);
	}
	
	vec2 d  = normalize( inPoint[1] - inPoint[0] );
	vec2 nd = vec2( -d.y, d.x ) / 90.0;
	
	outGs.drawId = inGs[0].drawId;
	
	gl_Position = vec4(inPoint[0]+nd, 0.0f, 1.0f);
	EmitVertex();
	
	gl_Position = vec4(inPoint[0]-nd, 0.0f, 1.0f);
	EmitVertex();
	
	gl_Position = vec4(inPoint[1]+nd, 0.0f, 1.0f);
	EmitVertex();
	
	gl_Position = vec4(inPoint[1]-nd, 0.0f, 1.0f);
	EmitVertex();
	
	//EndPrimitive();
}
