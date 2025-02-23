"#ifndef _SHADOWS_FXH_\n"
"#define _SHADOWS_FXH_\n"
"\n"
"// Must include BasicStructures.fxh\n"
"\n"
"#ifndef SHADOW_FILTER_SIZE\n"
"#   define SHADOW_FILTER_SIZE 2\n"
"#endif\n"
"\n"
"#ifndef FILTER_ACROSS_CASCADES\n"
"#   define FILTER_ACROSS_CASCADES 0\n"
"#endif\n"
"\n"
"#ifndef BEST_CASCADE_SEARCH\n"
"#   define BEST_CASCADE_SEARCH 0\n"
"#endif\n"
"\n"
"// Returns the minimum distance to cascade margin.\n"
"// If the point lies outside, the distance will be negative.\n"
"//  +1  ____________________\n"
"//     |   ______________ __|__ +1-Margin.y\n"
"//     |  |              |  |\n"
"//     |  |              |  |\n"
"//     |  |              |  |\n"
"//     |  |---*          |  |\n"
"//     |  |              |  |\n"
"//     |  |______________|__|__ -1+Margin.y\n"
"//  -1 |__|______________|__|\n"
"//    -1  |              | +1\n"
"//     -1+Margin.x    +1-Marign.x\n"
"//\n"
"float GetDistanceToCascadeMargin(float3 f3PosInCascadeProjSpace, float4 f4MarginProjSpace)\n"
"{\n"
"    float4 f4DistToEdges;\n"
"    f4DistToEdges.xy = float2(1.0, 1.0) - f4MarginProjSpace.xy - abs(f3PosInCascadeProjSpace.xy);\n"
"    const float ZScale = 2.0 / (1.0 - NDC_MIN_Z);\n"
"    f4DistToEdges.z = (f3PosInCascadeProjSpace.z - (NDC_MIN_Z + f4MarginProjSpace.z)) * ZScale;\n"
"    f4DistToEdges.w = (1.0 - f4MarginProjSpace.w - f3PosInCascadeProjSpace.z) * ZScale;\n"
"    return min(min(f4DistToEdges.x, f4DistToEdges.y), min(f4DistToEdges.z, f4DistToEdges.w));\n"
"}\n"
"\n"
"struct CascadeSamplingInfo\n"
"{\n"
"    int    iCascadeIdx;\n"
"    float2 f2UV;\n"
"    float  fDepth;\n"
"    float3 f3LightSpaceScale;\n"
"    float  fMinDistToMargin;\n"
"};\n"
"\n"
"CascadeSamplingInfo GetCascadeSamplingInfo(ShadowMapAttribs ShadowAttribs,\n"
"                                           float3           f3PosInLightViewSpace,\n"
"                                           int              iCascadeIdx)\n"
"{\n"
"    CascadeAttribs Cascade = ShadowAttribs.Cascades[iCascadeIdx];\n"
"    float3 f3CascadeLightSpaceScale = Cascade.f4LightSpaceScale.xyz;\n"
"    float3 f3PosInCascadeProjSpace  = f3PosInLightViewSpace * f3CascadeLightSpaceScale + Cascade.f4LightSpaceScaledBias.xyz;\n"
"    CascadeSamplingInfo SamplingInfo;\n"
"    SamplingInfo.iCascadeIdx       = iCascadeIdx;\n"
"    SamplingInfo.f2UV              = NormalizedDeviceXYToTexUV(f3PosInCascadeProjSpace.xy);\n"
"    SamplingInfo.fDepth            = NormalizedDeviceZToDepth(f3PosInCascadeProjSpace.z);\n"
"    SamplingInfo.f3LightSpaceScale = f3CascadeLightSpaceScale;\n"
"    SamplingInfo.fMinDistToMargin  = GetDistanceToCascadeMargin(f3PosInCascadeProjSpace, Cascade.f4MarginProjSpace);\n"
"    return SamplingInfo;\n"
"}\n"
"\n"
"CascadeSamplingInfo FindCascade(ShadowMapAttribs ShadowAttribs,\n"
"                                float3           f3PosInLightViewSpace,\n"
"                                float            fCameraViewSpaceZ)\n"
"{\n"
"    CascadeSamplingInfo SamplingInfo;\n"
"    float3 f3PosInCascadeProjSpace  = float3(0.0, 0.0, 0.0);\n"
"    float3 f3CascadeLightSpaceScale = float3(0.0, 0.0, 0.0);\n"
"    int    iCascadeIdx = 0;\n"
"#if BEST_CASCADE_SEARCH\n"
"    while (iCascadeIdx < ShadowAttribs.iNumCascades)\n"
"    {\n"
"        // Find the smallest cascade which covers current point\n"
"        CascadeAttribs Cascade         = ShadowAttribs.Cascades[iCascadeIdx];\n"
"        SamplingInfo.f3LightSpaceScale = Cascade.f4LightSpaceScale.xyz;\n"
"        f3PosInCascadeProjSpace        = f3PosInLightViewSpace * SamplingInfo.f3LightSpaceScale + Cascade.f4LightSpaceScaledBias.xyz;\n"
"        SamplingInfo.fMinDistToMargin  = GetDistanceToCascadeMargin(f3PosInCascadeProjSpace, Cascade.f4MarginProjSpace);\n"
"\n"
"        if (SamplingInfo.fMinDistToMargin > 0.0)\n"
"        {\n"
"            SamplingInfo.f2UV   = NormalizedDeviceXYToTexUV(f3PosInCascadeProjSpace.xy);\n"
"            SamplingInfo.fDepth = NormalizedDeviceZToDepth(f3PosInCascadeProjSpace.z);\n"
"            break;\n"
"        }\n"
"        else\n"
"            iCascadeIdx++;\n"
"    }\n"
"#else\n"
"    [unroll]\n"
"    for(int i=0; i< (ShadowAttribs.iNumCascades+3)/4; ++i)\n"
"    {\n"
"        float4 f4CascadeZEnd = ShadowAttribs.f4CascadeCamSpaceZEnd[i];\n"
"        float4 v = float4( f4CascadeZEnd.x < fCameraViewSpaceZ ? 1.0 : 0.0,\n"
"                           f4CascadeZEnd.y < fCameraViewSpaceZ ? 1.0 : 0.0,\n"
"                           f4CascadeZEnd.z < fCameraViewSpaceZ ? 1.0 : 0.0,\n"
"                           f4CascadeZEnd.w < fCameraViewSpaceZ ? 1.0 : 0.0);\n"
"	    //float4 v = float4(ShadowAttribs.f4CascadeCamSpaceZEnd[i] < fCameraViewSpaceZ);\n"
"	    iCascadeIdx += int(dot(float4(1.0, 1.0, 1.0, 1.0), v));\n"
"    }\n"
"\n"
"    if (iCascadeIdx < ShadowAttribs.iNumCascades)\n"
"    {\n"
"        //Cascade = min(Cascade, ShadowAttribs.iNumCascades - 1);\n"
"        SamplingInfo = GetCascadeSamplingInfo(ShadowAttribs, f3PosInLightViewSpace, iCascadeIdx);\n"
"    }\n"
"#endif\n"
"    SamplingInfo.iCascadeIdx = iCascadeIdx;\n"
"    return SamplingInfo;\n"
"}\n"
"\n"
"float GetNextCascadeBlendAmount(ShadowMapAttribs    ShadowAttribs,\n"
"                                float               fCameraViewSpaceZ,\n"
"                                CascadeSamplingInfo SamplingInfo,\n"
"                                CascadeSamplingInfo NextCscdSamplingInfo)\n"
"{\n"
"    float4 f4CascadeStartEndZ = ShadowAttribs.Cascades[SamplingInfo.iCascadeIdx].f4StartEndZ;\n"
"    float fDistToTransitionEdge = (f4CascadeStartEndZ.y - fCameraViewSpaceZ) / (f4CascadeStartEndZ.y - f4CascadeStartEndZ.x);\n"
"\n"
"#if BEST_CASCADE_SEARCH\n"
"    // Use the maximum of the camera Z - based distance and the minimal distance\n"
"    // to the cascade margin.\n"
"    // Using the maximum of the two avoids unnecessary transitions shown below.\n"
"    fDistToTransitionEdge = max(fDistToTransitionEdge, SamplingInfo.fMinDistToMargin);\n"
"    //        /\n"
"    //       /\n"
"    //      /______________\n"
"    //     /|              |\n"
"    //    / |              |\n"
"    //   /  | ___          |\n"
"    //  /   ||   |         |\n"
"    //  \\   ||___|         |\n"
"    //   \\  |<-.           |\n"
"    //    \\ |   \\          |\n"
"    //     \\|____\\_________|\n"
"    //      \\     \\\n"
"    //       \\    This area is very close to the margin of cascade 1.\n"
"    //        \\   If we only use fMinDistToMargin, it will be morphed with cascade 2,\n"
"    //            which will look very bad, especially near the boundary of cascade 0.\n"
"#endif\n"
"\n"
"    return saturate(1.0 - fDistToTransitionEdge / ShadowAttribs.fCascadeTransitionRegion) *\n"
"           saturate(NextCscdSamplingInfo.fMinDistToMargin / 0.01); // Make sure that we don\'t sample outside of the next cascade\n"
"}\n"
"\n"
"float2 ComputeReceiverPlaneDepthBias(float3 ShadowUVDepthDX,\n"
"                                     float3 ShadowUVDepthDY)\n"
"{\n"
"    // Compute (dDepth/dU, dDepth/dV):\n"
"    //\n"
"    //  | dDepth/dU |    | dX/dU    dX/dV |T  | dDepth/dX |     | dU/dX    dU/dY |-1T | dDepth/dX |\n"
"    //                 =                                     =                                      =\n"
"    //  | dDepth/dV |    | dY/dU    dY/dV |   | dDepth/dY |     | dV/dX    dV/dY |    | dDepth/dY |\n"
"    //\n"
"    //  | A B |-1   | D  -B |                      | A B |-1T   | D  -C |\n"
"    //            =           / det                           =           / det\n"
"    //  | C D |     |-C   A |                      | C D |      |-B   A |\n"
"    //\n"
"    //  | dDepth/dU |           | dV/dY   -dV/dX |  | dDepth/dX |\n"
"    //                 = 1/det\n"
"    //  | dDepth/dV |           |-dU/dY    dU/dX |  | dDepth/dY |\n"
"\n"
"    float2 biasUV;\n"
"    //               dV/dY       V      dDepth/dX    D       dV/dX       V     dDepth/dY     D\n"
"    biasUV.x =   ShadowUVDepthDY.y * ShadowUVDepthDX.z - ShadowUVDepthDX.y * ShadowUVDepthDY.z;\n"
"    //               dU/dY       U      dDepth/dX    D       dU/dX       U     dDepth/dY     D\n"
"    biasUV.y = - ShadowUVDepthDY.x * ShadowUVDepthDX.z + ShadowUVDepthDX.x * ShadowUVDepthDY.z;\n"
"\n"
"    float Det = (ShadowUVDepthDX.x * ShadowUVDepthDY.y) - (ShadowUVDepthDX.y * ShadowUVDepthDY.x);\n"
"	biasUV /= sign(Det) * max( abs(Det), 1e-10 );\n"
"    //biasUV = abs(Det) > 1e-7 ? biasUV / abs(Det) : 0;// sign(Det) * max( abs(Det), 1e-10 );\n"
"    return biasUV;\n"
"}\n"
"\n"
"// The method used in The Witness\n"
"float FilterShadowMapFixedPCF(in Texture2DArray<float>  tex2DShadowMap,\n"
"                              in SamplerComparisonState tex2DShadowMap_sampler,\n"
"                              in float4                 f4ShadowMapSize,\n"
"                              in CascadeSamplingInfo    SamplingInfo,\n"
"                              in float2                 f2ReceiverPlaneDepthBias)\n"
"{\n"
"    float lightDepth = SamplingInfo.fDepth;\n"
"\n"
"    float2 uv = SamplingInfo.f2UV * f4ShadowMapSize.xy;\n"
"    float2 base_uv = floor(uv + float2(0.5, 0.5));\n"
"    float s = (uv.x + 0.5 - base_uv.x);\n"
"    float t = (uv.y + 0.5 - base_uv.y);\n"
"    base_uv -= float2(0.5, 0.5);\n"
"    base_uv *= f4ShadowMapSize.zw;\n"
"\n"
"    float sum = 0.0;\n"
"\n"
"    // It is essential to clamp biased depth to 0 to avoid shadow leaks at near cascade depth boundary.\n"
"    //\n"
"    //            No clamping                 With clamping\n"
"    //\n"
"    //              \\ |                             ||\n"
"    //       ==>     \\|                             ||\n"
"    //                |                             ||\n"
"    // Light ==>      |\\                            |\\\n"
"    //                | \\Receiver plane             | \\ Receiver plane\n"
"    //       ==>      |  \\                          |  \\\n"
"    //                0   ...   1                   0   ...   1\n"
"    //\n"
"    // Note that clamping at far depth boundary makes no difference as 1 < 1 produces 0 and so does 1+x < 1\n"
"    const float DepthClamp = 1e-8;\n"
"#ifdef GLSL\n"
"    // There is no OpenGL counterpart for Texture2DArray.SampleCmpLevelZero()\n"
"    #define SAMPLE_SHADOW_MAP(u, v) tex2DShadowMap.SampleCmp(tex2DShadowMap_sampler, float3(base_uv.xy + float2(u,v) * f4ShadowMapSize.zw, SamplingInfo.iCascadeIdx), max(lightDepth + dot(float2(u, v), f2ReceiverPlaneDepthBias), DepthClamp))\n"
"#else\n"
"    #define SAMPLE_SHADOW_MAP(u, v) tex2DShadowMap.SampleCmpLevelZero(tex2DShadowMap_sampler, float3(base_uv.xy + float2(u,v) * f4ShadowMapSize.zw, SamplingInfo.iCascadeIdx), max(lightDepth + dot(float2(u, v), f2ReceiverPlaneDepthBias), DepthClamp))\n"
"#endif\n"
"\n"
"    #if SHADOW_FILTER_SIZE == 2\n"
"\n"
"        #ifdef GLSL\n"
"            return tex2DShadowMap.SampleCmp(tex2DShadowMap_sampler, float3(SamplingInfo.f2UV.xy, SamplingInfo.iCascadeIdx), max(lightDepth, DepthClamp));\n"
"        #else\n"
"            return tex2DShadowMap.SampleCmpLevelZero(tex2DShadowMap_sampler, float3(SamplingInfo.f2UV.xy, SamplingInfo.iCascadeIdx), max(lightDepth, DepthClamp));\n"
"        #endif\n"
"\n"
"    #elif SHADOW_FILTER_SIZE == 3\n"
"\n"
"        float uw0 = (3.0 - 2.0 * s);\n"
"        float uw1 = (1.0 + 2.0 * s);\n"
"\n"
"        float u0 = (2.0 - s) / uw0 - 1.0;\n"
"        float u1 = s / uw1 + 1.0;\n"
"\n"
"        float vw0 = (3.0 - 2.0 * t);\n"
"        float vw1 = (1.0 + 2.0 * t);\n"
"\n"
"        float v0 = (2.0 - t) / vw0 - 1.0;\n"
"        float v1 = t / vw1 + 1.0;\n"
"\n"
"        sum += uw0 * vw0 * SAMPLE_SHADOW_MAP(u0, v0);\n"
"        sum += uw1 * vw0 * SAMPLE_SHADOW_MAP(u1, v0);\n"
"        sum += uw0 * vw1 * SAMPLE_SHADOW_MAP(u0, v1);\n"
"        sum += uw1 * vw1 * SAMPLE_SHADOW_MAP(u1, v1);\n"
"\n"
"        return sum * 1.0 / 16.0;\n"
"\n"
"    #elif SHADOW_FILTER_SIZE == 5\n"
"\n"
"        float uw0 = (4.0 - 3.0 * s);\n"
"        float uw1 = 7.0;\n"
"        float uw2 = (1.0 + 3.0 * s);\n"
"\n"
"        float u0 = (3.0 - 2.0 * s) / uw0 - 2.0;\n"
"        float u1 = (3.0 + s) / uw1;\n"
"        float u2 = s / uw2 + 2.0;\n"
"\n"
"        float vw0 = (4.0 - 3.0 * t);\n"
"        float vw1 = 7.0;\n"
"        float vw2 = (1.0 + 3.0 * t);\n"
"\n"
"        float v0 = (3.0 - 2.0 * t) / vw0 - 2.0;\n"
"        float v1 = (3.0 + t) / vw1;\n"
"        float v2 = t / vw2 + 2.0;\n"
"\n"
"        sum += uw0 * vw0 * SAMPLE_SHADOW_MAP(u0, v0);\n"
"        sum += uw1 * vw0 * SAMPLE_SHADOW_MAP(u1, v0);\n"
"        sum += uw2 * vw0 * SAMPLE_SHADOW_MAP(u2, v0);\n"
"\n"
"        sum += uw0 * vw1 * SAMPLE_SHADOW_MAP(u0, v1);\n"
"        sum += uw1 * vw1 * SAMPLE_SHADOW_MAP(u1, v1);\n"
"        sum += uw2 * vw1 * SAMPLE_SHADOW_MAP(u2, v1);\n"
"\n"
"        sum += uw0 * vw2 * SAMPLE_SHADOW_MAP(u0, v2);\n"
"        sum += uw1 * vw2 * SAMPLE_SHADOW_MAP(u1, v2);\n"
"        sum += uw2 * vw2 * SAMPLE_SHADOW_MAP(u2, v2);\n"
"\n"
"        return sum * 1.0 / 144.0;\n"
"\n"
"    #elif SHADOW_FILTER_SIZE == 7\n"
"\n"
"        float uw0 = (5.0 * s - 6.0);\n"
"        float uw1 = (11.0 * s - 28.0);\n"
"        float uw2 = -(11.0 * s + 17.0);\n"
"        float uw3 = -(5.0 * s + 1.0);\n"
"\n"
"        float u0 = (4.0 * s - 5.0) / uw0 - 3.0;\n"
"        float u1 = (4.0 * s - 16.0) / uw1 - 1.0;\n"
"        float u2 = -(7.0 * s + 5.0) / uw2 + 1.0;\n"
"        float u3 = -s / uw3 + 3.0;\n"
"\n"
"        float vw0 = (5.0 * t - 6.0);\n"
"        float vw1 = (11.0 * t - 28.0);\n"
"        float vw2 = -(11.0 * t + 17.0);\n"
"        float vw3 = -(5.0 * t + 1.0);\n"
"\n"
"        float v0 = (4.0 * t - 5.0) / vw0 - 3.0;\n"
"        float v1 = (4.0 * t - 16.0) / vw1 - 1.0;\n"
"        float v2 = -(7.0 * t + 5.0) / vw2 + 1.0;\n"
"        float v3 = -t / vw3 + 3.0;\n"
"\n"
"        sum += uw0 * vw0 * SAMPLE_SHADOW_MAP(u0, v0);\n"
"        sum += uw1 * vw0 * SAMPLE_SHADOW_MAP(u1, v0);\n"
"        sum += uw2 * vw0 * SAMPLE_SHADOW_MAP(u2, v0);\n"
"        sum += uw3 * vw0 * SAMPLE_SHADOW_MAP(u3, v0);\n"
"\n"
"        sum += uw0 * vw1 * SAMPLE_SHADOW_MAP(u0, v1);\n"
"        sum += uw1 * vw1 * SAMPLE_SHADOW_MAP(u1, v1);\n"
"        sum += uw2 * vw1 * SAMPLE_SHADOW_MAP(u2, v1);\n"
"        sum += uw3 * vw1 * SAMPLE_SHADOW_MAP(u3, v1);\n"
"\n"
"        sum += uw0 * vw2 * SAMPLE_SHADOW_MAP(u0, v2);\n"
"        sum += uw1 * vw2 * SAMPLE_SHADOW_MAP(u1, v2);\n"
"        sum += uw2 * vw2 * SAMPLE_SHADOW_MAP(u2, v2);\n"
"        sum += uw3 * vw2 * SAMPLE_SHADOW_MAP(u3, v2);\n"
"\n"
"        sum += uw0 * vw3 * SAMPLE_SHADOW_MAP(u0, v3);\n"
"        sum += uw1 * vw3 * SAMPLE_SHADOW_MAP(u1, v3);\n"
"        sum += uw2 * vw3 * SAMPLE_SHADOW_MAP(u2, v3);\n"
"        sum += uw3 * vw3 * SAMPLE_SHADOW_MAP(u3, v3);\n"
"\n"
"        return sum * 1.0 / 2704.0;\n"
"    #else\n"
"        return 0.0;\n"
"    #endif\n"
"#undef SAMPLE_SHADOW_MAP\n"
"}\n"
"\n"
"\n"
"float FilterShadowMapVaryingPCF(in Texture2DArray<float>  tex2DShadowMap,\n"
"                                in SamplerComparisonState tex2DShadowMap_sampler,\n"
"                                in ShadowMapAttribs       ShadowAttribs,\n"
"                                in CascadeSamplingInfo    SamplingInfo,\n"
"                                in float2                 f2ReceiverPlaneDepthBias,\n"
"                                in float2                 f2FilterSize)\n"
"{\n"
"\n"
"    f2FilterSize = max(f2FilterSize * ShadowAttribs.f4ShadowMapDim.xy, float2(1.0, 1.0));\n"
"    float2 f2CenterTexel = SamplingInfo.f2UV * ShadowAttribs.f4ShadowMapDim.xy;\n"
"    // Clamp to the full texture extent, no need for 0.5 texel padding\n"
"    float2 f2MinBnd = clamp(f2CenterTexel - f2FilterSize / 2.0, float2(0.0, 0.0), ShadowAttribs.f4ShadowMapDim.xy);\n"
"    float2 f2MaxBnd = clamp(f2CenterTexel + f2FilterSize / 2.0, float2(0.0, 0.0), ShadowAttribs.f4ShadowMapDim.xy);\n"
"    //\n"
"    // StartTexel                                     EndTexel\n"
"    //   |  MinBnd                         MaxBnd        |\n"
"    //   V    V                              V           V\n"
"    //   |    :  X       |       X       |   :   X       |\n"
"    //   n              n+1             n+2             n+3\n"
"    //\n"
"    int2 StartTexelXY = int2(floor(f2MinBnd));\n"
"    int2 EndTexelXY   = int2(ceil (f2MaxBnd));\n"
"\n"
"    float TotalWeight = 0.0;\n"
"    float Sum = 0.0;\n"
"\n"
"    // Handle as many as 2x2 texels in one iteration\n"
"    [loop]\n"
"    for (int x = StartTexelXY.x; x < EndTexelXY.x; x += 2)\n"
"    {\n"
"        float U0 = float(x) + 0.5;\n"
"        // Compute horizontal coverage of this and the adjacent texel to the right\n"
"        //\n"
"        //        U0         U1                  U0         U1                  U0         U1\n"
"        //   |    X     |    X     |        |    X     |    X     |        |    X     |    X     |\n"
"        //    ####-----------------          ------###------------          ---############------\n"
"        //     0.4          0.0                    0.3     0.0                  0.7     0.5\n"
"        //\n"
"        float LeftTexelCoverage  = max(min(U0 + 0.5, f2MaxBnd.x) - max(U0 - 0.5, f2MinBnd.x), 0.0);\n"
"        float RightTexelCoverage = max(min(U0 + 1.5, f2MaxBnd.x) - max(U0 + 0.5, f2MinBnd.x), 0.0);\n"
"        float dU = RightTexelCoverage / max(RightTexelCoverage + LeftTexelCoverage, 1e-6);\n"
"        float HorzWeight = RightTexelCoverage + LeftTexelCoverage;\n"
"\n"
"        [loop]\n"
"        for (int y = StartTexelXY.y; y < EndTexelXY.y; y += 2)\n"
"        {\n"
"            // Compute vertical coverage of this and the top adjacent texels\n"
"            float V0 = float(y) + 0.5;\n"
"            float BottomTexelCoverage = max(min(V0 + 0.5, f2MaxBnd.y) - max(V0 - 0.5, f2MinBnd.y), 0.0);\n"
"            float TopTexelCoverage    = max(min(V0 + 1.5, f2MaxBnd.y) - max(V0 + 0.5, f2MinBnd.y), 0.0);\n"
"            float dV = TopTexelCoverage / max(BottomTexelCoverage + TopTexelCoverage, 1e-6);\n"
"            float VertWeight = BottomTexelCoverage + TopTexelCoverage;\n"
"\n"
"            float2 f2UV = float2(U0 + dU, V0 + dV);\n"
"\n"
"            float Weight = HorzWeight * VertWeight;\n"
"            const float DepthClamp = 1e-8;\n"
"            float fDepth = max(SamplingInfo.fDepth + dot(f2UV - f2CenterTexel, f2ReceiverPlaneDepthBias), DepthClamp);\n"
"            f2UV *= ShadowAttribs.f4ShadowMapDim.zw;\n"
"            #ifdef GLSL\n"
"                // There is no OpenGL counterpart for Texture2DArray.SampleCmpLevelZero()\n"
"                Sum += tex2DShadowMap.SampleCmp(tex2DShadowMap_sampler, float3(f2UV, SamplingInfo.iCascadeIdx), fDepth) * Weight;\n"
"            #else\n"
"                Sum += tex2DShadowMap.SampleCmpLevelZero(tex2DShadowMap_sampler, float3(f2UV, SamplingInfo.iCascadeIdx), fDepth) * Weight;\n"
"            #endif\n"
"            TotalWeight += Weight;\n"
"        }\n"
"    }\n"
"    return TotalWeight > 0.0 ? Sum / TotalWeight : 1.0;\n"
"}\n"
"\n"
"\n"
"float FilterShadowCascade(in ShadowMapAttribs       ShadowAttribs,\n"
"                          in Texture2DArray<float>  tex2DShadowMap,\n"
"                          in SamplerComparisonState tex2DShadowMap_sampler,\n"
"                          in float3                 f3ddXPosInLightViewSpace,\n"
"                          in float3                 f3ddYPosInLightViewSpace,\n"
"                          in CascadeSamplingInfo    SamplingInfo)\n"
"{\n"
"    float3 f3ddXShadowMapUVDepth  = f3ddXPosInLightViewSpace * SamplingInfo.f3LightSpaceScale * F3NDC_XYZ_TO_UVD_SCALE;\n"
"    float3 f3ddYShadowMapUVDepth  = f3ddYPosInLightViewSpace * SamplingInfo.f3LightSpaceScale * F3NDC_XYZ_TO_UVD_SCALE;\n"
"    float2 f2DepthSlopeScaledBias = ComputeReceiverPlaneDepthBias(f3ddXShadowMapUVDepth, f3ddYShadowMapUVDepth);\n"
"    // Rescale slope-scaled depth bias clamp to make it uniform across all cascades\n"
"    float2 f2SlopeScaledBiasClamp = abs( (SamplingInfo.f3LightSpaceScale.z  * F3NDC_XYZ_TO_UVD_SCALE.z) /\n"
"                                         (SamplingInfo.f3LightSpaceScale.xy * F3NDC_XYZ_TO_UVD_SCALE.xy) ) *\n"
"                                    ShadowAttribs.fReceiverPlaneDepthBiasClamp;\n"
"    f2DepthSlopeScaledBias = clamp(f2DepthSlopeScaledBias, -f2SlopeScaledBiasClamp, f2SlopeScaledBiasClamp);\n"
"    f2DepthSlopeScaledBias *= ShadowAttribs.f4ShadowMapDim.zw;\n"
"\n"
"    float FractionalSamplingError = dot( float2(1.0, 1.0), abs(f2DepthSlopeScaledBias.xy) ) + ShadowAttribs.fFixedDepthBias;\n"
"    SamplingInfo.fDepth -= FractionalSamplingError;\n"
"\n"
"#if SHADOW_FILTER_SIZE > 0\n"
"    return FilterShadowMapFixedPCF(tex2DShadowMap, tex2DShadowMap_sampler, ShadowAttribs.f4ShadowMapDim, SamplingInfo, f2DepthSlopeScaledBias);\n"
"#else\n"
"    float2 f2FilterSize = abs(ShadowAttribs.fFilterWorldSize * SamplingInfo.f3LightSpaceScale.xy * F3NDC_XYZ_TO_UVD_SCALE.xy);\n"
"    return FilterShadowMapVaryingPCF(tex2DShadowMap, tex2DShadowMap_sampler, ShadowAttribs, SamplingInfo, f2DepthSlopeScaledBias, f2FilterSize);\n"
"#endif\n"
"}\n"
"\n"
"\n"
"struct FilteredShadow\n"
"{\n"
"    float fLightAmount;\n"
"    int   iCascadeIdx;\n"
"    float fNextCascadeBlendAmount;\n"
"};\n"
"\n"
"FilteredShadow FilterShadowMap(in ShadowMapAttribs       ShadowAttribs,\n"
"                               in Texture2DArray<float>  tex2DShadowMap,\n"
"                               in SamplerComparisonState tex2DShadowMap_sampler,\n"
"                               in float3                 f3PosInLightViewSpace,\n"
"                               in float3                 f3ddXPosInLightViewSpace,\n"
"                               in float3                 f3ddYPosInLightViewSpace,\n"
"                               in float                  fCameraSpaceZ)\n"
"{\n"
"    CascadeSamplingInfo SamplingInfo = FindCascade(ShadowAttribs, f3PosInLightViewSpace.xyz, fCameraSpaceZ);\n"
"    FilteredShadow Shadow;\n"
"    Shadow.iCascadeIdx             = SamplingInfo.iCascadeIdx;\n"
"    Shadow.fNextCascadeBlendAmount = 0.0;\n"
"    Shadow.fLightAmount            = 1.0;\n"
"\n"
"    if (SamplingInfo.iCascadeIdx == ShadowAttribs.iNumCascades)\n"
"        return Shadow;\n"
"\n"
"    Shadow.fLightAmount = FilterShadowCascade(ShadowAttribs, tex2DShadowMap, tex2DShadowMap_sampler, f3ddXPosInLightViewSpace, f3ddYPosInLightViewSpace, SamplingInfo);\n"
"\n"
"#if FILTER_ACROSS_CASCADES\n"
"    if (SamplingInfo.iCascadeIdx+1 < ShadowAttribs.iNumCascades)\n"
"    {\n"
"        CascadeSamplingInfo NextCscdSamplingInfo = GetCascadeSamplingInfo(ShadowAttribs, f3PosInLightViewSpace, SamplingInfo.iCascadeIdx + 1);\n"
"        Shadow.fNextCascadeBlendAmount = GetNextCascadeBlendAmount(ShadowAttribs, fCameraSpaceZ, SamplingInfo, NextCscdSamplingInfo);\n"
"        float NextCascadeShadow = 1.0;\n"
"        if (Shadow.fNextCascadeBlendAmount > 0.0)\n"
"        {\n"
"            NextCascadeShadow = FilterShadowCascade(ShadowAttribs, tex2DShadowMap, tex2DShadowMap_sampler, f3ddXPosInLightViewSpace, f3ddYPosInLightViewSpace, NextCscdSamplingInfo);\n"
"        }\n"
"        Shadow.fLightAmount = lerp(Shadow.fLightAmount, NextCascadeShadow, Shadow.fNextCascadeBlendAmount);\n"
"    }\n"
"#endif\n"
"\n"
"    return Shadow;\n"
"}\n"
"\n"
"\n"
"\n"
"\n"
"// Reduces VSM light bleedning\n"
"float ReduceLightBleeding(float pMax, float amount)\n"
"{\n"
"    // Remove the [0, amount] tail and linearly rescale (amount, 1].\n"
"     return saturate((pMax - amount) / (1.0 - amount));\n"
"}\n"
"\n"
"float ChebyshevUpperBound(float2 f2Moments, float fMean, float fMinVariance, float fLightBleedingReduction)\n"
"{\n"
"    float Variance = f2Moments.y - (f2Moments.x * f2Moments.x);\n"
"    Variance = max(Variance, fMinVariance);\n"
"\n"
"    // Probabilistic upper bound\n"
"    float d = fMean - f2Moments.x;\n"
"    float pMax = Variance / (Variance + (d * d));\n"
"\n"
"    pMax = ReduceLightBleeding(pMax, fLightBleedingReduction);\n"
"\n"
"    // One-tailed Chebyshev\n"
"    return (fMean <= f2Moments.x ? 1.0 : pMax);\n"
"}\n"
"\n"
"float2 GetEVSMExponents(float positiveExponent, float negativeExponent, bool Is32BitFormat)\n"
"{\n"
"    float maxExponent = Is32BitFormat ? 42.0 : 5.54;\n"
"    // Clamp to maximum range of fp32/fp16 to prevent overflow/underflow\n"
"    return min(float2(positiveExponent, negativeExponent), float2(maxExponent, maxExponent));\n"
"}\n"
"\n"
"// Applies exponential warp to shadow map depth, input depth should be in [0, 1]\n"
"float2 WarpDepthEVSM(float depth, float2 exponents)\n"
"{\n"
"    // Rescale depth into [-1, 1]\n"
"    depth = 2.0 * depth - 1.0;\n"
"    float pos =  exp( exponents.x * depth);\n"
"    float neg = -exp(-exponents.y * depth);\n"
"    return float2(pos, neg);\n"
"}\n"
"\n"
"float SampleVSM(in ShadowMapAttribs       ShadowAttribs,\n"
"                in Texture2DArray<float4> tex2DVSM,\n"
"                in SamplerState           tex2DVSM_sampler,\n"
"                in CascadeSamplingInfo    SamplingInfo,\n"
"                in float2                 f2ddXShadowMapUV,\n"
"                in float2                 f2ddYShadowMapUV)\n"
"{\n"
"    float2 f2Occluder = tex2DVSM.SampleGrad(tex2DVSM_sampler, float3(SamplingInfo.f2UV, SamplingInfo.iCascadeIdx), f2ddXShadowMapUV, f2ddYShadowMapUV).xy;\n"
"    return ChebyshevUpperBound(f2Occluder, SamplingInfo.fDepth, ShadowAttribs.fVSMBias, ShadowAttribs.fVSMLightBleedingReduction);\n"
"}\n"
"\n"
"float SampleEVSM(in ShadowMapAttribs       ShadowAttribs,\n"
"                 in Texture2DArray<float4> tex2DEVSM,\n"
"                 in SamplerState           tex2DEVSM_sampler,\n"
"                 in CascadeSamplingInfo    SamplingInfo,\n"
"                 in float2                 f2ddXShadowMapUV,\n"
"                 in float2                 f2ddYShadowMapUV)\n"
"{\n"
"    float2 f2Exponents = GetEVSMExponents(ShadowAttribs.fEVSMPositiveExponent, ShadowAttribs.fEVSMNegativeExponent, ShadowAttribs.bIs32BitEVSM);\n"
"    float2 f2WarpedDepth = WarpDepthEVSM(SamplingInfo.fDepth, f2Exponents);\n"
"\n"
"    float4 f4Occluder = tex2DEVSM.SampleGrad(tex2DEVSM_sampler, float3(SamplingInfo.f2UV, SamplingInfo.iCascadeIdx), f2ddXShadowMapUV, f2ddYShadowMapUV);\n"
"\n"
"    float2 f2DepthScale  = ShadowAttribs.fVSMBias * f2Exponents * f2WarpedDepth;\n"
"    float2 f2MinVariance = f2DepthScale * f2DepthScale;\n"
"\n"
"    float fContrib = ChebyshevUpperBound(f4Occluder.xy, f2WarpedDepth.x, f2MinVariance.x, ShadowAttribs.fVSMLightBleedingReduction);\n"
"    #if SHADOW_MODE == SHADOW_MODE_EVSM4\n"
"        float fNegContrib = ChebyshevUpperBound(f4Occluder.zw, f2WarpedDepth.y, f2MinVariance.y, ShadowAttribs.fVSMLightBleedingReduction);\n"
"        fContrib = min(fContrib, fNegContrib);\n"
"    #endif\n"
"\n"
"    return fContrib;\n"
"}\n"
"\n"
"float SampleFilterableShadowCascade(in ShadowMapAttribs       ShadowAttribs,\n"
"                                    in Texture2DArray<float4> tex2DShadowMap,\n"
"                                    in SamplerState           tex2DShadowMap_sampler,\n"
"                                    in float3                 f3ddXPosInLightViewSpace,\n"
"                                    in float3                 f3ddYPosInLightViewSpace,\n"
"                                    in CascadeSamplingInfo    SamplingInfo)\n"
"{\n"
"    float3 f3ddXShadowMapUVDepth = f3ddXPosInLightViewSpace * SamplingInfo.f3LightSpaceScale * F3NDC_XYZ_TO_UVD_SCALE;\n"
"    float3 f3ddYShadowMapUVDepth = f3ddYPosInLightViewSpace * SamplingInfo.f3LightSpaceScale * F3NDC_XYZ_TO_UVD_SCALE;\n"
"#if SHADOW_MODE == SHADOW_MODE_VSM\n"
"    return SampleVSM(ShadowAttribs, tex2DShadowMap, tex2DShadowMap_sampler, SamplingInfo, f3ddXShadowMapUVDepth.xy, f3ddYShadowMapUVDepth.xy);\n"
"#elif SHADOW_MODE == SHADOW_MODE_EVSM2 || SHADOW_MODE == SHADOW_MODE_EVSM4\n"
"    return SampleEVSM(ShadowAttribs, tex2DShadowMap, tex2DShadowMap_sampler, SamplingInfo, f3ddXShadowMapUVDepth.xy, f3ddYShadowMapUVDepth.xy);\n"
"#else\n"
"    return 1.0;\n"
"#endif\n"
"}\n"
"\n"
"FilteredShadow SampleFilterableShadowMap(in ShadowMapAttribs       ShadowAttribs,\n"
"                                         in Texture2DArray<float4> tex2DShadowMap,\n"
"                                         in SamplerState           tex2DShadowMap_sampler,\n"
"                                         in float3                 f3PosInLightViewSpace,\n"
"                                         in float3                 f3ddXPosInLightViewSpace,\n"
"                                         in float3                 f3ddYPosInLightViewSpace,\n"
"                                         in float                  fCameraSpaceZ)\n"
"{\n"
"    CascadeSamplingInfo SamplingInfo = FindCascade(ShadowAttribs, f3PosInLightViewSpace.xyz, fCameraSpaceZ);\n"
"    FilteredShadow Shadow;\n"
"    Shadow.iCascadeIdx             = SamplingInfo.iCascadeIdx;\n"
"    Shadow.fNextCascadeBlendAmount = 0.0;\n"
"    Shadow.fLightAmount            = 1.0;\n"
"\n"
"    if (SamplingInfo.iCascadeIdx == ShadowAttribs.iNumCascades)\n"
"        return Shadow;\n"
"\n"
"    Shadow.fLightAmount = SampleFilterableShadowCascade(ShadowAttribs, tex2DShadowMap, tex2DShadowMap_sampler, f3ddXPosInLightViewSpace, f3ddYPosInLightViewSpace, SamplingInfo);\n"
"\n"
"#if FILTER_ACROSS_CASCADES\n"
"    if (SamplingInfo.iCascadeIdx+1 < ShadowAttribs.iNumCascades)\n"
"    {\n"
"        CascadeSamplingInfo NextCscdSamplingInfo = GetCascadeSamplingInfo(ShadowAttribs, f3PosInLightViewSpace, SamplingInfo.iCascadeIdx + 1);\n"
"        Shadow.fNextCascadeBlendAmount = GetNextCascadeBlendAmount(ShadowAttribs, fCameraSpaceZ, SamplingInfo, NextCscdSamplingInfo);\n"
"        float NextCascadeShadow = 1.0;\n"
"        if (Shadow.fNextCascadeBlendAmount > 0.0)\n"
"        {\n"
"            NextCascadeShadow = SampleFilterableShadowCascade(ShadowAttribs, tex2DShadowMap, tex2DShadowMap_sampler, f3ddXPosInLightViewSpace, f3ddYPosInLightViewSpace, NextCscdSamplingInfo);\n"
"        }\n"
"        Shadow.fLightAmount = lerp(Shadow.fLightAmount, NextCascadeShadow, Shadow.fNextCascadeBlendAmount);\n"
"    }\n"
"#endif\n"
"\n"
"    return Shadow;\n"
"}\n"
"\n"
"\n"
"\n"
"\n"
"float3 GetCascadeColor(FilteredShadow Shadow)\n"
"{\n"
"    float3 f3CascadeColors[MAX_CASCADES];\n"
"    f3CascadeColors[0] = float3(0,1,0);\n"
"    f3CascadeColors[1] = float3(0,0,1);\n"
"    f3CascadeColors[2] = float3(1,1,0);\n"
"    f3CascadeColors[3] = float3(0,1,1);\n"
"    f3CascadeColors[4] = float3(1,0,1);\n"
"    f3CascadeColors[5] = float3(0.3, 1, 0.7);\n"
"    f3CascadeColors[6] = float3(0.7, 0.3,1);\n"
"    f3CascadeColors[7] = float3(1.0, 0.7, 0.3);\n"
"    float3 Color = f3CascadeColors[min(Shadow.iCascadeIdx, MAX_CASCADES-1)];\n"
"#if FILTER_ACROSS_CASCADES\n"
"    float3 NextCascadeColor = f3CascadeColors[min(Shadow.iCascadeIdx+1, MAX_CASCADES-1)];\n"
"    Color = lerp(Color, NextCascadeColor, Shadow.fNextCascadeBlendAmount);\n"
"#endif\n"
"    return Color;\n"
"}\n"
"\n"
"#endif //_SHADOWS_FXH_\n"
