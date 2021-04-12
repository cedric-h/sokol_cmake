#pragma sokol @ctype mat4 Mat4

#pragma sokol @vs mui_vs
uniform mui_uniforms {
    mat4 mvp;
};
layout(location=0) in vec2 position;
layout(location=1) in vec2 tex0;
layout(location=2) in vec4 color0;
out vec4 color;
out vec2 uv;
void main() {
    gl_Position = mvp * vec4(position, 0, 1);
    color = color0;
    uv = tex0;
}
#pragma sokol @end

#pragma sokol @fs mui_fs
uniform sampler2D tex;
in vec4 color;
in vec2 uv;
out vec4 frag_color;
void main() {
    float alpha = texture(tex, uv).x;
    frag_color = vec4(1,1,1,alpha) * color;
}
#pragma sokol @end

#pragma sokol @program mui mui_vs mui_fs
