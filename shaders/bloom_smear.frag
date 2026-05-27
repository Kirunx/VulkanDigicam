#version 450
layout(location = 0) in vec2 fragUv;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D sceneTexture;

layout(push_constant) uniform Push {
    vec2 resolution;
    float threshold;
    float streakLength;
    float intensity;
    float direction;
} push;

float getBrightness(vec3 col) {
    return dot(col, vec3(0.2126, 0.7152, 0.0722));
}

void main() {
    vec2 uv = vec2(fragUv.x, 1.0 - fragUv.y);
    
    vec2 texelSize = 1.0 / push.resolution;
    vec3 result = vec3(0.0);

    int samples = 48; 
    
    for (int i = 0; i < samples; i++) {
        float fi = float(i);
        float weight = exp(-fi / push.streakLength);
        
        vec2 offset = vec2(0.0, fi * texelSize.y); 
        
        vec3 colUp = texture(sceneTexture, uv + offset).rgb;
        vec3 colDown = texture(sceneTexture, uv - offset).rgb;
        
        float brightUp = max(0.0, getBrightness(colUp) - push.threshold);
        float brightDown = max(0.0, getBrightness(colDown) - push.threshold);
        
        vec3 bloomUp = colUp * (brightUp / (getBrightness(colUp) + 0.001));
        vec3 bloomDown = colDown * (brightDown / (getBrightness(colDown) + 0.001));
        
        result += (bloomUp + bloomDown) * weight;
    }
    
    outColor = vec4(result * push.intensity, 1.0);
}