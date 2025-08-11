#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "wave.hpp"
#include "camera.hpp"


#define rgb(r, g, b) (glm::vec3((r) / 255.0f, (g) / 255.0f, (b) / 255.0f))

static void framebuffer_size_callback(GLFWwindow*, int, int);
static void mouse_callback(GLFWwindow*, double, double);
static void scroll_callback(GLFWwindow*, double, double);
static void mouse_button_callback(GLFWwindow*, int, int, int);
static void processInput(GLFWwindow*);
static void renderImGui();

// settings
const unsigned int SCREEN_WIDTH = 1200;
const unsigned int SCREEN_HEIGHT = 900;

// lighting
glm::vec3 ambient = rgb(0, 26, 51);
glm::vec3 diffuse = rgb(24, 70, 117);
glm::vec3 specular = rgb(244, 214, 118);
float shininess = 5.0f;

float sun_angle = 90.0f;

// waves
float desired_amplitude = 4.693f;
float start_amplitude = .1f; // .5f
float start_frequency = 0.015f; // .05f
float start_speed = 27.974f; // 26.647f
int wave_count = 64;

float decay_amplitude = 0.893f; // 0.873f
float decay_frequency = 1.078f; // 1.1f
float decay_speed = 0.963f;
float delta_seed = 5762.923f;

float peak_offset = -1.246f;
float peak_height = 2.785f;

// camera
Camera camera(0.0f, 50.0f, 140.0f, 0.0f, 1.0f, 0.0f, YAW, -15.0f);
float last_x = SCREEN_WIDTH / 2.0f;
float last_y = SCREEN_HEIGHT / 2.0f;
bool first_mouse = true;

// timing
float last_frame = 0.0f;
float delta_time = 0.0f;

int main() {
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "wave1", NULL, NULL);

	if (window == NULL) {
		std::cout << "Failed to create window." << std::endl;
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);	
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD." << std::endl;
		return 2;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	glEnable(GL_DEPTH_TEST);
	glClearColor(210 / 255.0f, 180 / 255.0f, 140 / 255.0f, 1.0f);
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	GLfloat skyboxVertices[] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*) 0);
	glEnableVertexAttribArray(0);

	std::vector<std::string> textures {
		"skybox/right.bmp",
		"skybox/left.bmp",
		"skybox/top.bmp",
		"skybox/bottom.bmp",
		"skybox/front.bmp",
		"skybox/back.bmp"
	};

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	int width, height, nrChannels;
	
	for (unsigned int i = 0; i < textures.size(); i++) {
		unsigned char* data = stbi_load(textures[i].c_str(), &width, &height, &nrChannels, 0);
		if (data != nullptr) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		} else {
			std::cout << "Couldn't load texture at path " << textures[i] << std::endl;
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	Shader skybox_shader("skybox.vert", "skybox.frag");

	Wave wave(2000.0, 6000);
	Shader shader("wave.vert", "wave.frag");
	shader.use();
	shader.set("Ia", glm::vec3(0.2f, 0.2f, 0.2f));
	shader.set("Id", glm::vec3(0.8f, 0.8f, 0.8f));
	shader.set("Is", glm::vec3(1.0f, 1.0f, 1.0f));

	while (!glfwWindowShouldClose(window)) {
		float current_frame = static_cast<float>(glfwGetTime());
		delta_time = current_frame - last_frame;
		last_frame = current_frame;
		
		processInput(window);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float) SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 100000.0f);
		shader.set("projection", projection);

		glm::mat4 view = camera.GetViewMatrix();
		shader.set("view", view);

		glDepthMask(GL_FALSE);
		skybox_shader.use();
		skybox_shader.set("projection", projection);
		skybox_shader.set("view", glm::mat4(glm::mat3(view)));
		glBindVertexArray(vao);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthMask(GL_TRUE);

		shader.use();
		glm::mat4 model(1.0f);
		shader.set("model", model);
		shader.set("iTime", (float) glfwGetTime());
		shader.set("cameraPosition", camera.Position);
		shader.set("sunDirection", glm::vec3(glm::cos(glm::radians(sun_angle)), glm::sin(glm::radians(sun_angle)), 0.0f));

		shader.set("ka", ambient);
		shader.set("kd", diffuse);
		shader.set("ks", specular);
		shader.set("shininess", shininess);

		shader.set("desired_amplitude", desired_amplitude);
		shader.set("wave_count", (unsigned int) wave_count);
		shader.set("peak_height", peak_height);
		shader.set("peak_offset", peak_offset);

		shader.set("start_amplitude", start_amplitude);
		shader.set("start_frequency", start_frequency);
		shader.set("start_speed", start_speed);

		shader.set("decay_amplitude", decay_amplitude);
		shader.set("decay_frequency", decay_frequency);
		shader.set("decay_speed", decay_speed);
		shader.set("delta_seed", delta_seed);
		
		
		wave.draw();

		renderImGui();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

static void renderImGui() {
	ImGui::Begin("Wave Attributes");
	ImGui::SliderInt("Wave Count", &wave_count, 1, 128);
	ImGui::SliderFloat("Desired Amplitude", &desired_amplitude, 0.5f, 100.0f);
	ImGui::SliderFloat("Peak Height", &peak_height, 0.0f, 10.0f);
	ImGui::SliderFloat("Peak Offset", &peak_offset, -10.0f, 10.0f);
	ImGui::Separator();

	ImGui::Text("Starting Values");
	ImGui::SliderFloat("Start Amplitude", &start_amplitude, 0.1f, 10.0f);
	ImGui::SliderFloat("Start Frequency", &start_frequency, 0.0f, 0.1f);
	ImGui::SliderFloat("Start Speed", &start_speed, 0.5f, 100.0f);
	ImGui::Separator();

	ImGui::Text("Change");
	ImGui::SliderFloat("Decay Amplitude", &decay_amplitude, 0.0f, 1.0f);
	ImGui::SliderFloat("Decay Frequency", &decay_frequency, 1.0f, 2.0f);
	ImGui::SliderFloat("Decay Speed", &decay_speed, 0.0f, 2.0f);
	ImGui::SliderFloat("Delta Seed", &delta_seed, 0.0f, 10000.0f);
	ImGui::Separator();

	ImGui::Text("Lighting");
	ImGui::SliderFloat("Sun Angle", &sun_angle, 0.0f, 359.99f);
	ImGui::ColorPicker3("Ambient", &ambient.x);
	ImGui::ColorPicker3("Diffuse", &diffuse.x);
	ImGui::ColorPicker3("Specular", &specular.x);
	ImGui::SliderFloat("Shininess", &shininess, 0.0f, 100.0f);
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD, delta_time);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.ProcessKeyboard(BACKWARD, delta_time);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.ProcessKeyboard(LEFT, delta_time);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.ProcessKeyboard(RIGHT, delta_time);
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		camera.ProcessKeyboard(UP, delta_time);
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		camera.ProcessKeyboard(DOWN, delta_time);
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		camera.MovementSpeed = SPEED * 3;
	} else {
		camera.MovementSpeed = SPEED;
	}
}

static unsigned int loadCubemap(std::vector<std::string> faces) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++) {
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

		if (data != NULL) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		} else {
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

static void framebuffer_size_callback(GLFWwindow* _window, int width, int height) {
	glViewport(0, 0, width, height);
}

static void mouse_callback(GLFWwindow* window, double xpos_in, double ypos_in) {
	float xpos = static_cast<float>(xpos_in);
	float ypos = static_cast<float>(ypos_in);

	if (first_mouse) {
		last_x = xpos;
		last_y = ypos;
		first_mouse = false;
	}

	float xoffset = xpos - last_x;
	float yoffset = last_y - ypos; // reversed since y-coordinates go from bottom to top

	last_x = xpos;
	last_y = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	static GLFWcursorposfun previous = NULL;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		first_mouse = true;
		previous = glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		glfwSetCursorPosCallback(window, previous);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	// camera.ProcessMouseScroll(static_cast<float>(yoffset));
}