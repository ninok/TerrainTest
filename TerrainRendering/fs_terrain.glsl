varying highp vec4 texc;
uniform sampler2D texture;
varying mediump float angle;
void main(void)
{
    highp vec3 color = texture2D(texture, texc.st).rgb;
    color = color * 0.2 + color * 0.8 * angle;
    gl_FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}