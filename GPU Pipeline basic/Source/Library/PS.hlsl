// texturing
Texture2D txDiffuse : register(t0); //  texture image를 GPU에 저장하는 객체
SamplerState samLinear : register(s0); // texture filtering, addressing하기 위한 추가적인 Sampler state
                                        // texel -> pixel에 맞게 sampling

// lighting
cbuffer cbLights : register(b3)
{
    float4 LightPositions[2]; // 2개의 광원위치와
    float4 LightColors[2]; // 2개의 광원 색깔
};

cbuffer cbChangeOnCameraMovement : register(b0) // memory register는 내가 원하는대로
{
    matrix View;
    float4 CameraPosition;
};
// world
cbuffer cbChangesEveryFrame : register(b2)
{
    matrix World;
    float4 OutColor; // 광원일 경우 그냥 색깔 반환
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    /*float4 Color : COLOR;*/
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL; // 물체의 normal vector for lighting caculations
    float3 WorldPosition : WORLDPOS; // 물체의 world position for lighting caculations
};

float4 PS(PS_INPUT input) : SV_TARGET
{
    /*return input.Color;*/
    
    float4 color = txDiffuse.Sample(samLinear, input.TexCoord); // 색깔 대신 sampling한 texture color를 반환
    
    // lighting 처리
    // diffuse + ambient + specular
    float3 diffuse = 0;
    float3 ambient = 0;
    float3 specular = 0;
    float3 LightDirection; // 광원->vertex 방향의 unit vector
    float3 viewDirection = normalize(CameraPosition.xyz - input.WorldPosition); // camera->vertex 방향의 unit vector
    
    for (uint i = 0; i < 2; i++)  // 광원마다 vertex에 영향주는 빛 계산
    {
        LightDirection = normalize(input.WorldPosition - LightPositions[i].xyz);
        diffuse += max(dot(input.Normal, -LightDirection), 0.0f) * LightColors[i].xyz;
        ambient += float3(0.1f, 0.1f, 0.1f);
        specular += (pow(max(dot(reflect(LightDirection, input.Normal), viewDirection), 0.0f), 4.0f) * LightColors[i]).rgb; // reflect는 Normal에 대해 반사된 빛의 방향
    }
    
    
    return float4(saturate((diffuse + specular + ambient) * color.rgb), color.a); // 더하고 색깔 곱해준 후, 마지막 요소인 알파값 추가. 최종 결과에는 saturate 사용
}