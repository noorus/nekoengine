#version 450 core
#extension GL_ARB_shading_language_include : require

#include "inc.buffers.glsl"

in VertexData {
  vec3 normal;
  vec2 texcoord;
  vec3 fragpos;
} vs_out;

layout ( location = 0 ) out vec4 out_color;
layout ( location = 1 ) out vec4 out_gbuffer;

uniform sampler2D texAlbedo;
uniform sampler2D texHeight;
uniform sampler2D texMetallic;
uniform sampler2D texNormal;

uniform float gamma;
uniform mat4 model;
uniform vec3 camera;

#include "inc.colorutils.glsl"

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
  float a = roughness*roughness;
  float a2 = a*a;
  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH*NdotH;

  float nom   = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = (c_PI * denom * denom);

  return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
  float r = (roughness + 1.0);
  float k = (r*r) / 8.0;

  float nom   = NdotV;
  float denom = NdotV * (1.0 - k) + k;

  return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float ggx2 = GeometrySchlickGGX(NdotV, roughness);
  float ggx1 = GeometrySchlickGGX(NdotL, roughness);

  return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
  return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

// world.pointLights[i], vs_out.fragpos, viewdir, albedo, smoothness, height, metallic, normal
vec3 pointLight(PointLight light, vec3 fragpos, vec3 viewdir, vec3 albedo, float roughness, float height, float metallic, vec3 normal)
{
  // calculate per-light radiance
  vec3 L = normalize(light.position.xyz - fragpos);
  vec3 H = normalize(viewdir + L);
  float distance = length(light.position.xyz - fragpos);
  float attenuation = 1.0 / (distance * distance);
  vec3 radiance = light.color.rgb * attenuation;

  // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
  // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metallic);

  // Cook-Torrance BRDF
  float NDF = DistributionGGX(normal, H, roughness);
  float G   = GeometrySmith(normal, viewdir, L, roughness);
  vec3 F    = fresnelSchlick(max(dot(H, viewdir), 0.0), F0);

  vec3 numerator    = NDF * G * F;
  float denominator = 4 * max(dot(normal, viewdir), 0.0) * max(dot(normal, L), 0.0) + 0.001;
  vec3 specular = numerator / denominator;

  // kS is equal to Fresnel
  vec3 kS = F;
  // for energy conservation, the diffuse and specular light can't
  // be above 1.0 (unless the surface emits light); to preserve this
  // relationship the diffuse component (kD) should equal 1.0 - kS.
  vec3 kD = vec3(1.0) - kS;
  // multiply kD by the inverse metalness such that only non-metals
  // have diffuse lighting, or a linear blend if partly metal (pure metals
  // have no diffuse light).
  kD *= 1.0 - metallic;

  // scale light by NdotL
  float NdotL = max(dot(normal, L), 0.0);
  return (kD * albedo / c_PI + specular) * radiance * NdotL;
}

void main()
{
  /*vec4 smp = texture(texAlbedo, vs_out.texcoord);
  vec3 albedo = pow(smp.rgb, vec3(2.2));
  float smoothness = smp.a;
  float roughness = (1.0 - smoothness);
  float height = texture(texHeight, vs_out.texcoord).r;
  smp = texture(texMetallic, vs_out.texcoord);
  float metallic = smp.r;
  vec3 normal = texture(texNormal, vs_out.texcoord).rgb;
  normal = normalize(normal * 2.0 - 1.0);*/

  vec2 tc = vec2(vs_out.texcoord.x, 1 - vs_out.texcoord.y);

  float height = 1;
  vec4 smp = texture(texAlbedo, tc);
  vec3 albedo = pow(smp.rgb, vec3(2.2));
  float roughness = 0;
  float metallic = texture(texMetallic, tc).r;
  vec3 normal = texture(texNormal, tc).rgb;
  normal = normalize(normal * 2.0 - 1.0);

  vec3 viewdir = normalize( world.camera.position.xyz - vs_out.fragpos );

  vec3 Lo = vec3(0.0);
  for (int i = 0; i < c_pointLightCount; i++)
  {
    if ( world.pointLights[i].dummy.r > 0.1 )
      Lo += pointLight(world.pointLights[i], vs_out.fragpos, viewdir, albedo, roughness, height, metallic, normal);
  }

  vec3 ambient = processing.ambient.rgb * albedo;

  vec3 color = ambient + Lo;

  float brightness = bt709LumaExtract( color );
  vec3 bloomColor = color * vec3(0.05) * brightness;
  bloomColor += vec3(smoothstep(0.1, 0.5, brightness)) * color;
  out_gbuffer = vec4(bloomColor, 1.0);

  out_color = vec4(color, 1.0);
}