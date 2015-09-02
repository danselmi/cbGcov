#include "cbGcovSummaryPanel.h"

//(*InternalHeaders(cbGcovSummaryPanel)
#include <wx/listctrl.h>
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


cbGcovSummaryPanel::cbGcovSummaryPanel(wxWindow* parent, const Summaries &summaries, wxWindowID id)
{
	//(*Initialize(cbGcovSummaryPanel)
	wxBoxSizer* BoxSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	listCtrl = new wxListCtrl(this, ID_LISTCTRL, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
	BoxSizer1->Add(listCtrl, 1, wxALL|wxEXPAND, 5);
	SetSizer(BoxSizer1);
	//*)

    wxListItem col0;
    col0.SetId(0);
    col0.SetText( _("Filename") );
    col0.SetWidth(50);
    listCtrl->InsertColumn(0, col0);

    // Add second column
    wxListItem col1;
    col1.SetId(1);
    col1.SetText( _("Total lines") );
    listCtrl->InsertColumn(1, col1);

    // Add third column
    wxListItem col2;
    col2.SetId(2);
    col2.SetText( _("Executed lines") );
    listCtrl->InsertColumn(2, col2);

    // Add third column
    wxListItem col3;
    col2.SetId(3);
    col2.SetText( _("Coverage [%]") );
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
        listCtrl->SetItem(n, 3, wxString::Format(_T("%f"), 100.0*data.totalCodeLinesCalled/data.totalCodeLines));
    }
}

cbGcovSummaryPanel::~cbGcovSummaryPanel()
{
	//(*Destroy(cbGcovSummaryPanel)
	//*)
}


