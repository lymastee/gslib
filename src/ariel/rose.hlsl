cbuffer rose_configs : register(b0)
{
    float4      g_screen;
    float3x3    g_mapscreen;
};

// used for texture batched draw
Texture2D       g_texture : register(t0);
SamplerState    g_sstate : register(s0);

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

struct rose_vsf_klm_tex_input
{
    float2      position : POSITION;
    float3      klm : TEXCOORD0;
    float2      tex : TEXCOORD1;
};

struct rose_vsf_coef_tex_input
{
    float2      position : POSITION;
    float4      coef : TEXCOORD0;
    float2      tex : TEXCOORD1;
};

float2 rose_mapping_point(float2 p)
{
    float3 cv = mul(float3(p, 1.f), g_mapscreen);
    return float2(cv.x, cv.y);
}

float rose_get_alpha(float4 pos, float4 coef)
{
    // the mapping has an up-down flip operation, so that now we have to flip the distance here
    float dist = -dot(float3(coef.x, coef.y, coef.z), float3(pos.x, pos.y, 1.f)) / length(float2(coef.x, coef.y));
    float dx = ddx(dist);
    float dy = ddy(dist);
    float grad = length(float2(dx, dy));
    float threshold = coef.w * grad;
    float alpha = saturate(1.f - abs(dist / threshold));
    return alpha;
}

void rose_vsf_cr(rose_vsf_cr_input input, out float4 pos : SV_POSITION, out float4 cr : COLOR)
{
    pos = float4(rose_mapping_point(input.position), 0.5f, 1.f);
    cr = input.color;
}

float4 rose_psf_cr(float4 pos : SV_POSITION, float4 cr : COLOR) : SV_TARGET
{
    return cr;
}

void rose_vsf_klm_cr(rose_vsf_klm_cr_input input, out float4 pos : SV_POSITION, out float3 klm : TEXCOORD, out float4 cr : COLOR)
{
    pos = float4(rose_mapping_point(input.position), 0.5f, 1.f);
    klm = input.klm;
    cr = input.color;
}

float4 rose_psf_klm_cr(float4 pos : SV_POSITION, float3 klm : TEXCOORD, float4 cr : COLOR) : SV_TARGET
{
    float c = klm.x * klm.x * klm.x - klm.y * klm.z;
    if(c < 0.f)
        discard;
    return cr;
}

float4 rose_psf_klm_cr_aa(float4 pos : SV_POSITION, float3 klm : TEXCOORD, float4 cr : COLOR) : SV_TARGET
{
    float3 px = ddx(klm);
    float3 py = ddy(klm);
    float k2 = klm.x * klm.x;
    float c = k2 * klm.x - klm.y * klm.z;
    float k23 = 3.f * k2;
    float cx = k23 * px.x - klm.z * px.y - klm.y * px.z;
    float cy = k23 * py.x - klm.z * py.y - klm.y * py.z;
    float sd = c / sqrt(cx * cx + cy * cy);
    float alpha = saturate(sd - 0.5f);
    return float4(cr.xyz, cr.w * alpha);
}

void rose_vss_coef_cr(rose_vsf_coef_cr_input input, out float4 pos : SV_POSITION, out float4 coef : TEXCOORD, out float4 cr : COLOR)
{
    pos = float4(rose_mapping_point(input.position), 0.5f, 1.f);
    coef = input.coef;
    cr = input.color;
}

float4 rose_pss_coef_cr(float4 pos : SV_POSITION, float4 coef : TEXCOORD, float4 cr : COLOR) : SV_TARGET
{
    float alpha = rose_get_alpha(pos, coef);
    return float4(cr.xyz, cr.w * alpha);
}

void rose_vsf_klm_tex(rose_vsf_klm_tex_input input, out float4 pos : SV_POSITION, out float3 klm : TEXCOORD0, out float2 tex : TEXCOORD1)
{
    pos = float4(rose_mapping_point(input.position), 0.5f, 1.f);
    klm = input.klm;
    tex = input.tex;
}

float4 rose_psf_klm_tex(float4 pos : SV_POSITION, float3 klm : TEXCOORD0, float2 tex : TEXCOORD1) : SV_TARGET
{
    float c = klm.x * klm.x * klm.x - klm.y * klm.z;
    if(c < 0.f)
        discard;
    float4 cr = g_texture.Sample(g_sstate, tex);
    return cr;
}

void rose_vss_coef_tex(rose_vsf_coef_tex_input input, out float4 pos : SV_POSITION, out float4 coef : TEXCOORD0, out float2 tex : TEXCOORD1)
{
    pos = float4(rose_mapping_point(input.position), 0.5f, 1.f);
    coef = input.coef;
    tex = input.tex;
}

float4 rose_pss_coef_tex(float4 pos : SV_POSITION, float4 coef : TEXCOORD0, float2 tex : TEXCOORD1) : SV_TARGET
{
    float alpha = rose_get_alpha(pos, coef);
    float4 cr = g_texture.Sample(g_sstate, tex);
    return float4(cr.xyz, cr.w * alpha);
}
