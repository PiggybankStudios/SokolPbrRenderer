@ctype mat4 mat4
@ctype vec2 v2
@ctype vec3 v3
@ctype vec4 v4r

// +--------------------------------------------------------------+
// |                        Vertex Shader                         |
// +--------------------------------------------------------------+
@vs vertex_shader

layout(binding=0) uniform pbr_VertParams
{
	uniform mat4 world;
	uniform mat4 view;
	uniform mat4 projection;
};

in vec3 position;
in vec3 normal;
in vec2 texCoord0;
in vec4 color0;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragSampleCoord;
out vec4 fragColor;

void main()
{
	gl_Position = projection * (view * (world * vec4(position, 1.0f)));
	fragPosition = (world * vec4(position, 1.0f)).xyz;
	fragNormal = (world * vec4(normal, 0.0f)).xyz;
	fragSampleCoord = texCoord0;
	fragColor = color0;
}
@end

// +--------------------------------------------------------------+
// |                       Fragment Shader                        |
// +--------------------------------------------------------------+
@fs fragment_shader

layout(binding=1) uniform pbr_FragParams
{
	uniform vec4 tint;
	uniform vec4 lightPos;
	uniform vec4 cameraPos;
};
layout(binding=0) uniform texture2D pbrAlbedoTexture;
layout(binding=0) uniform sampler pbrAlbedoSampler;

layout(binding=1) uniform texture2D pbrNormalTexture;
layout(binding=1) uniform sampler pbrNormalSampler;

layout(binding=2) uniform texture2D pbrMetallicTexture;
layout(binding=2) uniform sampler pbrMetallicSampler;

layout(binding=3) uniform texture2D pbrRoughnessTexture;
layout(binding=3) uniform sampler pbrRoughnessSampler;

layout(binding=4) uniform texture2D pbrOcclusionTexture;
layout(binding=4) uniform sampler pbrOcclusionSampler;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragSampleCoord;
in vec4 fragColor;
out vec4 frag_color;

const float PI = 3.141592653589793238462643383279502884197;

float DistributionGGX(vec3 normal, vec3 halfVec, float roughness)
{
	float roughnessSquared = roughness * roughness;
	float normalDotHalfVec = max(dot(normal, halfVec), 0.0f);
	float normalDotHalfVecSquared = normalDotHalfVec * normalDotHalfVec;
	float numerator = roughnessSquared;
	float denominator = (normalDotHalfVecSquared * (roughnessSquared - 1.0f) + 1.0f);
	denominator = PI * denominator * denominator;
	return numerator / denominator;
}

// The geometry function statistically approximates the relative surface area where its micro surface-details overshadow each other, causing light rays to be occluded. 
float GeometrySchlickGGX(float normalDotLightOrView, float roughness)
{
	float numerator = normalDotLightOrView;
	float denominator = normalDotLightOrView * (1.0f - roughness) + roughness;
	return numerator / denominator;
}
float GeometrySmith(vec3 normal, vec3 viewDir, vec3 lightVec, float roughness)
{
	float normalDotView = max(dot(normal, viewDir), 0.0f);
	float normalDotLight = max(dot(normal, lightVec), 0.0f);
	float geometryObstruction = GeometrySchlickGGX(normalDotView, roughness);
	float geometryShadowing = GeometrySchlickGGX(normalDotLight, roughness);
	return geometryObstruction * geometryShadowing;
}

vec4 fromLinear(vec4 linearRGB)
{
	bvec4 cutoff = lessThan(linearRGB, vec4(0.0031308));
	vec4 higher = vec4(1.055)*pow(linearRGB, vec4(1.0/2.4)) - vec4(0.055);
	vec4 lower = linearRGB * vec4(12.92);
	return mix(higher, lower, cutoff);
}

vec4 toLinear(vec4 sRGB)
{
	bvec4 cutoff = lessThan(sRGB, vec4(0.04045));
	vec4 higher = pow((sRGB + vec4(0.055))/vec4(1.055), vec4(2.4));
	vec4 lower = sRGB/vec4(12.92);
	return mix(higher, lower, cutoff);
}

void main()
{
	vec3 surfaceNormal = texture(sampler2D(pbrNormalTexture, pbrNormalSampler), fragSampleCoord).xyz * 2.0f - vec3(1.0f, 1.0f, 1.0f);
	vec4 albedo = toLinear(texture(sampler2D(pbrAlbedoTexture, pbrAlbedoSampler), fragSampleCoord));
	vec4 ambientOcclusion = texture(sampler2D(pbrOcclusionTexture, pbrOcclusionSampler), fragSampleCoord);
	float roughness = texture(sampler2D(pbrRoughnessTexture, pbrRoughnessSampler), fragSampleCoord).r;
	
	vec3 normalVec = normalize(fragNormal);
	vec3 lightVec = normalize(lightPos.xyz - fragPosition);
	vec3 viewDir = normalize(cameraPos.xyz - fragPosition);
	vec3 halfVec = (normalVec + lightVec) / 2.0f;
	
	float normalDistributionValue = DistributionGGX(normalVec, halfVec, roughness);
	vec4 normalDistributionMult = vec4(normalDistributionValue, normalDistributionValue, normalDistributionValue, 1.0f);
	
	float directLightRoughness = ((roughness + 1) * (roughness + 1)) / 8.0f;
	float geometryValue = GeometrySmith(normalVec, viewDir, lightVec, directLightRoughness);
	vec4 geometryMult = vec4(geometryValue, geometryValue, geometryValue, 1.0f);
	
	// frag_color = fragColor * albedo * tint;
	// frag_color = fragColor * albedo * tint * vec4((fragNormal + vec3(1.0f,1.0f,1.0f))/2.0f, 1.0f);
	// vec4 positionMult = vec4(
	// 	(sin(fragPosition.x * 3.1415926f) + 1.0f)/2.0f,
	// 	(sin(fragPosition.y * 3.1415926f) + 1.0f)/2.0f,
	// 	(sin(fragPosition.z * 3.1415926f) + 1.0f)/2.0f,
	// 	1.0f
	// );
	// frag_color = fragColor * albedo * tint * positionMult;
	// float lightDistance = length(cameraPos.xyz - fragPosition);
	// vec4 lightMult = vec4(1.0f - lightDistance/3.5f, 1.0f - lightDistance/4.0f, 1.0f - lightDistance/6.0f, 1.0f);
	// frag_color = fragColor * albedo * tint * lightMult;
	
	// float lightDot = dot(fragNormal + surfaceNorma, normalize(lightPos.xyz - fragPosition));
	// vec4 lightMult = vec4(lightDot, lightDot, lightDot, 1.0f);
	
	vec4 addedMult = vec4(vec3(normalDistributionValue + geometryValue), 1.0f);
	frag_color = fromLinear(toLinear(fragColor) * albedo * toLinear(tint) * addedMult);
}
@end

@program pbr vertex_shader fragment_shader
