#version 330

in vec4 v;
in vec2 selfCoord;  // for the raw state

uniform usampler2D lastTick;
uniform usampler2D positionMapTexture;
uniform usampler2D connectionMap;

out vec4 fragColor;

// if v is in the range -1..1
// move it to 0..1
vec4 to01(vec4 v) {
    return (v + vec4(1.0)) * 0.5;
}


void main() {
    ivec2 size_lastTick = textureSize(lastTick, 0);
    uint maxCapacity = uint(size_lastTick.x);
    uint maxVolumes  = uint(size_lastTick.y);


    ivec2 lookupPos = ivec2(
        to01(v).xy * (textureSize(positionMapTexture, 0)).y
        // selfCoord
    );
    uint row = texelFetch(positionMapTexture, lookupPos, 0).r;
    uvec4 accum = uvec4(0u);
    for (uint slot=0u; slot<maxCapacity; ++slot) {
        accum += texelFetch(lastTick, ivec2(slot, row), 0);
    }
    fragColor = vec4(accum) / float(maxCapacity) / 255.0;


    // the raw state of lastTick
    // fragColor = vec4(texelFetch(lastTick, ivec2(selfCoord), 0)) / 255.0;

    // // the connection map
    // fragColor.rg = (
    //     vec4(texelFetch(connectionMap, ivec2(selfCoord), 0)).xy
    //     /
    //     textureSize(connectionMap, 0)
    // );
    // fragColor.b = 0.0;


    fragColor.a = 1.0;
}
