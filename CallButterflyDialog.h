#ifndef CALLBUTTERFLYDIALOG_H
#define CALLBUTTERFLYDIALOG_H

//(*Headers(CallButterflyDialog)
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class CallButterflyDialog: public wxDialog
{
	public:

		CallButterflyDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~CallButterflyDialog();

		//(*Declarations(CallButterflyDialog)
		wxButton* Button1;
		wxListView* ListView2;
		wxStaticText* StaticText1;
		wxListView* ListView1;
		wxChoice* Choice1;
		//*)

	protected:

		//(*Identifiers(CallButterflyDialog)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_BUTTON1;
		static const long ID_LISTVIEW1;
		static const long ID_LISTVIEW2;
		//*)

	private:

		//(*Handlers(CallButterflyDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
