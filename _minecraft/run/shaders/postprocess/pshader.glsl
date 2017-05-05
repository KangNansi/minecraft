uniform sampler2D Texture0;
uniform sampler2D Texture1;
uniform float screen_width;
uniform float screen_height;

float stepx = 1/screen_width;
float stepy = 1/screen_height;

float LinearizeDepth(float z)
{
	float n = 0.5; // camera z near
  	float f = 10000.0; // camera z far
  	return (2.0 * n) / (f + n - z * (f - n));
}

void oil(int radius){
	vec2 src_size = vec2(screen_width, screen_height);
    vec2 uv = gl_TexCoord[0].xy;
    float n = float((radius + 1) * (radius + 1));

    vec3 m[4];
    vec3 s[4];
    for (int k = 0; k < 4; ++k) {
        m[k] = vec3(0.0);
        s[k] = vec3(0.0);
    }

    for (int j = -radius; j <= 0; ++j)  {
        for (int i = -radius; i <= 0; ++i)  {
            vec3 c = texture2D(Texture0, uv + vec2(i,j) / src_size).rgb;
            m[0] += c;
            s[0] += c * c;
        }
    }

    for (int j = -radius; j <= 0; ++j)  {
        for (int i = 0; i <= radius; ++i)  {
            vec3 c = texture2D(Texture0, uv + vec2(i,j) / src_size).rgb;
            m[1] += c;
            s[1] += c * c;
        }
    }

    for (int j = 0; j <= radius; ++j)  {
        for (int i = 0; i <= radius; ++i)  {
            vec3 c = texture2D(Texture0, uv + vec2(i,j) / src_size).rgb;
            m[2] += c;
            s[2] += c * c;
        }
    }

    for (int j = 0; j <= radius; ++j)  {
        for (int i = -radius; i <= 0; ++i)  {
            vec3 c = texture2D(Texture0, uv + vec2(i,j) / src_size).rgb;
            m[3] += c;
            s[3] += c * c;
        }
    }


    float min_sigma2 = 1e+2;
    for (int k = 0; k < 4; ++k) {
        m[k] /= n;
        s[k] = abs(s[k] / n - m[k] * m[k]);

        float sigma2 = s[k].r + s[k].g + s[k].b;
        if (sigma2 < min_sigma2) {
            min_sigma2 = sigma2;
            gl_FragColor = vec4(m[k], 1.0);
        }
    }
}

void main (void)
{
	float xstep = 1.0/screen_width;
	float ystep = 1.0/screen_height;
	float ratio = screen_width / screen_height;

	vec4 rcol = texture2D(Texture0, gl_TexCoord[0].xy);
	vec4 mcol = 0;
	vec4 ccol = 0;
	float depth = texture2D( Texture1 , vec2( gl_TexCoord[0] ) ).r;	
	float light_factor = 0;
	float depth_outline = 0;
	float luminosity = 0.5;
	float depth_max = 0;
	float depth_min = 10000;
	float close_nb = 0;
	//Permet de scaler la profondeur
	depth = LinearizeDepth(depth);
	//Bloom
	for(int i=-3; i<=3;i++){
		for(int j=-3;j<=3;j++){
			ccol = texture2D(Texture0, gl_TexCoord[0].xy+vec2(i*stepx, j*stepy));
			mcol += ccol;
			float d = LinearizeDepth(texture2D(Texture1, gl_TexCoord[0].xy+vec2(i*stepx, j*stepy)).r);
			depth_max = max(depth-d,depth_max);
			depth_min = min(depth-d,depth_max);
			if(d<depth){
				depth_outline+= abs(depth-d);
				close_nb++;
			}
			light_factor += (ccol.r*ccol.r+ccol.g*ccol.g+ccol.b*ccol.b)/3.;
		}
	}
	mcol/=(7*7);
	oil(4);
	depth_outline/=close_nb;
	
	light_factor/=(7*7);
	light_factor*=light_factor;
	
	rcol = texture2D(Texture0, gl_TexCoord[0].xy);
	
	//Brouillard de distance
	if(depth>0.4){ //Si c'est loin, on affiche normalement, pour pas que le soleil disparaisse
		gl_FragColor.rgb = rcol.rgb;
		gl_FragColor.a = 1;
	}
	else{
		float normDepth = clamp(depth/0.005,0,1);
		rcol = rcol*(1-normDepth)+gl_FragColor*normDepth;
		rcol = rcol*(1-light_factor)+light_factor;
		if(abs(depth_min+depth_max)>0.0001)// && depth_min<0.0003)
			rcol*=clamp(((1-depth_outline/depth_max)),0,1);
		gl_FragColor.rgb = rcol.rgb;//*(1-depth*20);
		gl_FragColor.a = (1-depth*25); //L'alpha diminue en fonction de la profondeur du pixel pour donner l'effet brouillard
	}
	//+vec3(1,1,1)*(depth*7);//(1-mult);//color;
	//gl_FragColor.a = 1;
	//oil(3);
}