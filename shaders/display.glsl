#version 330

in vec4 v;
in vec2 selfCoord;  // for the raw state

uniform usampler2D lastTick;
uniform usampler2D positionMapTexture;

out vec4 fragColor;

const uint maxCapacity = 8u;
const uint maxVolumes  = 128u;

// if v is in the range -1..1
// move it to 0..1
vec4 to01(vec4 v) {
    return (v + vec4(1.0)) * 0.5;
}


void main() {
    ivec2 texSize = textureSize(positionMapTexture, 0);

    ivec2 lookupPos = ivec2(
        to01(v).xy * (textureSize(positionMapTexture, 0)).y
    );
    uint row = texelFetch(positionMapTexture, lookupPos, 0).r;
    uvec4 accum = uvec4(0u);
    for (uint slot=0u; slot<maxCapacity; ++slot) {
        accum += texelFetch(lastTick, ivec2(slot, row), 0);
    }
    fragColor = vec4(accum) / maxCapacity;

    // the raw state of lastTick
    fragColor = vec4(texelFetch(lastTick, ivec2(selfCoord), 0)) / 255.0;

    // if (fragColor.r > 2.0) {
    //     fragColor = vec4(1.0, 0.5, 0.0, 1.0);
    // }
    // fragColor.xyz = vec3(texture(positionMapTexture, lookupPos).rgb) / 255.0;
    // // fragColor.xy = vec2() / vec2(maxCapacity*200000u, maxVolumes);

    // fragColor.rgb = vec3(
    //     (texSize.x != 10)? 1.0 : 0.0,
    //     (texSize.y != 10)? 1.0 : 0.0,
    //     0.0
    // );

    // fragColor.xyz = vec3(vec2(lookupPos)/20.0, 0.0);

    // fragColor.xyz = vec3(to01(v).xy, 0.0);
    fragColor.a = 1.0;
}
