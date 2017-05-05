varying vec4 color;
varying vec3 vertex_to_light;
varying vec3 normal;

uniform sampler2D Texture;

void main()
{
	float Diffuse = clamp(dot(vertex_to_light, normal), 0.0, 1.0);
	gl_FragColor.rgb = texture2D(Texture, gl_TexCoord[0].xy)/*color.rgb*/*(0.2+0.6*Diffuse);
	gl_FragColor.a = color.a;
}