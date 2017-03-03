varying vec3 normal;
varying vec3 vertex_to_light_vector;
varying vec4 color;

uniform float elapsed;
uniform mat4 invertView;

void main()
{
	// Transforming The Vertex
	//gl_Vertex.z += (sin((gl_Vertex.x)/20. + (elapsed*6.28)/40.)/10.0f) + cos((gl_Vertex.y)/20. + (elapsed*6.28)/40.)/10.0f;
	
	gl_Position = gl_ModelViewMatrix * gl_Vertex;
	vec4 worldPos = invertView * gl_Position ;
	vec3 vertex_to_cam = normalize(invertView*vec4(0,0,0,1)-worldPos);
	normal = gl_NormalMatrix*gl_Normal; 
	if(gl_Color.b > 0.7){
		gl_Position.z += (cos(worldPos.x*1.4 + (elapsed*6.28)/3.)-1+sin(worldPos.y*0.3 + (elapsed*6.28)/5.)-1+cos(worldPos.x*0.7 + (elapsed*6.28)/5.)-1)*0.9;
		normal.x = sin(worldPos.x*0.7 + (elapsed*6.28)/5.);
		normal.z = 1;
		normal.y = -cos(worldPos.y*0.3 + (elapsed*6.28)/5.)/4.;
		normal = normalize(normal);
	}
	gl_Position = gl_ProjectionMatrix * gl_Position;
	

	// Transforming The Normal To ModelView-Space

	//Direction lumiere
	vertex_to_light_vector = normalize(invertView*gl_LightSource[0].position);

	float specular = max(0,dot(normalize(vertex_to_light_vector+vertex_to_cam),normal));
	if(specular<0.95) specular = 0;
	else specular = (specular-0.95)/0.05;

	//Couleur
	color = gl_Color+specular*0.6;//+specular;//*max(0,dot(vertex_to_light_vector, normal));
	//normal = normal;
}