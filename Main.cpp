#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>

#include"VAO.h"
#include"VBO.h"
#include"EBO.h"
#include"shaderClass.h"

using namespace std;

/* CS 330: Computer Graphics & Viz - Module Three - Pyramid
 * @author Janai Cano
 * @date 9/14/2023
*/

GLfloat vertices[] = {
	    // coordinates                           ||  colors
		-0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f,	0.729f, 0.333f, 0.827f, // Lower left corner
		0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f,		0.729f, 0.333f, 0.827f, // Lower right corner
		0.0f, 0.5f * float(sqrt(3)) * 2 / 3, 0.0f,	0.729f, 0.333f, 0.827f, // Upper corner
		-0.5f / 2, 0.5f * float(sqrt(3)) / 6, 0.0f,	1.000f, 0.078f, 0.576f, // Inner left
		0.5f / 2, 0.5f * float(sqrt(3)) / 6, 0.0f,	0.000f, 1.000f, 1.000f, // Inner right
		0.0f, -0.5f * float(sqrt(3)) / 3, 0.0f,		0.000f, 1.000f, 0.000f // Inner down
};

GLuint indices[] = {
	0, 3, 5,       //lower left triangle
	3, 2, 4,       //lower right triangle
	5, 4, 1        //upper triangle
};


int main() {
	// initialize glfw               
	glfwInit();			                   


	/*creating a window 
	using openGL version 4.20*/ 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	

	GLFWwindow* window = glfwCreateWindow(800, 800, "Module Three Cano", NULL, NULL);
	if (window == NULL)  // error checking
	{    
		cout << "Failed to create window" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// initialize glad
	gladLoadGL();       

	// setup window parameters
	int WIDTH = 800;	
	int HEIGHT = 800;
	glViewport(0, 0, WIDTH, HEIGHT);

	//call shader class 
	Shader shaderProgram("default.vert", "default.frag");



	// creates VAO and binds it
	VAO VAO1;
	VAO1.Bind();

	// creates VBO, binds to vertices data
	VBO VBO1(vertices, sizeof(vertices));

	// creates EBO, binds to indices data
	EBO EBO1(indices, sizeof(indices));

	// links VBO to VAO
	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
	VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	// unbind all to prevent accidentally modifying them
	VAO1.Unbind();
	VBO1.Unbind();
	EBO1.Unbind();

	//creating a uniform object
	GLuint uniID = glGetUniformLocation(shaderProgram.ID, "scale");


	// Main while loop
	while (!glfwWindowShouldClose(window))
	{
		// black background window
		glClearColor(0.00f, 0.00f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// activates shaderProgram class
		shaderProgram.Activate();
		glUniform1f(uniID, 0.5);
		// binds the VAO so we can use it
		VAO1.Bind();
		// Draw primitives, number of indices, datatype of indices, index of indices
		glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);
		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Take care of all GLFW events
		glfwPollEvents();
	}



	// Delete all the objects created
	VAO1.Delete();
	VBO1.Delete();
	EBO1.Delete();
	shaderProgram.Delete();
	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}