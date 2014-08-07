#include "CallListDialog.h"

//(*InternalHeaders(CallListDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(CallListDialog)
const long CallListDialog::ID_STATICTEXT1 = wxNewId();
const long CallListDialog::ID_CHOICE1 = wxNewId();
const long CallListDialog::ID_BUTTON1 = wxNewId();
const long CallListDialog::ID_STATICTEXT2 = wxNewId();
const long CallListDialog::ID_CHOICE2 = wxNewId();
const long CallListDialog::ID_BUTTON2 = wxNewId();
const long CallListDialog::ID_LISTVIEW1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(CallListDialog,wxDialog)
	//(*EventTable(CallListDialog)
	//*)
END_EVENT_TABLE()

CallListDialog::CallListDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(CallListDialog)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	
	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("From Function:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice1 = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice1->Append(_("foo1"));
	Choice1->SetSelection( Choice1->Append(_("foo2")) );
	FlexGridSizer1->Add(Choice1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button1 = new wxButton(this, ID_BUTTON1, _("View..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer1->Add(Button1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("to Function:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice2 = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	Choice2->SetSelection( Choice2->Append(_("All Functions")) );
	Choice2->Append(_("foo1"));
	Choice2->Append(_("foo2"));
	FlexGridSizer1->Add(Choice2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button2 = new wxButton(this, ID_BUTTON2, _("View..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer1->Add(Button2, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(FlexGridSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ListView1 = new wxListView(this, ID_LISTVIEW1, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_LISTVIEW1"));
	BoxSizer1->Add(ListView1, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	//*)
}

CallListDialog::~CallListDialog()
{
	//(*Destroy(CallListDialog)
	//*)
}

