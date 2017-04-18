varying vec4 color;
varying vec3 vertex_to_light;
varying vec3 normal;

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = gl_ModelViewMatrix * gl_Vertex;
	gl_Position = gl_ProjectionMatrix * gl_Position;
	normal = gl_NormalMatrix*gl_Normal;
	vertex_to_light = normalize(gl_LightSource[0].position);
	color.rgb = gl_Color.rgb;//*max(0,dot(vertex_to_light_vector, normal));
	color.a = gl_Color.a;
}