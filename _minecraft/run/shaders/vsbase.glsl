varying vec3 normal;
varying vec3 vertex_to_light;
varying vec3 vertex_to_cam;
varying vec4 color;
varying vec3 worldPos;
varying vec2 texCoord;
varying mat3 TBN;
varying int simplified;

uniform float elapsed;
uniform mat4 invertView;

float magnitude(vec3 v){
	return v.x*v.x+v.y*v.y+v.z*v.z;
}

void main()
{
	// Transforming The Vertex
	//gl_Vertex.z += (sin((gl_Vertex.x)/20. + (elapsed*6.28)/40.)/10.0f) + cos((gl_Vertex.y)/20. + (elapsed*6.28)/40.)/10.0f;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = gl_ModelViewMatrix * gl_Vertex;
	worldPos = invertView * gl_Position ;
	normal = gl_Normal;

	vertex_to_cam = normalize(invertView*(vec4(0,0,0,1)-gl_Position));
	vertex_to_light = normalize(invertView*gl_LightSource[0].position);

		
	
	
	vec3 tangent;
	vec3 bitangent;
	
	//Définition de la tangente et bitangente
	if(normal.z!=0){
		tangent = vec3(-1,0,0);
		if(normal.z>0)
			bitangent = vec3(0,1,0);
		else
			bitangent = vec3(0,-1,0);
	}
	if(normal.y!=0){
		bitangent = vec3(0,0,-1);
		if(normal.y>0)
			tangent = vec3(1,0,0);
		else
			tangent = vec3(-1,0,0);
	}
	if(normal.x!=0){
		bitangent = vec3(0,0,-1);
		if(normal.x>0)
			tangent = vec3(0,-1,0);
		else
			tangent = vec3(0,1,0);
	}

	TBN = mat3(tangent, bitangent, normal);

	gl_Position = gl_ProjectionMatrix * gl_Position;
	

	// Transforming The Normal To ModelView-Space

	//Direction lumiere
	



	//Couleur
	
	color.rgb = gl_Color.rgb;//*max(0,dot(vertex_to_light_vector, normal));
	color.a = gl_Color.a;
	//normal = normal;
}