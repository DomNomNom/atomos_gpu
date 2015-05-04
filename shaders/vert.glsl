#version 330

layout(location = 0)in vec4 vert;

smooth out vec4 v;
smooth out vec2 selfCoord;

// smooth out uint volume;
// smooth out uint col;

uniform uint maxCapacity;
uniform uint maxVolumes;

// if v is in the range -1..1
// move it to 0..1
float to01(float v) {
    return (v + float(1.0)) * 0.5;
}

void main() {
    v = vec4(vert);
    selfCoord = ivec2(
        (to01(v.x) * maxCapacity),
        (to01(v.y) * maxVolumes)
    );
    // volume = uint(v.y * maxVolumes);
    // col    = uint(v.y * maxVolumes);
    gl_Position = vert;
}
