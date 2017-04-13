varying vec3 normal;
varying vec3 vertex_to_light_vector;
varying vec3 vertex_to_cam;
varying vec3 worldPos;
varying vec4 color;

uniform float ambientLevel;
uniform float elapsed;

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void main()
{
	// Scaling The Input Vector To Length 1
	//normal.x=cos(worldPos.x*2+elapsed);
	//normal.y=sin(worldPos.y*4+elapsed);
	vec3 normalized_normal = normalize(normal);
	if(color.b>0.5){
		normalized_normal.x += (cos(worldPos.x*3.+(elapsed*6.28))
							+cos(worldPos.x*2.+(elapsed*6.28)/2.)
							+cos(worldPos.x+(elapsed*6.28)/4.))/9.;
		normalized_normal.y += (sin(worldPos.y*3.+(elapsed*6.28))
							+sin(worldPos.y*2.+(elapsed*6.28)/2.)
							+sin(worldPos.y+(elapsed*6.28)/4.))/9.;
		normalized_normal = normalize(normalized_normal);
	}
	vec3 normalized_vertex_to_light_vector = normalize(vertex_to_light_vector);
	vec3 normalized_half = normalize(vertex_to_light_vector+vertex_to_cam);
	float specular = max(0,dot(normalized_half,normalized_normal));
	// Calculating The Diffuse Term And Clamping It To [0;1]
	float DiffuseTerm = clamp(dot(normalized_normal, vertex_to_light_vector), 0.0, 1.0);

	// Calculating The Final Color
	color.rgb *= (0.5+specular/2.);
	gl_FragColor.rgb = color.rgb;
	gl_FragColor.r = (int)(color.r*20.)/20.;//specular;//color.rgb * (specular*DiffuseTerm*(1-ambientLevel) + ambientLevel) - gl_FragColor.rgb*(1-gl_FragColor.a);
	gl_FragColor.g = (int)(color.g*20.)/20.;
	gl_FragColor.b = (int)(color.b*20.)/20.;
	
	gl_FragColor.a = color.a;
}