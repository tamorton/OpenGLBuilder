//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "Main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TMainForm *MainForm;
//---------------------------------------------------------------------------

__fastcall TMainForm::TMainForm(TComponent* Owner)
	: TForm(Owner)
{
	openglWindow = nullptr;
	cameraDist = 5.0f;
	cameraElev = 0.0f;
	cameraRot = 0.0f;
	cameraLookat = glm::vec3(0.0f, 0.0f, 0.0f);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
	if(openglWindow != nullptr) {
		delete openglWindow;
		openglWindow = nullptr;
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::WindowButtonClick(TObject *Sender)
{
	/// Create OpenGL window
	/// Display opengl version on screen

	if(openglWindow != nullptr) return;

	openglWindow = new TOpenGLWindow(640, 480, 4, "OpenGL Window");

	openglWindow->OnKeyEvent = KeyHandler;
	openglWindow->OnMouseButtonEvent = MouseButtonHandler;
	openglWindow->OnMousePositionEvent = MousePositionHandler;
	openglWindow->OnMouseScrollEvent = MouseScrollHandler;

	openglWindow->SetAmbientColor(AmbientColorPanel->Color);
	openglWindow->SetLightColor(LightColorPanel->Color);

	openglWindow->AddText2D(0.02f, 0.98f, 0.08f, GLTextPos::RIGHT, GLTextPos::BELOW, "OpenGL", glm::vec3(1.0f, 1.0f, 1.0f));
	openglWindow->AddText2D(0.02f, 0.93f, 0.08f, GLTextPos::RIGHT, GLTextPos::BELOW, "Version:", glm::vec3(1.0f, 1.0f, 1.0f));
	openglWindow->AddText2D(0.15f, 0.93f, 0.08f, GLTextPos::RIGHT, GLTextPos::BELOW, openglWindow->GetVersion(), glm::vec3(1.0f, 1.0f, 1.0f));
	openglWindow->AddText2D(0.02f, 0.88f, 0.08f, GLTextPos::RIGHT, GLTextPos::BELOW, "Vendor:", glm::vec3(1.0f, 1.0f, 1.0f));
	openglWindow->AddText2D(0.15f, 0.88f, 0.08, GLTextPos::RIGHT, GLTextPos::BELOW, openglWindow->GetVendor(), glm::vec3(1.0f, 1.0f, 1.0f));
	openglWindow->AddText2D(0.02f, 0.83f, 0.08f, GLTextPos::RIGHT, GLTextPos::BELOW, "Renderer:", glm::vec3(1.0f, 1.0f, 1.0f));
	openglWindow->AddText2D(0.15f, 0.83f, 0.08f, GLTextPos::RIGHT, GLTextPos::BELOW, openglWindow->GetRenderer(), glm::vec3(1.0f, 1.0f, 1.0f));
	openglWindow->AddText2D(0.02f, 0.78f, 0.08f, GLTextPos::RIGHT, GLTextPos::BELOW, "Shader:", glm::vec3(1.0f, 1.0f, 1.0f));
	openglWindow->AddText2D(0.15f, 0.78f, 0.08f, GLTextPos::RIGHT, GLTextPos::BELOW, openglWindow->GetShaderVersion(), glm::vec3(1.0f, 1.0f, 1.0f));

	Timer->Enabled = true;;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::TriangleButtonClick(TObject *Sender)
{
	/// Shows a single triangle with colored vertices

	if(openglWindow == nullptr) {
		ShowMessage("First create window");
		return;
	}

	openglWindow->ClearText2D();
	openglWindow->ClearTextures();
	openglWindow->ClearData();
	openglWindow->BackFaceCull(false);

	glm::vec3 p1(0.0f, 0.0f, 0.0f);
	glm::vec3 p2(0.8f, 0.0f, 0.0f);
	glm::vec3 p3(0.4f, 0.8f, 0.0f);

	glm::vec3 c1(1.0f, 0.0f, 0.0f);
	glm::vec3 c2(0.0f, 1.0f, 0.0f);
	glm::vec3 c3(0.0f, 0.0f, 1.0f);

	openglWindow->AddTriangleVC(p1, p2, p3, c1, c2, c3);

	cameraDist = 5.0f;
	cameraElev = 0.0f;
	cameraRot = 0.0f;
	cameraLookat = glm::vec3(0.4f, 0.4f, 0.0f);
	glm::vec3 camerapos(0.0f, 0.0f, cameraDist);
	camerapos = camerapos + cameraLookat;
	glm::vec3 cameraup = glm::vec3(0.0f, 1.0f, 0.0f);
	openglWindow->SetCamera(camerapos, cameraLookat, cameraup);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::TextureButtonClick(TObject *Sender)
{
	/// Choose bitmap for texture of cube

	OpenDialog->Title = "Choose Bitmap";
	if(OpenDialog->Execute()) {
		TextureImage->Bitmap->LoadFromFile(OpenDialog->FileName);
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::CubeButtonClick(TObject *Sender)
{
	/// Show colored cube

	if(openglWindow == nullptr) {
		ShowMessage("First create window");
		return;
	}

	openglWindow->ClearText2D();
	openglWindow->ClearTextures();
	openglWindow->ClearData();
	openglWindow->BackFaceCull(true);

	glm::vec3 p1(0.0f, 0.0f, 0.0f);
	glm::vec3 p2(0.0f, 0.0f, 1.0f);
	glm::vec3 p3(1.0f, 0.0f, 1.0f);
	glm::vec3 p4(1.0f, 0.0f, 0.0f);
	glm::vec3 p5(0.0f, 1.0f, 0.0f);
	glm::vec3 p6(0.0f, 1.0f, 1.0f);
	glm::vec3 p7(1.0f, 1.0f, 1.0f);
	glm::vec3 p8(1.0f, 1.0f, 0.0f);

	glm::vec3 red(1.0f, 0.0f, 0.0f);
	glm::vec3 green(0.0f, 1.0f, 0.0f);
	glm::vec3 blue(0.0f, 0.0f, 1.0f);

	// top  y=1
	openglWindow->AddTriangleVC(p7, p8, p5, green, green, green);
	openglWindow->AddTriangleVC(p7, p5, p6, green, green, green);

	// bottom  y=-1
	openglWindow->AddTriangleVC(p2, p1, p4, green, green, green);
	openglWindow->AddTriangleVC(p2, p4, p3, green, green, green);

	// front  z=1
	openglWindow->AddTriangleVC(p2, p3, p7, blue, blue, blue);
	openglWindow->AddTriangleVC(p2, p7, p6, blue, blue, blue);

	// back z=-1
	openglWindow->AddTriangleVC(p4, p1, p5, blue, blue, blue);
	openglWindow->AddTriangleVC(p4, p5, p8, blue, blue, blue);

	// left x=-1
	openglWindow->AddTriangleVC(p1, p2, p6, red, red, red);
	openglWindow->AddTriangleVC(p1, p6, p5, red, red, red);

	// right x=1
	openglWindow->AddTriangleVC(p3, p4, p8, red, red, red);
	openglWindow->AddTriangleVC(p3, p8, p7, red, red, red);

	cameraDist = 5.0f;
	cameraElev = 0.0f;
	cameraRot = 0.0f;
	cameraLookat = glm::vec3(0.5f, 0.5f, 0.5f);
	glm::vec3 camerapos(0.0f, 0.0f, cameraDist);
	camerapos = camerapos + cameraLookat;
	glm::vec3 cameraup = glm::vec3(0.0f, 1.0f, 0.0f);
	openglWindow->SetCamera(camerapos, cameraLookat, cameraup);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::CrateButtonClick(TObject *Sender)
{
	/// Show textured cube

	if(openglWindow == nullptr) {
		ShowMessage("First create window");
		return;
	}

	openglWindow->ClearText2D();
	openglWindow->ClearTextures();
	openglWindow->ClearData();
	openglWindow->BackFaceCull(true);

	int texid = openglWindow->AddTexture(TextureImage->Bitmap, false);

	glm::vec3 p1(0.0f, 0.0f, 0.0f);
	glm::vec3 p2(0.0f, 0.0f, 1.0f);
	glm::vec3 p3(1.0f, 0.0f, 1.0f);
	glm::vec3 p4(1.0f, 0.0f, 0.0f);
	glm::vec3 p5(0.0f, 1.0f, 0.0f);
	glm::vec3 p6(0.0f, 1.0f, 1.0f);
	glm::vec3 p7(1.0f, 1.0f, 1.0f);
	glm::vec3 p8(1.0f, 1.0f, 0.0f);

	glm::vec2 t0(0.0f, 0.0f);
	glm::vec2 t1(0.0f, 1.0f);
	glm::vec2 t2(1.0f, 0.0f);
	glm::vec2 t3(1.0f, 1.0f);

	// top  y=1
	openglWindow->AddTriangleVT(p7, p8, p5, t0, t1, t3, texid);
	openglWindow->AddTriangleVT(p7, p5, p6, t0, t3, t2, texid);

	// bottom  y=-1
	openglWindow->AddTriangleVT(p2, p1, p4, t0, t1, t3, texid);
	openglWindow->AddTriangleVT(p2, p4, p3, t0, t3, t2, texid);

	// front  z=1
	openglWindow->AddTriangleVT(p2, p3, p7, t0, t1, t3, texid);
	openglWindow->AddTriangleVT(p2, p7, p6, t0, t3, t2, texid);

	// back z=-1
	openglWindow->AddTriangleVT(p4, p1, p5, t0, t1, t3, texid);
	openglWindow->AddTriangleVT(p4, p5, p8, t0, t3, t2, texid);

	// left x=-1
	openglWindow->AddTriangleVT(p1, p2, p6, t0, t1, t3, texid);
	openglWindow->AddTriangleVT(p1, p6, p5, t0, t3, t2, texid);

	// right x=1
	openglWindow->AddTriangleVT(p3, p4, p8, t0, t1, t3, texid);
	openglWindow->AddTriangleVT(p3, p8, p7, t0, t3, t2, texid);

	cameraDist = 5.0f;
	cameraElev = 0.0f;
	cameraRot = 0.0f;
	cameraLookat = glm::vec3(0.5f, 0.5f, 0.5f);
	glm::vec3 camerapos(0.0f, 0.0f, cameraDist);
	camerapos = camerapos + cameraLookat;
	glm::vec3 cameraup = glm::vec3(0.0f, 1.0f, 0.0f);
	openglWindow->SetCamera(camerapos, cameraLookat, cameraup);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Text2DButtonClick(TObject *Sender)
{
	/// Add 2D text to screen
	/// Text coordinates are 0..1

	if(openglWindow == nullptr) {
		ShowMessage("First create window");
		return;
	}

	openglWindow->ClearText2D();

	openglWindow->AddText2D(0.5f, 0.5f, 0.1f, GLTextPos::CENTER, GLTextPos::CENTER, "Hello World", glm::vec3(1.0f, 1.0f, 1.0f));
	openglWindow->AddText2D(0.0f, 0.0f, 0.1f, GLTextPos::RIGHT, GLTextPos::ABOVE, "Bottom Left", glm::vec3(1.0f, 0.0f, 0.0f));
	openglWindow->AddText2D(1.0f, 0.0f, 0.1f, GLTextPos::LEFT, GLTextPos::ABOVE, "Bottom Right", glm::vec3(0.0f, 1.0f, 0.0f));
	openglWindow->AddText2D(0.0f, 1.0f, 0.1f, GLTextPos::RIGHT, GLTextPos::BELOW, "Top Left", glm::vec3(0.0f, 0.0f, 1.0f));
	openglWindow->AddText2D(1.0f, 1.0f, 0.1f, GLTextPos::LEFT, GLTextPos::BELOW, "Top Right", glm::vec3(1.0f, 0.0f, 1.0f));
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Text3DButtonClick(TObject *Sender)
{
	/// Add 3D text to corners of a cube
    /// Initially the text is not z tested

	if(openglWindow == nullptr) {
		ShowMessage("First create window");
		return;
	}

	openglWindow->ClearText2D();
	openglWindow->ClearText3D();

	glm::vec3 p1(0.0f, 0.0f, 0.0f);
	glm::vec3 p2(0.0f, 0.0f, 1.0f);
	glm::vec3 p3(1.0f, 0.0f, 1.0f);
	glm::vec3 p4(1.0f, 0.0f, 0.0f);
	glm::vec3 p5(0.0f, 1.0f, 0.0f);
	glm::vec3 p6(0.0f, 1.0f, 1.0f);
	glm::vec3 p7(1.0f, 1.0f, 1.0f);
	glm::vec3 p8(1.0f, 1.0f, 0.0f);

	glm::vec3 center = (p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8) / 8.0f;

	// corner labels
	openglWindow->AddText3D(center, 0.2f, GLTextPos::CENTER, GLTextPos::ABOVE, "Center", glm::vec3(0.0f, 1.0f, 0.0f), true);
	openglWindow->AddText3D(p1, 0.2f, GLTextPos::RIGHT, GLTextPos::CENTER, "1", glm::vec3(1.0f, 0.0f, 0.0f), true);
	openglWindow->AddText3D(p2, 0.2f, GLTextPos::RIGHT, GLTextPos::CENTER, "2", glm::vec3(1.0f, 1.0f, 1.0f), true);
	openglWindow->AddText3D(p3, 0.2f, GLTextPos::RIGHT, GLTextPos::CENTER, "3", glm::vec3(1.0f, 1.0f, 1.0f), true);
	openglWindow->AddText3D(p4, 0.2f, GLTextPos::RIGHT, GLTextPos::CENTER, "4", glm::vec3(1.0f, 1.0f, 1.0f), true);
	openglWindow->AddText3D(p5, 0.2f, GLTextPos::RIGHT, GLTextPos::CENTER, "5", glm::vec3(1.0f, 1.0f, 1.0f), true);
	openglWindow->AddText3D(p6, 0.2f, GLTextPos::RIGHT, GLTextPos::CENTER, "6", glm::vec3(1.0f, 1.0f, 1.0f), true);
	openglWindow->AddText3D(p7, 0.2f, GLTextPos::RIGHT, GLTextPos::CENTER, "7", glm::vec3(1.0f, 1.0f, 1.0f), true);
	openglWindow->AddText3D(p8, 0.2f, GLTextPos::RIGHT, GLTextPos::CENTER, "8", glm::vec3(1.0f, 1.0f, 1.0f), true);

	cameraDist = 5.0f;
	cameraElev = 0.0f;
	cameraRot = 0.0f;
	cameraLookat = glm::vec3(0.5f, 0.5f, 0.5f);
	glm::vec3 camerapos(0.0f, 0.0f, cameraDist);
	camerapos = camerapos + cameraLookat;
	glm::vec3 cameraup = glm::vec3(0.0f, 1.0f, 0.0f);
	openglWindow->SetCamera(camerapos, cameraLookat, cameraup);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ModelButtonClick(TObject *Sender)
{
	/// Load model from wavefront obj file

	OpenDialog->Title = "Choose Obj";
	if(OpenDialog->Execute()) {
		openglWindow->ClearText2D();
		AnsiString str = OpenDialog->FileName;
		openglWindow->AddModel(str.c_str());
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::KeyHandler(TOpenGLWindow* Sender, int key, int scancode, int action, int mods)
{
	/// Called from GLFW window on key press
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MouseButtonHandler(TOpenGLWindow* Sender, int button, int action, int mods)
{
	/// Called from GLFW window when mouse button pressed or released
	/// button: GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_RIGHT or GLFW_MOUSE_BUTTON_MIDDLE
	/// action: GLFW_PRESS or GLFW_RELEASE
	/// mods: GLFW_MOD_SHIFT, GLFW_MOD_CONTROL, GLFW_MOD_ALT, GLFW_MOD_SUPER, GLFW_MOD_CAPS_LOCK or GLFW_MOD_NUM_LOCK

	if(action == GLFW_PRESS) {
		mouseDown = true;
		openglWindow->GetMousePos(mouseX, mouseY);
	}
	else {
		mouseDown = false;
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MousePositionHandler(TOpenGLWindow* Sender, double x, double y)
{
	/// Called from GLFW window when mouse moved
    /// Rotates camera or highlights element

	double mousespeed = 0.01;

	if(mouseDown) {
		// rotate camera
		cameraRot += (float)(-mousespeed * (x - mouseX));
		cameraElev += (float)(-mousespeed * (y - mouseY));
		if(cameraElev > M_PI / 4.0) cameraElev = M_PI / 4.0;
		if(cameraElev < -M_PI / 4.0) cameraElev = -M_PI / 4.0;

		glm::vec3 camerapos = glm::vec3(0.0f, 0.0f, cameraDist);
		camerapos = glm::rotate(camerapos, cameraElev, glm::vec3(1.0f, 0.0f, 0.0f));
		camerapos = glm::rotate(camerapos, cameraRot, glm::vec3(0.0f, 1.0f, 0.0f));

		glm::vec3 dir = glm::normalize(camerapos);
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 right = glm::normalize(glm::cross(up, dir));
		up = glm::cross(dir, right);

		camerapos += cameraLookat;

		openglWindow->SetCamera(camerapos, cameraLookat, up);

		mouseX = x;
		mouseY = y;
	}
	else {
		if(PickCheck->IsChecked) {
			// highlight
			GLPickResult pick = openglWindow->PickElement(x, y);
			if(pick != highlightPick) {
				openglWindow->SetElementColor(highlightPick, highlightPick.color);
				if(pick.type == GLPickType::COLOR) {
					glm::vec3 newcolor = glm::normalize(pick.color + glm::vec3(0.5f, 0.5f, 1.0f));
					openglWindow->SetElementColor(pick, newcolor);
				}
				else {
					openglWindow->SetElementColor(pick, glm::vec3(0.2f, 0.2f, 0.5f));
				}
				highlightPick = pick;
			}
        }
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MouseScrollHandler(TOpenGLWindow* Sender, double delta)
{
	/// Called from GLFW window when mouse wheel scrolled
	/// Zooms camera

	double scrollspeed = 0.5;

    cameraDist += scrollspeed * delta;

	glm::vec3 camerapos = glm::vec3(0.0f, 0.0f, cameraDist);
	camerapos = glm::rotate(camerapos, cameraElev, glm::vec3(1.0f, 0.0f, 0.0f));
	camerapos = glm::rotate(camerapos, cameraRot, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::vec3 dir = glm::normalize(camerapos);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::normalize(glm::cross(up, dir));
	up = glm::cross(dir, right);

	camerapos += cameraLookat;

	openglWindow->SetCamera(camerapos, cameraLookat, up);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::TimerTimer(TObject *Sender)
{
	/// Redraw OpenGL window every 20ms
	/// OpenGL window does not have a redraw loop
	/// Need to call Render to update display

	if(openglWindow == nullptr) return;
	openglWindow->Render();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::DepthTextCheckChange(TObject *Sender)
{
	/// Use depth buffer for 3D text

	if(openglWindow == nullptr) return;
	openglWindow->DepthText(DepthTextCheck->IsChecked);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::AmbientColorPanelChange(TObject *Sender)
{
	/// Change ambient color

	if(openglWindow == nullptr) return;
	openglWindow->ClearText2D();
	AnsiString str;
	str.sprintf("Ambient color: %08X", AmbientColorPanel->Color);
	openglWindow->AddText2D(0.05f, 0.95f, 0.1f, GLTextPos::RIGHT, GLTextPos::BELOW, str.c_str(), glm::vec3(1.0f, 1.0f, 1.0f));
	openglWindow->SetAmbientColor(AmbientColorPanel->Color);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::LightColorPanelChange(TObject *Sender)
{
	/// Change light color

	if(openglWindow == nullptr) return;
	openglWindow->ClearText2D();
	AnsiString str;
	str.sprintf("Light color: %08X", LightColorPanel->Color);
	openglWindow->AddText2D(0.05f, 0.95f, 0.1f, GLTextPos::RIGHT, GLTextPos::BELOW, str.c_str(), glm::vec3(1.0f, 1.0f, 1.0f));
	openglWindow->SetLightColor(LightColorPanel->Color);
}
//---------------------------------------------------------------------------

