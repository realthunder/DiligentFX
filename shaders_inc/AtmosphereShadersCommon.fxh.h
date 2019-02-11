"#ifndef _ATMOSPHERE_SHADERS_COMMON_FXH_\n"
"#define _ATMOSPHERE_SHADERS_COMMON_FXH_\n"
"\n"
"#include \"Structures.fxh\"\n"
"\n"
"#define FLT_MAX 3.402823466e+38f\n"
"\n"
"#define F4ZERO float4(0.0, 0.0, 0.0, 0.0)\n"
"#define F4ONE float4(1.0, 1.0, 1.0, 1.0)\n"
"#define F3ZERO float3(0.0, 0.0, 0.0)\n"
"#define F3ONE float3(1.0, 1.0, 1.0)\n"
"#define F2ZERO float2(0.0, 0.0)\n"
"#define F2ONE float2(1.0, 1.0)\n"
"\n"
"#ifndef NUM_EPIPOLAR_SLICES\n"
"#   define NUM_EPIPOLAR_SLICES 1024\n"
"#endif\n"
"\n"
"#ifndef MAX_SAMPLES_IN_SLICE\n"
"#   define MAX_SAMPLES_IN_SLICE 512\n"
"#endif\n"
"\n"
"#ifndef SCREEN_RESLOUTION\n"
"#   define SCREEN_RESLOUTION float2(1024.0, 768.0)\n"
"#endif\n"
"\n"
"#ifndef OPTIMIZE_SAMPLE_LOCATIONS\n"
"#   define OPTIMIZE_SAMPLE_LOCATIONS 1\n"
"#endif\n"
"\n"
"#ifndef CORRECT_INSCATTERING_AT_DEPTH_BREAKS\n"
"#   define CORRECT_INSCATTERING_AT_DEPTH_BREAKS 0\n"
"#endif\n"
"\n"
"//#define SHADOW_MAP_DEPTH_BIAS 1e-4\n"
"\n"
"#ifndef TRAPEZOIDAL_INTEGRATION\n"
"#   define TRAPEZOIDAL_INTEGRATION 1\n"
"#endif\n"
"\n"
"#ifndef EARTH_RADIUS\n"
"#   define EARTH_RADIUS 6360000.0\n"
"#endif\n"
"\n"
"#ifndef ATM_TOP_HEIGHT\n"
"#   define ATM_TOP_HEIGHT 80000.0\n"
"#endif\n"
"\n"
"#ifndef ATM_TOP_RADIUS\n"
"#   define ATM_TOP_RADIUS (EARTH_RADIUS+ATM_TOP_HEIGHT)\n"
"#endif\n"
"\n"
"#ifndef PARTICLE_SCALE_HEIGHT\n"
"#   define PARTICLE_SCALE_HEIGHT float2(7994.0, 1200.0)\n"
"#endif\n"
"\n"
"#ifndef ENABLE_LIGHT_SHAFTS\n"
"#   define ENABLE_LIGHT_SHAFTS 1\n"
"#endif\n"
"\n"
"#ifndef USE_1D_MIN_MAX_TREE\n"
"#   define USE_1D_MIN_MAX_TREE 1\n"
"#endif\n"
"\n"
"#ifndef IS_32BIT_MIN_MAX_MAP\n"
"#   define IS_32BIT_MIN_MAX_MAP 0\n"
"#endif\n"
"\n"
"#ifndef SINGLE_SCATTERING_MODE\n"
"#   define SINGLE_SCATTERING_MODE SINGLE_SCTR_MODE_INTEGRATION\n"
"#endif\n"
"\n"
"#ifndef MULTIPLE_SCATTERING_MODE\n"
"#   define MULTIPLE_SCATTERING_MODE MULTIPLE_SCTR_MODE_NONE\n"
"#endif\n"
"\n"
"#ifndef PRECOMPUTED_SCTR_LUT_DIM\n"
"#   define PRECOMPUTED_SCTR_LUT_DIM float4(32.0, 128.0, 32.0, 16.0)\n"
"#endif\n"
"\n"
"#ifndef NUM_RANDOM_SPHERE_SAMPLES\n"
"#   define NUM_RANDOM_SPHERE_SAMPLES 128\n"
"#endif\n"
"\n"
"#ifndef PERFORM_TONE_MAPPING\n"
"#   define PERFORM_TONE_MAPPING 1\n"
"#endif\n"
"\n"
"#ifndef LOW_RES_LUMINANCE_MIPS\n"
"#   define LOW_RES_LUMINANCE_MIPS 7\n"
"#endif\n"
"\n"
"#ifndef TONE_MAPPING_MODE\n"
"#   define TONE_MAPPING_MODE TONE_MAPPING_MODE_REINHARD_MOD\n"
"#endif\n"
"\n"
"#ifndef LIGHT_ADAPTATION\n"
"#   define LIGHT_ADAPTATION 1\n"
"#endif\n"
"\n"
"#ifndef CASCADE_PROCESSING_MODE\n"
"#   define CASCADE_PROCESSING_MODE CASCADE_PROCESSING_MODE_SINGLE_PASS\n"
"#endif\n"
"\n"
"#ifndef USE_COMBINED_MIN_MAX_TEXTURE\n"
"#   define USE_COMBINED_MIN_MAX_TEXTURE 1\n"
"#endif\n"
"\n"
"#ifndef EXTINCTION_EVAL_MODE\n"
"#   define EXTINCTION_EVAL_MODE EXTINCTION_EVAL_MODE_EPIPOLAR\n"
"#endif\n"
"\n"
"#ifndef AUTO_EXPOSURE\n"
"#   define AUTO_EXPOSURE 1\n"
"#endif\n"
"\n"
"#define INVALID_EPIPOLAR_LINE float4(-1000.0, -1000.0, -100.0, -100.0)\n"
"\n"
"#define RGB_TO_LUMINANCE float3(0.212671, 0.715160, 0.072169)\n"
"\n"
"struct FullScreenTriangleVSOutput\n"
"{\n"
"    float4 f4PixelPos     : SV_Position;   // Pixel position on the screen\n"
"    float2 f2NormalizedXY : NORMALIZED_XY; // Normalized device XY coordinates [-1,1]x[-1,1]\n"
"    float  fInstID        : INSTANCE_ID;\n"
"};\n"
"\n"
"// GLSL compiler is so bad that it cannot properly handle matrices passed as structure members!\n"
"float3 ProjSpaceXYZToWorldSpace(in float3 f3PosPS, in float4x4 mProj, in float4x4 mViewProjInv /*CameraAttribs CamAttribs <- DO NOT DO THIS*/)\n"
"{\n"
"    // We need to compute normalized device z before applying view-proj inverse matrix\n"
"\n"
"    // It does not matter if we are in HLSL or GLSL. The way normalized device\n"
"    // coordinates are computed is the same in both APIs - simply transform by\n"
"    // matrix and then divide by w. Consequently, the inverse transform is also \n"
"    // the same.\n"
"    // What differs is that in GL, NDC z is transformed from [-1,+1] to [0,1]\n"
"    // before storing in the depth buffer\n"
"    float fNDC_Z = MATRIX_ELEMENT(mProj,2,2) + MATRIX_ELEMENT(mProj,3,2) / f3PosPS.z;\n"
"    float4 ReconstructedPosWS = mul( float4(f3PosPS.xy, fNDC_Z, 1.0), mViewProjInv );\n"
"    ReconstructedPosWS /= ReconstructedPosWS.w;\n"
"    return ReconstructedPosWS.xyz;\n"
"}\n"
"\n"
"float3 WorldSpaceToShadowMapUV(in float3 f3PosWS, in matrix mWorldToShadowMapUVDepth)\n"
"{\n"
"    float4 f4ShadowMapUVDepth = mul( float4(f3PosWS, 1), mWorldToShadowMapUVDepth );\n"
"    // Shadow map projection matrix is orthographic, so we do not need to divide by w\n"
"    //f4ShadowMapUVDepth.xyz /= f4ShadowMapUVDepth.w;\n"
"    \n"
"    // Applying depth bias results in light leaking through the opaque objects when looking directly\n"
"    // at the light source\n"
"    return f4ShadowMapUVDepth.xyz;\n"
"}\n"
"\n"
"\n"
"void GetRaySphereIntersection(in float3 f3RayOrigin,\n"
"                              in float3 f3RayDirection,\n"
"                              in float3 f3SphereCenter,\n"
"                              in float fSphereRadius,\n"
"                              out float2 f2Intersections)\n"
"{\n"
"    // http://wiki.cgsociety.org/index.php/Ray_Sphere_Intersection\n"
"    f3RayOrigin -= f3SphereCenter;\n"
"    float A = dot(f3RayDirection, f3RayDirection);\n"
"    float B = 2.0 * dot(f3RayOrigin, f3RayDirection);\n"
"    float C = dot(f3RayOrigin,f3RayOrigin) - fSphereRadius*fSphereRadius;\n"
"    float D = B*B - 4.0*A*C;\n"
"    // If discriminant is negative, there are no real roots hence the ray misses the\n"
"    // sphere\n"
"    if( D<0.0 )\n"
"    {\n"
"        f2Intersections = float2(-1.0, -1.0);\n"
"    }\n"
"    else\n"
"    {\n"
"        D = sqrt(D);\n"
"        f2Intersections = float2(-B - D, -B + D) / (2.0*A); // A must be positive here!!\n"
"    }\n"
"}\n"
"\n"
"void GetRaySphereIntersection2(in float3 f3RayOrigin,\n"
"                               in float3 f3RayDirection,\n"
"                               in float3 f3SphereCenter,\n"
"                               in float2 f2SphereRadius,\n"
"                               out float4 f4Intersections)\n"
"{\n"
"    // http://wiki.cgsociety.org/index.php/Ray_Sphere_Intersection\n"
"    f3RayOrigin -= f3SphereCenter;\n"
"    float A = dot(f3RayDirection, f3RayDirection);\n"
"    float B = 2.0 * dot(f3RayOrigin, f3RayDirection);\n"
"    float2 C = dot(f3RayOrigin,f3RayOrigin) - f2SphereRadius*f2SphereRadius;\n"
"    float2 D = B*B - 4.0*A*C;\n"
"    // If discriminant is negative, there are no real roots hence the ray misses the\n"
"    // sphere\n"
"    float2 f2RealRootMask = float2(D.x >= 0.0 ? 1.0 : 0.0, D.y >= 0.0 ? 1.0 : 0.0);\n"
"    D = sqrt( max(D,0.0) );\n"
"    f4Intersections =   f2RealRootMask.xxyy * float4(-B - D.x, -B + D.x, -B - D.y, -B + D.y) / (2.0*A) + \n"
"                      (F4ONE - f2RealRootMask.xxyy) * float4(-1.0,-1.0,-1.0,-1.0);\n"
"}\n"
"\n"
"\n"
"float4 GetOutermostScreenPixelCoords()\n"
"{\n"
"    // The outermost visible screen pixels centers do not lie exactly on the boundary (+1 or -1), but are biased by\n"
"    // 0.5 screen pixel size inwards\n"
"    //\n"
"    //                                        2.0\n"
"    //    |<---------------------------------------------------------------------->|\n"
"    //\n"
"    //       2.0/Res\n"
"    //    |<--------->|\n"
"    //    |     X     |      X     |     X     |    ...    |     X     |     X     |\n"
"    //   -1     |                                                            |    +1\n"
"    //          |                                                            |\n"
"    //          |                                                            |\n"
"    //      -1 + 1.0/Res                                                  +1 - 1.0/Res\n"
"    //\n"
"    // Using shader macro is much more efficient than using constant buffer variable\n"
"    // because the compiler is able to optimize the code more aggressively\n"
"    // return float4(-1,-1,1,1) + float4(1, 1, -1, -1)/g_PPAttribs.m_f2ScreenResolution.xyxy;\n"
"    return float4(-1.0,-1.0,1.0,1.0) + float4(1.0, 1.0, -1.0, -1.0) / SCREEN_RESLOUTION.xyxy;\n"
"}\n"
"\n"
"\n"
"// When checking if a point is inside the screen, we must test against \n"
"// the biased screen boundaries \n"
"bool IsValidScreenLocation(in float2 f2XY)\n"
"{\n"
"    const float2 SAFETY_EPSILON = float2(0.2, 0.2);\n"
"    return all( LessEqual( abs(f2XY), F2ONE - (F2ONE - SAFETY_EPSILON) / SCREEN_RESLOUTION.xy ) );\n"
"}\n"
"\n"
"\n"
"#endif //_ATMOSPHERE_SHADERS_COMMON_FXH_\n"
