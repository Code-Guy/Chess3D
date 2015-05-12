// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 T_cameraspace;
in vec3 B_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace1;
in vec3 LightDirection_cameraspace2;
in vec4 ShadowCoord;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2D DiffuseTexture;
uniform sampler2D NormalTexture;
uniform sampler2D SpecularTexture;
uniform sampler2D AmbientTexture;
uniform sampler2DShadow ShadowTexture;

uniform bool UseDiffuseMap = false;
uniform bool UseNormalMap = false;
uniform bool UseSpecularMap = false;
uniform bool UseAmbientMap = false;
uniform bool UseShadowMap = false;

uniform vec3 LightPosition_worldspace1;
uniform vec3 LightPosition_worldspace2;

uniform vec3 LightColor1;
uniform float LightPower1;

uniform vec3 LightColor2;
uniform float LightPower2;

uniform vec3 MaterialDiffuseColor;
uniform vec3 MaterialAmbientColor;
uniform vec3 MaterialSpecularColor;

uniform float Alpha;

vec3 DiffuseColor;
vec3 AmbientColor;
vec3 SpecularColor;

vec3 Normal_tangentspace;
vec3 EyeDirection_tangentspace;
vec3 LightDirection_tangentspace1;
vec3 LightDirection_tangentspace2;

float Visibility = 1.0f;

vec2 PoissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

// Returns a Random number based on a vec3 and an int.
float Random(vec3 seed, int i){
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

vec3 GetLightColor(vec3 LightDirection, vec3 Normal, vec3 EyeDirection, vec3 LightPosition_worldspace, vec3 LightColor, float LightPower);
void CalcVisibility();

void main()
{
	vec2 newUV = vec2(UV.x, 1 - UV.y);

	// Material properties
	DiffuseColor = UseDiffuseMap ? texture2D( DiffuseTexture, newUV ).rgb : MaterialDiffuseColor;
	AmbientColor = UseAmbientMap ? texture2D( AmbientTexture, newUV ).rgb : MaterialAmbientColor * DiffuseColor;
	SpecularColor = UseSpecularMap ? texture2D( SpecularTexture, newUV ).rgb : MaterialSpecularColor;

	if(UseNormalMap)
	{
		mat3 ITBN = transpose(mat3(
        T_cameraspace,
        B_cameraspace,
        Normal_cameraspace
    	));

    	EyeDirection_tangentspace = ITBN * EyeDirection_cameraspace;
    	LightDirection_tangentspace1 = ITBN * LightDirection_cameraspace1;
    	LightDirection_tangentspace2 = ITBN * LightDirection_cameraspace2;

    	Normal_tangentspace = normalize(texture2D( NormalTexture, newUV ).rgb*2.0 - 1.0);
	}

	// Ambient : simulates indirect lighting
	color.rgb =  AmbientColor;

	if(UseShadowMap)
	{
		CalcVisibility();
	}

	if(UseNormalMap)
	{
		color.rgb += Visibility * (GetLightColor(LightDirection_tangentspace1, Normal_tangentspace, EyeDirection_tangentspace, LightPosition_worldspace1, LightColor1, LightPower1) +
					 GetLightColor(LightDirection_tangentspace2, Normal_tangentspace, EyeDirection_tangentspace, LightPosition_worldspace2, LightColor2, LightPower2));
	}
	else
	{
		color.rgb += Visibility * (GetLightColor(LightDirection_cameraspace1, Normal_cameraspace, EyeDirection_cameraspace, LightPosition_worldspace1, LightColor1, LightPower1) +
					 GetLightColor(LightDirection_cameraspace2, Normal_cameraspace, EyeDirection_cameraspace, LightPosition_worldspace2, LightColor2, LightPower2));
	}
		
		
	clamp(color.rgb, 0, 1);

	color.a = Alpha;
}

vec3 GetLightColor(vec3 LightDirection, vec3 Normal, vec3 EyeDirection, vec3 LightPosition_worldspace, vec3 LightColor, float LightPower)
{
	// Distance to the light
	float distance = length( LightPosition_worldspace - Position_worldspace );

	// Normal of the computed fragment, in camera space
	vec3 n = normalize( Normal);
	// Direction of the light (from the fragment to the light)
	vec3 l = normalize( LightDirection );
	// Cosine of the angle between the normal and the light direction, 
	// clamped above 0
	//  - light is at the vertical of the triangle -> 1
	//  - light is perpendicular to the triangle -> 0
	//  - light is behind the triangle -> 0
	float cosTheta = clamp( dot( n,l ), 0,1 );
	
	// Eye vector (towards the camera)
	vec3 E = normalize(EyeDirection);
	// Direction in which the triangle reflects the light
	vec3 R = reflect(-l,n);
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( E,R ), 0,1 );

	// Diffuse : "color" of the object
	return DiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) +
		// Specular : reflective highlight, like a mirror
		SpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distance*distance);

}

void CalcVisibility()
{
	// Fixed bias, or...
	float bias = 0.005;

	// ...variable bias
	// float bias = 0.005*tan(acos(cosTheta));
	// bias = clamp(bias, 0,0.01);

	// Sample the shadow map 4 times
	for (int i=0;i<4;i++){
		// use either :
		//  - Always the same samples.
		//    Gives a fixed pattern in the shadow, but no noise
		int index = i;
		//  - A Random sample, based on the pixel's screen location. 
		//    No banding, but the shadow moves with the camera, which looks weird.
		// int index = int(16.0*Random(gl_FragCoord.xyy, i))%16;
		//  - A Random sample, based on the pixel's position in world space.
		//    The position is rounded to the millimeter to avoid too much aliasing
		// int index = int(16.0*Random(floor(Position_worldspace.xyz*1000.0), i))%16;
		
		// being fully in the shadow will eat up 4*0.2 = 0.8
		// 0.2 potentially remain, which is quite dark.
		Visibility -= 0.2*(1.0-texture( ShadowTexture, vec3(ShadowCoord.xy + PoissonDisk[index]/700.0,  (ShadowCoord.z-bias)/ShadowCoord.w) ));
	}
}