#define FLOWER 0
#define BRANCH 1
#define LEAF 2

struct FoliageBehavior {
  int type;
  float speed;
};

uniform FoliageBehavior behavior;
uniform float time;

vec3 getFlowerFoliageOffset(vec3 world_position) {
  float vertex_distance_from_ground = abs(vertexPosition.y);
  float rate = time * behavior.speed;
  float x_3 = world_position.x / 3.0;
  float z_3 = world_position.z / 3.0;
  float displacement_factor = pow(vertex_distance_from_ground / 1.5, 2);

  // @todo allow some of these magic numbers to be configurable
  float offset_x = sin(rate + x_3 + z_3) + cos(rate * 3.1) * 0.05;
  float offset_y = 0.0;
  float offset_z = sin(rate + z_3) + sin(rate * 1.3) * 0.4;

  return vec3(offset_x, offset_y, offset_z) * displacement_factor;
}

vec3 getBranchFoliageOffset(vec3 world_position) {
  // @todo
  return vec3(0);
}

vec3 getLeafFoliageOffset(vec3 world_position) {
  float x_3 = world_position.x / 3.0;
  float y_3 = world_position.y / 3.0;
  float z_3 = world_position.z / 3.0;
  float rate = time * behavior.speed;

  float offset_x = sin(rate + z_3) + sin(rate * 0.5 + z_3) * 0.5;
  float offset_y = cos(rate + x_3) + cos(rate * 0.7 + x_3) * 0.5;
  float offset_z = sin(rate + y_3) + sin(rate * 0.9 + y_3) * 0.5;

  return vec3(offset_x, offset_y, offset_z) * 0.1;
}