/* CS 330: Computer Graphics & Viz - Final Project
 * @author Janai Cano
 * SOME CODE SNIPPETS TAKEN FROM CS 330 TUTORIAL GITHUB REPO FOUND HERE: https://github.com/SNHU-CS/CS-330/blob/master/module05/mod5_tutorials.md
 * MESHES.H & MESHES.CPP CREATED BY BRIAN BATTERSBY FOR SNHU
 * @date 10/12/2023
*/


#include<iostream>
#include <cstdlib>        
#include <GL/glew.h>        
#include<GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "camera.h"           // camera functions
#include "meshes.h"          // object creation data
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"      // Image loading Utility functions

using namespace std;

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// global variables
namespace
{
	const char* const WINDOW_TITLE = "Final Project Cano"; 

	// Variables for window width and height
	const int WIDTH = 800;
	const int HEIGHT = 800;

	// mesh structure data
	struct GLMesh
	{
		GLuint vao;         
		GLuint vbos[2];     
		GLuint nIndices;   
	};

	// Main GLFW window
	GLFWwindow* gWindow = nullptr;
	// Shader program
	GLuint gProgramId;
	GLuint gLightId;
	// Camera location
	Camera gCamera(glm::vec3(0.0f, 2.0f, 5.0f));
	float gLastX = WIDTH / 2.0f;
	float gLastY = HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// Window frame timing
	float gDeltaTime = 0.0f; 
	float gLastFrame = 0.0f;

	//Shape Meshes from Professor Brian Battersby
	Meshes meshes;

	//Textures
	GLuint gTextureCounter;
	GLuint gTextureCuttingBoard;
	GLuint gTextureLemon;
	GLuint gTextureLime;
	GLuint gTextureCup;
	GLuint gTextureHandle;
	GLuint gTextureChopstick;
	GLuint gTexturePlate;
	GLuint gTextureMP;
	glm::vec2 gUVScale(1.0f, 1.0f);
	GLint gTexWrapMode = GL_REPEAT;
}

// FUNCTIONS WRITTEN BY CS 330 GITHUB TUTORIALS
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
bool isPerspective = true;       // @Janai added to change perspective
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
	layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
	layout(location = 1) in vec3 normal;  // Normal data from Vertex Attrib Pointer 1
	layout(location = 2) in vec2 textureCoordinate; // Texture data from Vertex Attrib Pointer 2

	out vec3 fragmentNormal; // for outgoing normals to fragment shader
	out vec3 fragmentPos; // for outgoing pixels to fragment shader
	out vec2 vertexTextureCoord; // variable to transfer texture data to the fragment shader

	//Uniform variables for the  transform matrices
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

	void main()
	{
		gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates

		fragmentPos = vec3(model * vec4(position, 1.0f)); // gets fragment / pixel position in world space only (exclude view and projection)

		fragmentNormal = mat3(transpose(inverse(model))) * normal; // gets normal vectors in world space only and exclude normal translation properties
		vertexTextureCoord = textureCoordinate; // references incoming texture data
	}
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
	in vec2 vertexTextureCoord; // Variable to hold incoming texture data from vertex shader
	in vec3 fragmentNormal;     // for incoming normals
	in vec3 fragmentPos;        // for incoming fragment position

	out vec4 fragmentColor;

	//Uniform variables for object color, light color, light position, and camera/view position
	uniform vec4 objectColor;
	uniform sampler2D uTexture;
	uniform bool multipleTextures;
	uniform vec2 uvScale;
	uniform vec3 ambientColor;
	uniform vec3 light1Color;
	uniform vec3 light1Position;
	uniform vec3 light2Color;
	uniform vec3 light2Position;
	uniform vec3 viewPosition;
	uniform bool ubHasTexture;
	uniform float ambientStrength = 0.1f; // Set ambient or global lighting strength
	uniform float specularIntensity1 = 0.8f;
	uniform float highlightSize1 = 16.0f;
	uniform float specularIntensity2 = 0.8f;
	uniform float highlightSize2 = 16.0f;

	void main()
	{
		
		/*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

		//Calculate Ambient lighting
		vec3 ambient = ambientStrength * ambientColor; // Generate ambient light color

		//**Calculate Diffuse lighting**
		vec3 norm = normalize(fragmentNormal); // Normalize vectors to 1 unit
		vec3 light1Direction = normalize(light1Position - fragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
		float impact1 = max(dot(norm, light1Direction), 0.0);// Calculate diffuse impact by generating dot product of normal and light
		vec3 diffuse1 = impact1 * light1Color; // Generate diffuse light color
		vec3 light2Direction = normalize(light2Position - fragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
		float impact2 = max(dot(norm, light2Direction), 0.0);// Calculate diffuse impact by generating dot product of normal and light
		vec3 diffuse2 = impact2 * light2Color; // Generate diffuse light color

		//**Calculate Specular lighting**
		vec3 viewDir = normalize(viewPosition - fragmentPos); // Calculate view direction
		vec3 reflectDir1 = reflect(-light1Direction, norm);// Calculate reflection vector
		//Calculate specular component
		float specularComponent1 = pow(max(dot(viewDir, reflectDir1), 0.0), highlightSize1);
		vec3 specular1 = specularIntensity1 * specularComponent1 * light1Color;
		vec3 reflectDir2 = reflect(-light2Direction, norm);// Calculate reflection vector
		//Calculate specular component
		float specularComponent2 = pow(max(dot(viewDir, reflectDir2), 0.0), highlightSize2);
		vec3 specular2 = specularIntensity2 * specularComponent2 * light2Color;

		//**Calculate phong result**
		//Texture holds the color to be used for all three components
		vec4 textureColor = texture(uTexture, vertexTextureCoord * uvScale);
		vec3 phong1;
		vec3 phong2;

		if (ubHasTexture == true)
		{
			phong1 = (ambient + diffuse1 + specular1) * textureColor.xyz;
			phong2 = (ambient + diffuse2 + specular2) * textureColor.xyz;
		}
		else
		{
			phong1 = (ambient + diffuse1 + specular1) * objectColor.xyz;
			phong2 = (ambient + diffuse2 + specular2) * objectColor.xyz;
		}

		fragmentColor = vec4(phong1 + phong2, 1.0); // Send lighting results to GPU
	}
);

/* Light Object Shader Source Code written by @Battersby*/
const GLchar* lightVertexShaderSource = GLSL(330,
	layout(location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}
);
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Light Object Shader Source Code written by @Battersby*/
const GLchar* lightFragmentShaderSource = GLSL(330,
	out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0); // set all 4 vector values to 1.0
}
);

/*
* WRITTEN BY @SNHU TUTORIALS
*/
// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
	for (int j = 0; j < height / 2; ++j)
	{
		int index1 = j * width * channels;
		int index2 = (height - 1 - j) * width * channels;

		for (int i = width * channels; i > 0; --i)
		{
			unsigned char tmp = image[index1];
			image[index1] = image[index2];
			image[index2] = tmp;
			++index1;
			++index2;
		}
	}
}

/* MAIN FUNCTION */
int main(int argc, char* argv[])
{
	if (!UInitialize(argc, argv, &gWindow))
		return EXIT_FAILURE;

	// Create the mesh from Battersby's mesh file
	meshes.CreateMeshes();

	// Create the objects shader program
	if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
		return EXIT_FAILURE;
	// Create the lights shader program
	if (!UCreateShaderProgram(lightVertexShaderSource, lightFragmentShaderSource, gLightId))
		return EXIT_FAILURE;

	// Loading textures
	// First Texture - Counter Top
	const char* texFilename = "countertop.jpg";
	if (!UCreateTexture(texFilename, gTextureCounter))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	//Second texture - Cutting  Board
	texFilename = "cutting_board.jpg";
	if (!UCreateTexture(texFilename, gTextureCuttingBoard))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	// Third texture - Measuring Cup
	texFilename = "cup.jpg";
	if (!UCreateTexture(texFilename, gTextureCup))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	} 

	// Fourth texture - Measuring cup handle
	texFilename = "cupHandle.jpg";
	if (!UCreateTexture(texFilename, gTextureHandle))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	// Lemon texture
	texFilename = "lemon.jpg";
	if (!UCreateTexture(texFilename, gTextureLemon))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	// Lime texture
	texFilename = "lime.jpg";
	if (!UCreateTexture(texFilename, gTextureLime))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	// Chopstick texture
	texFilename = "chopstick.jpg";
	if (!UCreateTexture(texFilename, gTextureChopstick))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	// Plate texture
	texFilename = "plate.png";
	if (!UCreateTexture(texFilename, gTexturePlate))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	//Mortar and Pestle texture
	texFilename = "marble.png";
	if (!UCreateTexture(texFilename, gTextureMP))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	glUseProgram(gProgramId);
	// We set the texture as texture unit 0
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);
	glUniform2f(glGetUniformLocation(gProgramId, "uvScale"), gUVScale.x, gUVScale.y);

	// Sets the background color of the window to black (it will be implicitely used by glClear)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(gWindow))
	{
		// per-frame timing
		// --------------------
		float currentFrame = glfwGetTime();
		gDeltaTime = currentFrame - gLastFrame;
		gLastFrame = currentFrame;

		// input
		// -----
		UProcessInput(gWindow);

		// Render this frame
		URender();

		glfwPollEvents();
	}

	// Release mesh data
	meshes.DestroyMeshes();

	// Release textures
	UDestroyTexture(gTextureCounter);
	UDestroyTexture(gTextureCuttingBoard);
	UDestroyTexture(gTextureCup);
	UDestroyTexture(gTextureHandle);
	UDestroyTexture(gTextureLemon);
	UDestroyTexture(gTextureLime);
	UDestroyTexture(gTextureChopstick);
	UDestroyTexture(gTexturePlate);

	// Release shaders
	UDestroyShaderProgram(gProgramId);
	UDestroyShaderProgram(gLightId);

	// Terminates the program
	exit(EXIT_SUCCESS); 
}

// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
	// GLFW: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// GLFW: window creation
	// ---------------------
	* window = glfwCreateWindow(WIDTH, HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (*window == NULL)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(*window);
	glfwSetFramebufferSizeCallback(*window, UResizeWindow);
	glfwSetCursorPosCallback(*window, UMousePositionCallback);
	glfwSetScrollCallback(*window, UMouseScrollCallback);
	glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// GLEW: initialize
	// Note: if using GLEW version 1.13 or earlier
	glewExperimental = GL_TRUE;
	GLenum GlewInitResult = glewInit();

	if (GLEW_OK != GlewInitResult)
	{
		cerr << glewGetErrorString(GlewInitResult) << endl;
		return false;
	}

	// Displays GPU OpenGL version
	cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

	return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		gCamera.ProcessKeyboard(LEFT, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		gCamera.ProcessKeyboard(UP, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		gCamera.ProcessKeyboard(DOWN, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)      // Changes the projection view 
		isPerspective = !isPerspective;
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (gFirstMouse)
	{
		gLastX = xpos;
		gLastY = ypos;
		gFirstMouse = false;
	}

	float xoffset = xpos - gLastX;
	float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

	gLastX = xpos;
	gLastY = ypos;

	gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	switch (button)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
	{
		if (action == GLFW_PRESS)
			cout << "Left mouse button pressed" << endl;
		else
			cout << "Left mouse button released" << endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_MIDDLE:
	{
		if (action == GLFW_PRESS)
			cout << "Middle mouse button pressed" << endl;
		else
			cout << "Middle mouse button released" << endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_RIGHT:
	{
		if (action == GLFW_PRESS)
			cout << "Right mouse button pressed" << endl;
		else
			cout << "Right mouse button released" << endl;
	}
	break;

	default:
		cout << "Unhandled mouse button event" << endl;
		break;
	}
}

// Functioned called to render a frame
void URender()
{
	const int nrows = 10;
	const int ncols = 10;
	const int nlevels = 10;

	const float xsize = 10.0f;
	const float ysize = 10.0f;
	const float zsize = 10.0f;

	GLint modelLoc;
	GLint viewLoc;
	GLint projLoc;
	GLint viewPosLoc;
	GLint ambStrLoc;
	GLint ambColLoc;
	GLint light1ColLoc;
	GLint light1PosLoc;
	GLint light2ColLoc;
	GLint light2PosLoc;
	GLint objColLoc;
	GLint specInt1Loc;
	GLint highlghtSz1Loc;
	GLint specInt2Loc;
	GLint highlghtSz2Loc;
	GLint uHasTextureLoc;
	bool ubHasTextureVal;

	glm::mat4 scale;
	glm::mat4 rotation;
	glm::mat4 translation;
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 flatten;      //@Janai added to flatten objects
	glm::mat4 projection;
	
	// Enable z-depth
	glEnable(GL_DEPTH_TEST);

	// Clear the frame and z buffers
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Transforms the camera
	view = gCamera.GetViewMatrix();

	//@Janai added ability to change from perspective to ortho projection by pushing 'P'
	if (isPerspective) {
		//Creates a perspective projection
		projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
	}
	else {
		// Creates a orthographic projection
		projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
	}

	// Set the shader to be used
	glUseProgram(gProgramId);

	// Retrieves and passes transform matrices to the Shader program
	modelLoc = glGetUniformLocation(gProgramId, "model");
	viewLoc = glGetUniformLocation(gProgramId, "view");
	projLoc = glGetUniformLocation(gProgramId, "projection");
	viewPosLoc = glGetUniformLocation(gProgramId, "viewPosition");
	ambStrLoc = glGetUniformLocation(gProgramId, "ambientStrength");
	ambColLoc = glGetUniformLocation(gProgramId, "ambientColor");
	light1ColLoc = glGetUniformLocation(gProgramId, "light1Color");
	light1PosLoc = glGetUniformLocation(gProgramId, "light1Position");
	light2ColLoc = glGetUniformLocation(gProgramId, "light2Color");
	light2PosLoc = glGetUniformLocation(gProgramId, "light2Position");
	objColLoc = glGetUniformLocation(gProgramId, "objectColor");
	specInt1Loc = glGetUniformLocation(gProgramId, "specularIntensity1");
	highlghtSz1Loc = glGetUniformLocation(gProgramId, "highlightSize1");
	specInt2Loc = glGetUniformLocation(gProgramId, "specularIntensity2");
	highlghtSz2Loc = glGetUniformLocation(gProgramId, "highlightSize2");
	uHasTextureLoc = glGetUniformLocation(gProgramId, "ubHasTexture");

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the camera view location
	glUniform3f(viewPosLoc, gCamera.Position.x, gCamera.Position.y, gCamera.Position.z);
	//set ambient lighting strength
	glUniform1f(ambStrLoc, 0.1f);
	//set ambient color
	glUniform3f(ambColLoc, 0.1f, 0.1f, 0.1f);
	glUniform3f(light1ColLoc, 1.000f, 1.000f, 0.878f);
	glUniform3f(light1PosLoc, 4.0f, 3.0f, 4.0f);
	glUniform3f(light2ColLoc, 1.000f, 0.894f, 0.882f);
	glUniform3f(light2PosLoc, -4.0f, 3.0f, -4.0f);
	//set specular intensity
	glUniform1f(specInt1Loc, .2f);
	glUniform1f(specInt2Loc, .1f);
	//set specular highlight size
	glUniform1f(highlghtSz1Loc, 1.0f);
	glUniform1f(highlghtSz2Loc, 1.0f);

	ubHasTextureVal = true;
	glUniform1i(uHasTextureLoc, ubHasTextureVal);


	// @Janai added a parent model to change complex objects
	// 1. Scales the object
	glm::mat4 parentScale = glm::scale(glm::vec3(1.0f));
	// 2. Rotate the object
	glm::mat4 parentRotation = glm::rotate(-0.785398f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	glm::mat4 parentTranslation = glm::translate(glm::vec3(2.89f, 0.1f, 1.0f));

	// Model matrix: transformations are applied right-to-left order
	glm::mat4 parentModel = parentTranslation * parentRotation * parentScale;


	// --------------------------------------------------
	// COUNTER TOP - PLANE OBJECT
	// Activate the VBOs contained within the mesh's VAO 
	glBindVertexArray(meshes.gPlaneMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(6.0f, 1.0f, 6.0f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 0.663f, 0.663f, 0.663f, 1.0f);
	
	//binding the texture 
	GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");  // only have to bind UVScaleLoc to uvScale once, unless scale changes
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureCounter);
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0); // Use texture unit 0

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gPlaneMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	//----------------------------------------------------
	// CUTTING BOARD - CUBE OBJECT
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(6.0f, 6.0f, 6.0f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(0.0f, 0.15f, 1.6f));
	//4. Flatten the cube
	flatten = glm::mat4(
		1.0, 0.0, 0.0, 0.0,
		0.0, 0.05, 0.0, 0.0, //flattens cube by factor of 0.05 along y-axis
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0);
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale * flatten;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 0.871f, 0.722f, 0.529f, 1.0f);

	//binding the texture 
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureCuttingBoard);
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0); // Use texture unit 0

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	//-----------------------------------------------------------------
	// MEASURING CUP OBJECT
	
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.0f, 0.5f, 0.5f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-1.0f, 0.5f, -0.5f));
	//4. Flatten the cube
	flatten = glm::mat4(
		1.0, 0.0, 0.0, 0.0,
		0.0, 0.05, 0.0, 0.0, //flattens cube by factor of 0.05 along y-axis
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0);
	// Model matrix: transformations are applied right-to-left order
	model = parentModel * translation * rotation * scale * flatten;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 0.753f, 0.753f, 0.753f, 1.0f);

	//binding the texture 
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureHandle);
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0); // Use texture unit 0

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.1f, 0.5f, 0.5f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-1.5f, 0.5f, -0.5f));
	//4. Flatten the cube
	flatten = glm::mat4(
		1.0, 0.0, 0.0, 0.0,
		0.0, 0.05, 0.0, 0.0, //flattens cube by factor of 0.05 along y-axis
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0);
	// Model matrix: transformations are applied right-to-left order
	model = parentModel * translation * rotation * scale * flatten;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 0.0f, 0.0f, 1.0f);
	
	//binding the texture 
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureCup);
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0); // Use texture unit 0

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-2.0f, 0.16f, -0.5f));

	// Model matrix: transformations are applied right-to-left order
	model = parentModel * translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 0.0f, 0.0f, 1.0f);
	
	//binding the texture 
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureCup);
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0); // Use texture unit 0

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	//glDrawArrays(GL_TRIANGLE_FAN, 36, 36);    //top, commented out since top of cylinder is open here
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	//-----------------------------------------------------------------------------------------------------------
	// LEMON OBJECT
	glBindVertexArray(meshes.gSphereMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.4f, 0.35f, 0.35f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-1.85f, 0.65f, 0.7f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 1.0f, 0.0f, 1.0f);

	//binding the texture 
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureLemon);
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0); // Use texture unit 0

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gSphereMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	//--------------------------------------------------------------------------------------
	// LIME OBJECT
	glBindVertexArray(meshes.gSphereMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.3f, 0.25f, 0.25f));
	// 2. Rotate the object
	rotation = glm::rotate(3.14f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-1.2f, 0.58f, -0.3f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 0.196f, 0.804f, 0.196f, 1.0f);

	//binding the texture 
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureLime);
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0); // Use texture unit 0

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gSphereMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	//-------------------------------------------------------------------------------------
	// FIRST CHOPSTICK
	glBindVertexArray(meshes.gCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.0f, 0.05f, 0.05f));
	// 2. Rotate the object
	rotation = glm::rotate(0.8f, glm::vec3(0.0, 1.0f, 0.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(0.2f, 0.31f, -0.2f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 0.502f, 0.000f, 0.502f, 1.0f);

	//binding the texture 
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureChopstick);
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0); // Use texture unit 0

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	//-----------------------------------------------------------------------------------------
	//SECOND CHOPSTICK
	glBindVertexArray(meshes.gCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.0f, 0.05f, 0.05f));
	// 2. Rotate the object
	rotation = glm::rotate(0.2f, glm::vec3(0.0, 1.0f, 0.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(0.5f, 0.31f, 0.25f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 0.502f, 0.000f, 0.502f, 1.0f);

	//binding the texture 
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureChopstick);
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0); // Use texture unit 0

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	//---------------------------------------------------------------------------------------------
	// PLATE OBJECT
	glBindVertexArray(meshes.gBoxMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(3.0f, 1.0f, 3.0f));
	// 2. Rotate the object
	rotation = glm::rotate(0.4f, glm::vec3(0.0, 1.0f, 0.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(1.5f, 0.31f, 2.6f));
	//4. Flatten the cube
	flatten = glm::mat4(
		1.0, 0.0, 0.0, 0.0,
		0.0, 0.05, 0.0, 0.0, //flattens cube by factor of 0.05 along y-axis
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0);
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale * flatten;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 0.871f, 0.722f, 0.529f, 1.0f);

	//binding the texture 
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTexturePlate);
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0); // Use texture unit 0

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	//---------------------------------------------------------------------------------------------
	// MORTAR (BOWL) OBJECT
	glBindVertexArray(meshes.gTaperedCylinderMesh.vao);

	//1. Scale the object
	scale = glm::scale(glm::vec3(0.6f, 0.6f, 0.5f));
	//2. Rotate the object
	rotation = glm::rotate(3.14f, glm::vec3(1.0f, 0.0f, 1.0f));
	//3. Position the object
	translation = glm::translate(glm::vec3(-1.85f, 0.93f, 3.0f));
	// Model matrix
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//bind the texture
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureMP);
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0); // use texture unit 0

	//draws the object
	//glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom, commented out since it is a bowl
	glDrawArrays(GL_TRIANGLE_FAN, 36, 72);		//top, 
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	//deactive the vao
	glBindVertexArray(0);

	//---------------------------------------------------------------------------------------------
	// PESTLE OBJECT
	glBindVertexArray(meshes.gCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.2f, 0.85f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(43.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-1.85f, 0.7f, 3.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//binding the texture 
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureMP);
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0); // Use texture unit 0

	//draws the object
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	//---------------------------------------------------------------------------------------------
	// LIGHTS
	// Set the shader to be used
	glUseProgram(gLightId);

	// Retrieves and passes transform matrices to the Shader program
	modelLoc = glGetUniformLocation(gLightId, "model");
	viewLoc = glGetUniformLocation(gLightId, "view");
	projLoc = glGetUniformLocation(gLightId, "projection");

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// drawing the light objects
	glBindVertexArray(meshes.gTorusMesh.vao);

	//FIRST LIGHT OBJECT
	// 1. Scales the light 
	scale = glm::scale(glm::vec3(0.4f, 0.4f, 0.4f));
	// 2. Rotates the light to point down
	rotation = glm::rotate(1.57f, glm::vec3(1.0, 0.0f, 0.0f));
	// 3. Place light object above the scene, in the center
	translation = glm::translate(glm::vec3(-1.0f, 6.0f, -1.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawArrays(GL_TRIANGLES, 0, meshes.gTorusMesh.nVertices);

	//SECOND LIGHT OBJECT
	// 1. Scales the light
	scale = glm::scale(glm::vec3(0.4f, 0.4f, 0.4f));
	// 2. Rotates the light to point down
	rotation = glm::rotate(1.57f, glm::vec3(1.0, 0.0f, 0.0f));
	// 3. Place object above the scene, in the center
	translation = glm::translate(glm::vec3(1.0f, 6.0f, -1.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawArrays(GL_TRIANGLES, 0, meshes.gTorusMesh.nVertices);

	glBindVertexArray(0);

	glUseProgram(0);

	//----------------------------------------------------------------------------------------------
	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}

void UDestroyMesh(GLMesh& mesh)
{
	glDeleteVertexArrays(1, &mesh.vao);
	glDeleteBuffers(2, mesh.vbos);
}

/*Generate and load the texture, written by @SNHU*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
	int width, height, channels;
	unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
	if (image)
	{
		flipImageVertically(image, width, height, channels);

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (channels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		else if (channels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			cout << "Not implemented to handle image with " << channels << " channels" << endl;
			return false;
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		return true;
	}

	// Error loading the image
	return false;
}

// Releases texture
void UDestroyTexture(GLuint textureId)
{
	glGenTextures(1, &textureId);
}



// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
	// Compilation and linkage error reporting
	int success = 0;
	char infoLog[512];

	// Create a Shader program object.
	programId = glCreateProgram();

	// Create the vertex and fragment shader objects
	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	// Retrive the shader source
	glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
	glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

	// Compile the vertex shader, and print compilation errors (if any)
	glCompileShader(vertexShaderId); // compile the vertex shader
	// check for shader compile errors
	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glCompileShader(fragmentShaderId); // compile the fragment shader
	// check for shader compile errors
	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	// Attached compiled shaders to the shader program
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);

	glLinkProgram(programId);   // links the shader program
	// check for linking errors
	glGetProgramiv(programId, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glUseProgram(programId);    // Uses the shader program

	return true;
}


void UDestroyShaderProgram(GLuint programId)
{
	glDeleteProgram(programId);
}