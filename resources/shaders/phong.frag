#version 330 core

in vec3 world_pos;
in vec3 world_normal;
in vec4 camera_pos;
out vec4 fragColor;
in vec2 fragUV;


struct Light {
    vec4 position;
    vec3 color;
    vec4 direction;
};

uniform Light light;

uniform float ka;
uniform float kd;
uniform float ks;

uniform sampler2D myTexture;


void main() {
    float blend = 1.0f;
    vec3 textureColor = vec3(texture(myTexture, fragUV));
    vec3 toCamera = normalize(vec3(camera_pos) - vec3(world_pos));
//    vec3 toLight = normalize(vec3(light.position - world_pos));
    vec3 toLight = vec3(normalize(-light.direction));
    float diffuse  = clamp(dot(normalize(vec3(world_normal)), toLight), 0, 1);

   vec3 reflectedLight = reflect(-toLight,
                                 normalize(vec3(world_normal)));

   float specular = pow(clamp(dot(toCamera, reflectedLight),0,100), 30);

   fragColor = vec4(vec3((kd*(1.0f-blend)+textureColor*blend)*light.color*diffuse + ka*light.color*textureColor + ks * specular*light.color),1);
//   fragColor = vec4(textureColor,1.0);
}
