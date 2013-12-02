#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <GL/glew.h>

#include <GL/glfw.h>

#include <glm/glm.hpp>

#include "common/transform.hpp"
#include "common/matrix_transform.hpp"
using namespace glm;

#include "common/shader.hpp"
#include "common/texture.hpp"
#include "common/controls.hpp"
#include "common/objloader.hpp"

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4); // 4x antialiasing
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

	// Open a window and create its OpenGL context
	if( !glfwOpenWindow( 1024, 768, 0,0,0,0, 32,0, GLFW_WINDOW ) )
	{
		fprintf( stderr, "Faile to open GLFW window\n" );
		glfwTerminate();
		return -1;
	}

	//Initialize GLEW
	glewExperimental = true; //Needed in core profile
	if (glewInit() != GLEW_OK)
	{
		fprintf( stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	glfwSetWindowTitle( "Wonsz in Outer Space");

	// Ensure we can capture the escape key being pressed below
	glfwEnable( GLFW_STICKY_KEYS );
	glfwSetMousePos(1024/2, 768/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it is closer to the camera than the former one 
	glDepthFunc(GL_LESS);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program for the shaders
	GLuint programID = LoadShaders( "shaders/SimpleVertexShader.vertexshader", "shaders/SimpleFragmentShader.fragmentshader" );

	// Get a handle for our "MVP" uniform
	// Only at initialisation time.
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Load the texture using any two methods
	//GLuint Texture = loadBMP_custom("uvtemplate.bmp");
	GLuint Texture = loadDDS("assets/sun_map.DDS");
	GLuint Texture2 = loadDDS("assets/wonsz.DDS");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals;
	bool res = loadOBJ("assets/sun.obj", vertices, uvs, normals);

	std::vector< glm::vec3 > vertices2;
	std::vector< glm::vec2 > uvs2;
	std::vector< glm::vec3 > normals2;
	bool res2 = loadOBJ("assets/wonsz.obj", vertices2, uvs2, normals2);

	// This will identify our vertex buffer
	GLuint vertexbuffer[2];

	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(2, vertexbuffer);

	//The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);

	// Give our verticles to OpenGL
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, vertices2.size() * sizeof(glm::vec3), &vertices2[0], GL_STATIC_DRAW);

	//UV buffer
	GLuint uvbuffer[2];
	glGenBuffers(2, uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2) , &uvs[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, uvs2.size() * sizeof(glm::vec2) , &uvs2[0], GL_STATIC_DRAW);
	
	glm::mat4 wonszModelMatrix = glm::mat4(1.0);
	glm::mat4 camera1;
	glm::mat4 camera2;
	glm::vec3 wonszPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	do
	{
		
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs(wonszPosition);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
	
		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		// For each model you render, since the MVP will be different (at least the M part)
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);


		// 1st attribute buffer: vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
		glVertexAttribPointer(0, 3,	GL_FLOAT, GL_FALSE,	0, (void*)0);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[0]);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,	0, (void*)0	);

		// Draw the triangle!
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() ); // Starting from vertex 0; 3 vertices total -> 1 triangle

		glm::vec3 wonszDestination = glm::vec3(0.02f, 0.0f, 0.0f);
		//wonszModelMatrix = glm::rotate(wonszModelMatrix, 10.0f, wonszDestination);
		//wonszPosition += wonszDestination;
		wonszModelMatrix = glm::translate(wonszModelMatrix, wonszDestination);
		glm::vec4 wonszPositiontemp = wonszModelMatrix * vec4(0.0f, 0.0f, 0.0f, 1.0f);
		wonszPosition.x = wonszPositiontemp.x;
		wonszPosition.y = wonszPositiontemp.y;
		wonszPosition.z = wonszPositiontemp.z;

		MVP = ProjectionMatrix * ViewMatrix * wonszModelMatrix;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);

		
		// 1st attribute buffer: vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[1]);
		glVertexAttribPointer(0, 3,	GL_FLOAT, GL_FALSE,	0, (void*)0);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[1]);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,	0, (void*)0	);

		// Draw the triangle!
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() ); // Starting from vertex 0; 3 vertices total -> 1 triangle

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);


		//Swap buffers
		glfwSwapBuffers();

	}while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS && glfwGetWindowParam( GLFW_OPENED ) );

	// Cleanup VBO and shader
	glDeleteBuffers(2, vertexbuffer);
	glDeleteBuffers(2, uvbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &TextureID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}