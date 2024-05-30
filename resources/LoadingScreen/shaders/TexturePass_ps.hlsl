Texture2D<float3> RT		: register(t0);
SamplerState samplerState	: register(s0);

float4 main( float2 uv0 : TEXCOORD0 ) : SV_Target {
	return float4(RT.Sample(samplerState, uv0).xyz, 1.0);
}
