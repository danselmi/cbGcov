#include "CallButterflyDialog.h"

//(*InternalHeaders(CallButterflyDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(CallButterflyDialog)
const long CallButterflyDialog::ID_STATICTEXT1 = wxNewId();
const long CallButterflyDialog::ID_CHOICE1 = wxNewId();
const long CallButterflyDialog::ID_BUTTON1 = wxNewId();
const long CallButterflyDialog::ID_LISTVIEW1 = wxNewId();
const long CallButterflyDialog::ID_LISTVIEW2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(CallButterflyDialog,wxDialog)
	//(*EventTable(CallButterflyDialog)
	//*)
END_EVENT_TABLE()

CallButterflyDialog::CallButterflyDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(CallButterflyDialog)
	wxStaticBoxSizer* StaticBoxSizer2;
	wxBoxSizer* BoxSizer2;
	wxBoxSizer* BoxSizer1;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxBoxSizer* BoxSizer3;
	
	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Function:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	BoxSizer2->Add(StaticText1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Choice1 = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxSize(271,21), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice1->SetSelection( Choice1->Append(_("foo1")) );
	BoxSizer2->Add(Choice1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button1 = new wxButton(this, ID_BUTTON1, _("View"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	BoxSizer2->Add(Button1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer2, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Called from"));
	ListView1 = new wxListView(this, ID_LISTVIEW1, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_LISTVIEW1"));
	StaticBoxSizer1->Add(ListView1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer3->Add(StaticBoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Calls to"));
	ListView2 = new wxListView(this, ID_LISTVIEW2, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_LISTVIEW2"));
	StaticBoxSizer2->Add(ListView2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer3->Add(StaticBoxSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer3, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	//*)
}

CallButterflyDialog::~CallButterflyDialog()
{
	//(*Destroy(CallButterflyDialog)
	//*)
}

