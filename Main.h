//---------------------------------------------------------------------------

#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Edit.hpp>
#include <FMX.Objects.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>
#include <FMX.Dialogs.hpp>

#include "OpenGLWindow.h"
#include <FMX.Colors.hpp>
//---------------------------------------------------------------------------

class TMainForm : public TForm
{
__published:	// IDE-managed Components
	TButton *WindowButton;
	TButton *TriangleButton;
	TButton *TextureButton;
	TButton *CubeButton;
	TImage *TextureImage;
	TOpenDialog *OpenDialog;
	TButton *Text2DButton;
	TTimer *Timer;
	TButton *Text3DButton;
	TCheckBox *DepthTextCheck;
	TColorPanel *AmbientColorPanel;
	TLabel *AmbientLabel;
	TLabel *LightLabel;
	TColorPanel *LightColorPanel;
	TButton *ModelButton;
	TButton *CrateButton;
	TCheckBox *PickCheck;
	void __fastcall TextureButtonClick(TObject *Sender);
	void __fastcall WindowButtonClick(TObject *Sender);
	void __fastcall TriangleButtonClick(TObject *Sender);
	void __fastcall CubeButtonClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall Text2DButtonClick(TObject *Sender);
	void __fastcall TimerTimer(TObject *Sender);
	void __fastcall Text3DButtonClick(TObject *Sender);
	void __fastcall DepthTextCheckChange(TObject *Sender);
	void __fastcall AmbientColorPanelChange(TObject *Sender);
	void __fastcall LightColorPanelChange(TObject *Sender);
	void __fastcall ModelButtonClick(TObject *Sender);
	void __fastcall CrateButtonClick(TObject *Sender);

private:	// User declarations
	TOpenGLWindow* openglWindow;
	bool mouseDown;
	double mouseX, mouseY;
	float cameraDist;
	float cameraElev;
	float cameraRot;
	glm::vec3 cameraLookat;
    GLPickResult highlightPick;

	void __fastcall KeyHandler(TOpenGLWindow* Sender, int key, int scancode, int action, int mods);
	void __fastcall MouseButtonHandler(TOpenGLWindow* Sender, int button, int action, int mods);
	void __fastcall MousePositionHandler(TOpenGLWindow* Sender, double x, double y);
	void __fastcall MouseScrollHandler(TOpenGLWindow* Sender, double delta);

public:		// User declarations
	__fastcall TMainForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
