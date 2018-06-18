cbuffer texop_configs : register(b0)
{
    int2        g_offset;
};

Texture2D<float4>       g_src : register(t0);
RWTexture2D<float4>     g_dest : register(u0);

[numthreads(1, 1, 1)]
void ariel_transpose_image_cs(uint3 dtid : SV_DispatchThreadID)
{
	g_dest[dtid.yx + g_offset] = g_src[dtid.xy];
}
