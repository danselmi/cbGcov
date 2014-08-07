#ifndef CALLLISTDIALOG_H
#define CALLLISTDIALOG_H

//(*Headers(CallListDialog)
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class CallListDialog: public wxDialog
{
	public:

		CallListDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~CallListDialog();

		//(*Declarations(CallListDialog)
		wxStaticText* StaticText2;
		wxButton* Button1;
		wxStaticText* StaticText1;
		wxButton* Button2;
		wxListView* ListView1;
		wxChoice* Choice1;
		wxChoice* Choice2;
		//*)

	protected:

		//(*Identifiers(CallListDialog)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_BUTTON1;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE2;
		static const long ID_BUTTON2;
		static const long ID_LISTVIEW1;
		//*)

	private:

		//(*Handlers(CallListDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
