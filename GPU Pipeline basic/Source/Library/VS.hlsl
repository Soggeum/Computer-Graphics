cbuffer cbChangeOnCameraMovement : register(b0) // memory register�� ���� ���ϴ´��
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
    float3 Normal : NORMAL; // ��ü�� normal vector for lighting caculations
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    /*float4 Color : COLOR;*/
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL; // ��ü�� normal vector for lighting caculations
    float3 WorldPosition : WORLDPOS; // ��ü�� world position for lighting caculations
};

// vertex position�� float3������, 
// float4�� ������ (x, y, z, 1)�� �ڵ����� �ʱ�ȭ����
PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    
    // ����-��-���� ��ȯ ����
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    /*output.Color = input.Color;*/
    output.TexCoord = input.TexCoord;
    
    // lighting���� ����. World������ vertex�� normal vector, ��ġ ���
    output.Normal = normalize(mul(float4(input.Normal, 0.0f), World).xyz);
    output.WorldPosition = mul(input.Position, World);
    
    return output;
}