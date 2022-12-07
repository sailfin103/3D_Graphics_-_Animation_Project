// 3D Graphics and Animation - Main Template
// This uses Visual Studio Code - https://code.visualstudio.com/docs/cpp/introvideos-cpp
// Two versions available -  Win64 and Apple MacOS - please see notes
// Last changed August 2022

//#pragma comment(linker, "/NODEFAULTLIB:MSVCRT")

#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

//#define GL_SILENCE_DEPRECATION
#include <glad/glad.h> // Extend OpenGL Specfication to version 4.5 for WIN64 and versions 4.1 for Apple (note: two different files).
//#include <stb_image.h>

#include <imgui/imgui.h>			  // Load GUI library - Dear ImGui - https://github.com/ocornut/imgui
#include <imgui/imgui_impl_glfw.h>	  // Platform ImGui using GLFW
#include <imgui/imgui_impl_opengl3.h> // Platform new OpenGL - aka better than 3.3 core version.

#include <GLFW/glfw3.h> // Add GLFW - library to launch a window and callback mouse and keyboard - https://www.glfw.org

#define GLM_ENABLE_EXPERIMENTAL	 // Enable all functions include non LTS
#include <glm/glm.hpp>			 // Add helper maths library - GLM 0.9.9.9 - https://github.com/g-truc/glm - for example variables vec3, mat and operators.
#include <glm/gtx/transform.hpp> // Help us with transforms
using namespace glm;
 
//#include <tinygltf/tiny_gltf.h> // Model loading library - tiny gltf - https://github.com/syoyo/tinygltf
//#include "src/stb_image.hpp" // Add library to load images for textures

//#include "src/Mesh.hpp" // Simplest mesh holder and OBJ loader - can update more - from https://github.com/BennyQBD/ModernOpenGLTutorial

#include "src/Pipeline.hpp"		// Setup pipeline and load shaders.
#include "src/Content.hpp"		// Setup content loader and drawing functions - https://github.com/KhronosGroup/glTF - https://github.com/syoyo/tinygltf 
#include "src/Debugger.hpp"		// Setup debugger functions.


// Main fuctions
void startup();
void update();
void render();
void ui();
void endProgram();

// HELPER FUNCTIONS OPENGL
void hintsGLFW();
string readShader(string name);
void checkErrorShader(GLuint shader);
inline void errorCallbackGLFW(int error, const char *description){cout << "Error GLFW: " << description << "\n";};
void debugGL();

void APIENTRY openGLDebugCallback(GLenum source,
								  GLenum type,
								  GLuint id,
								  GLenum severity,
								  GLsizei length,
								  const GLchar *message,
								  const GLvoid *userParam);
GLenum glCheckError_(const char *file, int line);
#define glCheckError() glCheckError_(__FILE__, __LINE__)

// Setup all the message loop callbacks - do this before Dear ImGui
// Callback functions for the window and interface devices
void onResizeCallback(GLFWwindow *window, int w, int h);
void onKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void onMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
void onMouseMoveCallback(GLFWwindow *window, double x, double y);
void onMouseWheelCallback(GLFWwindow *window, double xoffset, double yoffset);

// VARIABLES
GLFWwindow *window; 								// Keep track of the window
auto windowWidth = 800;								// Window width					
auto windowHeight = 800;								// Window height
auto running(true);							  		// Are we still running our main loop
mat4 projMatrix;							 		// Our Projection Matrix
vec3 cameraPosition = vec3(0.0f, 0.0f, 5.0f);		// Where is our camera
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);			// Camera front vector
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);				// Camera up vector
auto aspect = (float)windowWidth / (float)windowHeight;	// Window aspect ration
auto fovy = 45.0f;									// Field of view (y axis)
bool keyStatus[1024];								// Track key strokes
auto currentTime = 0.0f;							// Framerate
auto deltaTime = 0.0f;								// time passed
auto lastTime = 0.0f;								// Used to calculate Frame rate

float yaw = -90.0f;
float pitch = 0.0f;

bool firstMouse = true;
float lastX = 800.0f /2.0;
float lastY = 600.0f /2.0;
bool mouseToggleView = false;


Pipeline pipeline;									// Add one pipeline plus some shaders.
vector <Content> content;
Debugger debugger; // Add one debugger to use for callbacks ( Win64 - openGLDebugCallback() ) or manual calls ( Apple - glCheckError() )

vector <vec3> modelPosition;									// Model position
vector <vec3> modelRotation;									// Model rotation
vector <vec3> modelScale;

vec4 lightPos = vec4(1.0f, 10.0f, 1.0f, 1.0f);
vec4 ia = vec4(1.0f, 1.0f, 1.0f, 1.0f);
vec4 id = vec4(1.0f, 1.0f, 1.0f, 1.0f);
vec4 is = vec4(1.0f, 1.0f, 1.0f, 1.0f);

float ka = 0.8f;
float kd = 0.2f;
float ks = 1.0f;
float shininess = 32.0f;

// string modelsFiles[]{
// 	"assets/Sand.gltf"

// };


int main()
{
	cout << endl << "===" << endl << "3D Graphics and Animation - Running..." << endl;

	if (!glfwInit()) // Check if GLFW is working, if not then stop all
	{
		cout << "Could not initialise GLFW...";
		return -1;
	} 

	glfwSetErrorCallback(errorCallbackGLFW); // Setup a function callback to catch and display all GLFW errors.
	hintsGLFW();							 // Setup glfw with various hints.

	const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor()); // Grab reference to monitor
	// windowWidth = mode->width; windowHeight = mode->height; //fullscreen
	// window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), glfwGetPrimaryMonitor(), NULL); // fullscreen

	// Create our Window
	const auto windowTitle = "My 3D Graphics and Animation OpenGL Application"s;
	window = glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), NULL, NULL);
	if (!window) // Test if Window or OpenGL context creation failed
	{
		cout << "Could not initialise GLFW...";
		glfwTerminate();
		return -1;
	} 

	glfwSetWindowPos(window, 10, 10); // Place it in top corner for easy debugging.
	glfwMakeContextCurrent(window);	  // making the OpenGL context currentw

	// GLAD: Load OpenGL function pointers - aka update specs to newest versions - plus test for errors.
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD...";
		glfwMakeContextCurrent(NULL);
		glfwTerminate();
		return -1;
	}

	glfwSetWindowSizeCallback(window, onResizeCallback);	   // Set callback for resize
	glfwSetKeyCallback(window, onKeyCallback);				   // Set Callback for keys
	glfwSetMouseButtonCallback(window, onMouseButtonCallback); // Set callback for mouse click
	glfwSetCursorPosCallback(window, onMouseMoveCallback);	   // Set callback for mouse move
	glfwSetScrollCallback(window, onMouseWheelCallback);	   // Set callback for mouse wheel.
	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);	// Set mouse cursor Fullscreen
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	// Set mouse cursor FPS fullscreen.

	// Setup Dear ImGui and add context	-	https://blog.conan.io/2019/06/26/An-introduction-to-the-Dear-ImGui-library.html
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO(); //(void)io;
								  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	ImGui::StyleColorsLight(); // ImGui::StyleColorsDark(); 		// Setup Dear ImGui style

	// Setup Platform/Renderer ImGui backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	const auto glsl_version = "#version 410";
	ImGui_ImplOpenGL3_Init(glsl_version);

	#if defined(__WIN64__)
		debugGL(); // Setup callback to catch openGL errors.	V4.2 or newer
	#elif(__APPLE__)
		glCheckError(); // Old way of checking for errors. Newest not implemented by Apple. Manually call function anywhere in code to check for errors.
	#endif

	glfwSwapInterval(1);			 // Ony render when synced (V SYNC) - https://www.tomsguide.com/features/what-is-vsync-and-should-you-turn-it-on-or-off
	glfwWindowHint(GLFW_SAMPLES, 2); // Multisampling - covered in lectures - https://www.khronos.org/opengl/wiki/Multisampling

	startup(); // Setup all necessary information for startup (aka. load texture, shaders, models, etc).

	cout << endl << "Starting main loop and rendering..." << endl;	

	do{											 // run until the window is closed
		auto currentTime = (float)glfwGetTime(); // retrieve timelapse
		deltaTime = currentTime - lastTime;		 // Calculate delta time
		lastTime = currentTime;					 // Save for next frame calculations.

		glfwPollEvents(); 						// poll callbacks

		update(); 								// update (physics, animation, structures, etc)
		render(); 								// call render function.
		ui();									// call function to render ui.

		#if defined(__APPLE__)
			glCheckError();				// Manually checking for errors for MacOS, Windows has a callback.
		#endif

		glfwSwapBuffers(window); 		// swap buffers (avoid flickering and tearing)

		running &= (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE); // exit if escape key pressed
		running &= (glfwWindowShouldClose(window) != GL_TRUE);
	} while (running);

	endProgram(); // Close and clean everything up...

	// cout << "\nPress any key to continue...\n";
	// cin.ignore(); cin.get(); // delay closing console to read debugging errors.

	return 0;
}


void hintsGLFW(){
	
	auto majorVersion = 3; auto minorVersion = 3; // define OpenGL version - at least 3.3 for bare basic NEW OpenGL

	#if defined(__WIN64__)	
		majorVersion = 4; minorVersion = 5;					// Recommended for Windows 4.5, but latest is 4.6 (not all drivers support 4.6 version).
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // Create context in debug mode - for debug message callback
	#elif(__APPLE__)
		majorVersion = 4; minorVersion = 1; 				// Max support for OpenGL in MacOS
	#endif

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 	// https://www.glfw.org/docs/3.3/window_guide.html
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, majorVersion); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minorVersion);
}

void endProgram()
{
	// Clean ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwMakeContextCurrent(NULL); 	// destroys window handler
	glfwTerminate();				// destroys all windows and releases resources.
}

void startup()
{
	// Output some debugging information
	cout << "VENDOR: " << (char *)glGetString(GL_VENDOR) << endl;
	cout << "VERSION: " << (char *)glGetString(GL_VERSION) << endl;
	cout << "RENDERER: " << (char *)glGetString(GL_RENDERER) << endl;	

	cout << endl << "Loading content..." << endl;	

	Content c1;

	Content c2;
	Content c3;
	Content c4;
	Content c5;
	Content c6;

	Content c7;
	Content c8;
	Content c9;

	Content c10;
	Content c11;
	Content c12;

	Content c13;

	Content c14;

	c1.LoadGLTF("assets/Sand1.gltf");

	c2.LoadGLTF("assets/ElectricalPost.gltf");
	c3.LoadGLTF("assets/ElectricalPost.gltf");
	c4.LoadGLTF("assets/ElectricalPost.gltf");
	c5.LoadGLTF("assets/ElectricalPost.gltf");
	c6.LoadGLTF("assets/ElectricalPost.gltf");

	c7.LoadGLTF("assets/LampPost.gltf");
	c8.LoadGLTF("assets/LampPost.gltf");
	c9.LoadGLTF("assets/LampPost.gltf");

	c10.LoadGLTF("assets/Building.gltf");
	c11.LoadGLTF("assets/Building.gltf");
	c12.LoadGLTF("assets/Building.gltf");

	c13.LoadGLTF("assets/Ship.gltf");

	c14.LoadGLTF("assets/dog.gltf");

	//Sand
	content.push_back(c1);

	//Electrical posts
	content.push_back(c2);
	content.push_back(c3);
	content.push_back(c4);
	content.push_back(c5);
	content.push_back(c6);

	content.push_back(c7);
	content.push_back(c8);
	content.push_back(c9);

	content.push_back(c10);
	content.push_back(c11);
	content.push_back(c12);

	content.push_back(c13);

	content.push_back(c14);

	pipeline.CreatePipeline();
	pipeline.LoadShaders("shaders/vs_model.glsl", "shaders/fs_model.glsl");

	// Start from the centre

	//Sand Environment--
	modelPosition.push_back(vec3(0.0f, 0.0f, 0.0f));
	modelRotation.push_back(vec3(0.0f, 0.0f, 0.0f));
	modelScale.push_back(vec3(10.0f, 0.125f, 10.0f));
	//--

	//Electrical Posts --
	modelPosition.push_back(vec3(-2.0f, 0.2f, -2.7f));
	modelRotation.push_back(vec3(1.0f, 0.0f, 1.8f));
	modelScale.push_back(vec3(0.3f, 0.3f, 0.3f));

	modelPosition.push_back(vec3(3.5f, 0.0f, -4.0f));
	modelRotation.push_back(vec3(-1.0f, 0.0f, 2.0f));
	modelScale.push_back(vec3(0.3f, 0.3f, 0.3f));

	modelPosition.push_back(vec3(0.9f, 0.2f, 1.0f));
	modelRotation.push_back(vec3(0.8f, 0.0f, 0.2f));
	modelScale.push_back(vec3(0.3f, 0.3f, 0.3f));

	modelPosition.push_back(vec3(5.5f, 0.0f, 4.0f));
	modelRotation.push_back(vec3(0.0f, 1.0f, 0.0f));
	modelScale.push_back(vec3(0.3f, 0.3f, 0.3f));

	modelPosition.push_back(vec3(-7.5f, 0.2f, -4.0f));
	modelRotation.push_back(vec3(0.0f, 0.0f, 0.7f));
	modelScale.push_back(vec3(0.3f, 0.3f, 0.3f));
	//--

	//LampPosts --
	modelPosition.push_back(vec3(4.0f, 0.0f, 3.0f));
	modelRotation.push_back(vec3(0.0f, 3.2f, 2.0f));
	modelScale.push_back(vec3(0.1f, 0.1f, 0.1f));

	modelPosition.push_back(vec3(-5.0f, -1.0f, -2.0f));
	modelRotation.push_back(vec3(0.0f, 0.0f, 0.0f));
	modelScale.push_back(vec3(0.1f, 0.1f, 0.1f));

	modelPosition.push_back(vec3(0.0f, 0.7f, 0.0f));
	modelRotation.push_back(vec3(0.5f, 0.0f, 0.0f));
	modelScale.push_back(vec3(0.1f, 0.1f, 0.1f));
	//--

	//--
	modelPosition.push_back(vec3(2.3f, 0.1f, 7.5f));
	modelRotation.push_back(vec3(20.0f, 0.0f, 20.0f));
	modelScale.push_back(vec3(1.0f, 1.0f, 1.0f));

	modelPosition.push_back(vec3(0.0f, -0.7f, 0.0f));
	modelRotation.push_back(vec3(0.0f, 0.0f, 6.0f));
	modelScale.push_back(vec3(1.0f, 1.0f, 1.0f));

	modelPosition.push_back(vec3(-8.0f, 0.0f, -4.0f));
	modelRotation.push_back(vec3(19.0f, 0.0f, 15.0f));
	modelScale.push_back(vec3(1.0f, 1.0f, 1.0f));
	//--

	//--
	modelPosition.push_back(vec3(0.0f, 2.8f, 0.0f));
	modelRotation.push_back(vec3(0.0f, 0.0f, 0.0f));
	modelScale.push_back(vec3(0.1f, 0.1f, 0.1f));
	//--

	//--
	modelPosition.push_back(vec3(0.0f, 1.0f, 0.0f));
	modelRotation.push_back(vec3(0.0f, 0.0f, 0.0f));
	modelScale.push_back(vec3(0.1f, 0.1f, 0.1f));

	// A few optimizations.
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glActiveTexture(GL_TEXTURE0);
	
	// Get the correct size in pixels - E.g. if retina display or monitor scaling
	glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

	// Calculate proj_matrix for the first time.
	aspect = (float)windowWidth / (float)windowHeight;
	projMatrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 1000.0f);
}

void update()
{
	//Camera Movement
	float cameraSpeed = 2.5 * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPosition += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPosition -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		cameraPosition += cameraSpeed * cameraUp;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		cameraPosition -= cameraSpeed * cameraUp;

	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS){
		mouseToggleView = true;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
		
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS){
		mouseToggleView = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
		

	if (keyStatus[GLFW_KEY_R]) pipeline.ReloadShaders();

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void render()
{
	glViewport(0, 0, windowWidth, windowHeight);

	// Clear colour buffer
	glm::vec4 inchyraBlue = glm::vec4(0.345f, 0.404f, 0.408f, 1.0f);
	glm::vec4 backgroundColor = inchyraBlue;
	glClearBufferfv(GL_COLOR, 0, &backgroundColor[0]);

	// Clear deep buffer
	static const GLfloat one = 1.0f;
	glClearBufferfv(GL_DEPTH, 0, &one);

	// Enable blend
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Use our shader programs
	glUseProgram(pipeline.pipe.program);

	// Setup camera
	glm::mat4 viewMatrix = glm::lookAt(cameraPosition,				 // eye
									   cameraPosition + cameraFront, // centre
									   cameraUp);					 // up


	vector <mat4> modelMatrix; 
	for(int i = 0; i<modelPosition.size(); i++){


		// Do some translations, rotations and scaling
		modelMatrix.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(modelPosition[i].x, modelPosition[i].y, modelPosition[i].z)));
		//glm::mat4 modelMatrix[i] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		modelMatrix[i] = glm::rotate(modelMatrix[i], modelRotation[i].x, glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix[i] = glm::rotate(modelMatrix[i], modelRotation[i].y, glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix[i] = glm::scale(modelMatrix[i], modelScale[i]);

		glm::mat4 mv_matrix = viewMatrix * modelMatrix[i];

		glUniformMatrix4fv(glGetUniformLocation(pipeline.pipe.program, "model_matrix"), 1, GL_FALSE, &modelMatrix[i][0][0]);
		glUniformMatrix4fv(glGetUniformLocation(pipeline.pipe.program, "view_matrix"), 1, GL_FALSE, &viewMatrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(pipeline.pipe.program, "proj_matrix"), 1, GL_FALSE, &projMatrix[0][0]);

		glUniform4f(glGetUniformLocation(pipeline.pipe.program, "viewPosition"), cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0);
		glUniform4f(glGetUniformLocation(pipeline.pipe.program, "lightPosition"), lightPos.x, lightPos.y, lightPos.z, 1.0);
		glUniform4f(glGetUniformLocation(pipeline.pipe.program, "ia"), ia.r, ia.g, ia.b, 1.0);
		glUniform1f(glGetUniformLocation(pipeline.pipe.program, "ka"), ka);
		glUniform4f(glGetUniformLocation(pipeline.pipe.program, "id"), id.r, id.g, id.b, 1.0);
		glUniform1f(glGetUniformLocation(pipeline.pipe.program, "kd"), 1.0f);
		glUniform4f(glGetUniformLocation(pipeline.pipe.program, "is"), is.r, is.g, is.b, 1.0);
		glUniform1f(glGetUniformLocation(pipeline.pipe.program, "ks"), 1.0f);
		glUniform1f(glGetUniformLocation(pipeline.pipe.program, "shininess"), 32.0f);

		GLuint tex;

		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		// set the texture wrapping/filtering options (on the currently bound texture object)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		GLenum type = GL_UNSIGNED_BYTE;
		GLenum format = GL_RGBA;

		auto s = content[i].model.textures[0].source;
		tinygltf::Image &image = content[i].model.images[s];
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, format, type, &image.image.at(0));

		content[i].DrawModel(content[i].vaoAndEbos, content[i].model); 

		glDeleteTextures(1, &tex);
		
	}

#if defined(__APPLE__)
	glCheckError();
	#endif
}

void ui()
{
	ImGuiIO &io = ImGui::GetIO();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration; 
	window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
	window_flags |= ImGuiWindowFlags_NoSavedSettings; 
	window_flags |= ImGuiWindowFlags_NoFocusOnAppearing; 
	window_flags |= ImGuiWindowFlags_NoNav;

	const auto PAD = 10.0f;
	const ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
	ImVec2 work_size = viewport->WorkSize;
	ImVec2 window_pos, window_pos_pivot;
	window_pos.x = work_pos.x + work_size.x - PAD;
	window_pos.y = work_pos.y + work_size.y - PAD;
	window_pos_pivot.x = 1.0f;
	window_pos_pivot.y = 1.0f;

	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	window_flags |= ImGuiWindowFlags_NoMove;

	ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
	bool *p_open = NULL;
	if (ImGui::Begin("Info", nullptr, window_flags)) {
		ImGui::Text("About: 3D Graphics and Animation 2022"); // ImGui::Separator();
		ImGui::Text("Performance: %.3fms/Frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Pipeline: %s", pipeline.pipe.error?"ERROR":"OK");

		float *lightPosition = &lightPos[0];
		float *lightAmbience= &ia[0];
		float *lightDiffuse= &id[0];
		float *lightSpecular = &is[0];
		ImGui::DragFloat4("Light Position", lightPosition, 0.1f, -20.0f, 20.0f, "%.1f", 1.0f);
		ImGui::DragFloat4("Light Ambience", lightAmbience, 0.01f, 0.0f, 1.0f, "%.2f", 1.0f);
		ImGui::DragFloat4("Light Diffusion", lightDiffuse, 0.01f, 0.0f, 1.0f, "%.2f", 1.0f);
		ImGui::DragFloat4("Specular Light", lightSpecular, 0.01f, 0.0f, 1.0f, "%.2f", 1.0f);
	}


	ImGui::End();

	// Rendering imgui
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void onResizeCallback(GLFWwindow *window, int w, int h)
{
	windowWidth = w;
	windowHeight = h;

	// Get the correct size in pixels
	glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

	if (windowWidth > 0 && windowHeight > 0)
	{ // Avoid issues when minimising window - it gives size of 0 which fails division.
		aspect = (float)w / (float)h;
		projMatrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 1000.0f);
	}
}

void onKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
		keyStatus[key] = true;
	else if (action == GLFW_RELEASE)
		keyStatus[key] = false;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void onMouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
}

void onMouseMoveCallback(GLFWwindow *window, double x, double y)
{

	if(mouseToggleView){
		return;
	}

	if (firstMouse)
	{
		lastX = x;
		lastY = y;
		firstMouse = false;
	}

	float xOffset = x - lastX;
	float yOffset = lastY - y;

	lastX = x;
	lastY = y;

	float sens = 0.1f;
	xOffset *= sens;
	yOffset *= sens;

	yaw += xOffset;
	pitch += yOffset;

	if(pitch > 89.0f)
		pitch = 89.0f;
	if(pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;

	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);


}

void onMouseWheelCallback(GLFWwindow *window, double xoffset, double yoffset)
{
	if (fovy >= 1.0f && fovy <= 45.0f)
		fovy -= yoffset;
	if (fovy <= 1.0f)
		fovy =1.0f;
	if (fovy >= 45.0f)
		fovy = 45.0f;
}

void APIENTRY openGLDebugCallback(GLenum source,
								  GLenum type,
								  GLuint id,
								  GLenum severity,
								  GLsizei length,
								  const GLchar *message,
								  const GLvoid *userParam)  // Debugger callback for Win64 - OpenGL versions 4.2 or better.
{
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
		return;
	debugger.OpenGLDebugCallback(source, type, id, severity, length, message, userParam);
}

void debugGL() // Debugger function for Win64 - OpenGL versions 4.2 or better.
{
	// Enable Opengl Debug
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback((GLDEBUGPROC)openGLDebugCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
}

GLenum glCheckError_(const char *file, int line) // Debugger manual function for Apple - OpenGL versions 3.3 to 4.1.
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) debugger.GlGetError(errorCode, file, line);
	return errorCode;
}
