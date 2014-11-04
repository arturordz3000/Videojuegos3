//--------------------------------------------------------------------------------------
// File: ParticleShader.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

Texture2D colorMap : register(t0);
SamplerState colorSampler : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
	float uvOffset;
	float3 padding;
}

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
	float2 UV : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS_Main( float4 Pos : POSITION0, float4 Color : COLOR0, float2 UV : TEXCOORD0 )
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Pos = mul( Pos, World );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
    output.Color = Color;
	output.UV = UV;
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS_Main( VS_OUTPUT input ) : SV_Target
{
	input.UV.x = input.UV.x + uvOffset;
	float4 color = colorMap.Sample(colorSampler, input.UV);

	clip(color.x < 0.1f ? -1 : 1);

	return color;
}
