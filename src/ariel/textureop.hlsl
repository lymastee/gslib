/*
 * Copyright (c) 2016-2021 lymastee, All rights reserved.
 * Contact: lymastee@hotmail.com
 *
 * This file is part of the gslib project.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

cbuffer texop_configs : register(b0)
{
    int4        g_rect;     // x, y, width, height
    float4      g_color;
    float       g_arg;
};

Texture2D<float4>       g_src : register(t0);
RWTexture2D<float4>     g_dest : register(u0);

[numthreads(8, 8, 1)]
void ariel_transpose_image_cs(uint3 dtid : SV_DispatchThreadID)
{
    if(dtid.x >= (uint)g_rect.z || dtid.y >= (uint)g_rect.w)
        return;
    g_dest[dtid.yx + g_rect.xy] = g_src[dtid.xy];
}

[numthreads(8, 8, 1)]
void ariel_initialize_image_cs(uint3 dtid : SV_DispatchThreadID)
{
    if(dtid.x >= (uint)g_rect.z || dtid.y >= (uint)g_rect.w)
        return;
    g_dest[dtid.xy + g_rect.xy] = g_color;
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
void ariel_conv_from_premul_cs(uint3 dtid : SV_DispatchThreadID)
{
    float4 cr = g_src[dtid.xy];
    float alpha = cr.w;
    cr /= alpha;
    g_dest[dtid.xy] = float4(cr.xyz, alpha);
}
