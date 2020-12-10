/*
 * Copyright (c) 2016-2020 lymastee, All rights reserved.
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

/* SMAA T2x */

cbuffer smaaConfigs : register(b0)
{
    float4      subsampleIndices;
    float4      SMAA_RT_METRICS;
};

#define SMAA_PRESET_HIGH
#define SMAA_PREDICATION    0
#define SMAA_REPROJECTION   1
#define SMAA_HLSL_4_1       1

#include "../../ext/smaa/smaa.hlsl"

Texture2D       colorTex : register(t0);
Texture2D       colorTexPrev : register(t1);
Texture2D       velocityTex : register(t2);
Texture2D       areaTex : register(t3);
Texture2D       searchTex : register(t4);
Texture2D       edgesTex : register(t5);
Texture2D       blendTex : register(t6);

void ariel_smaa_edge_detection_vs(float4 position : POSITION,
                                  out float4 svPosition : SV_POSITION,
                                  inout float2 texcoord : TEXCOORD0,
                                  out float4 offset[3] : TEXCOORD1)
{
    svPosition = position;
    SMAAEdgeDetectionVS(texcoord, offset);
}

void ariel_smaa_blending_weight_calculation_vs(float4 position : POSITION,
                                               out float4 svPosition : SV_POSITION,
                                               inout float2 texcoord : TEXCOORD0,
                                               out float2 pixcoord : TEXCOORD1,
                                               out float4 offset[3] : TEXCOORD2)
{
    svPosition = position;
    SMAABlendingWeightCalculationVS(texcoord, pixcoord, offset);
}

void ariel_smaa_neighborhood_blending_vs(float4 position : POSITION,
                                         out float4 svPosition : SV_POSITION,
                                         inout float2 texcoord : TEXCOORD0,
                                         out float4 offset : TEXCOORD1)
{
    svPosition = position;
    SMAANeighborhoodBlendingVS(texcoord, offset);
}

float2 ariel_smaa_luma_edge_detection_ps(float4 position : SV_POSITION,
                                         float2 texcoord : TEXCOORD0,
                                         float4 offset[3] : TEXCOORD1) : SV_TARGET
{
    return SMAALumaEdgeDetectionPS(texcoord, offset, colorTex);
}

float2 ariel_smaa_color_edge_detection_ps(float4 position : SV_POSITION,
                                          float2 texcoord : TEXCOORD0,
                                          float4 offset[3] : TEXCOORD1) : SV_TARGET
{
    return SMAAColorEdgeDetectionPS(texcoord, offset, colorTex);
}

float4 ariel_smaa_blending_weight_calculation_ps(float4 position : SV_POSITION,
                                                 float2 texcoord : TEXCOORD0,
                                                 float2 pixcoord : TEXCOORD1,
                                                 float4 offset[3] : TEXCOORD2) : SV_TARGET
{
    return SMAABlendingWeightCalculationPS(texcoord, pixcoord, offset, edgesTex, areaTex, searchTex, subsampleIndices);
}

float4 ariel_smaa_neighborhood_blending_ps(float4 position : SV_POSITION,
                                           float2 texcoord : TEXCOORD0,
                                           float4 offset : TEXCOORD1) : SV_TARGET
{
    return SMAANeighborhoodBlendingPS(texcoord, offset, colorTex, blendTex, velocityTex);
}
