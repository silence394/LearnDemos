Texture2D diffuseMap : register(t0);
SamplerState sampleClamp : register(s0);

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
	float4 color = diffuseMap.Sample(sampleClamp, input.uv);
	return color;
}
