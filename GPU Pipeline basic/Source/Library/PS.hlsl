// texturing
Texture2D txDiffuse : register(t0); //  texture image�� GPU�� �����ϴ� ��ü
SamplerState samLinear : register(s0); // texture filtering, addressing�ϱ� ���� �߰����� Sampler state
                                        // texel -> pixel�� �°� sampling

// lighting
cbuffer cbLights : register(b3)
{
    float4 LightPositions[2]; // 2���� ������ġ��
    float4 LightColors[2]; // 2���� ���� ����
};

cbuffer cbChangeOnCameraMovement : register(b0) // memory register�� ���� ���ϴ´��
{
    matrix View;
    float4 CameraPosition;
};
// world
cbuffer cbChangesEveryFrame : register(b2)
{
    matrix World;
    float4 OutColor; // ������ ��� �׳� ���� ��ȯ
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    /*float4 Color : COLOR;*/
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL; // ��ü�� normal vector for lighting caculations
    float3 WorldPosition : WORLDPOS; // ��ü�� world position for lighting caculations
};

float4 PS(PS_INPUT input) : SV_TARGET
{
    /*return input.Color;*/
    
    float4 color = txDiffuse.Sample(samLinear, input.TexCoord); // ���� ��� sampling�� texture color�� ��ȯ
    
    // lighting ó��
    // diffuse + ambient + specular
    float3 diffuse = 0;
    float3 ambient = 0;
    float3 specular = 0;
    float3 LightDirection; // ����->vertex ������ unit vector
    float3 viewDirection = normalize(CameraPosition.xyz - input.WorldPosition); // camera->vertex ������ unit vector
    
    for (uint i = 0; i < 2; i++)  // �������� vertex�� �����ִ� �� ���
    {
        LightDirection = normalize(input.WorldPosition - LightPositions[i].xyz);
        diffuse += max(dot(input.Normal, -LightDirection), 0.0f) * LightColors[i].xyz;
        ambient += float3(0.1f, 0.1f, 0.1f);
        specular += (pow(max(dot(reflect(LightDirection, input.Normal), viewDirection), 0.0f), 4.0f) * LightColors[i]).rgb; // reflect�� Normal�� ���� �ݻ�� ���� ����
    }
    
    
    return float4(saturate((diffuse + specular + ambient) * color.rgb), color.a); // ���ϰ� ���� ������ ��, ������ ����� ���İ� �߰�. ���� ������� saturate ���
}