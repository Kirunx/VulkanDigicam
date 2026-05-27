#version 450
layout(location = 0) in vec2 fragUv;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D sceneTexture;
layout(set = 0, binding = 1) uniform sampler2D bloomTexture;

layout(push_constant) uniform PushConstants {
    vec2 resolution;
    float ctr;
    float brt;
    float str;
    float zoom;
    float time;
} push;

mat4 ctrMatrix(float ctr) {
    float t = (1.0 - ctr) / 2.0;
    return mat4(ctr, 0.0, 0.0, 0.0, 0.0, ctr, 0.0, 0.0, 0.0, 0.0, ctr, 0.0, t, t, t, 1.0);
}

// Procedural pseudo-random noise
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}
float hash(vec2 p) {
    vec3 a = fract(p.xyx * 0.1031);
    a += dot(a, a.yzx + 33.33);
    return fract((a.x + a.y) * a.z);
}
void main() {
    vec2 uv = vec2(fragUv.x, 1.0 - fragUv.y);

    // Zoom
    vec2 centerUv = uv - 0.5;
    vec2 zoomUv = centerUv / push.zoom + 0.5;

    // Chromatic Aberration
    vec2 dir = zoomUv - vec2(0.5);
    float dist = length(dir);
    float aberrationStrength = 0.015;
    vec2 offset = dir * dist * aberrationStrength;

    float r = texture(sceneTexture, zoomUv + offset).r;
    float g = texture(sceneTexture, zoomUv).g;
    float b = texture(sceneTexture, zoomUv - offset).b;
    vec3 col = vec3(r, g, b);

    // noise

    vec2 noiseCoord = zoomUv * push.resolution;
    vec2 timeShift = vec2(push.time * 127.1, push.time * 311.7);
    float grain = hash(floor(noiseCoord) + timeShift);

    col += (grain - 0.5) * 0.14;

    //bloom
    vec3 bloom = texture(bloomTexture, zoomUv).rgb;
    col += bloom; 

    // Brightness + tint
    col *= vec3(push.brt) * vec3(1.0, 1.03, 1.14);

    // Saturation
    if(push.str != 1.0) {
        float gray = dot(col, vec3(0.299, 0.587, 0.114));
        col = mix(vec3(gray), col, push.str);
    }

    // Contrast
    vec4 result = ctrMatrix(push.ctr) * vec4(col, 1.0);
    outColor = vec4(clamp(result.rgb, 0.0, 1.0), 1.0);
}