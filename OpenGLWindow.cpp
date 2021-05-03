//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include <System.SysUtils.hpp>
#include <debugapi.h>
#include <iostream>
#include <fstream>
#include <glm/gtx/string_cast.hpp>

#include "OpenGLWindow.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
/// Vertex shader to color triangle by color of vertices
/// Shades triangle based on angle to light
/// Reads position, normal and color from vertex buffer
/// input pvm is composite perspective / view / model matrix
const char *colorVertexSource ="#version 330 core\n"
	"layout (location = 0) in vec3 pos;\n"
	"layout (location = 1) in vec3 norm;\n"
	"layout (location = 2) in vec3 color;\n"
	"uniform mat4 pvm;\n"
	"out vec3 normalvec;\n"
	"out vec3 vertcolor;\n"
	"void main()\n"
	"{\n"
	"   gl_Position = pvm * vec4(pos, 1.0);\n"
	"   vertcolor = color;\n"
	"   normalvec = norm;\n"
	"}\0";

/// Pixel shader to color triangle by color of vertices
/// Shades triangle based on angle to light
/// input light_dir is negative of light direction
const char *colorFragmentSource = "#version 330 core\n"
	"uniform vec3 light_dir;\n"
	"uniform vec3 light_color;\n"
	"uniform vec3 ambient_color;\n"
	"in vec3 normalvec;\n"
	"in vec3 vertcolor;\n"
	"out vec4 fragcolor;\n"
	"void main()\n"
	"{\n"
	"	vec3 norm = normalize(normalvec);\n"
	"	float diff = max(dot(norm, light_dir), 0.0);\n"
	"   vec3 color = vertcolor * (diff * light_color + ambient_color);\n"
	"   fragcolor = vec4(color, 1.0f);\n"
	"}\n\0";

/// Vertex shader to color triangle using texture
/// Shades triangle based on angle to light
/// Reads position, normal and texture coordinates from vertex buffer
/// input pvm is composite perspective / view / model matrix
const char *textureVertexSource ="#version 330 core\n"
	"layout (location = 0) in vec3 pos;\n"
	"layout (location = 1) in vec3 norm;\n"
	"layout (location = 2) in vec3 color;\n"
	"layout (location = 3) in vec2 tex;\n"
	"uniform mat4 pvm;\n"
	"out vec3 vertnormal;\n"
	"out vec3 vertcolor;\n"
	"out vec2 texcoord;\n"
	"void main()\n"
	"{\n"
	"   gl_Position = pvm * vec4(pos, 1.0);\n"
	"   texcoord = tex;\n"
	"   vertnormal = norm;\n"
	"   vertcolor = color;\n"
	"}\0";

/// Pixel shader to color triangle using texture
/// Shades triangle based on angle to light
/// input light_dir is negative of light direction
const char *textureFragmentSource = "#version 330 core\n"
	"uniform sampler2D texture0;\n"
	"uniform vec3 light_dir;\n"
	"uniform vec3 light_color;\n"
	"uniform vec3 ambient_color;\n"
	"in vec3 vertnormal;\n"
	"in vec3 vertcolor;\n"
	"in vec2 texcoord;\n"
	"out vec4 fragcolor;\n"
	"void main()\n"
	"{\n"
	"	vec3 norm = normalize(vertnormal);\n"
	"	float diff = max(dot(norm, light_dir), 0.0);\n"
	"   vec3 color = diff * light_color + vertcolor + ambient_color;\n"
	"   fragcolor = texture(texture0, texcoord) * vec4(color, 1.0);\n"
	"}\n\0";

/// Vertex shader to draw bitmap in world always facing camera
/// Reads position, center, color and texture coordinates from vertex buffer
/// input pvm is composite perspective / view / model matrix
/// scalex accounts for screen ratio
const char *billboardVertexSource ="#version 330 core\n"
	"layout (location = 0) in vec3 pos;\n"
	"layout (location = 1) in vec3 center;\n"
	"layout (location = 2) in vec3 color;\n"
	"layout (location = 3) in vec2 tex;\n"
	"uniform mat4 pvm;\n"
	"uniform float xscale;\n"
	"out vec3 vertcolor;\n"
	"out vec2 texcoord;\n"
	"void main()\n"
	"{\n"
	"   gl_Position = pvm * vec4(center, 1.0f) + vec4(pos.x*xscale, pos.y, pos.z, 0.0);\n"
	"   vertcolor = color;\n"
	"   texcoord = tex;\n"
	"}\0";

/// Pixel shader to draw bitmap in world always facing camera
const char *billboardFragmentSource = "#version 330 core\n"
	"uniform sampler2D texture0;\n"
	"in vec3 vertcolor;\n"
	"in vec2 texcoord;\n"
	"out vec4 fragcolor;\n"
	"void main()\n"
	"{\n"
	"   fragcolor = vec4(vertcolor, 1.0f) * texture(texture0, texcoord);\n"
    "   if(fragcolor.a == 0) discard;\n"
	"}\n\0";

//---------------------------------------------------------------------------

static unsigned int __fastcall CreateShader(const char* vertexsource, const char* fragmentsource)
{
	/// Internal function to compile shaders from source strings

	int  success;
	char infoLog[512];

	// compile vertex shader
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexsource, NULL);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		ShowMessage(infoLog);
		return 0;
	}

	// compile fragment shader
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentsource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		ShowMessage(infoLog);
		return 0;
	}

	// link shaders
	unsigned int shaderprogram;
	shaderprogram = glCreateProgram();

	glAttachShader(shaderprogram, vertexShader);
	glAttachShader(shaderprogram, fragmentShader);
	glLinkProgram(shaderprogram);

	glGetProgramiv(shaderprogram, GL_LINK_STATUS, &success);
	if(!success) {
		glGetProgramInfoLog(shaderprogram, 512, NULL, infoLog);
		ShowMessage(infoLog);
		return 0;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderprogram;
}
//---------------------------------------------------------------------------

static bool __fastcall RayTriangle(glm::vec3& raystart, glm::vec3& raydir, float* vpos0, float* vpos1, float* vpos2, float& mindist)
{
	/// Test for intersection of ray with triangle
	/// Input raystart (camerapos) raydir - normalized direction of ray
	/// Returns true if intersects and distance less than mindist
	/// Returns updated mindist

	const float EPSILON = 0.0000001;

	glm::vec3 v0 = glm::vec3(vpos0[0], vpos0[1], vpos0[2]);
	glm::vec3 v1 = glm::vec3(vpos1[0], vpos1[1], vpos1[2]);
	glm::vec3 v2 = glm::vec3(vpos2[0], vpos2[1], vpos2[2]);

	glm::vec3 v0v1 = v1 - v0;
	glm::vec3 v0v2 = v2 - v0;
	glm::vec3 pvec = glm::cross(raydir, v0v2);
	float det = glm::dot(v0v1, pvec);

	if(det < EPSILON) return false;

	float invdet = 1.0f / det;

	glm::vec3 tvec = raystart - v0;
	float u = glm::dot(tvec, pvec) * invdet;
	if(u < 0.0f || u > 1.0f) return false;

	glm::vec3 qvec = glm::cross(tvec, v0v1);
	float v = glm::dot(raydir, qvec) * invdet;
	if(v < 0.0f || u+v > 1.0f) return false;

	float dist = glm::dot(v0v2, qvec) * invdet;

	if(dist < mindist) {
		mindist = dist;
		return true;
	}

	return false;

}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

static void error_callback(int error, const char* description)
{
	/// Called by glfw on error
	 ShowMessage(description);
}
//---------------------------------------------------------------------------

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	/// Called by glfw when key pressed
	/// Sends call to openglwindow object stored in user pointer
	TOpenGLWindow* openglwindow = (TOpenGLWindow*)glfwGetWindowUserPointer(window);
	if(openglwindow != nullptr) openglwindow->KeyCallback(key, scancode, action, mods);
}
//---------------------------------------------------------------------------

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	/// Called by glfw when window resized
	/// Sends call to openglwindow object stored in user pointer
	TOpenGLWindow* openglwindow = (TOpenGLWindow*)glfwGetWindowUserPointer(window);
	if(openglwindow != nullptr) openglwindow->ResizeCallback(width, height);
}
//---------------------------------------------------------------------------

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	/// Called by glfw when mouse button pressed or released
	/// Sends call to openglwindow object stored in user pointer
	TOpenGLWindow* openglwindow = (TOpenGLWindow*)glfwGetWindowUserPointer(window);
	if(openglwindow != nullptr) openglwindow->MouseButtonCallback(button, action, mods);
}
//---------------------------------------------------------------------------

static void cursor_position_callback(GLFWwindow* window, double x, double y)
{
	/// Called by glfw when mouse moved
	/// Sends call to openglwindow object stored in user pointer
	TOpenGLWindow* openglwindow = (TOpenGLWindow*)glfwGetWindowUserPointer(window);
	if(openglwindow != nullptr) openglwindow->MousePositionCallback(x, y);
}
//---------------------------------------------------------------------------

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	/// Called by glfw when mouse wheel scrolled
	/// Sends call to openglwindow object stored in user pointer
	TOpenGLWindow* openglwindow = (TOpenGLWindow*)glfwGetWindowUserPointer(window);
	if(openglwindow != nullptr) openglwindow->MouseScrollCallback(xoffset, yoffset);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void GLAPIENTRY message_callback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam )
{
	/// Called on message from OpenGL
	/// Output message on debug window if error

	std::string typestr;
	bool show = true;
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		typestr = "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		typestr ="DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		typestr ="UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		typestr ="PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		typestr ="PERFORMANCE";
		break;
	default:
		// don't show info messages
		show = false;
		break;
	 }

	if(show) {
		std::string severitystr;
		switch (severity){
		case GL_DEBUG_SEVERITY_LOW:
			severitystr = "LOW severity ";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			severitystr = "MEDIUM severity ";
			break;
		case GL_DEBUG_SEVERITY_HIGH:
			severitystr = "HIGH severity ";
			break;
		}

		std::string str = std::string("GL callback ") + severitystr + typestr + " message = " + std::string(message);

		OutputDebugStringA(str.c_str());
	}
}
//---------------------------------------------------------------------------

TOpenGLWindow::TOpenGLWindow(int width, int height, int samples, const char* title)
{
	/// TOpenGLWindow constructor
	/// Creates GLFW window, shaders and default font

	OnKeyEvent = nullptr;
	OnResizeEvent = nullptr;
	OnMouseButtonEvent = nullptr;
	OnMousePositionEvent = nullptr;

	cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
	cameraLookat = glm::vec3(0.0f, 0.0f, 0.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	cameraFOV = glm::pi<float>() * 0.25f;
	cameraNear = 0.1f;
    cameraFar = 100.0f;

	ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	lightDir = glm::vec3(-0.5f, -0.8f, -1.0f);
	lightDir = -glm::normalize(lightDir);

	dataChanged = true;
	window = nullptr;
	colorShader = 0;
	textureShader = 0;
	colorVAO = 0;
	backFaceCull = true;
	depthText = false;
	highlightPoint = -1;
	highlightTexture = -1;
    highlightTriangle = -1;

	window = nullptr;
	CreateWindow(width, height, samples, title);

	colorShader = CreateShader(colorVertexSource, colorFragmentSource);
	textureShader = CreateShader(textureVertexSource, textureFragmentSource);

	defaultFont = new GLFont((wchar_t*)L"FONT_PNG", (wchar_t*)L"FONT_CSV");
}
//---------------------------------------------------------------------------

TOpenGLWindow::~TOpenGLWindow()
{
	/// Destructor
	/// Cleanup GLFW
	if(colorVAO > 0) {
		glDeleteVertexArrays(1, &colorVAO);
		colorVAO = 0;
	}
	if(colorVBO > 0) {
		glDeleteBuffers(1, &colorVBO);
		colorVBO = 0;
	}

	if(window != nullptr) {
		glfwDestroyWindow(window);
        window = nullptr;
	}
	glfwTerminate();
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::CreateWindow(int width, int height, int samples, const char* title)
{
	/// Create GLFW window
	/// Setup for OpenGL version 3.3

	if (!glfwInit()) {
		// Initialization failed
		ShowMessage("GLFW initialization failed !!!");
		return;
	}
	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, samples);

	window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!window) {
		// Window or OpenGL context creation failed
		ShowMessage("Window or OpenGL context creation failed !!!");
		return;
	}

	// user defined pointer
	// used to pass window pointer to callbacks so they can call member functions
	glfwSetWindowUserPointer(window, (void*)this);

	glfwMakeContextCurrent(window);

	if(!gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress )) {
		ShowMessage("GLAD load failed !!!");
		return;
	}

	glViewport(0, 0, width, height);
	glfwSwapInterval(1);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);


	#ifdef _DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(message_callback, 0);
	#endif
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::ClearTextures()
{
	/// Delete added textures

	textureList.clear();
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::ClearData()
{
	/// Delete added triangle data

	colorList.clear();

	for(GLTexture& tex : textureList) {
		tex.ClearTriangles();
	}
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::ClearText2D()
{
	/// Delete added screen text

	defaultFont->ClearText2D();
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::ClearText3D()
{
	/// Delete added world text

	defaultFont->ClearText3D();
}
//---------------------------------------------------------------------------

int  __fastcall TOpenGLWindow::AddTexture(const std::wstring& file, bool flip)
{
	/// Creates texture from bitmap file
	/// Returns the index of the texture in the texture list

	GLTexture& tex = textureList.emplace_back();
	tex.LoadTextureFromFile(file, flip);

	return textureList.size() - 1;
}
//---------------------------------------------------------------------------

int  __fastcall TOpenGLWindow::AddTexture(TBitmap* textureBMP, bool flip)
{
	/// Creates texture from bitmap object
	/// Returns the index of the texture in the texture list
	/// If flip, bitmap is flipped vertically

	GLTexture& tex = textureList.emplace_back();
	tex.LoadTextureFromBitmap(textureBMP, flip);

	return textureList.size() - 1;
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::CreateColorArrays()
{
	/// Set up vertex data and buffers and configure vertex attributes
	/// for color triangles

	if(colorVAO > 0) {
		glDeleteVertexArrays(1, &colorVAO);
		colorVAO = 0;
	}
	if(colorVBO > 0) {
		glDeleteBuffers(1, &colorVBO);
		colorVBO = 0;
	}
	if(colorList.size() == 0) return;

	glGenVertexArrays(1, &colorVAO);
	glBindVertexArray(colorVAO);

	glGenBuffers(1, &colorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glBufferData(GL_ARRAY_BUFFER, colorList.size() * sizeof(GLColorTriangle), colorList.data(), GL_STATIC_DRAW);

	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLColorVertex), (void*)0);
	glEnableVertexAttribArray(0);

	// norm
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLColorVertex), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// color
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GLColorVertex), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::Render()
{
	/// Draw the triangles and text to the window

	if(backFaceCull) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
	else {
		glDisable(GL_CULL_FACE);
	}
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	// setup perspective view from camera position
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glm::mat4 projection = glm::perspective(cameraFOV, (float)width / (float)height, cameraNear, cameraFar);

	glm::mat4 lookat = glm::lookAt(cameraPos, cameraLookat, cameraUp);
	glm::mat4 pvm = projection * lookat;

	// clear screen
	glClearColor (0.1, 0.1, 0.2, 0.0);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(dataChanged) {
		CreateColorArrays();
		dataChanged = false;
	}

	if(colorList.size() > 0) {
		// draw color triangles
		glUseProgram(colorShader);

		int pvm_loc = glGetUniformLocation(colorShader, "pvm");
		glUniformMatrix4fv(pvm_loc, 1, GL_FALSE, glm::value_ptr(pvm));
		int light_loc = glGetUniformLocation(colorShader, "light_dir");
		glUniform3fv(light_loc, 1, glm::value_ptr(lightDir));
		int color_loc = glGetUniformLocation(colorShader, "light_color");
		glUniform3fv(color_loc, 1, glm::value_ptr(lightColor));
		int ambient_loc = glGetUniformLocation(colorShader, "ambient_color");
		glUniform3fv(ambient_loc, 1, glm::value_ptr(ambientColor));

		glBindVertexArray(colorVAO);

		glDrawArrays(GL_TRIANGLES, 0, colorList.size() * 3);
	}

	if(textureList.size() > 0) {
		// draw texture triangles
		glUseProgram(textureShader);

		int pvm_loc = glGetUniformLocation(textureShader, "pvm");
		glUniformMatrix4fv(pvm_loc, 1, GL_FALSE, glm::value_ptr(pvm));
		int light_loc = glGetUniformLocation(textureShader, "light_dir");
		glUniform3fv(light_loc, 1, glm::value_ptr(lightDir));
		int color_loc = glGetUniformLocation(textureShader, "light_color");
		glUniform3fv(color_loc, 1, glm::value_ptr(lightColor));
		int ambient_loc = glGetUniformLocation(textureShader, "ambient_color");
		glUniform3fv(ambient_loc, 1, glm::value_ptr(ambientColor));

		for(GLTexture& tex : textureList) {
            tex.Render();
		}
	}

	// draw text
	defaultFont->Render3D(window, pvm, depthText);
	defaultFont->Render2D(window);

	// display result
	glfwSwapBuffers(window);
    glfwPollEvents();
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::SetCamera(glm::vec3& pos, glm::vec3& lookat, glm::vec3& up)
{
	/// Set camera position, lookat point and up vector

	cameraPos = pos;
	cameraLookat = lookat;
	cameraUp = up;
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::AddText2D(float x, float y, float height, GLTextPos horiz_align, GLTextPos vert_align, const char* str, glm::vec3 color)
{
	/// Display text on screen using default font
	/// x, y are screen position (0 to 1)
	/// height is character size
	/// horiz_align, vert_align are position of text relative to x,y
	/// Can be LEFT, CENTER or RIGHT / ABOVE, CENTER or BELOW
	/// str is the text string - ASCII only
	/// color is the text color

    if(str == nullptr || strlen(str) == 0) return;
	defaultFont->AddText2D(x, y, height, horiz_align, vert_align, str, color);
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::AddText3D(glm::vec3 pos, float height, GLTextPos xpos, GLTextPos ypos, const char* str, glm::vec3 color, bool point)
{
	/// Display text in the world using default font
	/// pos is world coordinates
	/// height is character size
	/// horiz_align, vert_align are position of text relative to pos
	/// Can be LEFT, CENTER or RIGHT / ABOVE, CENTER or BELOW
	/// str is the text string - ASCII only
	/// color is the text color
	/// point if true draws a point at pos

	defaultFont->AddText3D(pos, height, xpos, ypos, str, color, point);
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::AddTriangleVC(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, glm::vec3& c1, glm::vec3& c2, glm::vec3& c3)
{
	/// Add triangle colored by color of vertices
	/// The normal of the vertices is the normal of the triangle (flat shaded)
	/// p1, p2, p3: position of three vertices
	/// c1, c2, c3: color of three vertices

	GLColorTriangle& tri = colorList.emplace_back();

	memcpy(tri.vert[0].pos, glm::value_ptr(p1), 3 * sizeof(float));
	memcpy(tri.vert[1].pos, glm::value_ptr(p2), 3 * sizeof(float));
	memcpy(tri.vert[2].pos, glm::value_ptr(p3), 3 * sizeof(float));

	memcpy(tri.vert[0].color, glm::value_ptr(c1), 3 * sizeof(float));
	memcpy(tri.vert[1].color, glm::value_ptr(c2), 3 * sizeof(float));
	memcpy(tri.vert[2].color, glm::value_ptr(c3), 3 * sizeof(float));

	glm::vec3 norm = glm::triangleNormal(p1, p2, p3);

	memcpy(tri.vert[0].norm, glm::value_ptr(norm), 3 * sizeof(float));
	memcpy(tri.vert[1].norm, glm::value_ptr(norm), 3 * sizeof(float));
	memcpy(tri.vert[2].norm, glm::value_ptr(norm), 3 * sizeof(float));

	dataChanged = true;
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::AddTriangleVNC(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, glm::vec3& n1, glm::vec3& n2, glm::vec3& n3, glm::vec3& c1, glm::vec3& c2, glm::vec3& c3)
{
	/// Add triangle colored by color of vertices
	/// p1, p2, p3: position of three vertices
	/// n1, n2, n3: normal at each vertex
	/// c1, c2, c3: color of three vertices

	GLColorTriangle& tri = colorList.emplace_back();

	memcpy(tri.vert[0].pos, glm::value_ptr(p1), 3 * sizeof(float));
	memcpy(tri.vert[1].pos, glm::value_ptr(p2), 3 * sizeof(float));
	memcpy(tri.vert[2].pos, glm::value_ptr(p3), 3 * sizeof(float));

	memcpy(tri.vert[0].color, glm::value_ptr(c1), 3 * sizeof(float));
	memcpy(tri.vert[1].color, glm::value_ptr(c2), 3 * sizeof(float));
	memcpy(tri.vert[2].color, glm::value_ptr(c3), 3 * sizeof(float));

	memcpy(tri.vert[0].norm, glm::value_ptr(n1), 3 * sizeof(float));
	memcpy(tri.vert[1].norm, glm::value_ptr(n2), 3 * sizeof(float));
	memcpy(tri.vert[2].norm, glm::value_ptr(n3), 3 * sizeof(float));

	dataChanged = true;
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::AddTriangleVT(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, glm::vec2& t1, glm::vec2& t2, glm::vec2& t3, int texid)
{
	/// Add triangle colored by texture
	/// The normal of the vertices is the normal of the triangle (flat shaded)
	/// p1, p2, p3: position of three vertices
	/// t1, t2, t3: texture coordinates of three vertices
	/// texid: index of texture returned by AddTexture

	GLTexture& tex = textureList[texid];
	tex.AddTriangleVT(p1, p2, p3, t1, t2, t3);
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::AddTriangleVNT(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, glm::vec3& n1, glm::vec3& n2, glm::vec3& n3, glm::vec2& t1, glm::vec2& t2, glm::vec2& t3, int texid)
{
	/// Add triangle colored by texture
	/// p1, p2, p3: position of three vertices
	/// n1, n2, n3: normal at each vertex
	/// t1, t2, t3: texture coordinates of three vertices
	/// texid: index of texture returned by AddTexture

	GLTexture& tex = textureList[texid];
	tex.AddTriangleVNT(p1, p2, p3, n1, n2, n3, t1, t2, t3);
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::GetMousePos(double& x, double& y)
{
	/// Returns current cursor position

	if(window == nullptr) return;
	glfwGetCursorPos(window, &x, &y);
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::SetLightDir(glm::vec3& dir)
{
	/// Set the light direction
	/// Stored as negative of given direction
	/// Because surface normal in same direction as light should be fully lit

	lightDir = -glm::normalize(dir);
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::SetAmbientColor(TAlphaColor color)
{
	float red = (float)((color >> 16) & 0xFF) / 255.0f;
	float green = (float)((color >> 8) & 0xFF) / 255.0f;
	float blue = (float)(color & 0xFF) / 255.0f;
	ambientColor = glm::vec3(red, green, blue);
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::SetLightColor(TAlphaColor color)
{
	float red = (float)((color >> 16) & 0xFF) / 255.0f;
	float green = (float)((color >> 8) & 0xFF) / 255.0f;
	float blue = (float)(color & 0xFF) / 255.0f;
	lightColor = glm::vec3(red, green, blue);
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::KeyCallback(int key, int scancode, int action, int mods)
{
	/// Called by key press in GLFW window
	/// Passed to handler if set

	if(OnKeyEvent != nullptr) OnKeyEvent(this, key, scancode, action, mods);
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::ResizeCallback(int width, int height)
{
	/// Called by resize of GLFW window
	/// Passed to handler if set
	/// Repaint window if no handler

	if(OnResizeEvent != nullptr) OnResizeEvent(this, width, height);
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::MouseButtonCallback(int button, int action, int mods)
{
	/// Called by mouse click in GLFW window
	/// Passed to handler if set
	/// button: GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_RIGHT or GLFW_MOUSE_BUTTON_MIDDLE
	/// action: GLFW_PRESS or GLFW_RELEASE
	/// mods: GLFW_MOD_SHIFT, GLFW_MOD_CONTROL, GLFW_MOD_ALT, GLFW_MOD_SUPER, GLFW_MOD_CAPS_LOCK or GLFW_MOD_NUM_LOCK

	if(OnMouseButtonEvent != nullptr) OnMouseButtonEvent(this, button, action, mods);
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::MousePositionCallback(double x, double y)
{
	/// Called by mouse move in GLFW window
	/// Passed to handler if set

	if(OnMousePositionEvent != nullptr) OnMousePositionEvent(this, x, y);
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::MouseScrollCallback(double xoffset, double yoffset)
{
	/// Called by mouse wheel scroll in GLFW window
	/// Passed to handler if set

	if(OnMouseScrollEvent != nullptr) OnMouseScrollEvent(this, yoffset);
}
//---------------------------------------------------------------------------

GLPickResult __fastcall TOpenGLWindow::PickElement(double x, double y)
{
	/// Gets closest element at screenx, screeny

	glm::vec3 raydir = CreateRay(x, y);
	glm::vec3 raystart = cameraPos;

	float mindist = cameraFar;
	int mintex = -2; // no triangle
	int mintri = -1;
	for(int t=0; t<textureList.size(); t++) {
		int tri = textureList[t].PickTriangle(raystart, raydir, mindist);
		if(tri >= 0) {
			mintex = t; // texture triangle
			mintri = tri;
		}
	}

	for(int c=0; c<colorList.size(); c++) {
		if(RayTriangle(raystart, raydir, colorList[c].vert[0].pos, colorList[c].vert[1].pos, colorList[c].vert[2].pos, mindist)) {
			mintex = -1; // color triangle
			mintri = c;
		}

    }

	GLPickResult result;
	result.dist = mindist;

	int point = defaultFont->PickPoint(raystart, raydir, mindist);
	if(point >= 0) {
		result.type = GLPickType::POINT;
		result.group = 0;
		result.index = point;
		result.color = defaultFont->GetPointColor(point);
	}
	else if(mintex == -1) {
		result.type = GLPickType::COLOR;
		result.group = 0;
		result.index = mintri;
		result.color = GetColorTriangleColor(mintri);
	}
	else if(mintex >= 0) {
		result.type = GLPickType::TRIANGLE;
		result.group = mintex;
		result.index = mintri;
		result.color = textureList[mintex].GetTriangleColor(mintri);
	}

	return result;
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::SetElementColor(GLPickResult& pick, glm::vec3 newcolor)
{
	if(pick.type == GLPickType::NONE) return;
	else if(pick.type == GLPickType::POINT) {
		defaultFont->SetPointColor(pick.index, newcolor);
	}
	else if(pick.type == GLPickType::COLOR) {
		SetColorTriangleColor(pick.index, newcolor);
	}
	else if(pick.type == GLPickType::TRIANGLE) {
		textureList[pick.group].SetTriangleColor(pick.index, newcolor);
	}

}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::SetColorTriangleColor(int trinum, glm::vec3& color)
{
	if(trinum < 0 || trinum >= colorList.size()) return;

	glBindVertexArray(colorVAO);
	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);

	GLColorTriangle& tri = colorList[trinum];
	memcpy(tri.vert[0].color, glm::value_ptr(color), sizeof(glm::vec3));
	memcpy(tri.vert[1].color, glm::value_ptr(color), sizeof(glm::vec3));
	memcpy(tri.vert[2].color, glm::value_ptr(color), sizeof(glm::vec3));
	glBufferSubData(GL_ARRAY_BUFFER, trinum * sizeof(GLColorTriangle), sizeof(GLColorTriangle), &tri);
}
//---------------------------------------------------------------------------

glm::vec3 __fastcall TOpenGLWindow::GetColorTriangleColor(int trinum)
{
	if(trinum < 0 || trinum >= colorList.size()) return glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 color;
	GLColorTriangle& tri = colorList[trinum];
	memcpy(glm::value_ptr(color), tri.vert[0].color, sizeof(glm::vec3));

    return color;
}
//---------------------------------------------------------------------------

glm::vec3 __fastcall TOpenGLWindow::CreateRay(double x, double y)
{
	/// Create ray from screen position

	int width, height;
	glfwGetWindowSize(window, &width, &height);

	glm::mat4 projection = glm::perspective(cameraFOV, (float)width / (float)height, cameraNear, cameraFar);
	glm::mat4 lookat = glm::lookAt(cameraPos, cameraLookat, cameraUp);
	glm::vec4 viewport = glm::vec4(0, 0, width, height);

	// get world coordinate of far z
	glm::vec3 wincoord = glm::vec3(x, height - y, 1.0f);
	glm::vec3 worldcoord = glm::unProject(wincoord, lookat, projection, viewport);

	// ray from camera to far point
	glm::vec3 dir = worldcoord - cameraPos;
	dir = glm::normalize(dir);

	return dir;
}
//---------------------------------------------------------------------------

void __fastcall TOpenGLWindow::AddModel(const char* filename)
{
	//tinyobj::attrib_t attributes;
	//  struct attrib_t {
	//    std::vector<real_t> vertices;  			// 'v'(xyz)
	//    std::vector<real_t> vertex_weights;  		// 'v'(w) For backward compatibility, we store vertex weight in separate array.
	//    std::vector<real_t> normals;         		// 'vn'
	//    std::vector<real_t> texcoords;       		// 'vt'(uv)
	//    std::vector<real_t> texcoord_ws;  		// 'vt'(w) For backward compatibility, we store texture coordinate 'w' in separate
	//    std::vector<real_t> colors;       		// extension: vertex colors
	//    std::vector<skin_weight_t> skin_weights;	//`skin_weight_t::vertex_id` == `vid`
	//		  struct skin_weight_t {
	//		      int vertex_id;  					// Corresponding vertex index in `attrib_t::vertices`.
	//		      std::vector<joint_and_weight_t> weightValues;
	//std::vector<tinyobj::shape_t> shapes;
	//	struct shape_t {
	//    std::string name;
	//
	//	  mesh_t mesh;
	//	      struct mesh_t {
	//		      std::vector<index_t> indices;
	//				  struct index_t {
	//					  int vertex_index;
	//					  int normal_index;
	//					  int texcoord_index;
	//			  std::vector<unsigned char> num_face_vertices;         // The number of vertices per face. 3 = triangle, 4 = quad,
	//			  std::vector<int> material_ids;  						// per-face material ID
	//			  std::vector<unsigned int> smoothing_group_ids;  		// per-face smoothing group ID(0 = off. positive value = group id)
	//			  std::vector<tag_t> tags;                        		// SubD tag
	//
	//    lines_t lines;
	//		  struct lines_t {
	//		      std::vector<index_t> indices;        // indices for vertices(poly lines)
	//		      std::vector<int> num_line_vertices;  // The number of vertices per line.
	//
	//    points_t points;
	//	      struct points_t {
	//			  std::vector<index_t> indices;  // indices for points
	//std::vector<tinyobj::material_t> materials;
	//   struct material_t {
	//		std::string name;
	//
	//		real_t ambient[3];
	//		real_t diffuse[3];
	//		real_t specular[3];
	//		real_t transmittance[3];
	//		real_t emission[3];
	//		real_t shininess;
	//		real_t ior;       		     // index of refraction
	//		real_t dissolve;  			 // 1 == opaque; 0 == fully transparent
	//		real_t roughness;            // [0, 1] default 0
	//		real_t metallic;             // [0, 1] default 0
	//		real_t sheen;                // [0, 1] default 0
	//		real_t clearcoat_thickness;  // [0, 1] default 0
	//		real_t clearcoat_roughness;  // [0, 1] default 0
	//		real_t anisotropy;           // aniso. [0, 1] default 0
	//		real_t anisotropy_rotation;  // anisor. [0, 1] default 0
	//		int illum;                   // illumination model
	//			0 Color on and Ambient off
	//			1 Color on and Ambient on
	//			2 Highlight on
	//			3 Reflection on and Ray trace on
	//			4 Transparency: Glass on, Reflection: Ray trace on
	//			5 Reflection: Fresnel on and Ray trace on
	//			6 Transparency: Refraction on, Reflection: Fresnel off and Ray trace on
	//			7 Transparency: Refraction on, Reflection: Fresnel on and Ray trace on
	//			8 Reflection on and Ray trace off
	//			9 Transparency: Glass on, Reflection: Ray trace off
	//			10 Casts shadows onto invisible surfaces
	//
	//		std::string ambient_texname;             // map_Ka
	//		std::string diffuse_texname;             // map_Kd
	//		std::string specular_texname;            // map_Ks
	//		std::string specular_highlight_texname;  // map_Ns
	//		std::string bump_texname;                // map_bump, map_Bump, bump
	//		std::string displacement_texname;        // disp
	//		std::string alpha_texname;               // map_d
	//		std::string reflection_texname;          // refl
	//		std::string roughness_texname; 			 // map_Pr
	//		std::string metallic_texname;   		 // map_Pm
	//		std::string sheen_texname;      		 // map_Ps
	//		std::string emissive_texname;   		 // map_Ke
	//		std::string normal_texname;     		 // norm. For normal mapping.
	//
	//		texture_option_t ambient_texopt;
	//		texture_option_t diffuse_texopt;
	//		texture_option_t specular_texopt;
	//		texture_option_t specular_highlight_texopt;
	//		texture_option_t bump_texopt;
	//		texture_option_t displacement_texopt;
	//		texture_option_t alpha_texopt;
	//		texture_option_t reflection_texopt;
	//		texture_option_t roughness_texopt;
	//		texture_option_t metallic_texopt;
	//		texture_option_t sheen_texopt;
	//		texture_option_t emissive_texopt;
	//		texture_option_t normal_texopt;
	//
	//			struct texture_option_t {
	//			  texture_type_t type;      // -type (default TEXTURE_TYPE_NONE)
	//			  real_t sharpness;         // -boost (default 1.0?)
	//			  real_t brightness;        // base_value in -mm option (default 0)
	//			  real_t contrast;          // gain_value in -mm option (default 1)
	//			  real_t origin_offset[3];  // -o u [v [w]] (default 0 0 0)
	//			  real_t scale[3];          // -s u [v [w]] (default 1 1 1)
	//			  real_t turbulence[3];     // -t u [v [w]] (default 0 0 0)
	//			  int texture_resolution;   // -texres resolution (No default value in the spec.We'll use -1)
	//			  bool clamp;               // -clamp (default false)
	//			  char imfchan;  			// -imfchan (the default for bump is 'l' and for decal is 'm')
	//			  bool blendu;   			// -blendu (default on)
	//			  bool blendv;  			// -blendv (default on)
	//			  real_t bump_multiplier;   // -bm (for bump maps only, default 1.0)
	//			  std::string colorspace;   // Explicitly specify color space of stored texel value. Usually `sRGB` or `linear` (default empty).

	tinyobj::attrib_t attributes;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warning;
	std::string error;

	AnsiString dir = ExtractFileDir(filename);
	if (!tinyobj::LoadObj( &attributes, &shapes, &materials, &warning, &error, filename, dir.c_str())) {
		std::string message = "Load obj error: " + warning + error;
		ShowMessage(message.c_str());
		return;
	}

	// add diffuse map from materials as texture
    // assume material files are in same directory as obj file
	int firstmat = textureList.size();
	for(tinyobj::material_t& material : materials) {
		String matfile = dir + "\\" + material.diffuse_texname.c_str();
		AddTexture(matfile.c_str(), true);
	}

	for(tinyobj::shape_t& shape : shapes) {
		int vertcount = shape.mesh.indices.size() / 3;
		for(int v=0; v<vertcount; v++) {

			tinyobj::index_t i0 = shape.mesh.indices[3 * v + 0];
			tinyobj::index_t i1 = shape.mesh.indices[3 * v + 1];
			tinyobj::index_t i2 = shape.mesh.indices[3 * v + 2];
			glm::vec3 v1 = glm::vec3(attributes.vertices[3 * i0.vertex_index + 0], attributes.vertices[3 * i0.vertex_index + 1], attributes.vertices[3 * i0.vertex_index + 2]);
			glm::vec3 v2 = glm::vec3(attributes.vertices[3 * i1.vertex_index + 0], attributes.vertices[3 * i1.vertex_index + 1], attributes.vertices[3 * i1.vertex_index + 2]);
			glm::vec3 v3 = glm::vec3(attributes.vertices[3 * i2.vertex_index + 0], attributes.vertices[3 * i2.vertex_index + 1], attributes.vertices[3 * i2.vertex_index + 2]);
			glm::vec3 n1 = glm::vec3(attributes.normals[3 * i0.normal_index + 0], attributes.normals[3 * i0.normal_index + 1], attributes.normals[3 * i0.normal_index + 2]);
			glm::vec3 n2 = glm::vec3(attributes.normals[3 * i1.normal_index + 0], attributes.normals[3 * i1.normal_index + 1], attributes.normals[3 * i1.normal_index + 2]);
			glm::vec3 n3 = glm::vec3(attributes.normals[3 * i2.normal_index + 0], attributes.normals[3 * i2.normal_index + 1], attributes.normals[3 * i2.normal_index + 2]);
			glm::vec2 t1 = glm::vec2(attributes.texcoords[2 * i0.texcoord_index + 0], attributes.texcoords[2 * i0.texcoord_index + 1]);
			glm::vec2 t2 = glm::vec2(attributes.texcoords[2 * i1.texcoord_index + 0], attributes.texcoords[2 * i1.texcoord_index + 1]);
			glm::vec2 t3 = glm::vec2(attributes.texcoords[2 * i2.texcoord_index + 0], attributes.texcoords[2 * i2.texcoord_index + 1]);
			int matid = shape.mesh.material_ids[v] + firstmat;
			AddTriangleVNT(v1, v2, v3, n1, n2, n3, t1, t2, t3, matid);
		}
	}

}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void __fastcall GLTexture::LoadTextureFromFile(const std::wstring& file, bool flip)
{
	/// Loads texture from file

	filename = file;
	TBitmap* textureBMP = new TBitmap();
	try {
		textureBMP->LoadFromFile(filename.c_str());
		LoadTextureFromBitmap(textureBMP, flip);
	} catch (EFOpenError& e) {
		String message = "Failed to load ";
		message = message + filename.c_str();
		ShowMessage(message.c_str());
	}

	delete textureBMP;
}
//---------------------------------------------------------------------------

void __fastcall GLTexture::LoadTextureFromResource(const wchar_t* bmpresource, bool flip)
{
	/// Loads texture from program resource

	TBitmap* bmp = new TBitmap();
	TResourceStream *bmp_stream = new TResourceStream((int)HInstance, bmpresource, (System::WideChar*)RT_RCDATA);
	__try {
		__try {
			bmp->LoadFromStream(bmp_stream);
		}
		__finally {
		}
	}
	__finally {
		bmp_stream->DisposeOf();
	}

	LoadTextureFromBitmap(bmp, flip);
	delete bmp;
}
//---------------------------------------------------------------------------

void __fastcall GLTexture::LoadTextureFromBitmap(TBitmap* textureBMP, bool flip)
{
	/// Loads bitmap file into texture
	/// Uses TBitmap to read file
    /// if flip, bitmap is flipped vertically

	textureID = 0;
	VAO = 0;
	VBO = 0;

	TBitmapData data;
	if(!textureBMP->Map(TMapAccess::Read, data)) {
		ShowMessage("Failed to access texture !!!");
		return;
	}

	int numpix = textureBMP->Width * textureBMP->Height;
	if(numpix == 0) {
		ShowMessage("Failed to load texture !!!");
		textureBMP->Unmap(data);
		return;
	}

	if(flip) {
		// texture is flipped
		textureBMP->FlipVertical();
	}

	int bpp = data.BytesPerPixel;

	//unsigned int { None, RGB, RGBA, BGR, BGRA, RGBA16, BGR_565, BGRA4, BGR4, BGR5_A1, BGR5, BGR10_A2, RGB10_A2, L, LA, LA4, L16, A, R16F, RG16F, RGBA16F, R32F, RG32F, RGBA32F };
	Fmx::Types::TPixelFormat bmpformat = data.PixelFormat;

	// GL_ALPHA, GL_ALPHA4, GL_ALPHA8, GL_ALPHA12, GL_ALPHA16, GL_COMPRESSED_ALPHA, GL_COMPRESSED_LUMINANCE, GL_COMPRESSED_LUMINANCE_ALPHA,
	// GL_COMPRESSED_INTENSITY, GL_COMPRESSED_RGB, GL_COMPRESSED_RGBA, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32,
	// GL_LUMINANCE, GL_LUMINANCE4, GL_LUMINANCE8, GL_LUMINANCE12, GL_LUMINANCE16, GL_LUMINANCE_ALPHA, GL_LUMINANCE4_ALPHA4, GL_LUMINANCE6_ALPHA2,
	// GL_LUMINANCE8_ALPHA8, GL_LUMINANCE12_ALPHA4, GL_LUMINANCE12_ALPHA12, GL_LUMINANCE16_ALPHA16, GL_INTENSITY, GL_INTENSITY4, GL_INTENSITY8,
	// GL_INTENSITY12, GL_INTENSITY16, GL_R3_G3_B2, GL_RGB, GL_RGB4, GL_RGB5, GL_RGB8, GL_RGB10, GL_RGB12, GL_RGB16, GL_RGBA, GL_RGBA2, GL_RGBA4,
	//  GL_RGB5_A1, GL_RGBA8, GL_RGB10_A2, GL_RGBA12, GL_RGBA16, GL_SLUMINANCE, GL_SLUMINANCE8, GL_SLUMINANCE_ALPHA, GL_SLUMINANCE8_ALPHA8, GL_SRGB,
	// GL_SRGB8, GL_SRGB_ALPHA, or GL_SRGB8_ALPHA8.
	GLenum glformat;


	unsigned char* buffer = new unsigned char[numpix * bpp];
	memcpy(buffer, data.Data, numpix * bpp);

	if(bmpformat == TPixelFormat::RGB) glformat = GL_RGB;
	else if(bmpformat == TPixelFormat::RGBA) glformat = GL_RGBA;
	else if(bmpformat == TPixelFormat::BGR) {
		glformat = GL_RGB;
		for(int p=0; p<numpix; p++) {
			unsigned char r = buffer[p*3];
			buffer[p*3] = buffer[p*3+2];
			buffer[p*3+2] = r;
		}
	}
	else if(bmpformat == TPixelFormat::BGRA) {
		glformat = GL_RGBA;
		for(int p=0; p<numpix; p++) {
			unsigned char r = buffer[p*4];
			buffer[p*4] = buffer[p*4+2];
			buffer[p*4+2] = r;
		}
	}
	else {
		ShowMessage("Unknown texture file format !!!");
		textureID = 0;
		textureBMP->Unmap(data);
		delete textureBMP;
		delete[] buffer;
		return;
	}

	glGenTextures(1, &textureID); // Create The Texture

	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, glformat, textureBMP->Width, textureBMP->Height, 0, glformat, GL_UNSIGNED_BYTE, buffer);
    glGenerateMipmap(GL_TEXTURE_2D);

	GLenum error = glGetError();
	if(error != GL_NO_ERROR) {
		ShowMessage("Error creating texture !!!");
		textureID = 0;
	}

	textureBMP->Unmap(data);
	delete[] buffer;
}
//---------------------------------------------------------------------------

GLTexture::GLTexture()
{
	/// Constructor
	changed = true;
	VAO = 0;
	VBO = 0;
    textureID = 0;
}
//---------------------------------------------------------------------------

GLTexture::~GLTexture()
{
	/// Destructor deletes texture
	if(VAO > 0) {
		glDeleteVertexArrays(1, &VAO);
		VAO = 0;
	}
	if(VBO > 0) {
		glDeleteBuffers(1, &VBO);
		VBO = 0;
	}
	glDeleteTextures(1, &textureID);
}
//---------------------------------------------------------------------------

void __fastcall GLTexture::SetTriangleColor(int trinum, glm::vec3& color)
{
	if(trinum < 0 || trinum >= triangleList.size()) return;

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	GLTextureTriangle& tri = triangleList[trinum];
	memcpy(tri.vert[0].color, glm::value_ptr(color), sizeof(glm::vec3));
	memcpy(tri.vert[1].color, glm::value_ptr(color), sizeof(glm::vec3));
	memcpy(tri.vert[2].color, glm::value_ptr(color), sizeof(glm::vec3));
	glBufferSubData(GL_ARRAY_BUFFER, trinum * sizeof(GLTextureTriangle), sizeof(GLTextureTriangle), &tri);
}
//---------------------------------------------------------------------------

glm::vec3 __fastcall GLTexture::GetTriangleColor(int trinum)
{
	if(trinum < 0 || trinum >= triangleList.size()) return glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 color;
	GLTextureTriangle& tri = triangleList[trinum];
	memcpy(glm::value_ptr(color), tri.vert[0].color, sizeof(glm::vec3));

    return color;
}
//---------------------------------------------------------------------------

void __fastcall GLTexture::CreateArrays()
{
	/// Set up vertex data and buffers and configure vertex attributes
	/// for textured triangles

	if(VAO > 0) {
		glDeleteVertexArrays(1, &VAO);
		VAO = 0;
	}
	if(VBO > 0) {
		glDeleteBuffers(1, &VBO);
		VBO = 0;
	}

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// triangle buffer is GL_DYNAMIC_DRAW to allow changes to color for highlighting
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, triangleList.size() * sizeof(GLTextureTriangle), triangleList.data(), GL_DYNAMIC_DRAW);


	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLTextureVertex), (void*)0);
	glEnableVertexAttribArray(0);

	// norm
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLTextureVertex), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// color
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GLTextureVertex), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// texture coord
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(GLTextureVertex), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
//---------------------------------------------------------------------------

void __fastcall GLTexture::Render()
{
	if(changed) {
		CreateArrays();
		changed = false;
	}

	if(triangleList.size() > 0) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, triangleList.size() * 3);
	}
}
//---------------------------------------------------------------------------

void __fastcall GLTexture::AddTriangleVT(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, glm::vec2& t1, glm::vec2& t2, glm::vec2& t3)
{
	/// Add triangle colored by texture
	/// The normal of the vertices is the normal of the triangle (flat shaded)
	/// p1, p2, p3: position of three vertices
	/// t1, t2, t3: texture coordinates of three vertices
	/// texid: index of texture returned by AddTexture

	GLTextureTriangle& tri = triangleList.emplace_back();

	memcpy(tri.vert[0].pos, glm::value_ptr(p1), 3 * sizeof(float));
	memcpy(tri.vert[1].pos, glm::value_ptr(p2), 3 * sizeof(float));
	memcpy(tri.vert[2].pos, glm::value_ptr(p3), 3 * sizeof(float));

	glm::vec3 norm = glm::triangleNormal(p1, p2, p3);
	memcpy(tri.vert[0].norm, glm::value_ptr(norm), 3 * sizeof(float));
	memcpy(tri.vert[1].norm, glm::value_ptr(norm), 3 * sizeof(float));
	memcpy(tri.vert[2].norm, glm::value_ptr(norm), 3 * sizeof(float));

	glm::vec3 color(0.0f, 0.0f, 0.0f);
	memcpy(tri.vert[0].color, glm::value_ptr(color), 3 * sizeof(float));
	memcpy(tri.vert[1].color, glm::value_ptr(color), 3 * sizeof(float));
	memcpy(tri.vert[2].color, glm::value_ptr(color), 3 * sizeof(float));

	memcpy(tri.vert[0].tex, glm::value_ptr(t1), 2 * sizeof(float));
	memcpy(tri.vert[1].tex, glm::value_ptr(t2), 2 * sizeof(float));
	memcpy(tri.vert[2].tex, glm::value_ptr(t3), 2 * sizeof(float));

	changed = true;
}
//---------------------------------------------------------------------------

void __fastcall GLTexture::AddTriangleVNT(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, glm::vec3& n1, glm::vec3& n2, glm::vec3& n3, glm::vec2& t1, glm::vec2& t2, glm::vec2& t3)
{
	/// Add triangle colored by texture
	/// p1, p2, p3: position of three vertices
	/// n1, n2, n3: normal at each vertex
	/// t1, t2, t3: texture coordinates of three vertices
	/// texid: index of texture returned by AddTexture

	GLTextureTriangle& tri = triangleList.emplace_back();

	memcpy(tri.vert[0].pos, glm::value_ptr(p1), 3 * sizeof(float));
	memcpy(tri.vert[1].pos, glm::value_ptr(p2), 3 * sizeof(float));
	memcpy(tri.vert[2].pos, glm::value_ptr(p3), 3 * sizeof(float));

	memcpy(tri.vert[0].norm, glm::value_ptr(n1), 3 * sizeof(float));
	memcpy(tri.vert[1].norm, glm::value_ptr(n2), 3 * sizeof(float));
	memcpy(tri.vert[2].norm, glm::value_ptr(n3), 3 * sizeof(float));

	glm::vec3 color(0.0f, 0.0f, 0.0f);
	memcpy(tri.vert[0].color, glm::value_ptr(color), 3 * sizeof(float));
	memcpy(tri.vert[1].color, glm::value_ptr(color), 3 * sizeof(float));
	memcpy(tri.vert[2].color, glm::value_ptr(color), 3 * sizeof(float));

	memcpy(tri.vert[0].tex, glm::value_ptr(t1), 2 * sizeof(float));
	memcpy(tri.vert[1].tex, glm::value_ptr(t2), 2 * sizeof(float));
	memcpy(tri.vert[2].tex, glm::value_ptr(t3), 2 * sizeof(float));

	changed = true;
}
//---------------------------------------------------------------------------

int __fastcall GLTexture::PickTriangle(glm::vec3& raystart, glm::vec3& raydir, float& mindist)
{
	int tri = -1;
	for(int i=0; i<triangleList.size(); i++) {
		if(RayTriangle(raystart, raydir, triangleList[i].vert[0].pos, triangleList[i].vert[1].pos, triangleList[i].vert[2].pos, mindist)) {
			tri = i;
		}
	}
	return tri;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

static std::string __fastcall getNext(TResourceStream* stream)
{
	/// Used by GLFont to read csv font data

	std::string str;
	char c;

	while(stream->Read(&c, 1) == 1) {
		if(c == ',' || c == '\n') break;
		str = str + c;
	}

	return str;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

GLFont::GLFont(wchar_t* bmpresource, wchar_t* dataresource)
{
	/// Create Font bitmap from image resource and data resource
    /// Resources created using "Codehead's Bitmap Font Generator"

	text2DChanged = true;
	text3DChanged = true;
	VAO2D = 0;
	VAO3D = 0;
	VAOP = 0;
	VBOP = 0;
	pointSize = 0.05f;


	billboardShader = CreateShader(billboardVertexSource, billboardFragmentSource);

	fontTexture.LoadTextureFromResource(bmpresource, false);
	pointTexture.LoadTextureFromResource(L"POINT_PNG", false);

	// character spacing
	TResourceStream *spacing_stream = new TResourceStream((NativeUInt)HInstance, dataresource, (System::WideChar*)RT_RCDATA);
	__try {
		__try {
			// load key-value map
			bool finished = false;
			while(!finished) {
				std::string key = getNext(spacing_stream);
				if(key.size() == 0) {
					finished = true;
				}
				else {
					std::string value = getNext(spacing_stream);
					if(isdigit(value[0])) {
						int v = std::stoi(value, nullptr);
						props.emplace(std::make_pair(key, v));
					}
				}
			}
		}
		__finally {
		}
	}
	__finally {
		spacing_stream->DisposeOf();
	}

	for(int w=0; w<256; w++) {
		std::string str = "Char " + std::to_string(w) + " Base Width";
		charWidth[w] = props[str];
	}

	imageWidth = props["Image Width"];
	imageHeight = props["Image Height"];
	cellWidth = props["Cell Width"];
	cellHeight = props["Cell Height"];
	startChar = props["Start Char"];
	fontHeight = props["Font Height"];
}
//---------------------------------------------------------------------------

GLFont::~GLFont()
{
	if(VAO2D != 0) glDeleteBuffers(1, &VAO2D);
	if(VAO3D != 0) glDeleteBuffers(1, &VAO3D);
	if(VBOP != 0) glDeleteBuffers(1, &VBOP);
	if(VAOP != 0) glDeleteBuffers(1, &VAOP);
}
//---------------------------------------------------------------------------

void __fastcall GLFont::AddText2D(float centerx, float centery, float height, GLTextPos horiz_align, GLTextPos vert_align, const char* str, glm::vec3 color)
{
	/// Create textured triangles and add to list
	/// centerx, centery are screen position (0 to 1)
	/// height is character size
	/// horiz_align, vert_align are position of text relative to center
	/// Can be LEFT, CENTER or RIGHT / ABOVE, CENTER or BELOW
	/// str is the text string - ASCII only
	/// color is the text color

	glm::vec3 pos(centerx, centery, 0.0);

	// scale converts font pixel width to screen size
	float scalex = height / (float)fontHeight;

	float dx = 0.0f;
	float dy = 0.0f;

	float strsize = 0;
	for(int i=0; i<strlen(str); i++) {
		int c = (int)(str[i]);
		strsize += charWidth[c];
	}
	strsize *= scalex;

	if(horiz_align == GLTextPos::LEFT) {
		dx -= strsize;
	}
	if(horiz_align == GLTextPos::CENTER) {
		dx -= strsize / 2.0f;
	}
	if(vert_align == GLTextPos::BELOW) {
		dy -= height;
	}
	if(vert_align == GLTextPos::CENTER) {
		dy -= height / 2.0f;
	}

	int char_per_line = imageWidth / cellWidth;
	float texh = (float)fontHeight / (float)imageHeight;

	for(int i=0; i<strlen(str); i++) {
		int c = (int)(str[i]);

		float charwidth = charWidth[c] * scalex;

		glm::vec3 p0(dx, dy, 0.0f);   					 // BL
		glm::vec3 p1(dx + charwidth, dy, 0.0f);    		 // BR
		glm::vec3 p2(dx + charwidth, dy + height, 0.0f);   // TR
		glm::vec3 p3(dx, dy + height, 0.0f); 				 // TL

		int pixleft = cellWidth * ((c  - startChar) % char_per_line);
		int pixtop = cellHeight * ((c  - startChar) / char_per_line);
		float texleft = (float)pixleft / (float)imageWidth;
		float textop = ((float)pixtop / (float)imageHeight);
		float texw = (float)charWidth[c] / (float)imageWidth;

		glm::vec2 t0(texleft, textop+texh);      // BL
		glm::vec2 t1(texleft+texw, textop+texh); // BR
		glm::vec2 t2(texleft+texw, textop);      // TR
		glm::vec2 t3(texleft, textop);           // TL

		GLBillboardQuad& quad = quad2DList.emplace_back();

		// triangle 1
		memcpy(quad.tri1[0].pos, glm::value_ptr(p0), 3 * sizeof(float));
		memcpy(quad.tri1[1].pos, glm::value_ptr(p1), 3 * sizeof(float));
		memcpy(quad.tri1[2].pos, glm::value_ptr(p2), 3 * sizeof(float));

		memcpy(quad.tri1[0].center, glm::value_ptr(pos), 3 * sizeof(float));
		memcpy(quad.tri1[1].center, glm::value_ptr(pos), 3 * sizeof(float));
		memcpy(quad.tri1[2].center, glm::value_ptr(pos), 3 * sizeof(float));

		memcpy(quad.tri1[0].color, glm::value_ptr(color), 3 * sizeof(float));
		memcpy(quad.tri1[1].color, glm::value_ptr(color), 3 * sizeof(float));
		memcpy(quad.tri1[2].color, glm::value_ptr(color), 3 * sizeof(float));

		memcpy(quad.tri1[0].tex, glm::value_ptr(t0), 2 * sizeof(float));
		memcpy(quad.tri1[1].tex, glm::value_ptr(t1), 2 * sizeof(float));
		memcpy(quad.tri1[2].tex, glm::value_ptr(t2), 2 * sizeof(float));

		// triangle 2
		memcpy(quad.tri2[0].pos, glm::value_ptr(p0), 3 * sizeof(float));
		memcpy(quad.tri2[1].pos, glm::value_ptr(p2), 3 * sizeof(float));
		memcpy(quad.tri2[2].pos, glm::value_ptr(p3), 3 * sizeof(float));

		memcpy(quad.tri2[0].center, glm::value_ptr(pos), 3 * sizeof(float));
		memcpy(quad.tri2[1].center, glm::value_ptr(pos), 3 * sizeof(float));
		memcpy(quad.tri2[2].center, glm::value_ptr(pos), 3 * sizeof(float));

		memcpy(quad.tri2[0].color, glm::value_ptr(color), 3 * sizeof(float));
		memcpy(quad.tri2[1].color, glm::value_ptr(color), 3 * sizeof(float));
		memcpy(quad.tri2[2].color, glm::value_ptr(color), 3 * sizeof(float));

		memcpy(quad.tri2[0].tex, glm::value_ptr(t0), 2 * sizeof(float));
		memcpy(quad.tri2[1].tex, glm::value_ptr(t2), 2 * sizeof(float));
		memcpy(quad.tri2[2].tex, glm::value_ptr(t3), 2 * sizeof(float));

		dx += charwidth;
	}

	text2DChanged = true;
}
//---------------------------------------------------------------------------

void __fastcall GLFont::AddText3D(glm::vec3 pos, float height, GLTextPos horiz_align, GLTextPos vert_align, const char* str, glm::vec3 color, bool point)
{
	/// Create textured triangles and add to list
	/// pos is world space, height is world scale
	/// centerx, centery are screen position (0 to 1)
	/// height is character size
	/// horiz_align, vert_align are position of text relative to center
	/// Can be LEFT, CENTER or RIGHT / ABOVE, CENTER or BELOW
	/// str is the text string - ASCII only
	/// color is the text color

	// scale converts font bmp pixel to world size
	float scalex = height / (float)fontHeight;

	float strsize = 0;
	for(int i=0; i<strlen(str); i++) {
		int c = (int)(str[i]);
		strsize += charWidth[c];
	}
	strsize *= scalex;

	float x = 0.0f;
	float y = 0.0f;

	if(horiz_align == GLTextPos::RIGHT) {
		if(point) x += 2 * pointSize;
	}
	if(horiz_align == GLTextPos::LEFT) {
		x -= strsize;
		if(point) x -= 2 * pointSize;
	}
	if(horiz_align == GLTextPos::CENTER) {
		x -= strsize / 2.0f;
	}
	if(vert_align == GLTextPos::ABOVE) {
		if(horiz_align == GLTextPos::CENTER && point) {
			y += 2 * pointSize;
		}
	}
	if(vert_align == GLTextPos::BELOW) {
		y -= height;
		if(horiz_align == GLTextPos::CENTER && point) {
			y -= 2 * pointSize;
		}
	}
	if(vert_align == GLTextPos::CENTER) {
		y -= height / 2.0f;
	}

	int char_per_line = imageWidth / cellWidth;
	float texh = (float)fontHeight / (float)imageHeight;

	for(int i=0; i<strlen(str); i++) {
		int c = (int)(str[i]);

		float charwidth = charWidth[c] * scalex;

		glm::vec3 p0(x, y, 0.0f);   					 // BL
		glm::vec3 p1(x + charwidth, y, 0.0f);    		 // BR
		glm::vec3 p2(x + charwidth, y + height, 0.0f);   // TR
		glm::vec3 p3(x, y + height, 0.0f); 				 // TL

		int pixleft = cellWidth * ((c  - startChar) % char_per_line);
		int pixtop = cellHeight * ((c  - startChar) / char_per_line);
		float texleft = (float)pixleft / (float)imageWidth;
		float textop = ((float)pixtop / (float)imageHeight);
		float texw = (float)charWidth[c] / (float)imageWidth;

		glm::vec2 t0(texleft, textop+texh);      // BL
		glm::vec2 t1(texleft+texw, textop+texh); // BR
		glm::vec2 t2(texleft+texw, textop);      // TR
		glm::vec2 t3(texleft, textop);           // TL

		GLBillboardQuad& quad = quad3DList.emplace_back();

		// triangle 1
		memcpy(quad.tri1[0].pos, glm::value_ptr(p0), 3 * sizeof(float));
		memcpy(quad.tri1[1].pos, glm::value_ptr(p1), 3 * sizeof(float));
		memcpy(quad.tri1[2].pos, glm::value_ptr(p2), 3 * sizeof(float));

		memcpy(quad.tri1[0].center, glm::value_ptr(pos), 3 * sizeof(float));
		memcpy(quad.tri1[1].center, glm::value_ptr(pos), 3 * sizeof(float));
		memcpy(quad.tri1[2].center, glm::value_ptr(pos), 3 * sizeof(float));

		memcpy(quad.tri1[0].color, glm::value_ptr(color), 3 * sizeof(float));
		memcpy(quad.tri1[1].color, glm::value_ptr(color), 3 * sizeof(float));
		memcpy(quad.tri1[2].color, glm::value_ptr(color), 3 * sizeof(float));

		memcpy(quad.tri1[0].tex, glm::value_ptr(t0), 2 * sizeof(float));
		memcpy(quad.tri1[1].tex, glm::value_ptr(t1), 2 * sizeof(float));
		memcpy(quad.tri1[2].tex, glm::value_ptr(t2), 2 * sizeof(float));

		// triangle 2
		memcpy(quad.tri2[0].pos, glm::value_ptr(p0), 3 * sizeof(float));
		memcpy(quad.tri2[1].pos, glm::value_ptr(p2), 3 * sizeof(float));
		memcpy(quad.tri2[2].pos, glm::value_ptr(p3), 3 * sizeof(float));

		memcpy(quad.tri2[0].center, glm::value_ptr(pos), 3 * sizeof(float));
		memcpy(quad.tri2[1].center, glm::value_ptr(pos), 3 * sizeof(float));
		memcpy(quad.tri2[2].center, glm::value_ptr(pos), 3 * sizeof(float));

		memcpy(quad.tri2[0].color, glm::value_ptr(color), 3 * sizeof(float));
		memcpy(quad.tri2[1].color, glm::value_ptr(color), 3 * sizeof(float));
		memcpy(quad.tri2[2].color, glm::value_ptr(color), 3 * sizeof(float));

		memcpy(quad.tri2[0].tex, glm::value_ptr(t0), 2 * sizeof(float));
		memcpy(quad.tri2[1].tex, glm::value_ptr(t2), 2 * sizeof(float));
		memcpy(quad.tri2[2].tex, glm::value_ptr(t3), 2 * sizeof(float));

		x += charwidth;
	}

	if(point) {

		glm::vec3 p0(-pointSize, -pointSize, 0.0f);   // BL
		glm::vec3 p1(+pointSize, -pointSize, 0.0f);   // BR
		glm::vec3 p2(+pointSize, +pointSize, 0.0f);   // TR
		glm::vec3 p3(-pointSize, +pointSize, 0.0f); 	// TL

		glm::vec2 t0(0.0f, 0.0f); // BL
		glm::vec2 t1(1.0f, 0.0f); // BR
		glm::vec2 t2(1.0f, 1.0f); // TR
		glm::vec2 t3(0.0f, 1.0f); // TL

		GLBillboardQuad& quad = pointList.emplace_back();

		// quad - 2 triangles
		memcpy(quad.tri1[0].pos, glm::value_ptr(p0), 3 * sizeof(float));
		memcpy(quad.tri1[1].pos, glm::value_ptr(p1), 3 * sizeof(float));
		memcpy(quad.tri1[2].pos, glm::value_ptr(p2), 3 * sizeof(float));

		memcpy(quad.tri1[0].center, glm::value_ptr(pos), 3 * sizeof(float));
		memcpy(quad.tri1[1].center, glm::value_ptr(pos), 3 * sizeof(float));
		memcpy(quad.tri1[2].center, glm::value_ptr(pos), 3 * sizeof(float));

		memcpy(quad.tri1[0].color, glm::value_ptr(color), 3 * sizeof(float));
		memcpy(quad.tri1[1].color, glm::value_ptr(color), 3 * sizeof(float));
		memcpy(quad.tri1[2].color, glm::value_ptr(color), 3 * sizeof(float));

		memcpy(quad.tri1[0].tex, glm::value_ptr(t0), 2 * sizeof(float));
		memcpy(quad.tri1[1].tex, glm::value_ptr(t1), 2 * sizeof(float));
		memcpy(quad.tri1[2].tex, glm::value_ptr(t2), 2 * sizeof(float));

		memcpy(quad.tri2[0].pos, glm::value_ptr(p0), 3 * sizeof(float));
		memcpy(quad.tri2[1].pos, glm::value_ptr(p2), 3 * sizeof(float));
		memcpy(quad.tri2[2].pos, glm::value_ptr(p3), 3 * sizeof(float));

		memcpy(quad.tri2[0].center, glm::value_ptr(pos), 3 * sizeof(float));
		memcpy(quad.tri2[1].center, glm::value_ptr(pos), 3 * sizeof(float));
		memcpy(quad.tri2[2].center, glm::value_ptr(pos), 3 * sizeof(float));

		memcpy(quad.tri2[0].color, glm::value_ptr(color), 3 * sizeof(float));
		memcpy(quad.tri2[1].color, glm::value_ptr(color), 3 * sizeof(float));
		memcpy(quad.tri2[2].color, glm::value_ptr(color), 3 * sizeof(float));

		memcpy(quad.tri2[0].tex, glm::value_ptr(t0), 2 * sizeof(float));
		memcpy(quad.tri2[1].tex, glm::value_ptr(t2), 2 * sizeof(float));
		memcpy(quad.tri2[2].tex, glm::value_ptr(t3), 2 * sizeof(float));

	}

	text3DChanged = true;
}
//---------------------------------------------------------------------------

void __fastcall GLFont::Create2DArrays()
{
	/// Set up vertex data and buffers and configure vertex attributes
	/// for screen text triangles

	if(VAO2D > 0) {
		glDeleteVertexArrays(1, &VAO2D);
		VAO2D = 0;
	}


	if(quad2DList.size() > 0) {

		glGenVertexArrays(1, &VAO2D);
		glBindVertexArray(VAO2D);

		unsigned int VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, quad2DList.size() * sizeof(GLBillboardQuad), quad2DList.data(), GL_STATIC_DRAW);

		// position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLBillboardVertex), (void*)0);
		glEnableVertexAttribArray(0);

		// center
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLBillboardVertex), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		// color
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GLBillboardVertex), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		// texture coord
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(GLBillboardVertex), (void*)(9 * sizeof(float)));
		glEnableVertexAttribArray(3);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &VBO);
	}
}
//---------------------------------------------------------------------------

void __fastcall GLFont::Create3DArrays()
{
	/// Set up vertex data and buffers and configure vertex attributes
	/// for world text and point triangles

	if(VAO3D > 0) {
		glDeleteVertexArrays(1, &VAO3D);
		VAO3D = 0;
	}

	if(quad3DList.size() > 0) {

		glGenVertexArrays(1, &VAO3D);
		glBindVertexArray(VAO3D);

		unsigned int VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, quad3DList.size() * sizeof(GLBillboardQuad), quad3DList.data(), GL_STATIC_DRAW);

		// position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLBillboardVertex), (void*)0);
		glEnableVertexAttribArray(0);

		// center
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLBillboardVertex), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		// color
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GLBillboardVertex), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		// texture coord
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(GLBillboardVertex), (void*)(9 * sizeof(float)));
		glEnableVertexAttribArray(3);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &VBO);
	}


	// point array
	if(VAOP > 0) {
		glDeleteVertexArrays(1, &VAOP);
		VAOP = 0;
	}
	if(VBOP > 0) {
		glDeleteVertexArrays(1, &VBOP);
		VBOP = 0;
	}

	if(pointList.size() > 0) {

		glGenVertexArrays(1, &VAOP);
		glBindVertexArray(VAOP);

		// point buffer is GL_DYNAMIC_DRAW to allow changes to color for highlighting
		glGenBuffers(1, &VBOP);
		glBindBuffer(GL_ARRAY_BUFFER, VBOP);
		glBufferData(GL_ARRAY_BUFFER, pointList.size() * sizeof(GLBillboardQuad), pointList.data(), GL_DYNAMIC_DRAW);


		// position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLBillboardVertex), (void*)0);
		glEnableVertexAttribArray(0);

		// center
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLBillboardVertex), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		// color
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GLBillboardVertex), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		// texture coord
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(GLBillboardVertex), (void*)(9 * sizeof(float)));
		glEnableVertexAttribArray(3);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}
//---------------------------------------------------------------------------

void __fastcall GLFont::Render2D(GLFWwindow* window)
{
	/// Render 2D text

	if(text2DChanged) {
		Create2DArrays();
		text2DChanged = false;
	}

	if(quad2DList.size() > 0) {

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// orthogonal projection
		const GLfloat left = 0.0f;
		const GLfloat right = 1.0f;
		const GLfloat bottom = 0.0f;
		const GLfloat top = 1.0f;
		const GLfloat zNear = -1.0f;
		const GLfloat zFar = 1.0f;

		const GLfloat p[16] = {
			(2.0f / (right - left)), 0.0f, 0.0f, 0.0f,
			0.0f, (2.0f / (top - bottom)), 0.0f, 0.0f,
			0.0f, 0.0f, (-2.0f / (zFar - zNear)), 0.0f,

			-((right + left) / (right - left)),
			-((top + bottom) / (top - bottom)),
			-((zFar + zNear) / (zFar - zNear)),
			1.0f,
		};

		glUseProgram(billboardShader);

		// projection matrix
		int pvm_loc = glGetUniformLocation(billboardShader, "pvm");
		glUniformMatrix4fv(pvm_loc, 1, GL_FALSE, p);

		// scale factor to correct for distortion from screen size
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		int scale_loc = glGetUniformLocation(billboardShader, "xscale");
		glUniform1f(scale_loc, (float)height/(float)width);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fontTexture.textureID);
		glBindVertexArray(VAO2D);

		glDrawArrays(GL_TRIANGLES, 0, quad2DList.size() * 6);
	}

}
//---------------------------------------------------------------------------

void __fastcall GLFont::Render3D(GLFWwindow* window, glm::mat4& pvm, bool depthtext)
{
	/// Render 3D text
	/// Uses projection matrix (pvm) from camera

	glDisable(GL_CULL_FACE);

	if(depthtext) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(text3DChanged) {
		Create3DArrays();
		text3DChanged = false;
	}

	if(quad3DList.size() > 0 || pointList.size() > 0) {

		glUseProgram(billboardShader);

		// projection matrix
		int pvm_loc = glGetUniformLocation(billboardShader, "pvm");
		glUniformMatrix4fv(pvm_loc, 1, GL_FALSE, glm::value_ptr(pvm));

		// scale factor to correct for distortion from screen size
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		int scale_loc = glGetUniformLocation(billboardShader, "xscale");
		glUniform1f(scale_loc, (float)height/(float)width);
	}

	if(quad3DList.size() > 0) {

		// draw text triangles
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fontTexture.textureID);
		glBindVertexArray(VAO3D);

		glDrawArrays(GL_TRIANGLES, 0, quad3DList.size() * 6);
	}

	if(pointList.size() > 0) {

        // draw dots
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pointTexture.textureID);
		glBindVertexArray(VAOP);

		glDrawArrays(GL_TRIANGLES, 0, pointList.size() * 6);
	}

}
//---------------------------------------------------------------------------

static bool RaySphere(glm::vec3& rayorigin, glm::vec3& raydir, glm::vec3& sphere, float radius, float& mindist)
{
	/// Find intersection between ray and sphere
	/// Returns true if intersection found less than mindist away

   float a, b, c;
   float bb4ac;

   a = raydir.x * raydir.x + raydir.y * raydir.y + raydir.z * raydir.z;
   b = 2 * (raydir.x * (rayorigin.x - sphere.x) + raydir.y * (rayorigin.y - sphere.y) + raydir.z * (rayorigin.z - sphere.z));
   c = sphere.x * sphere.x + sphere.y * sphere.y + sphere.z * sphere.z;
   c += rayorigin.x * rayorigin.x + rayorigin.y * rayorigin.y + rayorigin.z * rayorigin.z;
   c -= 2 * (sphere.x * rayorigin.x + sphere.y * rayorigin.y + sphere.z * rayorigin.z);
   c -= radius * radius;
   bb4ac = b * b - 4 * a * c;

   if (fabs(a) < 1e-20 || bb4ac < 0) {
	  return false;
   }

   float mu1 = (-b + sqrt(bb4ac)) / (2 * a);
   float mu2 = (-b - sqrt(bb4ac)) / (2 * a);

   float dist;
   if(mu1 < mu2) dist = mu1;
   else dist = mu2;

   if(dist < mindist) {
	   mindist = dist;
	   return true;
   }

   return false;
}
//---------------------------------------------------------------------------

int __fastcall GLFont::PickPoint(glm::vec3& raystart, glm::vec3& raydir, float& mindist)
{
	/// Finds point intersected by ray and closer than mindist

	int minp = -1;
	for(int p=0; p<pointList.size(); p++) {
		GLBillboardQuad& quad = pointList[p];
		glm::vec3 circle(quad.tri1[0].center[0], quad.tri1[0].center[1], quad.tri1[0].center[2]);
		if(RaySphere(raystart, raydir, circle, pointSize, mindist)) {
			minp = p;
		}
	}
	return minp;
}
//---------------------------------------------------------------------------

void __fastcall GLFont::SetPointColor(int point, glm::vec3& color)
{
	/// Sets the color of a point

	if(point < 0 || point >= pointList.size()) return;

	glBindVertexArray(VAOP);
	glBindBuffer(GL_ARRAY_BUFFER, VBOP);

	GLBillboardQuad& quad = pointList[point];
	memcpy(quad.tri1[0].color, glm::value_ptr(color), sizeof(glm::vec3));
	memcpy(quad.tri1[1].color, glm::value_ptr(color), sizeof(glm::vec3));
	memcpy(quad.tri1[2].color, glm::value_ptr(color), sizeof(glm::vec3));
	memcpy(quad.tri2[0].color, glm::value_ptr(color), sizeof(glm::vec3));
	memcpy(quad.tri2[1].color, glm::value_ptr(color), sizeof(glm::vec3));
	memcpy(quad.tri2[2].color, glm::value_ptr(color), sizeof(glm::vec3));
	glBufferSubData(GL_ARRAY_BUFFER, point * sizeof(GLBillboardQuad), sizeof(GLBillboardQuad), &quad);
}
//---------------------------------------------------------------------------

glm::vec3 __fastcall GLFont::GetPointColor(int point)
{
	/// Gets the color of a point

	if(point < 0 || point >= pointList.size()) return glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 color;
	GLBillboardQuad& quad = pointList[point];
	memcpy(glm::value_ptr(color), quad.tri1[0].color, sizeof(glm::vec3));
	return color;
}
//---------------------------------------------------------------------------

