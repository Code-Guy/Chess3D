uniform vec3 BrickColor, MortarColor;
uniform vec2 BrickSize;
uniform vec2 BrickPct;

in vec2 MCposition;
in float LightIntensity;

out vec4 FragColor;

vec2 CircleMask(vec2 position);

void main()
{
	vec3 color;
	vec2 position, useBrick;
	position = MCposition / BrickSize;

	if (fract(position.y * 0.5) > 0.5)
		position.x += 0.5;

	position = fract(position);

	//useBrick = step(position, BrickPct);
	useBrick = CircleMask(position);

	color = mix(MortarColor, BrickColor, useBrick.x * useBrick.y);
	color *= LightIntensity;
	FragColor = vec4(color, 1.0);
}

vec2 CircleMask(vec2 position)
{
	vec2 center = BrickPct / 2;
	float radius = BrickPct.y / 2;

	if(distance(position, center) < radius)
		return vec2(1, 1);
	return vec2(0, 0);
}