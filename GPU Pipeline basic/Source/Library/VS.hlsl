cbuffer cbChangeOnCameraMovement : register(b0) // memory register는 내가 원하는대로
{
    matrix View;
    float4 CameraPosition;
};
cbuffer cbProjection : register(b1)
{
    matrix Projection;
};
cbuffer cbChangesEveryFrame : register(b2)
{
    matrix World;
    float4 OutColor;
};

struct VS_INPUT
{
    float4 Position : POSITION;
    //float4 Color : COLOR;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL; // 물체의 normal vector for lighting caculations
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    /*float4 Color : COLOR;*/
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL; // 물체의 normal vector for lighting caculations
    float3 WorldPosition : WORLDPOS; // 물체의 world position for lighting caculations
};

// vertex position은 float3이지만, 
// float4로 받으면 (x, y, z, 1)로 자동으로 초기화해줌
PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    
    // 월드-뷰-투영 변환 적용
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    /*output.Color = input.Color;*/
    output.TexCoord = input.TexCoord;
    
    // lighting위한 과정. World에서의 vertex의 normal vector, 위치 계산
    output.Normal = normalize(mul(float4(input.Normal, 0.0f), World).xyz);
    output.WorldPosition = mul(input.Position, World);
    
    return output;
}