#include "cbGcovSummaryPanel.h"
#include "editormanager.h"

//(*InternalHeaders(cbGcovSummaryPanel)
#include <wx/sizer.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(cbGcovSummaryPanel)
const long cbGcovSummaryPanel::ID_LISTCTRL = wxNewId();
//*)

BEGIN_EVENT_TABLE(cbGcovSummaryPanel,wxPanel)
	//(*EventTable(cbGcovSummaryPanel)
	//*)
END_EVENT_TABLE()

wxString cbGcovSummaryPanel::shortName_(_("cbGcov summary"));


cbGcovSummaryPanel::cbGcovSummaryPanel(wxWindow* parent, const Summaries &summaries):
    EditorBase(parent, _("cbGcov summary"))
{
	//(*Initialize(cbGcovSummaryPanel)
	wxBoxSizer* BoxSizer1;

	//Create(0, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	listCtrl = new wxListCtrl(this, ID_LISTCTRL, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_LISTCTRL"));
	BoxSizer1->Add(listCtrl, 1, wxALL|wxEXPAND, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);

	Connect(ID_LISTCTRL,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&cbGcovSummaryPanel::ItemActivated);
	//*)

    wxListItem col0;
    col0.SetId(0);
    col0.SetAlign(wxLIST_FORMAT_LEFT);
    col0.SetText( _("Filename") );
    col0.SetWidth(400);
    listCtrl->InsertColumn(0, col0);

    wxListItem col1;
    col1.SetId(1);
    col1.SetAlign(wxLIST_FORMAT_RIGHT);
    col1.SetText( _("Total lines") );
    col1.SetWidth(100);
    listCtrl->InsertColumn(1, col1);

    wxListItem col2;
    col2.SetId(2);
    col2.SetAlign(wxLIST_FORMAT_RIGHT);
    col2.SetText( _("Executed lines") );
    col2.SetWidth(100);
    listCtrl->InsertColumn(2, col2);

    wxListItem col3;
    col3.SetId(3);
    col3.SetAlign(wxLIST_FORMAT_RIGHT);
    col3.SetText( _("Coverage [\%]") );
    col3.SetWidth(100);
    listCtrl->InsertColumn(3, col3);

    for(size_t n = 0 ; n < summaries.size() ; ++n)
    {
        const gcovSummaryFileData data = summaries[n];
        wxListItem item;
        item.SetId(n);
        item.SetText( data.filename );
        listCtrl->InsertItem( item );

        listCtrl->SetItem(n, 0, data.filename);
        listCtrl->SetItem(n, 1, wxString::Format(_T("%d"), data.totalCodeLines));
        listCtrl->SetItem(n, 2, wxString::Format(_T("%d"), data.totalCodeLinesCalled));
        listCtrl->SetItem(n, 3, wxString::Format(_T("%.2f"), 100.0*data.totalCodeLinesCalled/data.totalCodeLines));
    }
}

cbGcovSummaryPanel::~cbGcovSummaryPanel()
{
	//(*Destroy(cbGcovSummaryPanel)
	//*)
}


const wxString& cbGcovSummaryPanel::GetFilename() const
{
    return shortName_;
}

const wxString& cbGcovSummaryPanel::GetShortName() const
{
    return shortName_;
}

const wxString& cbGcovSummaryPanel::GetTitle()
{
    return shortName_;
}

void cbGcovSummaryPanel::ItemActivated(wxListEvent& event)
{
    const wxListItem &item = event.GetItem();

    wxString filename(item.GetText());

    Manager::Get()->GetEditorManager()->Open(filename);
}
