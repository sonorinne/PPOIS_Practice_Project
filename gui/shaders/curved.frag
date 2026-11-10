#version 440
layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float scanlineStrength;
};
layout(binding = 1) uniform sampler2D source;
void main() {
    vec2 p = qt_TexCoord0 * 2.0 - 1.0;
    // Gentle radial barrel distortion. The centre stays readable.
    vec2 uv = 0.5 + 0.5 * p * (1.0 + 0.045 * dot(p, p));
    vec2 edge = smoothstep(vec2(0.0), vec2(0.006), uv)
              * (1.0 - smoothstep(vec2(0.994), vec2(1.0), uv));
    vec4 color = texture(source, clamp(uv, vec2(0.0), vec2(1.0)));
    // Generate scanlines after distortion, in output pixels. Warping a one-pixel
    // source grid creates moire and uneven dark bands at fractional DPI scales.
    float scanline = 0.5 + 0.5 * cos(6.28318530718 * gl_FragCoord.y / 4.0);
    color.rgb *= 1.0 - scanlineStrength * scanline;
    fragColor = mix(vec4(0.015, 0.02, 0.04, 1.0), color, edge.x * edge.y) * qt_Opacity;
}
