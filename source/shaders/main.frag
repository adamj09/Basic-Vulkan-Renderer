#version 460

layout(location = 0) in vec3 inFragColor;
layout(location = 1) in vec3 inFragPosWorld;
layout(location = 2) in vec3 inFragNormalWorld;
layout(location = 3) in vec2 inFragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sceneUbo{
  mat4 projection;
  mat4 view;
  mat4 inverseView;

  bool enableOcclusionCulling;
  bool enableFrustumCulling;

  vec3 planeNormals;
  vec3 planePoints;

  uint instanceCount;
} globalUBO;

layout(set = 0, binding = 1) uniform objectDubo{
  uint modelId;
  uint diffuseId;

  uint boundSphereRadius;
  vec3 boundingSphereCenter;
} object;

layout(set = 0, binding = 2) uniform sampler texSampler;
layout(set = 0, binding = 3) uniform texture2D textures[1];

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -3.0, -1.0));
const float AMBIENT = 0.02f;

void main(){
  //vec3 cameraPosWorld = globalUBO.inverseView[3].xyz;
  //vec3 viewDirection = normalize(cameraPosWorld - inFragPosWorld);

  float lightIntensity = max(dot(inFragNormalWorld, DIRECTION_TO_LIGHT), 0);
  vec4 fragmentColor = vec4(inFragColor, 1.0) * lightIntensity + AMBIENT;

  if(object.diffuseId != -1)
    outColor = texture(sampler2D(textures[object.diffuseId], texSampler), inFragTexCoord) * fragmentColor;
  else
    outColor = vec4(inFragColor * lightIntensity + AMBIENT, 1.0);
}