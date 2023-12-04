#version 330 core

in vec3 world_pos;
in vec3 world_normal;
in vec4 camera_pos;
out vec4 fragColor;

struct Light {
    vec4 position;
    vec3 color;
    vec4 direction;
};

uniform Light light;

uniform float ka;
uniform float kd;
uniform float ks;

void main() {


    vec3 toCamera = normalize(vec3(camera_pos) - vec3(world_pos));
//    vec3 toLight = normalize(vec3(light.position - world_pos));
    vec3 toLight = vec3(normalize(-light.direction));
    float diffuse  = clamp(dot(normalize(vec3(world_normal)),
                              toLight),
                          0,
                          1);

   vec3 reflectedLight = reflect(-toLight,
                                 normalize(vec3(world_normal)));

   float specular = pow(clamp(dot(toCamera, reflectedLight),0,100), 30);

   fragColor = vec4(vec3(kd * diffuse*light.color + ka*light.color + ks * specular*light.color),1);
}
