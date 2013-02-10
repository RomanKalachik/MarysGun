float2 R;
float Invert;
float Grayscale;
float AdjustR;
float AdjustG;
float AdjustB;
float4 Factor <float uimin=0.0; float uimax=1.0;> = 1;
texture tex0;
texture tex1;

sampler s0 = sampler_state{
	Texture=(tex0);	MipFilter=LINEAR; MinFilter=LINEAR; MagFilter=LINEAR;
};

sampler s1 = sampler_state{
	Texture=(tex1);	MipFilter=LINEAR; MinFilter=LINEAR;	MagFilter=LINEAR;
};

float4 p0(float2 x:TEXCOORD0):color{
	
	// inverted texture
	float4 colA = tex2D(s0, x);
	
	// projection texture
    float4 colB = tex2D(s1, x);
	
	float4 colG;	// color grayscale	
	float4 colD;	// color deviation
	float4 colF;	// color final
	
	// calculate grayscale	
	colG = colB.r*0.3+colB.g*0.59+colB.b*0.11;
	
	// calculate deviation
	colD = colG - colB;
	
	// calculate refined texture
	colF = colA + colD;
	
	// manual adjustment
	colF.r += AdjustR;
	colF.g += AdjustG;
	colF.b += AdjustB;
	
	return colF;
}
void vs2d(inout float4 vp:POSITION0,inout float2 uv:TEXCOORD0){vp.xy*=2;uv+=.5/R;}
technique InvertColor{pass pp0{vertexshader=compile vs_2_0 vs2d();pixelshader=compile ps_2_0 p0();}}