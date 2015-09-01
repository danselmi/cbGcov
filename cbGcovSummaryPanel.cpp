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
    long col = 0;
    listCtrl->InsertColumn(col++, wxString(_("Filename")));
    //listCtrl->InsertColumn(col++, wxString(_("Function")));
    listCtrl->InsertColumn(col++, wxString(_("Total lines")));
    //listCtrl->InsertColumn(col++, wxString(_("Instrumented lines")));
    listCtrl->InsertColumn(col++, wxString(_("Executed lines")));
    listCtrl->InsertColumn(col++, wxString(_("Coverage [%&&]")));

    Summaries::const_iterator it;
    long idx = 1;

    listCtrl->SetItem(idx, 0, _T("total"));
    listCtrl->SetItem(idx, 1, wxString::Format(_T("%d"), 42));
    listCtrl->SetItem(idx, 2, wxString::Format(_T("%d"), 21));
    listCtrl->SetItem(idx, 3, wxString::Format(_T("%f"), 100.0*21/42));
    idx++;

    for(it = summaries.begin() ; it != summaries.end() ; ++it)
    {
        gcovSummaryFileData data = *it;
        listCtrl->SetItem(idx, 0, data.filename);
        listCtrl->SetItem(idx, 1, wxString::Format(_T("%d"), data.totalCodeLines));
        listCtrl->SetItem(idx, 2, wxString::Format(_T("%d"), data.totalCodeLinesCalled));
        listCtrl->SetItem(idx, 3, wxString::Format(_T("%f"), 100.0*data.totalCodeLinesCalled/data.totalCodeLines));
        idx++;
    }
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
}

cbGcovSummaryPanel::~cbGcovSummaryPanel()
{
	//(*Destroy(cbGcovSummaryPanel)
	//*)
}


