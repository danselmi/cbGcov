#ifndef SUMMARYDIALOG_H
#define SUMMARYDIALOG_H

//(*Headers(SummaryDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/radiobut.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class SummaryDialog: public wxDialog
{
	public:

		SummaryDialog(wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~SummaryDialog();

		//(*Declarations(SummaryDialog)
		wxStaticText* StaticText10;
		wxStaticText* StaticText9;
		wxRadioButton* RadioButton1;
		wxPanel* Panel5;
		wxSpinCtrl* SpinCtrl1;
		wxStaticText* StaticText13;
		wxStaticText* StaticText2;
		wxPanel* Panel4;
		wxStaticText* StaticText14;
		wxChoice* Choice3;
		wxRadioButton* RadioButton2;
		wxButton* Button1;
		wxStaticText* StaticText6;
		wxRadioButton* RadioButton4;
		wxStaticText* StaticText8;
		wxStaticText* StaticText11;
		wxPanel* Panel1;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxButton* Button2;
		wxRadioButton* RadioButton3;
		wxPanel* Panel3;
		wxButton* Button3;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxSpinCtrl* SpinCtrl2;
		wxStaticText* StaticText15;
		wxStaticText* StaticText12;
		wxPanel* Panel2;
		wxStaticText* StaticText4;
		wxChoice* Choice1;
		wxStaticText* StaticText16;
		wxChoice* Choice2;
		//*)

	protected:

		//(*Identifiers(SummaryDialog)
		static const long ID_RADIOBUTTON1;
		static const long ID_RADIOBUTTON2;
		static const long ID_CHOICE2;
		static const long ID_BUTTON1;
		static const long ID_RADIOBUTTON3;
		static const long ID_CHOICE1;
		static const long ID_BUTTON2;
		static const long ID_RADIOBUTTON4;
		static const long ID_CHOICE3;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL2;
		static const long ID_BUTTON3;
		static const long ID_STATICTEXT2;
		static const long ID_STATICTEXT3;
		static const long ID_STATICTEXT4;
		static const long ID_PANEL1;
		static const long ID_STATICTEXT5;
		static const long ID_STATICTEXT6;
		static const long ID_STATICTEXT7;
		static const long ID_PANEL2;
		static const long ID_STATICTEXT8;
		static const long ID_STATICTEXT9;
		static const long ID_STATICTEXT10;
		static const long ID_PANEL3;
		static const long ID_STATICTEXT11;
		static const long ID_STATICTEXT12;
		static const long ID_STATICTEXT13;
		static const long ID_PANEL4;
		static const long ID_STATICTEXT16;
		static const long ID_STATICTEXT15;
		static const long ID_STATICTEXT14;
		static const long ID_PANEL5;
		//*)

	private:

		//(*Handlers(SummaryDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
