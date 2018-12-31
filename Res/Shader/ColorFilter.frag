#version 410

layout(location=0) in vec2 inTexCoord;

out vec4 fragColor;

//uniform sampler2D colorSampler;
uniform sampler2D colorSampler[2];

// ポストエフェクトデータ.
layout(std140) uniform PostEffectData
{
  mat4x4 matColor; // 色変換行列.
  float luminanceScale; // 輝度増減係数.
  float bloomThreshould; // ブルームを発生させるしきい値.

} postEffect;

/**
* ACESフィルム風トーンマッピング.
*
* @param rgb 入力カラー.
*
* @return トーンマッピングされたrgbカラー.
*/
vec3 ACESFilmicToneMapping(vec3 rgb)
{
  float a = 2.51f;
  float b = 0.03f;
  float c = 2.43f;
  float d = 0.59f;
  float e = 0.14f;
  return (rgb * (a * rgb + b)) / (rgb * (c * rgb + d) + e);
}

void main()
{
#if 1
  // 1/4縮小バッファ
  vec4 ts;
  ts.xy = vec2(0.25) / vec2(textureSize(colorSampler[1], 0));
  ts.zw = -ts.xy;
  vec3 bloom = texture(colorSampler[1], inTexCoord + ts.xy).rgb;
  bloom += texture(colorSampler[1], inTexCoord + ts.zy).rgb;
  bloom += texture(colorSampler[1], inTexCoord + ts.xw).rgb;
  bloom += texture(colorSampler[1], inTexCoord + ts.zw).rgb;
  bloom *= 1.0 / 4.0;

#else
  // 1/2縮小バッファ
  vec3 bloom = texture(colorSampler[1], inTexCoord).rgb;
#endif
  //fragColor = texture(colorSampler, inTexCoord);
  fragColor.rgb = texture(colorSampler[0], inTexCoord).rgb;
  fragColor.rgb += bloom;

  fragColor.rgb *= postEffect.luminanceScale;
  fragColor.rgb = ACESFilmicToneMapping(fragColor.rgb);
  //fragColor.rgb = vec3(dot(vec3(0.299, 0.587, 0.114), fragColor.rgb));	// モノトーン変換
  fragColor.rgb = (postEffect.matColor * vec4(fragColor.rgb, 1)).rgb;
  //fragColor.rgb = vec3(1 - fragColor.r,1 - fragColor.g,1 - fragColor.b);	// ネガポジ変換

  fragColor.a = 1.0;
  //fragColor *= inColor;
}
