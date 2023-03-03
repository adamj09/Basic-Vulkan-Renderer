#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out vec2 fragTexCoord;

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

layout(set = 0, binding = 1) uniform modelDubo{
  uint modelId;
  uint diffuseId;

  uint boundSphereRadius;
  vec3 boundingSphereCenter;
  
  mat4 modelMatrix;
  mat3 normalMatrix;
} model;

void main(){
  vec4 positionWorld = model.modelMatrix * vec4(inPosition, 1.0);
  gl_Position = globalUBO.projection * globalUBO.view * positionWorld;
  fragNormalWorld = normalize(mat3(model.normalMatrix) * inNormal);
  fragPosWorld = positionWorld.xyz;
  fragColor = inColor;
  fragTexCoord = inTexCoord;
}