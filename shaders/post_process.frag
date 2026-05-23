#version 450
layout(location = 0) in vec2 fragUv;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D sceneTexture;
layout(set = 0, binding = 1) uniform sampler2D noiseTexture;

layout(push_constant) uniform PushConstants {
    vec2 resolution;
    float ctr;
    float brt;
    float str;
    float zoom;
    float glowThreshold;
} push;

mat4 ctrMatrix(float ctr) {
    float t = (1.0 - ctr) / 2.0;
    return mat4(
        ctr, 0.0, 0.0, 0.0,
        0.0, ctr, 0.0, 0.0,
        0.0, 0.0, ctr, 0.0,
        t,   t,   t,   1.0
    );
}

void main() {
    vec2 uv = vec2(fragUv.x, 1.0 - fragUv.y);
    
    // Zoom
    vec2 centerUv = uv - 0.5;
    vec2 zoomUv = centerUv / push.zoom + 0.5;
    
    // Chromatic aberration
    float offset = 1.0 / min(push.resolution.x, push.resolution.y);
    float r = texture(sceneTexture, zoomUv + vec2(offset, offset)).r;
    float g = texture(sceneTexture, zoomUv).g;
    float b = texture(sceneTexture, zoomUv - vec2(offset, offset)).b;
    vec3 col = vec3(r, g, b);
    
    // Noise
    vec3 noise = texture(noiseTexture, zoomUv * 4.0).rgb;
    col = mix(col, noise, 0.02);
    
    // Brightness + tint
    col *= vec3(push.brt) * vec3(1.0, 1.03, 1.14);
    
    // Saturation
    if (push.str != 1.0) {
        float gray = dot(col, vec3(0.299, 0.587, 0.114));
        col = mix(vec3(gray), col, push.str);
    }
    
    // Contrast
    vec4 result = ctrMatrix(push.ctr) * vec4(col, 1.0);
    outColor = vec4(clamp(result.rgb, 0.0, 1.0), 1.0);
}