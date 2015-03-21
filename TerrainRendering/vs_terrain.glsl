#extension GL_EXT_gpu_shader4 : enable

attribute highp vec4 vertex;
attribute highp vec2 tex_coords;

uniform mediump mat4 MV;
uniform mediump mat4 MVP;

varying highp vec2 texc;
varying highp float altitude;
varying highp vec3 normal;

uniform sampler2D texture;

void main(void)
{
    ivec2 coords = ivec2(tex_coords*256);

    vec4 overlay_vertex = vertex;
    overlay_vertex.z = texelFetch2DOffset(texture, coords, 0, ivec2(0,0)).r;
    //altitude = texture2D(texture, texCoord.st).r;
    
    altitude = overlay_vertex.z;
    //overlay_vertex.z =  altitude / 16.0;
    
    vec3 vector[6];
	vector[0] = vec3( 0.0, texelFetch2DOffset( texture, coords, 0, ivec2( 0, -1)).r, -1.0);
	vector[1] = vec3(-1.0, texelFetch2DOffset( texture, coords, 0, ivec2(-1, -1)).r, -1.0);
	vector[2] = vec3(-1.0, texelFetch2DOffset( texture, coords, 0, ivec2(-1,  0)).r,  0.0);
	vector[3] = vec3( 0.0, texelFetch2DOffset( texture, coords, 0, ivec2( 0,  1)).r,  1.0);
	vector[4] = vec3( 1.0, texelFetch2DOffset( texture, coords, 0, ivec2( 1,  1)).r,  1.0);
	vector[5] = vec3( 1.0, texelFetch2DOffset( texture, coords, 0, ivec2( 1,  0)).r,  0.0);
	for (int i=0; i<6; ++i) {
		vector[i].y = vector[i].y - altitude;
	}
    normal = cross(vector[5], vector[0]);
    for (int i=1; i<6; ++i) {
        normal += cross(vector[i-1], vector[i]);
    }
    
    //vec3 toLight = normalize(vec3(0.0, 0.3, 1.0));
    //angle = max(dot( normal, (MV*vec4(toLight,0.0))), 0.0);
    gl_Position = MVP * overlay_vertex;
    texc = tex_coords;
}