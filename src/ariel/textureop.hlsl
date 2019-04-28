cbuffer texop_configs : register(b0)
{
    int4        g_offset;
    float4      g_color;
    float       g_arg;
};

Texture2D<float4>       g_src : register(t0);
RWTexture2D<float4>     g_dest : register(u0);

[numthreads(8, 8, 1)]
void ariel_transpose_image_cs(uint3 dtid : SV_DispatchThreadID)
{
    g_dest[dtid.yx + g_offset.xy] = g_src[dtid.xy];
}

[numthreads(8, 8, 1)]
void ariel_initialize_image_cs(uint3 dtid : SV_DispatchThreadID)
{
    g_dest[dtid.xy + g_offset.xy] = g_color;
}

[numthreads(8, 8, 1)]
void ariel_set_brightness_cs(uint3 dtid : SV_DispatchThreadID)
{
    float4 cr = g_src[dtid.xy];
    g_dest[dtid.xy] = (g_arg > 1.f) ? lerp(cr, float4(1.f, 1.f, 1.f, cr.w), g_arg - 1.f) :
        lerp(float4(0.f, 0.f, 0.f, cr.w), cr, g_arg);
}

[numthreads(8, 8, 1)]
void ariel_set_gray_cs(uint3 dtid : SV_DispatchThreadID)
{
    float4 cr = g_src[dtid.xy];
    float aver = (cr.x + cr.y + cr.z) / 3.f;
    g_dest[dtid.xy] = float4(aver, aver, aver, cr.w);
}

[numthreads(8, 8, 1)]
void ariel_set_fade_cs(uint3 dtid : SV_DispatchThreadID)
{
    float4 cr = g_src[dtid.xy];
    g_dest[dtid.xy] = float4(cr.xyz, cr.w * g_arg);
}

[numthreads(8, 8, 1)]
void ariel_set_inverse_cs(uint3 dtid : SV_DispatchThreadID)
{
    float4 cr = g_src[dtid.xy];
    float4 s = float4(1.f, 1.f, 1.f, 1.f) - cr;
    s.w = cr.w;
    g_dest[dtid.xy] = s;
}

[numthreads(8, 8, 1)]
void ariel_conv_from_premul(uint3 dtid : SV_DispatchThreadID)
{
    float4 cr = g_src[dtid.xy];
    float alpha = cr.w;
    cr /= alpha;
    g_dest[dtid.xy] = float4(cr.xyz, alpha);
}
