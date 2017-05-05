varying vec3 normal;
varying vec3 vertex_to_light_vector;
varying vec3 vertex_to_cam;
varying vec4 color;
varying vec3 worldPos;

uniform float ambientLevel;

uniform float elapsed;

float texValue(vec2 position){
	return (sin(position.x+position.y*1.3+elapsed*3.)
			+sin(position.x*0.2+position.y*1.9+1.5+elapsed*7.)
			+cos(-position.x*0.5-position.y*2.7+3.14-elapsed*2.)
			+sin(position.x*1.3-position.y*0.2+4.5+elapsed*6.)
			+cos(-position.x*1.5+position.y*0.5-elapsed*4.)
			+sin(position.x*1.9-position.y*0.4+elapsed)
			+cos(position.x*0.1+position.y*0.4-elapsed*9.)
			+3*cos(position.x*0.02+position.y*0.07-elapsed*1.3))/11.;//4.*(texture2D(noiseTex,position.xy/128.+vec2(cos(elapsed)/30.,elapsed/15.+cos(elapsed)/50.)).r)/2.5;//(texture2D(noiseTex,(position.xy/64)+elapsed/20.).r*2-1);//+((cos((worldPos.x+elapsed/10.))+sin((worldPos.y+elapsed/10.)))/2.);
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
	vec3 final_color = vec3(texValue(worldPos.xy)*color.r,
	 texValue(worldPos.xy)*color.g,
	  0.5*color.b+(0.5*texValue(worldPos.xy)*color.b));
	gl_FragColor.rgb = (0.3+0.6*DiffuseTerm*DiffuseTerm)*final_color+specular*specular*specular*0.5;
	//*texture2D(noise, worldPos.xy/128+elapsed/20.).r;// color * (DiffuseTerm*(1 - ambientLevel) + ambientLevel);
	/*gl_FragColor.r = (int)(color.r*10.)/10.;//specular;//color.rgb * (specular*DiffuseTerm*(1-ambientLevel) + ambientLevel) - gl_FragColor.rgb*(1-gl_FragColor.a);
	gl_FragColor.g = (int)(color.g*10.)/10.;
	gl_FragColor.b = (int)(color.b*10.)/10.;*/
	gl_FragColor.a = color.a;//+(specular*specular)/10.;
}