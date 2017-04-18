varying vec3 normal;
varying vec3 vertex_to_light_vector;
varying vec3 vertex_to_cam;
varying vec4 color;
varying vec3 worldPos;

uniform float ambientLevel;
uniform sampler2D noiseTex;
uniform float elapsed;

float texValue(vec2 position){
	return (cos(position.y/10.+elapsed)+sin(position.x/10.+elapsed))/4.+4.*(texture2D(noiseTex,position.xy/128.+elapsed/15.))/2.5;//(texture2D(noiseTex,(position.xy/64)+elapsed/20.).r*2-1);//+((cos((worldPos.x+elapsed/10.))+sin((worldPos.y+elapsed/10.)))/2.);
}

void main()
{
	// Scaling The Input Vector To Length 1
	vec3 normalized_normal;
	normalized_normal.x = (texValue(worldPos.xy+vec2(-1,0))-texValue(worldPos.xy+vec2(1,0)));
	normalized_normal.y = (texValue(worldPos.xy+vec2(0,-1))-texValue(worldPos.xy+vec2(0,1)));
	normalized_normal.z = 1-(normalized_normal.x+normalized_normal.y)*0.5;
	normalized_normal = normalize(normalized_normal);
	vec3 normalized_vertex_to_light_vector = normalize(vertex_to_light_vector);
	vec3 normalized_half = normalize(vertex_to_light_vector+vertex_to_cam);
	float specular=clamp(dot(normalized_half,normalized_normal),0.0,1.0);
	
	// Calculating The Diffuse Term And Clamping It To [0;1]
	float DiffuseTerm = clamp(dot(normal, vertex_to_light_vector), 0.0, 1.0);

	if(DiffuseTerm<0.2)
		specular*=DiffuseTerm*5;

	// Calculating The Final Color
	gl_FragColor.rgb = (0.1+0.9*DiffuseTerm*DiffuseTerm)*color.rgb+0.5*specular*specular;
	//*texture2D(noise, worldPos.xy/128+elapsed/20.).r;// color * (DiffuseTerm*(1 - ambientLevel) + ambientLevel);
	/*gl_FragColor.r = (int)(color.r*10.)/10.;//specular;//color.rgb * (specular*DiffuseTerm*(1-ambientLevel) + ambientLevel) - gl_FragColor.rgb*(1-gl_FragColor.a);
	gl_FragColor.g = (int)(color.g*10.)/10.;
	gl_FragColor.b = (int)(color.b*10.)/10.;*/
	gl_FragColor.a = color.a+(specular*specular)/3.;
}