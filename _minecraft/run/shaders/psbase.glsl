varying vec3 normal;
varying vec3 vertex_to_light;
varying vec3 vertex_to_cam;
varying vec3 worldPos;
varying vec4 color;
varying vec2 texCoord;
varying mat3 TBN;

uniform float ambientLevel;
uniform float elapsed;
uniform sampler2D Texture;
uniform sampler2D TexNormal;

void main()
{

	//Normal de base du fragment
	vec3 base_normal = normalize(normal);

	//Normal de la normal map en world space
	vec3 normal_map = texture2D(TexNormal, gl_TexCoord[0]).rgb*2-1;
	vec3 normalized_normal = normalize(TBN * normal_map);
	

	vec3 normalized_vertex_to_light_vector = normalize(vertex_to_light);
	vec3 normalized_half = normalize((vertex_to_light+vertex_to_cam));

	//Calcul de la diffuse de base, permets d'éviter certains artefacts
	float normal_diffuse = clamp(dot(base_normal, vertex_to_light), 0.0, 1.0);

	//Calcul de la speculaire avec un half vector
	float specular=clamp(dot(normalized_half,normalized_normal),0.0,1.0);

	//Calcul de la diffuse avec la normal map
	float DiffuseTerm = clamp(dot(normalized_normal, vertex_to_light), 0.0, 1.0);

	//On fade la speculaire quand la diffuse s'approche de 0
	//Permets d'éviter de voir la speculaire quand la lumiere est de l'autre coté du polygone
	if(normal_diffuse<0.2){
		specular*=normal_diffuse*5;
		DiffuseTerm*=normal_diffuse*5;
	}
	
	//color.rgb *= specular;
	vec4 final_color = texture2D(Texture, gl_TexCoord[0].xy);//gl_TexCoord[0].xy*(128/32.);
	final_color.rgb = (0.2+(0.6*DiffuseTerm+(0.2*specular)))*final_color.rgb;

	gl_FragColor.rgb = final_color.rgb;

	
	gl_FragColor.a = final_color.a;
}