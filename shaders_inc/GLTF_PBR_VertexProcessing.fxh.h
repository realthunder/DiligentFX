"#ifndef _GLTF_PBR_VERTEX_PROCESSING_FXH_\n"
"#define _GLTF_PBR_VERTEX_PROCESSING_FXH_\n"
"\n"
"#include \"GLTF_PBR_Structures.fxh\"\n"
"\n"
"struct GLTF_TransformedVertex\n"
"{\n"
"    float3 WorldPos;\n"
"    float3 Normal;\n"
"};\n"
"\n"
"\n"
"float3x3 InverseTranspose3x3(float3x3 M)\n"
"{\n"
"    // Note that in HLSL, M_t[0] is the first row, while in GLSL, it is the\n"
"    // first column. Luckily, determinant and inverse matrix can be equally\n"
"    // defined through both rows and columns.\n"
"    float det = dot(cross(M[0], M[1]), M[2]);\n"
"    float3x3 adjugate = float3x3(cross(M[1], M[2]),\n"
"                                 cross(M[2], M[0]),\n"
"                                 cross(M[0], M[1]));\n"
"    return adjugate / det;\n"
"}\n"
"\n"
"GLTF_TransformedVertex GLTF_TransformVertex(in float3    Pos,\n"
"                                            in float3    Normal,\n"
"                                            in float4x4  Transform)\n"
"{\n"
"    GLTF_TransformedVertex TransformedVert;\n"
"\n"
"    float4 locPos = mul(Transform, float4(Pos, 1.0));\n"
"    float3x3 NormalTransform = float3x3(Transform[0].xyz, Transform[1].xyz, Transform[2].xyz);\n"
"    NormalTransform = InverseTranspose3x3(NormalTransform);\n"
"    Normal = mul(NormalTransform, Normal);\n"
"    float NormalLen = length(Normal);\n"
"    TransformedVert.Normal = Normal / max(NormalLen, 1e-5);\n"
"\n"
"    TransformedVert.WorldPos = locPos.xyz / locPos.w;\n"
"\n"
"    return TransformedVert;\n"
"}\n"
"\n"
"#endif // _GLTF_PBR_VERTEX_PROCESSING_FXH_\n"
