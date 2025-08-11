#version 330 core

uniform vec3 sunDirection;
uniform float iTime;
uniform vec3 cameraPosition;

uniform vec3 ka;
uniform vec3 Ia;
uniform vec3 kd;
uniform vec3 Id;
uniform vec3 ks;
uniform vec3 Is;
uniform float shininess;

uniform samplerCube skybox;

in vec3 position;
in vec3 normal;

out vec4 fragColor;

float calculate_fresnel(vec3 camera_direction, vec3 normal) {
	return pow(1 - dot(camera_direction, normal), 5);
}

vec3 calculate_ambient(vec3 ka, vec3 Ia) {
	return ka * Ia;
}

vec3 calculate_diffuse(vec3 kd, vec3 Ia, vec3 normal, vec3 light_direction, vec3 position, vec3 camera_position, samplerCube skybox) {
	vec3 camera_direction = normalize(camera_position - position);
	vec3 reflection_angle = reflect(-camera_direction, normal);

	return mix(kd, texture(skybox, reflection_angle).rgb, calculate_fresnel(camera_direction, normal)) * Id * max(0, dot(normal, light_direction));
}

vec3 calculate_specular(vec3 ks, vec3 Is, vec3 normal, vec3 light_direction, vec3 position, vec3 camera_position, float shininess) {
	vec3 reflection = reflect(-light_direction, normal);
	vec3 camera_direction = normalize(camera_position - position);

	return ks * Is * pow(max(0, dot(camera_direction, reflection)), shininess) * calculate_fresnel(camera_direction, normal);
}

void main() {
	vec3 direction = normalize(vec3(cos(iTime), sin(iTime), 0.0));
	vec3 ambient = calculate_ambient(ka, Ia);
	vec3 diffuse = calculate_diffuse(kd, Ia, normal, sunDirection, position, cameraPosition, skybox);
	vec3 specular = calculate_specular(ks, Is, normal, sunDirection, position, cameraPosition, shininess);

	vec3 color = ambient + diffuse + specular;
	fragColor = vec4(color, 1.0);
}