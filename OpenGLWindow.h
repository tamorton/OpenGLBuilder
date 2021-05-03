//---------------------------------------------------------------------------

#ifndef OpenGLWindowH
#define OpenGLWindowH
//---------------------------------------------------------------------------
#include <System.hpp>
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>

#include <vector>
#include <string>
#include <functional>
#include <map>

#include "glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "glm/gtx/normal.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include "tinyobjloader/tiny_obj_loader.h"

class TOpenGLWindow;
class GLFont;
typedef void __fastcall (__closure *TGLKeyEvent)(TOpenGLWindow* Sender, int key, int scancode, int action, int mods);
typedef void __fastcall (__closure *TGLResizeEvent)(TOpenGLWindow* Sender, int width, int height);
typedef void __fastcall (__closure *TGLMouseButtonEvent)(TOpenGLWindow* Sender, int button, int action, int mods);
typedef void __fastcall (__closure *TGLMousePositionEvent)(TOpenGLWindow* Sender, double x, double y);
typedef void __fastcall (__closure *TGLMouseScrollEvent)(TOpenGLWindow* Sender, double delta);

enum class GLTextPos { LEFT, CENTER, RIGHT, ABOVE, BELOW };

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Arrays of these strucures are directly copied to video buffer
// So don't add extra elements

struct GLColorVertex {
	float pos[3];
	float norm[3];
	float color[3];
};
//---------------------------------------------------------------------------

struct GLTextureVertex {
	float pos[3];
	float norm[3];
	float color[3];
	float tex[2];
};
//---------------------------------------------------------------------------

struct GLBillboardVertex {
	float pos[3];
	float center[3];
	float color[3];
	float tex[2];
};
//---------------------------------------------------------------------------

struct GLPointVertex {
	float pos[3];
	float color[3];
};
//---------------------------------------------------------------------------

struct GLColorTriangle
{
	GLColorVertex vert[3];
};
//---------------------------------------------------------------------------

struct GLTextureTriangle
{
	GLTextureVertex vert[3];
};
//---------------------------------------------------------------------------

struct GLBillboardQuad
{
	GLBillboardVertex tri1[3];
	GLBillboardVertex tri2[3];
};
//---------------------------------------------------------------------------

enum class GLPickType { NONE, COLOR, TRIANGLE, POINT };
struct GLPickResult
{
	GLPickType type;
	int group;
	int index;
	float dist;
    glm::vec3 color;

	GLPickResult() { type = GLPickType::NONE; }
	bool operator ==(const GLPickResult &other) const { return compare(other); }
	bool operator !=(const GLPickResult &other) const { return !compare(other); }
	bool __fastcall compare(const GLPickResult &other) const {
		if(type == GLPickType::NONE) {
			return (other.type == GLPickType::NONE);
		}
		else if(type == GLPickType::COLOR) {
			if(other.type != GLPickType::COLOR) return false;
			if(index != other.index) return false;
			return true;
		}
		else if(type == GLPickType::TRIANGLE) {
			if(other.type != GLPickType::TRIANGLE) return false;
			if(group != other.group || index != other.index) return false;
			return true;
		}
		else if(type == GLPickType::POINT) {
			if(other.type != GLPickType::POINT) return false;
			if(index != other.index) return false;
			return true;
		}
		return false;
	}
};
//---------------------------------------------------------------------------
 //---------------------------------------------------------------------------

class GLTexture
{
public:
	GLTexture();
	~GLTexture();
	void __fastcall LoadTextureFromFile(const std::wstring& file, bool flip);
	void __fastcall LoadTextureFromBitmap(TBitmap* textureBMP, bool flip);
	void __fastcall LoadTextureFromResource(const wchar_t* bmpresource, bool flip);
	void __fastcall ClearTriangles() { triangleList.clear(); }
	void __fastcall Render();
	void __fastcall AddTriangleVT(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, glm::vec2& t1, glm::vec2& t2, glm::vec2& t3);
	void __fastcall AddTriangleVNT(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, glm::vec3& n1, glm::vec3& n2, glm::vec3& n3, glm::vec2& t1, glm::vec2& t2, glm::vec2& t3);
	int  __fastcall PickTriangle(glm::vec3& raystart, glm::vec3& raydir, float& mindist);
	void __fastcall SetTriangleColor(int tri, glm::vec3& color);
    glm::vec3 __fastcall GetTriangleColor(int trinum);

	GLuint textureID;

private:
	std::wstring filename;
	std::vector<GLTextureTriangle> triangleList;
	unsigned int VAO;
	unsigned int VBO;
	bool changed;

   	void __fastcall CreateArrays();

};
//---------------------------------------------------------------------------

class GLFont
{
private:
	GLTexture fontTexture;
	GLTexture pointTexture;
	TOpenGLWindow* glWindow;
	std::map<std::string, int> props;
	int charWidth[256];
	int imageWidth;
	int imageHeight;
	int cellWidth;
	int cellHeight;
	int startChar;
	int fontHeight;
    float pointSize;
	bool text2DChanged;
	bool text3DChanged;

	unsigned int VAO2D;
	unsigned int VAO3D;
	unsigned int VAOP;
	unsigned int VBOP;
	unsigned int unlitShader;
	unsigned int billboardShader;

	std::vector<GLBillboardQuad> quad2DList;
	std::vector<GLBillboardQuad> quad3DList;
	std::vector<GLBillboardQuad> pointList;

	void __fastcall Create2DArrays();
	void __fastcall Create3DArrays();

public:
	GLFont(wchar_t* bmpresource, wchar_t* dataresource);
    ~GLFont();
	void __fastcall AddText2D(float centerx, float centery, float height, GLTextPos horiz_align, GLTextPos vert_align, const char* str, glm::vec3 color);
	void __fastcall AddText3D(glm::vec3 pos, float height, GLTextPos horiz_align, GLTextPos vert_align, const char* str, glm::vec3 color, bool point);
	void __fastcall ClearText2D() { quad2DList.clear(); text2DChanged = true; }
	void __fastcall ClearText3D() { quad3DList.clear(); pointList.clear(); text3DChanged = true; }
	void __fastcall Render2D(GLFWwindow* window);
	void __fastcall Render3D(GLFWwindow* window, glm::mat4& pvm, bool depthtext);
	int  __fastcall PickPoint(glm::vec3& raystart, glm::vec3& raydir, float& dist);
	void __fastcall SetPointColor(int point, glm::vec3& color);
    glm::vec3 __fastcall GetPointColor(int point);
};
//---------------------------------------------------------------------------

class TOpenGLWindow
{
public:
	TOpenGLWindow(int width, int height, int samples, const char* title);
	~TOpenGLWindow();

	void __fastcall ClearTextures();
	void __fastcall ClearData();
	void __fastcall ClearText2D();
	void __fastcall ClearText3D();
	int  __fastcall AddTexture(const std::wstring& file, bool flip);
	int  __fastcall AddTexture(TBitmap* textureBMP, bool flip);
	void __fastcall AddTriangleVC(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, glm::vec3& c1, glm::vec3& c2, glm::vec3& c3);
	void __fastcall AddTriangleVNC(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, glm::vec3& n1, glm::vec3& n2, glm::vec3& n3, glm::vec3& c1, glm::vec3& c2, glm::vec3& c3);
	void __fastcall AddTriangleVT(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, glm::vec2& t1, glm::vec2& t2, glm::vec2& t3, int texid);
	void __fastcall AddTriangleVNT(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, glm::vec3& n1, glm::vec3& n2, glm::vec3& n3, glm::vec2& t1, glm::vec2& t2, glm::vec2& t3, int texid);
	void __fastcall AddText2D(float x, float y, float scale, GLTextPos xpos, GLTextPos ypos, const char* str, glm::vec3 color);
	void __fastcall AddText3D(glm::vec3 pos, float scale, GLTextPos xpos, GLTextPos ypos, const char* str, glm::vec3 color, bool point = false);
	void __fastcall AddModel(const char* filename);
	void __fastcall Render();

	void __fastcall SetLightDir(glm::vec3& dir);
	void __fastcall BackFaceCull(bool docull) { backFaceCull = docull; };
	void __fastcall SetCamera(glm::vec3& pos, glm::vec3& lookat, glm::vec3& up);
	void __fastcall DepthText(bool dt) { depthText = dt; }
	void __fastcall SetAmbientColor(TAlphaColor color);
	void __fastcall SetLightColor(TAlphaColor color);
	glm::vec3 __fastcall CreateRay(double x, double y);

	const char* __fastcall GetVersion() { if(window == nullptr) return ""; return (char*)glGetString(GL_VERSION); }
	const char* __fastcall GetVendor(){ if(window == nullptr) return ""; return (char*)glGetString(GL_VENDOR); }
	const char* __fastcall GetRenderer(){ if(window == nullptr) return ""; return (char*)glGetString(GL_RENDERER); }
	const char* __fastcall GetShaderVersion(){ if(window == nullptr) return ""; return (char*)glGetString(GL_SHADING_LANGUAGE_VERSION); }

	void __fastcall GetMousePos(double& x, double& y);

	void __fastcall KeyCallback(int key, int scancode, int action, int mods);
	void __fastcall ResizeCallback(int width, int height);
	void __fastcall MouseButtonCallback(int button, int action, int mods);
	void __fastcall MousePositionCallback(double x, double y);
	void __fastcall MouseScrollCallback(double xoffset, double yoffset);
	GLPickResult __fastcall PickElement(double x, double y);
	void __fastcall SetElementColor(GLPickResult& pick, glm::vec3 newcolor);
	void __fastcall SetColorTriangleColor(int tri, glm::vec3& color);
    glm::vec3 __fastcall GetColorTriangleColor(int trinum);

	TGLKeyEvent OnKeyEvent;
	TGLResizeEvent OnResizeEvent;
	TGLMouseButtonEvent OnMouseButtonEvent;
	TGLMousePositionEvent OnMousePositionEvent;
	TGLMouseScrollEvent OnMouseScrollEvent;

private:
	GLFWwindow* window;

	glm::vec3 cameraPos;
	glm::vec3 cameraLookat;
	glm::vec3 cameraUp;
	glm::vec3 lightDir;
	glm::vec3 ambientColor;
	glm::vec3 lightColor;
	float cameraFOV;
	float cameraNear;
    float cameraFar;
	bool backFaceCull;
	bool depthText;
	int highlightPoint;
	int highlightTexture;
	int highlightTriangle;

	unsigned int colorShader;
	unsigned int textureShader;
	unsigned int vertexArray;
	unsigned int colorVAO;
	unsigned int colorVBO;

    GLFont* defaultFont;

	bool dataChanged;

	std::vector<GLTexture> textureList;
	std::vector<GLColorTriangle> colorList;

	void __fastcall CreateWindow(int width, int height, int samples, const char* title);
	void __fastcall CreateColorArrays();
	void __fastcall LoadFont();
};
//---------------------------------------------------------------------------

#endif
