#version 410

layout(location=0) in vec2 inTexCoord;

out vec4 fragColor;

uniform sampler2D colorSampler;

// ポストエフェクトデータ.
layout(std140) uniform PostEffectData {
  mat4x4 matColor; // 色変換行列.
  float luminanceScale; // 輝度増減係数.
  float bloomThreshould; // ブルームを発生させるしきい値.
} postEffect;

/**
* 輝度を計算する.
*
* @param rgb RGBカラー.
*
* @return rgbをITU-BT.2020にしたがって変換した輝度の対数.
*/
float luminance(vec3 rgb)
{
  return log(dot(rgb, vec3(0.2627, 0.678, 0.0593)) + 0.0001);
}


void main()
{  
  //vec3 threshould = vec3(1.0);
  vec3 threshould = vec3(postEffect.bloomThreshould);

#if 1
  // 1/4縮小バッファ
  vec4 ts;
  ts.xy = vec2(1.0) / vec2(textureSize(colorSampler, 0));
  ts.zw = -ts.xy;
//  fragColor.rgb = max(texture(colorSampler, inTexCoord + ts.xy).rgb, threshould) - threshould;
//  fragColor.rgb += max(texture(colorSampler, inTexCoord + ts.zy).rgb, threshould) - threshould;
//  fragColor.rgb += max(texture(colorSampler, inTexCoord + ts.xw).rgb, threshould) - threshould;
//  fragColor.rgb += max(texture(colorSampler, inTexCoord + ts.zw).rgb, threshould) - threshould;
//  fragColor.rgb *= 1.0 / 4.0;
  vec3 rgb;
  rgb = texture(colorSampler, inTexCoord + ts.xy).rgb;
  fragColor.a = luminance(rgb);
  fragColor.rgb = max(rgb, threshould) - threshould;
  rgb = texture(colorSampler, inTexCoord + ts.zy).rgb;
  fragColor.a += luminance(rgb);
  fragColor.rgb += max(rgb, threshould) - threshould;
  rgb = texture(colorSampler, inTexCoord + ts.xw).rgb;
  fragColor.a += luminance(rgb);
  fragColor.rgb += max(rgb, threshould) - threshould;
  rgb = texture(colorSampler, inTexCoord + ts.zw).rgb;
  fragColor.a += luminance(rgb);
  fragColor.rgb += max(rgb, threshould) - threshould;
  fragColor *= 1.0 / 4.0;
#else
  // 1/2縮小バッファ
  fragColor.rgb = max(texture(colorSampler, inTexCoord).rgb, threshould) - threshould;
  fragColor.a = 1.0;
#endif  
}
