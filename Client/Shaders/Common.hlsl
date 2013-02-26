
float GetLinearDepth(float DeviceDepth, float ParamX, float ParamY)
{
	return ParamY/(DeviceDepth - ParamX);
}

float3 GetViewPosition(float LinearDepth, float2 ScreenPosition, float Proj11, float Proj22)
{
	float2 screenSpaceRay = float2(ScreenPosition.x / Proj11,
                                   ScreenPosition.y / Proj22);
    
    float3 ViewPosition;
    ViewPosition.z = LinearDepth;
    // Solve the two projection equations
    ViewPosition.xy = screenSpaceRay.xy * ViewPosition.z;
    ViewPosition.z *= -1;
    return ViewPosition;
}

float CalcBlinPhong(float3 LightDir, float3 ViewNormal, float SpecularExponent)
{
	float3 ViewDir = float3(0, 0, 1);
	float3 HalfVector = normalize( LightDir + ViewDir );
	return pow( saturate( dot( ViewNormal, HalfVector ) ), SpecularExponent ) ;
}