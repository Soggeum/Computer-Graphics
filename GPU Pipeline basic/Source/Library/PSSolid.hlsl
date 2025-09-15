cbuffer cbChangesEveryFrame : register(b2)
{
    matrix World;
    float4 OutColor;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 WorldPosition : WORLDPOS;
};

float4 PSSolid(PS_INPUT input) : SV_Target
{
    return OutColor;
}