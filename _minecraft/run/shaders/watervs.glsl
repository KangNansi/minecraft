varying vec3 normal;
varying vec3 vertex_to_light_vector;
varying vec3 vertex_to_cam;
varying vec4 color;
varying vec3 worldPos;

uniform float elapsed;
uniform mat4 invertView;

uniform sampler2D noiseTex;

float texValue(vec2 position){
	return (cos(position.y/10.+elapsed)+sin(position.x/10.+elapsed))/4.+4.*(texture2D(noiseTex,position.xy/128.+elapsed/15.))/2.5;//(texture2D(noiseTex,(position.xy/64)+elapsed/20.).r*2-1);//+((cos((worldPos.x+elapsed/10.))+sin((worldPos.y+elapsed/10.)))/2.);
}

void main()
{

	// Transforming The Vertex

	worldPos = invertView * gl_ModelViewMatrix * gl_Vertex ;
	gl_Vertex.z += -2+texValue(worldPos.xy);
	gl_Position = gl_ModelViewMatrix * gl_Vertex;
gl_Position = gl_ProjectionMatrix * gl_Position;
	vertex_to_cam = normalize(invertView*vec4(0,0,0,1)-worldPos);
	// Transforming The Normal To ModelView-Space
	normal.x = (texValue(worldPos.xy+vec2(-1,0))-texValue(worldPos.xy+vec2(1,0)));
	normal.y = (texValue(worldPos.xy+vec2(0,-1))-texValue(worldPos.xy+vec2(0,1)));
	normal.z = 1;
	//normal = gl_NormalMatrix * normal; //+ vec3(texture2D(noiseTex,(worldPos.y/64)+elapsed/20.).r*2-1,texture2D(noiseTex,(worldPos.x/64)+elapsed/20.).r*2-1,0); 
	
	//Direction lumiere
	vertex_to_light_vector = normalize(invertView*gl_LightSource[0].position);

	//Couleur
	color = gl_Color;
}