attribute highp vec4 vertex;
attribute highp vec4 texCoord;
attribute mediump vec3 normal;

uniform mediump mat4 MV;
uniform mediump mat4 MVP;

varying highp vec4 texc;
varying mediump float angle;

uniform sampler2D texture;

void main(void)
{
    vec4 overlay_vertex = vertex;
    overlay_vertex.z = length(texture2D(texture, texCoord.st).rgb) / 16.0;
    vec3 toLight = normalize(vec3(0.0, 0.3, 1.0));
    angle = max(dot( normal, (MV*vec4(toLight,0.0))), 0.0);
    gl_Position = MVP * overlay_vertex;
    texc = texCoord;
}