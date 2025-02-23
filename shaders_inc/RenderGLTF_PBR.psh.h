"// PBR shader based on the Khronos WebGL PBR implementation\n"
"// See https://github.com/KhronosGroup/glTF-WebGL-PBR\n"
"// Supports both metallic roughness and specular glossiness inputs\n"
"\n"
"#include \"BasicStructures.fxh\"\n"
"#include \"GLTF_PBR_Shading.fxh\"\n"
"#include \"ToneMapping.fxh\"\n"
"\n"
"#ifndef USE_TEXTURE_ATLAS\n"
"#   define USE_TEXTURE_ATLAS 0\n"
"#endif\n"
"\n"
"#ifndef ALLOW_DEBUG_VIEW\n"
"#   define ALLOW_DEBUG_VIEW 0\n"
"#endif\n"
"\n"
"#if USE_TEXTURE_ATLAS\n"
"#   include \"AtlasSampling.fxh\"\n"
"#endif\n"
"\n"
"cbuffer cbCameraAttribs\n"
"{\n"
"    CameraAttribs g_CameraAttribs;\n"
"}\n"
"\n"
"cbuffer cbLightAttribs\n"
"{\n"
"    LightAttribs g_LightAttribs;\n"
"}\n"
"\n"
"cbuffer cbGLTFAttribs\n"
"{\n"
"    GLTFRendererShaderParameters g_RenderParameters;\n"
"    GLTFMaterialShaderInfo       g_MaterialInfo;\n"
"}\n"
"\n"
"#if GLTF_PBR_USE_IBL\n"
"TextureCube  g_IrradianceMap;\n"
"SamplerState g_IrradianceMap_sampler;\n"
"\n"
"TextureCube  g_PrefilteredEnvMap;\n"
"SamplerState g_PrefilteredEnvMap_sampler;\n"
"\n"
"Texture2D     g_BRDF_LUT;\n"
"SamplerState  g_BRDF_LUT_sampler;\n"
"#endif\n"
"\n"
"Texture2DArray g_ColorMap;\n"
"SamplerState   g_ColorMap_sampler;\n"
"\n"
"Texture2DArray g_PhysicalDescriptorMap;\n"
"SamplerState   g_PhysicalDescriptorMap_sampler;\n"
"\n"
"Texture2DArray g_NormalMap;\n"
"SamplerState   g_NormalMap_sampler;\n"
"\n"
"#if GLTF_PBR_USE_AO\n"
"Texture2DArray g_AOMap;\n"
"SamplerState   g_AOMap_sampler;\n"
"#endif\n"
"\n"
"#if GLTF_PBR_USE_EMISSIVE\n"
"Texture2DArray g_EmissiveMap;\n"
"SamplerState   g_EmissiveMap_sampler;\n"
"#endif\n"
"\n"
"float4 SampleGLTFTexture(Texture2DArray Tex,\n"
"                         SamplerState   Tex_sampler,\n"
"                         float2         UV0,\n"
"                         float2         UV1,\n"
"                         float          Selector,\n"
"                         float4         ScaleBias,\n"
"                         float          Slice,\n"
"                         float4         DefaultValue)\n"
"{\n"
"    float2 UV = lerp(UV0, UV1, Selector);\n"
"#if USE_TEXTURE_ATLAS\n"
"    if (Selector < 0.0)\n"
"    {\n"
"        return DefaultValue;\n"
"    }\n"
"    else\n"
"    {\n"
"        SampleTextureAtlasAttribs SampleAttribs;\n"
"        SampleAttribs.f2UV                   = frac(UV) * ScaleBias.xy + ScaleBias.zw;\n"
"        SampleAttribs.f2SmoothUV             = UV * ScaleBias.xy;\n"
"        SampleAttribs.f2dSmoothUV_dx         = ddx(UV) * ScaleBias.xy;\n"
"        SampleAttribs.f2dSmoothUV_dy         = ddy(UV) * ScaleBias.xy;\n"
"        SampleAttribs.fSlice                 = Slice;\n"
"        SampleAttribs.f4UVRegion             = ScaleBias;\n"
"        SampleAttribs.fSmallestValidLevelDim = 4.0;\n"
"        SampleAttribs.IsNonFilterable        = false;\n"
"        return SampleTextureAtlas(Tex, Tex_sampler, SampleAttribs);\n"
"    }\n"
"#else\n"
"    return Tex.Sample(Tex_sampler, float3(UV, Slice));\n"
"#endif\n"
"}\n"
"\n"
"void main(in  float4 ClipPos     : SV_Position,\n"
"          in  float3 WorldPos    : WORLD_POS,\n"
"          in  float3 Normal      : NORMAL,\n"
"          in  float2 UV0         : UV0,\n"
"          in  float2 UV1         : UV1,\n"
"          in  bool   IsFrontFace : SV_IsFrontFace,\n"
"          out float4 OutColor    : SV_Target)\n"
"{\n"
"    float4 BaseColor = SampleGLTFTexture(g_ColorMap, g_ColorMap_sampler, UV0, UV1, g_MaterialInfo.BaseColorTextureUVSelector,\n"
"                                         g_MaterialInfo.BaseColorUVScaleBias, g_MaterialInfo.BaseColorSlice, float4(1.0, 1.0, 1.0, 1.0));\n"
"    BaseColor = SRGBtoLINEAR(BaseColor) * g_MaterialInfo.BaseColorFactor;\n"
"    //BaseColor *= getVertexColor();\n"
"\n"
"    float2 NormalMapUV  = lerp(UV0, UV1, g_MaterialInfo.NormalTextureUVSelector);\n"
"\n"
"    // We have to compute gradients in uniform flow control to avoid issues with perturbed normal\n"
"    float3 dWorldPos_dx = ddx(WorldPos);\n"
"    float3 dWorldPos_dy = ddy(WorldPos);\n"
"    float2 dNormalMapUV_dx = ddx(NormalMapUV);\n"
"    float2 dNormalMapUV_dy = ddy(NormalMapUV);\n"
"#if USE_TEXTURE_ATLAS\n"
"    {\n"
"        NormalMapUV = frac(NormalMapUV);\n"
"        NormalMapUV = NormalMapUV * g_MaterialInfo.NormalMapUVScaleBias.xy + g_MaterialInfo.NormalMapUVScaleBias.zw;\n"
"        dNormalMapUV_dx *= g_MaterialInfo.NormalMapUVScaleBias.xy;\n"
"        dNormalMapUV_dy *= g_MaterialInfo.NormalMapUVScaleBias.xy;\n"
"    }\n"
"#endif\n"
"\n"
"    if (g_MaterialInfo.AlphaMode == GLTF_ALPHA_MODE_MASK && BaseColor.a < g_MaterialInfo.AlphaMaskCutoff)\n"
"    {\n"
"        discard;\n"
"    }\n"
"\n"
"    float3 TSNormal = float3(0.0, 0.0, 1.0);\n"
"    if (g_MaterialInfo.NormalTextureUVSelector >= 0.0)\n"
"    {\n"
"#if USE_TEXTURE_ATLAS\n"
"        {\n"
"            SampleTextureAtlasAttribs SampleAttribs;\n"
"            SampleAttribs.f2UV                   = NormalMapUV;\n"
"            SampleAttribs.f2SmoothUV             = lerp(UV0, UV1, g_MaterialInfo.NormalTextureUVSelector) * g_MaterialInfo.NormalMapUVScaleBias.xy;\n"
"            SampleAttribs.f2dSmoothUV_dx         = dNormalMapUV_dx;\n"
"            SampleAttribs.f2dSmoothUV_dy         = dNormalMapUV_dy;\n"
"            SampleAttribs.fSlice                 = g_MaterialInfo.NormalSlice;\n"
"            SampleAttribs.f4UVRegion             = g_MaterialInfo.NormalMapUVScaleBias;\n"
"            SampleAttribs.fSmallestValidLevelDim = 4.0;\n"
"            SampleAttribs.IsNonFilterable        = false;\n"
"            TSNormal = SampleTextureAtlas(g_NormalMap, g_NormalMap_sampler, SampleAttribs).xyz;\n"
"        }\n"
"#else\n"
"        {\n"
"            TSNormal = g_NormalMap.Sample(g_NormalMap_sampler, float3(NormalMapUV, g_MaterialInfo.NormalSlice)).xyz;\n"
"        }\n"
"#endif\n"
"        TSNormal = TSNormal * float3(2.0, 2.0, 2.0) - float3(1.0, 1.0, 1.0);\n"
"    }\n"
"\n"
"    float Occlusion = 1.0;\n"
"#if GLTF_PBR_USE_AO\n"
"    Occlusion = SampleGLTFTexture(g_AOMap, g_AOMap_sampler, UV0, UV1, g_MaterialInfo.OcclusionTextureUVSelector,\n"
"                                  g_MaterialInfo.OcclusionUVScaleBias, g_MaterialInfo.OcclusionSlice, float4(1.0, 1.0, 1.0, 1.0)).r;\n"
"#endif\n"
"\n"
"    float3 Emissive = float3(0.0, 0.0, 0.0);\n"
"#if GLTF_PBR_USE_EMISSIVE\n"
"    Emissive = SampleGLTFTexture(g_EmissiveMap, g_EmissiveMap_sampler, UV0, UV1, g_MaterialInfo.EmissiveTextureUVSelector,\n"
"                                 g_MaterialInfo.EmissiveUVScaleBias, g_MaterialInfo.EmissiveSlice, float4(0.0, 0.0, 0.0, 0.0)).rgb;\n"
"#endif\n"
"\n"
"    float4 PhysicalDesc = SampleGLTFTexture(g_PhysicalDescriptorMap, g_PhysicalDescriptorMap_sampler,\n"
"                                            UV0, UV1, g_MaterialInfo.PhysicalDescriptorTextureUVSelector,\n"
"                                            g_MaterialInfo.PhysicalDescriptorUVScaleBias, g_MaterialInfo.PhysicalDescriptorSlice,\n"
"                                             float4(0.0, 1.0, 0.0, 0.0));\n"
"\n"
"    float metallic;\n"
"    if (g_MaterialInfo.Workflow == PBR_WORKFLOW_SPECULAR_GLOSINESS)\n"
"    {\n"
"        PhysicalDesc.rgb = SRGBtoLINEAR(PhysicalDesc.rgb) * g_MaterialInfo.SpecularFactor.rgb;\n"
"        const float u_GlossinessFactor = 1.0;\n"
"        PhysicalDesc.a *= u_GlossinessFactor;\n"
"    }\n"
"    else if(g_MaterialInfo.Workflow == PBR_WORKFLOW_METALLIC_ROUGHNESS)\n"
"    {\n"
"        PhysicalDesc.g = saturate(PhysicalDesc.g * g_MaterialInfo.RoughnessFactor);\n"
"        PhysicalDesc.b = saturate(PhysicalDesc.b * g_MaterialInfo.MetallicFactor);\n"
"    }\n"
"    SurfaceReflectanceInfo SrfInfo = GLTF_PBR_GetSurfaceReflectance(g_MaterialInfo.Workflow, BaseColor, PhysicalDesc, metallic);\n"
"\n"
"    // LIGHTING\n"
"    float3 perturbedNormal = GLTF_PBR_PerturbNormal(dWorldPos_dx, dWorldPos_dy, dNormalMapUV_dx, dNormalMapUV_dy,\n"
"                                                    Normal, TSNormal, g_MaterialInfo.NormalTextureUVSelector >= 0.0, IsFrontFace);\n"
"    float3 view = normalize(g_CameraAttribs.f4Position.xyz - WorldPos.xyz); // Direction from surface point to camera\n"
"\n"
"    float3 color = float3(0.0, 0.0, 0.0);\n"
"    color += GLTF_PBR_ApplyDirectionalLight(g_LightAttribs.f4Direction.xyz, g_LightAttribs.f4Intensity.rgb, SrfInfo, perturbedNormal, view);\n"
"\n"
"//#ifdef USE_PUNCTUAL\n"
"//    for (int i = 0; i < LIGHT_COUNT; ++i)\n"
"//    {\n"
"//        Light light = u_Lights[i];\n"
"//        if (light.type == LightType_Directional)\n"
"//        {\n"
"//            color += applyDirectionalLight(light, materialInfo, normal, view);\n"
"//        }\n"
"//        else if (light.type == LightType_Point)\n"
"//        {\n"
"//            color += applyPointLight(light, materialInfo, normal, view);\n"
"//        }\n"
"//        else if (light.type == LightType_Spot)\n"
"//        {\n"
"//            color += applySpotLight(light, materialInfo, normal, view);\n"
"//        }\n"
"//    }\n"
"//#endif\n"
"//\n"
"\n"
"\n"
"    // Calculate lighting contribution from image based lighting source (IBL)\n"
"    GLTF_PBR_IBL_Contribution IBLContrib;\n"
"    IBLContrib.f3Diffuse  = float3(0.0, 0.0, 0.0);\n"
"    IBLContrib.f3Specular = float3(0.0, 0.0, 0.0);\n"
"#if GLTF_PBR_USE_IBL\n"
"    IBLContrib =\n"
"        GLTF_PBR_GetIBLContribution(SrfInfo, perturbedNormal, view, float(g_RenderParameters.PrefilteredCubeMipLevels),\n"
"                           g_BRDF_LUT,          g_BRDF_LUT_sampler,\n"
"                           g_IrradianceMap,     g_IrradianceMap_sampler,\n"
"                           g_PrefilteredEnvMap, g_PrefilteredEnvMap_sampler);\n"
"    color += (IBLContrib.f3Diffuse + IBLContrib.f3Specular) * g_RenderParameters.IBLScale;\n"
"#endif\n"
"\n"
"#if GLTF_PBR_USE_AO\n"
"    color = lerp(color, color * Occlusion, g_RenderParameters.OcclusionStrength);\n"
"#endif\n"
"\n"
"#if GLTF_PBR_USE_EMISSIVE\n"
"    const float u_EmissiveFactor = 1.0;\n"
"    Emissive = SRGBtoLINEAR(Emissive);\n"
"    color += Emissive.rgb * g_MaterialInfo.EmissiveFactor.rgb * g_RenderParameters.EmissionScale;\n"
"#endif\n"
"\n"
"    ToneMappingAttribs TMAttribs;\n"
"    TMAttribs.iToneMappingMode     = TONE_MAPPING_MODE_UNCHARTED2;\n"
"    TMAttribs.bAutoExposure        = false;\n"
"    TMAttribs.fMiddleGray          = g_RenderParameters.MiddleGray;\n"
"    TMAttribs.bLightAdaptation     = false;\n"
"    TMAttribs.fWhitePoint          = g_RenderParameters.WhitePoint;\n"
"    TMAttribs.fLuminanceSaturation = 1.0;\n"
"    color = ToneMap(color, TMAttribs, g_RenderParameters.AverageLogLum);\n"
"    OutColor = float4(color, BaseColor.a);\n"
"\n"
"#if ALLOW_DEBUG_VIEW\n"
"    // Shader inputs debug visualization\n"
"    if (g_RenderParameters.DebugViewType != 0)\n"
"    {\n"
"        switch (g_RenderParameters.DebugViewType)\n"
"        {\n"
"            case  1: OutColor.rgba = BaseColor;                                                         break;\n"
"            case  2: OutColor.rgba = float4(BaseColor.a, BaseColor.a, BaseColor.a, 1.0);                break;\n"
"            // Apply extra srgb->linear transform to make the maps look better\n"
"            case  3: OutColor.rgb  = SRGBtoLINEAR(TSNormal.xyz);                                        break;\n"
"            case  4: OutColor.rgb  = SRGBtoLINEAR(Occlusion * float3(1.0, 1.0, 1.0));                   break;\n"
"            case  5: OutColor.rgb  = SRGBtoLINEAR(Emissive.rgb);                                        break;\n"
"            case  6: OutColor.rgb  = SRGBtoLINEAR(metallic * float3(1.0, 1.0, 1.0) );                   break;\n"
"            case  7: OutColor.rgb  = SRGBtoLINEAR(SrfInfo.PerceptualRoughness * float3(1.0, 1.0, 1.0)); break;\n"
"            case  8: OutColor.rgb  = SrfInfo.DiffuseColor;                                              break;\n"
"            case  9: OutColor.rgb  = SrfInfo.Reflectance0;                                              break;\n"
"            case 10: OutColor.rgb  = SrfInfo.Reflectance90;                                             break;\n"
"            case 11: OutColor.rgb  = SRGBtoLINEAR(abs(Normal / max(length(Normal), 1e-3)));             break;\n"
"            case 12: OutColor.rgb  = SRGBtoLINEAR(abs(perturbedNormal));                                break;\n"
"            case 13: OutColor.rgb  = dot(perturbedNormal, view) * float3(1.0, 1.0, 1.0);                break;\n"
"#if GLTF_PBR_USE_IBL\n"
"            case 14: OutColor.rgb  = IBLContrib.f3Diffuse;                                              break;\n"
"            case 15: OutColor.rgb  = IBLContrib.f3Specular;                                             break;\n"
"#endif\n"
"        }\n"
"    }\n"
"#endif\n"
"\n"
"}\n"
