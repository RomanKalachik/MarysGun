float2 R;
int2 CellSize <float uimin=0;> = (16,16);
float2 Morph = (1,1);
texture tex0,tex1;
sampler s0=sampler_state{Texture=(tex0);MipFilter=NONE;MinFilter=LINEAR;MagFilter=LINEAR;};
sampler s1=sampler_state{Texture=(tex1);MipFilter=NONE;MinFilter=POINT;MagFilter=POINT;};
float4 lm(float4 c){c.rgb=dot(c.rgb,normalize(float3(.33,.59,.11))/1.5);return c;}
float4 p0(float2 x:TEXCOORD0):color{
    float2 vp=x*R-.25;
    float2 sz=R/min(max(0.5/R,CellSize),R);
    float4 m=tex2D(s1,round((x-.5)*sz)/sz+.5);
    float4 c=tex2D(s0,lerp(x,round((x-.5)*sz)/sz+.5,lm(m).x*Morph));
    return c;
}
void vs2d(inout float4 vp:POSITION0,inout float2 uv:TEXCOORD0){vp.xy*=2;uv+=.5/R;}
technique Pixelize{pass pp0{vertexshader=compile vs_2_0 vs2d();pixelshader=compile ps_2_0 p0();}}
