texture Texture;
sampler DecaleSamp = sampler_state
{
    Texture = <Texture>;
    MinFilter = POINT;//LINEAR;
    MagFilter = POINT;//LINEAR;
    MipFilter = NONE;

    AddressU = CLAMP;
    AddressV = CLAMP;
};

sampler DecaleSamp2 = sampler_state
{
    Texture = <Texture>;
    MinFilter = GAUSSIANQUAD;
    MagFilter = GAUSSIANQUAD;
    MipFilter = NONE;

    AddressU = CLAMP;
    AddressV = CLAMP;
};


// -------------------------------------------------------------
// 頂点シェーダからピクセルシェーダに渡すデータ
// -------------------------------------------------------------
struct VS_OUTPUT_G
{
    float4 Pos			: POSITION;
	float2 Tex			: TEXCOORD0;
    float4 Color		: COLOR0;
};

// -------------------------------------------------------------
// 頂点シェーダプログラム
// -------------------------------------------------------------
VS_OUTPUT_G VS_pass1 (
      float4 Pos    : POSITION,          // モデルの頂点
      float4 Tex    : TEXCOORD0,	     // テクスチャ座標
      float4 Color  : COLOR0
){
    VS_OUTPUT_G Out;        // 出力データ
    
    // 位置座標
    Out.Pos = Pos;
    Out.Tex = Tex;
    Out.Color = Color;
    return Out;
}

/*Global*/
float exposure = 2.1f;


// -------------------------------------------------------------
// ピクセルシェーダプログラム
// -------------------------------------------------------------
float4 PS_pass2(VS_OUTPUT_G In) : COLOR
{
	return In.Color * tex2D( DecaleSamp2, In.Tex );
}

float4 PS_pass_HDR(VS_OUTPUT_G In) : COLOR
{
	float4 OUT;
	OUT = In.Color * tex2D(DecaleSamp2, In.Tex);

	OUT.rgb -= 0.5f;

	OUT.r = max(0.0f, min(0.3f, OUT.r));
	OUT.g = max(0.0f, min(0.3f, OUT.g));
	OUT.b = max(0.0f, min(0.3f, OUT.b));


	//OUT.rgba -= 0.5f;

	//　赤で判定
	//float check = OUT.r - 0.5f;
	//	OUT.rgb *= step(0.0f, check);
		////　緑で判定
		// check = OUT.g - 0.5f;
		//OUT.rgb *= step(0.0f, check);
		////　青で判定
		// check = OUT.b - 0.5f;
		//OUT.rgb *= step(0.0f, check);

	//	//　赤で判定
	//float3 check = OUT.rgb - 0.5f;
	//	float3 juge;
	//juge.rgb = 0.0f;
	//OUT.rgb *= step(juge, check);

	//if (OUT.r >= 0.4f&&OUT.g >= 0.4f)
	//{
	//	OUT.rgb = 0.3f;
	//}

	//if (OUT.rgb<=0.5f)
	//{
	//	OUT.rgb = 0.0f;
	//}

	//OUT.a = 1.0f;

		//OUT.rgb *= 0.5f;

	//OUT.rgb *= 1.0f - exp(-tex2D(DecaleSamp2, In.Tex)* exposure);


	return OUT;
}

float4 PS_pass1(VS_OUTPUT_G In) : COLOR
{   
    float4	Color;
    float4 temp;

	temp = tex2D( DecaleSamp, In.Tex );
	Color = temp*8;

	//	横	14t
	temp = tex2D( DecaleSamp, In.Tex + float2( + 0.002f, 0 )) + tex2D( DecaleSamp, In.Tex + float2( - 0.002f, 0 ) );
	Color += temp*4;
	
	temp = tex2D( DecaleSamp, In.Tex + float2( + 0.006f, 0 )) + tex2D( DecaleSamp, In.Tex + float2( - 0.005f, 0 ) );
	Color += temp*2;

	temp = tex2D( DecaleSamp, In.Tex + float2( + 0.01f, 0 )) + tex2D( DecaleSamp, In.Tex + float2( - 0.008f, 0 ) );
	Color += temp;

	//	縦	14t
	temp = tex2D( DecaleSamp, In.Tex + float2( 0, + 0.002f )) + tex2D( DecaleSamp, In.Tex + float2( 0, - 0.002f ) );
	Color += temp*4;
	
	temp = tex2D( DecaleSamp, In.Tex + float2( 0, + 0.006f )) + tex2D( DecaleSamp, In.Tex + float2( 0, - 0.005f ) );
	Color += temp*2;

	temp = tex2D( DecaleSamp, In.Tex + float2( 0, + 0.01f )) + tex2D( DecaleSamp, In.Tex + float2( 0, - 0.008f ) );
	Color += temp;


	//	上横	8t	
	temp = tex2D( DecaleSamp, In.Tex + float2( + 0.002f, -0.002f )) + tex2D( DecaleSamp, In.Tex + float2( - 0.002f, -0.002f ) );
	Color += temp*4;
	
	//	下横	8t
	temp = tex2D( DecaleSamp, In.Tex + float2( + 0.002f, +0.002f )) + tex2D( DecaleSamp, In.Tex + float2( - 0.002f, +0.002f ) );
	Color += temp*4;
	
	Color /= 52;
	
    return Color;
}
// -------------------------------------------------------------
// テクニック
// -------------------------------------------------------------
technique gauss
{
    pass P0
    {
 		AlphaBlendEnable = false;
		BlendOp          = Add;
		SrcBlend         = SrcAlpha;
		DestBlend        = Zero;
		// シェーダ
        VertexShader = compile vs_3_0 VS_pass1();
        PixelShader  = compile ps_3_0 PS_pass1();
    }
}

technique copy
{
    pass P0
    {
		AlphaBlendEnable = true;
		BlendOp          = Add;
		SrcBlend         = SrcAlpha;
		DestBlend        = InvSrcAlpha;
		// シェーダ
        VertexShader = compile vs_3_0 VS_pass1();
        PixelShader  = compile ps_3_0 PS_pass2();
    }
}

technique copy_hdr
{
    pass P0
    {
		AlphaBlendEnable = true;
		BlendOp          = Add;
		SrcBlend         = SrcAlpha;
		DestBlend        = InvSrcAlpha;
		// シェーダ
        VertexShader = compile vs_3_0 VS_pass1();
		PixelShader = compile ps_3_0 PS_pass_HDR();
    }
}

technique add
{
    pass P0
    {
		AlphaBlendEnable = true;
		BlendOp          = Add;
		SrcBlend         = SrcAlpha;
		DestBlend        = one;
		// シェーダ
        VertexShader = compile vs_3_0 VS_pass1();
        PixelShader  = compile ps_3_0 PS_pass2();
    }
}



//------------------------------------------------------
//		スクリーンフィルタ
//------------------------------------------------------
float	contrast = 0.9;
float	chroma	= 1.0f;
float3	ScreenColor = { 1.0f, 1.0f, 1.0f };

struct VS_M
{
    float4 Pos		: POSITION;
    float4 Color	: COLOR0;
    float2 Tex		: TEXCOORD0;
};

//------------------------------------------------------
//		基本頂点シェーダー	
//------------------------------------------------------
VS_M VS_Master( VS_M In ){
	return In;
}

//------------------------------------------------------
//		ピクセルシェーダー	
//------------------------------------------------------
float4 PS_Master( VS_M In) : COLOR
{   
	float4	OUT;

	float3	col = tex2D( DecaleSamp, In.Tex );

	col = ((col-0.5f)*contrast) + 0.5f;

	float	avr = (col.r + col.g + col.b) / 3;
	col = (col-avr) * chroma + avr;

	//	ピクセル色決定
	OUT.rgb = col * ScreenColor;
	OUT.w = 1;
	
	return OUT;
}


//------------------------------------------------------
//
//------------------------------------------------------
technique Mastering
{
    pass P0
    {
		AlphaBlendEnable = false;
		BlendOp          = Add;
		SrcBlend         = one;
		DestBlend        = zero;

		VertexShader = compile vs_2_0 VS_Master();
		PixelShader  = compile ps_2_0 PS_Master();
    }
}


//------------------------------------------------------
//		パーティクル
//------------------------------------------------------

float3 LightDir = { 0.707f,0.707f,-0.707f };

texture NormalMap;	//	法線マップテクスチャ
sampler NormalSamp = sampler_state
{
    Texture = <NormalMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Wrap;
    AddressV = Wrap;
};

//------------------------------------------------------
//		基本頂点シェーダー	
//------------------------------------------------------
VS_M VS_Particle( VS_M In ){
	return In;
}

//------------------------------------------------------
//		ピクセルシェーダー	
//------------------------------------------------------
float4 PS_Particle( VS_M In) : COLOR
{   
	float4	OUT;

	//	ライト計算
	float3 N = tex2D( NormalSamp, In.Tex ).xyz * 2.0f - 1.0f;
	float rate = max( 0, dot( -LightDir, N ) );
	float4	col = tex2D( DecaleSamp2, In.Tex );
	//	ピクセル色決定
	OUT = In.Color * col;
	OUT.rgb *= rate;
	
	return OUT;
}


//------------------------------------------------------
//
//------------------------------------------------------
technique Particle
{
    pass P0
    {
		AlphaBlendEnable = true;
		BlendOp          = Add;
		SrcBlend         = SrcAlpha;
		DestBlend        = InvSrcAlpha;
		ZWriteEnable     = false;

		VertexShader = compile vs_3_0 VS_Particle();
		PixelShader  = compile ps_3_0 PS_Particle();
    }
}


//*********************************
//		GaussianBlur
//*********************************

sampler GaussianSamp = sampler_state
{
	Texture = <Texture>;

	MinFilter = LINEAR;//補間
	MagFilter = LINEAR;//補間

	MipFilter = LINEAR;

	AddressU = CLAMP;
	AddressV = CLAMP;
};
//------------------------------------------------------
//		頂点フォーマット
//------------------------------------------------------
struct VS_2D
{
	float4 Pos		: POSITION;
	float4 Color	: COLOR0;
	float2 Tex		: TEXCOORD0;
};

//------------------------------------------------------
//		頂点シェーダー	
//------------------------------------------------------
VS_2D VS_Basic(VS_2D In)
{
	return In;
}

float TU = 1.0f / 1280.0f, TV = 1.0f / 720.0f;
float BlurValue = 1.0f;
float4 PS_gaussX(VS_2D In) : COLOR
{

	//テクセルを取得   
	float2 Texel0 = In.Tex + float2(-TU * 1 * BlurValue, 0.0f);
	float2 Texel1 = In.Tex + float2(-TU * 2 * BlurValue, 0.0f);
	float2 Texel2 = In.Tex + float2(-TU * 3 * BlurValue, 0.0f);
	float2 Texel3 = In.Tex + float2(-TU * 4 * BlurValue, 0.0f);
	float2 Texel4 = In.Tex + float2(-TU * 5 * BlurValue, 0.0f);

	float2 Texel5 = In.Tex + float2(TU * 1 * BlurValue, 0.0f);
	float2 Texel6 = In.Tex + float2(TU * 2 * BlurValue, 0.0f);
	float2 Texel7 = In.Tex + float2(TU * 3 * BlurValue, 0.0f);
	float2 Texel8 = In.Tex + float2(TU * 4 * BlurValue, 0.0f);
	float2 Texel9 = In.Tex + float2(TU * 5 * BlurValue, 0.0f);

	//取得したテクセル位置のカラー情報を取得する。
	//それぞれのカラー値に重みをかけている。この重み値はすべての合計が 1.0f になるように調整する。
	float4 p0 = tex2D(GaussianSamp, In.Tex) * 0.20f;

	float4 p1 = tex2D(GaussianSamp, Texel0) * 0.12f;
	float4 p2 = tex2D(GaussianSamp, Texel1) * 0.10f;
	float4 p3 = tex2D(GaussianSamp, Texel2) * 0.08f;
	float4 p4 = tex2D(GaussianSamp, Texel3) * 0.06f;
	float4 p5 = tex2D(GaussianSamp, Texel4) * 0.04f;

	float4 p6 = tex2D(GaussianSamp, Texel5) * 0.12f;
	float4 p7 = tex2D(GaussianSamp, Texel6) * 0.10f;
	float4 p8 = tex2D(GaussianSamp, Texel7) * 0.08f;
	float4 p9 = tex2D(GaussianSamp, Texel8) * 0.06f;
	float4 p10 = tex2D(GaussianSamp, Texel9) * 0.04f;

	//カラーを合成する
	return p0 + p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9 + p10;

}

float4 PS_gaussY(VS_2D In) : COLOR
{

	//テクセルを取得   
	float2 Texel0 = In.Tex + float2(0.0, -TV * 1 * BlurValue);
	float2 Texel1 = In.Tex + float2(0.0, -TV * 2 * BlurValue);
	float2 Texel2 = In.Tex + float2(0.0, -TV * 3 * BlurValue);
	float2 Texel3 = In.Tex + float2(0.0, -TV * 4 * BlurValue);
	float2 Texel4 = In.Tex + float2(0.0, -TV * 5 * BlurValue);

	float2 Texel5 = In.Tex + float2(0.0, TV * 1 * BlurValue);
	float2 Texel6 = In.Tex + float2(0.0, TV * 2 * BlurValue);
	float2 Texel7 = In.Tex + float2(0.0, TV * 3 * BlurValue);
	float2 Texel8 = In.Tex + float2(0.0, TV * 4 * BlurValue);
	float2 Texel9 = In.Tex + float2(0.0, TV * 5 * BlurValue);

	//取得したテクセル位置のカラー情報を取得する。
	//それぞれのカラー値に重みをかけている。この重み値はすべての合計が 1.0f になるように調整する。
	float4 p0 = tex2D(GaussianSamp, In.Tex) * 0.20f;

	float4 p1 = tex2D(GaussianSamp, Texel0) * 0.12f;
	float4 p2 = tex2D(GaussianSamp, Texel1) * 0.10f;
	float4 p3 = tex2D(GaussianSamp, Texel2) * 0.08f;
	float4 p4 = tex2D(GaussianSamp, Texel3) * 0.06f;
	float4 p5 = tex2D(GaussianSamp, Texel4) * 0.04f;

	float4 p6 = tex2D(GaussianSamp, Texel5) * 0.12f;
	float4 p7 = tex2D(GaussianSamp, Texel6) * 0.10f;
	float4 p8 = tex2D(GaussianSamp, Texel7) * 0.08f;
	float4 p9 = tex2D(GaussianSamp, Texel8) * 0.06f;
	float4 p10 = tex2D(GaussianSamp, Texel9) * 0.04f;

	//カラーを合成する
	return p0 + p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9 + p10;

}


technique gaussZ
{

	pass P0
	{
		AlphaBlendEnable = true;
		BlendOp = Add;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		// シェーダ
		VertexShader = compile vs_3_0 VS_Basic();
		PixelShader = compile ps_3_0 PS_gaussX();
	}

	pass P1
	{
		AlphaBlendEnable = true;
		BlendOp = Add;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		// シェーダ
		VertexShader = compile vs_3_0 VS_Basic();
		PixelShader = compile ps_3_0 PS_gaussY();
	}
}


/************************/
/*		ToneMap		*/
/************************/



/*ピクセルシェーダー*/
float4 PS_Tone(VS_2D In) : COLOR
{
	float4 OUT = In.Color;


	//No.1	0.0から1.0	暗い
	OUT.rgb = 1.0f - exp(-tex2D(DecaleSamp2, In.Tex)* exposure);


	//No.2	1.0から2.0　明るい
	//OUT.rgb = tex2D(DecaleSamp2, In.Tex)* exp2(exposure);

	//No.3 よく使われるReinhard計算式　0.3から1.0　ふつう
	//OUT.rgb = tex2D(DecaleSamp2, In.Tex);
	//OUT.rgb *= exp2(exposure);
	//OUT.rgb /= 1.0f + OUT.rgb;
	

	//No.4 映像フィルム　0.7から1.0 白っぽい
	//OUT.rgb = tex2D(DecaleSamp2, In.Tex);
	//OUT.rgb *= exp2(exposure);
	//OUT.rgb = max(OUT.rgb - 0.004f, 0.0f);
	//OUT.rgb = (OUT.rgb*(6.2f*OUT.rgb + 0.5f)) / (OUT.rgb*(6.2f*OUT.rgb + 1.7f) + 0.06f);
	

	return OUT;
}

technique ToneMap
{
	
	pass P0
	{
		AlphaBlendEnable = true;
		BlendOp = Add;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		// シェーダ
		VertexShader = compile vs_3_0 VS_Basic();
		PixelShader = compile ps_3_0 PS_Tone();
	}

}

//
///************************/
///*		ToneMap	ADD	*/
///************************/
//
///*Global*/
//float exposure_Add = 0.8f;
//
///*ピクセルシェーダー*/
//float4 PS_Tone_Add(VS_2D In) : COLOR
//{
//	float4 OUT = In.Color;
//
//
//	//No.1	0.0から1.0	暗い
//	//OUT.rgb = 1.0f - exp(-tex2D(DecaleSamp2, In.Tex)* exposure);
//
//
//	//No.2	1.0から2.0　明るい
//	//OUT.rgb = tex2D(DecaleSamp2, In.Tex)* exp2(exposure);
//
//	//No.3 よく使われるReinhard計算式　0.3から1.0　ふつう
//	OUT.rgb = tex2D(DecaleSamp2, In.Tex);
//	OUT.rgb *= exp2(exposure);
//	OUT.rgb /= 1.0f + OUT.rgb;
//
//
//	//No.4 映像フィルム　0.7から1.0 白っぽい
//	//OUT.rgb = tex2D(DecaleSamp2, In.Tex);
//	//OUT.rgb *= exp2(exposure);
//	//OUT.rgb = max(OUT.rgb - 0.004f, 0.0f);
//	//OUT.rgb = (OUT.rgb*(6.2f*OUT.rgb + 0.5f)) / (OUT.rgb*(6.2f*OUT.rgb + 1.7f) + 0.06f);
//
//
//	return OUT;
//}
//
//technique ToneMap_Add
//{
//
//	pass P0
//	{
//		AlphaBlendEnable = true;
//		BlendOp = Add;
//		SrcBlend = SrcAlpha;
//		DestBlend = one;
//		// シェーダ
//		VertexShader = compile vs_3_0 VS_Basic();
//		PixelShader = compile ps_3_0 PS_Tone_Add();
//	}
//
//}

//********************************************************************
//		放射ブラ―
//********************************************************************

float CenterX = 0.0f;
float CenterY = 0.0f;

float BluePower = 3.0f;
const float IMAGE_SIZE = 512.0f;

//------------------------------------------------------
//		ピクセルシェーダー	
//------------------------------------------------------
float4 PS_RadialBlur(VS_OUTPUT_G In) : COLOR
{
	float4	OUT;

	//　放射中心
	float2 ss_center = float2((CenterX + 1.0f) * 0.5f, (-CenterY + 1.0f) * 0.5f);

		//　オフセット
		float2 uvOffset = (ss_center - In.Tex) * (BluePower / IMAGE_SIZE);

		//　サンプリング数の逆数 　for文で回す回数文色を減らし　完成したときに元の色にする。
		float InvSampling = 1.0f / 8.0f;

	//　テクスチャ座標　動かすために今のテクスチャーの場所を渡す。
	float2 uv = In.Tex;

		//　サンプリングの回数だけ実行
	for (int i = 0; i<8; i++)
	{
		OUT += tex2D(DecaleSamp, uv) * InvSampling;
		uv += uvOffset;
	}



	return OUT;

}
//------------------------------------------------------
// テクニック
//------------------------------------------------------
technique RadialBlur
{
	pass P0
	{
		AlphaBlendEnable = true;
		BlendOp = Add;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		CullMode = None;
		ZEnable = false;

		PixelShader = compile ps_3_0 PS_RadialBlur();
	}
}


float4 PS_YBlur(VS_2D In) : COLOR
{

	//テクセルを取得   
	float2 Texel0 = In.Tex + float2(0.0, -TV * 1 * BluePower);
	float2 Texel1 = In.Tex + float2(0.0, -TV * 2 * BluePower);
	float2 Texel2 = In.Tex + float2(0.0, -TV * 3 * BluePower);
	float2 Texel3 = In.Tex + float2(0.0, -TV * 4 * BluePower);
	float2 Texel4 = In.Tex + float2(0.0, -TV * 5 * BluePower);

	float2 Texel5 = In.Tex + float2(0.0, TV * 1 * BluePower);
	float2 Texel6 = In.Tex + float2(0.0, TV * 2 * BluePower);
	float2 Texel7 = In.Tex + float2(0.0, TV * 3 * BluePower);
	float2 Texel8 = In.Tex + float2(0.0, TV * 4 * BluePower);
	float2 Texel9 = In.Tex + float2(0.0, TV * 5 * BluePower);

	//取得したテクセル位置のカラー情報を取得する。
	//それぞれのカラー値に重みをかけている。この重み値はすべての合計が 1.0f になるように調整する。
	float4 p0 = tex2D(GaussianSamp, In.Tex) * 0.20f;

	float4 p1 = tex2D(GaussianSamp, Texel0) * 0.12f;
	float4 p2 = tex2D(GaussianSamp, Texel1) * 0.10f;
	float4 p3 = tex2D(GaussianSamp, Texel2) * 0.08f;
	float4 p4 = tex2D(GaussianSamp, Texel3) * 0.06f;
	float4 p5 = tex2D(GaussianSamp, Texel4) * 0.04f;

	float4 p6 = tex2D(GaussianSamp, Texel5) * 0.12f;
	float4 p7 = tex2D(GaussianSamp, Texel6) * 0.10f;
	float4 p8 = tex2D(GaussianSamp, Texel7) * 0.08f;
	float4 p9 = tex2D(GaussianSamp, Texel8) * 0.06f;
	float4 p10 = tex2D(GaussianSamp, Texel9) * 0.04f;

	//カラーを合成する
	return p0 + p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9 + p10;

}

//------------------------------------------------------
// テクニック
//------------------------------------------------------
technique YBlur
{
	pass P0
	{
		AlphaBlendEnable = true;
		BlendOp = Add;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		CullMode = None;
		ZEnable = false;

		PixelShader = compile ps_3_0 PS_YBlur();
	}
}