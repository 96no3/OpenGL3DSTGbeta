#version 410

layout(location=0) in vec2 inTexCoord;

out vec4 fragColor;

uniform sampler2D colorSampler;

void main()
{
#if 1
  // 1/4縮小バッファ
  vec4 ts;
  ts.xy = vec2(1.0) / vec2(textureSize(colorSampler, 0));
  ts.zw = -ts.xy;
  fragColor = texture(colorSampler, inTexCoord + ts.xy);
  fragColor += texture(colorSampler, inTexCoord + ts.zy);
  fragColor += texture(colorSampler, inTexCoord + ts.xw);
  fragColor += texture(colorSampler, inTexCoord + ts.zw);
  fragColor *= 1.0 / 4.0;
#else
  // 1/2縮小バッファ
  fragColor = texture(colorSampler, inTexCoord);
  fragColor.a = 1.0;
#endif  
}
