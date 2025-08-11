#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float iTime;

out vec3 position;
out vec3 normal;

uniform uint wave_count;
uniform float desired_amplitude;

uniform float start_amplitude;
uniform float start_frequency;
uniform float start_speed;

uniform float decay_amplitude;
uniform float decay_frequency;
uniform float decay_speed;
uniform float delta_seed;

uniform float peak_offset;
uniform float peak_height;


float euler_height(vec2 position, vec2 direction, float amplitude, float frequency, float speed) {
	return amplitude * exp(peak_height * sin(frequency * dot(direction, position) - iTime * frequency * speed) + peak_offset);
}

float euler_tangent(vec2 position, vec2 direction, float amplitude, float frequency, float speed) {
	return amplitude * exp(peak_height * sin(frequency * dot(direction, position) - iTime * frequency * speed) + peak_offset) * cos(frequency * dot(direction, position) - iTime * frequency * speed) * frequency * direction.x;
}

float euler_binormal(vec2 position, vec2 direction, float amplitude, float frequency, float speed) {
	return amplitude * exp(peak_height * sin(frequency * dot(direction, position) - iTime * frequency * speed) + peak_offset) * cos(frequency * dot(direction, position) - iTime * frequency * speed) * frequency * direction.y;
}

void main() {
	float height = 0.0;
	vec3 tangent = vec3(1.0, 0.0, 0.0);
	vec3 binormal = vec3(0.0, 0.0, 1.0);

	float total_amplitude = 0.0;
	float amplitude = start_amplitude;
	float frequency = start_frequency;
	float speed = start_speed;
	float seed = 0;
	vec2 sample_position = aPos.xz;
	for (uint i = 0u; i < wave_count; i++) {
		vec2 direction = vec2(cos(seed), sin(seed));

		height += euler_height(sample_position, direction, amplitude, frequency, speed);
		float x_partial = euler_tangent(sample_position, direction, amplitude, frequency, speed);
		float z_partial = euler_binormal(sample_position, direction, amplitude, frequency, speed);
		tangent.y += x_partial;
		binormal.y += z_partial;

		sample_position += vec2(x_partial, z_partial);
		total_amplitude += amplitude;
		amplitude *= decay_amplitude;
		frequency *= decay_frequency;
		speed *= decay_speed;
		seed += delta_seed;
	}

	height /= total_amplitude;
	tangent.y /= total_amplitude;
	binormal.y /= total_amplitude;
	height *= desired_amplitude;
	tangent.y *= desired_amplitude;
	binormal.y *= desired_amplitude;

	position = vec3(model * vec4(aPos.x, height, aPos.z, 1.0));
	normal = cross(normalize(binormal), normalize(tangent));
	gl_Position = projection * view * model * vec4(position, 1.0);
}