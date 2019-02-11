struct ShaderIncInfo
{
    const char* const FileName;
    const char* const Source;
};

static const ShaderIncInfo g_Shaders[] =
{
    {
        "AtmosphereShadersCommon.fxh",
        #include "AtmosphereShadersCommon.fxh.h"
    },
    {
        "CoarseInsctr.fx",
        #include "CoarseInsctr.fx.h"
    },
    {
        "ComputeMinMaxShadowMapLevel.fx",
        #include "ComputeMinMaxShadowMapLevel.fx.h"
    },
    {
        "FullScreenTriangleVS.fx",
        #include "FullScreenTriangleVS.fx.h"
    },
    {
        "InitializeMinMaxShadowMap.fx",
        #include "InitializeMinMaxShadowMap.fx.h"
    },
    {
        "InterpolateIrradiance.fx",
        #include "InterpolateIrradiance.fx.h"
    },
    {
        "LookUpTables.fxh",
        #include "LookUpTables.fxh.h"
    },
    {
        "MarkRayMarchingSamples.fx",
        #include "MarkRayMarchingSamples.fx.h"
    },
    {
        "RayMarch.fx",
        #include "RayMarch.fx.h"
    },
    {
        "ReconstructCameraSpaceZ.fx",
        #include "ReconstructCameraSpaceZ.fx.h"
    },
    {
        "RefineSampleLocations.fx",
        #include "RefineSampleLocations.fx.h"
    },
    {
        "RenderCoordinateTexture.fx",
        #include "RenderCoordinateTexture.fx.h"
    },
    {
        "RenderSampling.fx",
        #include "RenderSampling.fx.h"
    },
    {
        "RenderSliceEndPoints.fx",
        #include "RenderSliceEndPoints.fx.h"
    },
    {
        "ScatteringIntegrals.fxh",
        #include "ScatteringIntegrals.fxh.h"
    },
    {
        "SliceUVDirection.fx",
        #include "SliceUVDirection.fx.h"
    },
    {
        "Sun.fx",
        #include "Sun.fx.h"
    },
    {
        "ToneMapping.fxh",
        #include "ToneMapping.fxh.h"
    },
    {
        "UnshadowedScattering.fxh",
        #include "UnshadowedScattering.fxh.h"
    },
    {
        "UnwarpEpipolarScattering.fx",
        #include "UnwarpEpipolarScattering.fx.h"
    },
    {
        "UpdateAverageLuminance.fx",
        #include "UpdateAverageLuminance.fx.h"
    },
    {
        "CombineScatteringOrders.fx",
        #include "CombineScatteringOrders.fx.h"
    },
    {
        "ComputeScatteringOrder.fx",
        #include "ComputeScatteringOrder.fx.h"
    },
    {
        "ComputeSctrRadiance.fx",
        #include "ComputeSctrRadiance.fx.h"
    },
    {
        "InitHighOrderScattering.fx",
        #include "InitHighOrderScattering.fx.h"
    },
    {
        "PrecomputeAmbientSkyLight.fx",
        #include "PrecomputeAmbientSkyLight.fx.h"
    },
    {
        "PrecomputeCommon.fxh",
        #include "PrecomputeCommon.fxh.h"
    },
    {
        "PrecomputeNetDensityToAtmTop.fx",
        #include "PrecomputeNetDensityToAtmTop.fx.h"
    },
    {
        "PrecomputeSingleScattering.fx",
        #include "PrecomputeSingleScattering.fx.h"
    },
    {
        "UpdateHighOrderScattering.fx",
        #include "UpdateHighOrderScattering.fx.h"
    },
    {
        "Structures.fxh",
        #include "Structures.fxh.h"
    },
};
