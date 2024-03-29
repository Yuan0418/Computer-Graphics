//Some Windows Headers (For Time, IO, etc.)
#define _CRT_SECURE_NO_DEPRECATE
#include <windows.h>
#include <mmsystem.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include "maths_funcs.h"
#include "teapot.h" // teapot mesh

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;
GLuint shaderProgramID;

unsigned int teapot_vao = 0;
int width = 800;
int height = 600;

GLuint loc1;
GLuint loc2;
GLfloat rotatez = 0.0f;
GLfloat x_trans = 0.0f;
GLfloat trans_z = 10.0f, trans_y=0, trans_x = 0;//first person view to xyz


// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS

// Create a NULL-terminated string by reading the provided file
char* readShaderSource(const char* shaderFile) {
	FILE* fp = fopen(shaderFile, "rb"); //!->Why does binary flag "RB" work and not "R"... wierd msvc thing?

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders()
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, "C:/Project1/Project1/Shaders/simpleVertexShader.txt", GL_VERTEX_SHADER);
	AddShader(shaderProgramID, "C:/Project1/Project1/Shaders/simpleFragmentShader.txt", GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS

void generateObjectBufferTeapot() {
	GLuint vp_vbo = 0;

	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normals");

	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, 3 * teapot_vertex_count * sizeof(float), teapot_vertex_points, GL_STATIC_DRAW);
	GLuint vn_vbo = 0;
	glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, 3 * teapot_vertex_count * sizeof(float), teapot_normals, GL_STATIC_DRAW);

	glGenVertexArrays(1, &teapot_vao);
	glBindVertexArray(teapot_vao);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}


#pragma endregion VBO_FUNCTIONS


void display() {

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);
	
	
	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation(shaderProgramID, "model");//object
	int view_mat_location = glGetUniformLocation(shaderProgramID, "view");//camera
	int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");//

	// Hierarchy of Teapots

	// Root of the Hierarchy
	mat4 view = identity_mat4();
	mat4 persp_proj = perspective(60.0, (float)width / (float)height, 0.1, 100.0);//45
	mat4 local1 = identity_mat4();
	local1 = rotate_y_deg(local1, 0);//45
	local1 = translate(local1, vec3(x_trans, 0.0, -90.0f));//-60
	//mat4 look_at(const vec3& cam_pos, vec3 targ_pos, const vec3& up);
	view = look_at(vec3(trans_x, trans_y, trans_z), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
	// for the root, we orient it in global space
	mat4 global1 = local1;
	// update uniforms & draw
	glViewport(0, 0, width , height );
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global1.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);

	

	// child of hierarchy head 
	mat4 local2 = identity_mat4();
	local2 = rotate_y_deg(local2, rotatez);
	// translation is 15 units in the y direction from the parents coordinate system
	local2 = translate(local2, vec3(0.0, 15.0, 0.0));
	// global of the child is got by pre-multiplying the local of the child by the global of the parent
	mat4 global2 = global1 * local2;
	// update uniform & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global2.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);

	mat4 local3 = identity_mat4();
	// translation is 15 units in the y direction from the parents coordinate system
	local3 = translate(local3, vec3(-15.0,-15.0, 0.0));
	// global of the child is got by pre-multiplying the local of the child by the global of the parent
	mat4 global3 = global2 * local3;//one to one relationship global2 & local3
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global3.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);
	

	
	//TOP DOWN VIEW
	//view = rotate_z_deg(identity_mat4(),90);
	view = look_at(vec3(0,10,0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0));//make the camera looking to y
	mat4 rot=rotate_x_deg(identity_mat4(), 90);
	global1 =  global1* view;
	mat4 ort_proj = orthographic(-60.0, 60.0, -45.0, 45.0, -300.0, 300.0);//the same scale objects
	glViewport(400, 300, width / 2, height / 2);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global1.m);
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, ort_proj.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);

	mat4 rott=translate(rot, vec3(0.0, -15.0, 0.0));
	global2= global2* view;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global2.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);

	global3 = global3 * view;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global3.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);
/*
	mat4 local4 = identity_mat4();
	// translation is 15 units in the y direction from the parents coordinate system
	local4 = translate(local3, vec3(120.0, 15.0, 0.0));
	// global of the child is got by pre-multiplying the local of the child by the global of the parent
	mat4 global4 = global3 * local4;
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj2.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global4.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);
*/

	glutSwapBuffers();
}



void updateScene() {

	// Placeholder code, if you want to work with framerate
	// Wait until at least 16ms passed since start of last frame (Effectively caps framerate at ~60fps)
	static DWORD  last_time = 0;
	DWORD  curr_time = timeGetTime();
	float  delta = (curr_time - last_time) * 0.001f;
	if (delta > 0.03f)
		delta = 0.03f;
	last_time = curr_time;

	rotatez += 0.2f;
	// Draw the next frame
	glutPostRedisplay();
}


void init()
{
	// Set up the shaders
	GLuint shaderProgramID = CompileShaders();
	// load teapot mesh into a vertex buffer array
	generateObjectBufferTeapot();

}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'x':x_trans += 0.5; break; //move root to x, one to many relationship
	case 'w':trans_z += 0.5;  break; // control the camera position
	case 'a':trans_x += 0.5;  break;
	case 'd':trans_y += 2;  break;
	}

}

int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Hello Triangle");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keypress);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}











