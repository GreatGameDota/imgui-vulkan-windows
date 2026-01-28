#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 baseColor = texture(texSampler, fragTexCoord);
    if (fragTexCoord.x < 0)
    {
        outColor = fragColor;
        if (outColor.w == 0)
            discard;
        return;
    }
    if (baseColor.w == 0)
        discard;
    outColor = mix(vec4(0,0,0,0), baseColor, fragColor.a);
}