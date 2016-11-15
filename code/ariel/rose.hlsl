cbuffer rose_configs : register(b0)
{
    float4      g_screen;
    float3x3    g_mapscreen;
};

struct rose_vsf_cr_input
{
    float2      position : POSITION;
    float4      color : COLOR;
};

struct rose_vsf_klm_cr_input
{
    float2      position : POSITION;
    float3      klm : TEXCOORD;
    float4      color : COLOR;
};

struct rose_vsf_coef_cr_input
{
    float2      position : POSITION;
    float4      coef : TEXCOORD;        // linear coef & AA tune
    float4      color : COLOR;
};

float2 rose_mapping_point(float2 p)
{
    float3 cv = mul(float3(p, 1), g_mapscreen);
    return float2(cv.x, cv.y);
}

void rose_vsf_cr(rose_vsf_cr_input input, out float4 pos : SV_POSITION, out float4 cr : COLOR)
{
    pos = float4(rose_mapping_point(input.position), 0.5, 1.0);
    cr = input.color;
}

float4 rose_psf_cr(float4 pos : SV_POSITION, float4 cr : COLOR) : SV_TARGET
{
    return cr;
}

void rose_vsf_klm_cr(rose_vsf_klm_cr_input input, out float4 pos : SV_POSITION, out float3 klm : TEXCOORD, out float4 cr : COLOR)
{
    pos = float4(rose_mapping_point(input.position), 0.5, 1.0);
    klm = input.klm;
    cr = input.color;
}

float4 rose_psf_klm_cr(float4 pos : SV_POSITION, float3 klm : TEXCOORD, float4 cr : COLOR) : SV_TARGET
{
    float c = klm.x * klm.x * klm.x - klm.y * klm.z;
    float alpha = c > 0.0f ? 1.0f : 0.0f;
    return float4(cr.x, cr.y, cr.z, cr.w * alpha);
}

float4 rose_psf_klm_cr_aa(float4 pos : SV_POSITION, float3 klm : TEXCOORD, float4 cr : COLOR) : SV_TARGET
{
    float3 px = ddx(klm);
    float3 py = ddy(klm);
    float k2 = klm.x * klm.x;
    float c = k2 * klm.x - klm.y * klm.z;
    float k23 = 3.0 * k2;
    float cx = k23 * px.x - klm.z * px.y - klm.y * px.z;
    float cy = k23 * py.x - klm.z * py.y - klm.y * py.z;
    float sd = c / sqrt(cx * cx + cy * cy);
    float alpha = saturate(sd - 0.5);
    return float4(cr.x, cr.y, cr.z, cr.w * alpha);
}

void rose_vss_coef_cr(rose_vsf_coef_cr_input input, out float4 pos : SV_POSITION, out float4 coef : TEXCOORD, out float4 cr : COLOR)
{
    pos = float4(rose_mapping_point(input.position), 0.5, 1.0);
    coef = input.coef;
    cr = input.color;
}

float4 rose_pss_coef_cr(float4 pos : SV_POSITION, float4 coef : TEXCOORD, float4 cr : COLOR) : SV_TARGET
{
    // the mapping has an up-down flip operation, so that now we have to flip the distance here
    float dist = -dot(float3(coef.x, coef.y, coef.z), float3(pos.x, pos.y, 1.0)) / length(float2(coef.x, coef.y));
    float dx = ddx(dist);
    float dy = ddy(dist);
    float grad = length(float2(dx, dy));
    float threshold = coef.w * grad;
    float alpha = saturate(1.0 - abs(dist/threshold));
    return float4(cr.x, cr.y, cr.z, cr.w * alpha);
}
