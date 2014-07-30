Texture2D colorMap : register(t0);
SamplerState colorSampler : register(s0);

cbuffer constantBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projMatrix;
};

struct VS_Input 
{
	float4 pos : POSITION;
	float2 tex0 : TEXCOORD0;
	float3 normal : NORMAL0;
	float3 tangent : TANGENT0;
};

struct PS_Input 
{
	float4 pos : SV_POSITION;
	float2 tex0 : TEXCOORD0;
	float3 normal : NORMAL0;
	float3 tangent : TANGENT0;
	float3 binormal : BINORMAL;
};

PS_Input VS_Main(VS_Input vertex)
{
	PS_Input vsOut = (PS_Input)0;
	vsOut.pos = mul(vertex.pos, worldMatrix);
	vsOut.pos = mul(vsOut.pos, viewMatrix);
	vsOut.pos = mul(vsOut.pos, projMatrix);

	vsOut.tex0 = vertex.tex0;
	vsOut.normal = normalize(mul(vertex.normal, worldMatrix));
	vsOut.tangent = 0;
	vsOut.binormal = 0;

	return vsOut;
}

float4 PS_Main(PS_Input pix) : SV_TARGET
{
	float3 ambient = float3(0.1f, 0.1f, 0.1f);

	float4 text = colorMap.Sample(colorSampler, pix.tex0);

	float3 DiffuseDirection = float3(0.0f, -1.0f, 0.2f);
	float4 DiffuseColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float3 diffuse = dot(-DiffuseDirection, pix.normal);
	diffuse = saturate(diffuse*DiffuseColor.rgb);
	diffuse = saturate(diffuse+ambient);

	float4 fColor = float4(text.rgb*diffuse, 1.0f);

	return fColor;
}
