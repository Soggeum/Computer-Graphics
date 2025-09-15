struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

// vertex position은 float3이지만, 
// float4로 받으면 (x, y, z, 1)로 자동으로 초기화해줌
VOut VS(float4 position : POSITION, float4 color : COLOR)
{
    VOut output;

    output.position = position;
    output.color = color;

    return output;
}

float4 PS(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
    return color;
}