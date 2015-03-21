uniform sampler2D texture;

varying highp vec2 texc;
varying highp float altitude;
varying highp vec3 normal;

void main(void)
{
    //highp vec3 color = texture2D(texture, texc.st).rgb;
    //color = color * 0.2 + color * 0.8 * angle;
    //gl_FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);

    vec4 color;    
    if ( altitude < 0.1 )
    {
        color = vec4( 0.0, 0.1, 0.0, 1.0);
    }
    else if ( altitude < 0.2 )
    {
        color = vec4( 0.0, 0.2, 0.0, 1.0);
    }
    else if ( altitude < 0.5 )
    {
        color = vec4( 0.0, 0.5, 0.0, 1.0);
    }
    else if ( altitude < 0.8 )
    {
        color = vec4( 0.0, 0.8, 0.0, 1.0);
    }
    else // if ( altitude <= 1.0 )
    {
        color = vec4( 0.0, 1.0, 0.0, 1.0);
    }
    
    vec3 light = normalize( vec3( 1.0, 0.1, 1.0) );
    float diffuse = min( max(dot( light, normal), 0.0), 1.0);
    
    gl_FragColor = color * diffuse;
}