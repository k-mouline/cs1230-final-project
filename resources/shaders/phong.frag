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

uniform float xOffset;
uniform float yOffset;

void main() {
    float blend = 1.0f;
    vec4 texCol = texture(myTexture, vec2(fragUV.x + xOffset, fragUV.y + yOffset));
    vec3 textureColor = vec3(texCol);
    vec3 toCamera = normalize(vec3(camera_pos) - vec3(world_pos));
    vec3 toLight = vec3(normalize(-light.direction));
    float diffuse  = clamp(dot(normalize(vec3(world_normal)), toLight), 0, 1);

    vec3 reflectedLight = reflect(-toLight, normalize(vec3(world_normal)));

    float specular = pow(clamp(dot(toCamera, reflectedLight),0,100), 30);

    float distance = length(vec3(camera_pos) - vec3(world_pos));
    float opacity = (distance > 25.f) ? 1.f - clamp((distance - 25.f) / 10.f, 0.f, 1.f): texCol[3];

    fragColor = vec4(vec3((kd*(1.0f-blend)+textureColor*blend)*light.color*diffuse +
                          ka*light.color*textureColor + ks * specular*light.color), opacity);
}
