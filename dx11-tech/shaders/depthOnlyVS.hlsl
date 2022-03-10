#include "ShaderInterop_Common.h"

CBUFFER(PerDrawCB, 1)
{
    matrix g_world_mat;
}

CBUFFER(PerLightCB, 2)
{
    matrix g_light_view_projection;
    matrix g_light_view_projection_inv;
    float4 g_light_direction;
}


float4 main( float4 pos : POSITION ) : SV_POSITION
{
    //return mul(g_light_view_projection, mul(g_world_mat, pos));
    return mul(g_world_mat, pos);
}