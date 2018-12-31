#version 410

layout(location=0) in vec2 inTexCoord;
layout(location=1) in vec4 inColor;

out vec4 fragColor;

uniform sampler2D colorSampler;

const float edge = 0.43; ///< ƒtƒHƒ“ƒg‚Ì—ÖŠs‚Æ‚Ý‚È‚·‹——£.
const float smoothing = 1.0 / 16.0; ///< —ÖŠs‚ðŠŠ‚ç‚©‚É‚·‚é‚½‚ß‚ÌŒW”.

void main()
{
  float distance = texture(colorSampler, inTexCoord).r;
  float alpha = smoothstep(edge - smoothing, edge + smoothing, 1 - distance);
  fragColor = vec4(inColor.rgb, inColor.a * alpha);
}
