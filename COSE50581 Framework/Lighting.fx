cbuffer cbData
{
	float4x4 World;
	float4x4 View;
	float4x4 Projection;

	float4 gDiffuseMtrl;
	float4 gDiffuseLight;
	float4 gAmbientMtrl;
	float4 gAmbientLight;
	float4 gSpecularMtrl;
	float4 gSpecularLight;
	float gSpecularPower;

	float3 gEyePosW;
	float3 gLightVecW;
};

struct VS_IN
{
	float4 posL   : POSITION;
	float3 normalL : NORMAL;
};

struct VS_OUT
{
	float4 Pos    : SV_POSITION;
	float3 Norm   : NORMAL;
	float3 PosW	  : POSITION;
};

VS_OUT VS(VS_IN vIn)
{
	VS_OUT output = (VS_OUT)0;

	output.Pos = mul(vIn.posL, World);
	output.PosW = output.Pos.xyz;
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);

	// Convert from local to world normal
	float3 normalW = mul(float4(vIn.normalL, 0.0f), World).xyz;
		normalW = normalize(normalW);

	output.Norm = normalW;

	return output;
}

float4 PS(VS_OUT pIn) : SV_Target
{
	pIn.Norm = normalize(pIn.Norm);

	float3 toEye = normalize(gEyePosW - pIn.PosW);

		// Compute Colour
		float3 r = reflect(-gLightVecW, pIn.Norm);
		float t = pow(max(dot(r, toEye), 0.0f), gSpecularPower);
	float s = max(dot(gLightVecW, pIn.Norm), 0.0f);
	float3 spec = t * (gSpecularMtrl * gSpecularLight).rgb;
		float3 diffuse = s*(gDiffuseMtrl*gDiffuseLight).rgb;
		float3 ambient = (gAmbientMtrl * gAmbientLight).rgb;

		float4 col;
	col.rgb = saturate(spec + diffuse + ambient);
	col.a = gDiffuseMtrl.a;

	return col;
}

technique11 Render
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, VS())); SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PS()));
	}
}
