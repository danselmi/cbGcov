#include "cbGcovSummaryEdPanel.h"
#include "cbGcovSummaryPanel.h"

#include <wx/sizer.h>

wxString cbGcovSummaryEdPanel::shortName_(_("cbGcov summary"));

cbGcovSummaryEdPanel::cbGcovSummaryEdPanel(wxWindow* parent, const Summaries &summaries, const wxString& filename):
    EditorBase(parent, _("cbGcov summary"))
{
    //ctor
//    wxBoxSizer* BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
//	BoxSizer1->Add(, 1, wxALL|wxEXPAND, 5);

    wxBoxSizer* BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	BoxSizer1->Add(new cbGcovSummaryPanel(this, summaries, wxID_ANY), 1, wxALL|wxEXPAND, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
}

cbGcovSummaryEdPanel::~cbGcovSummaryEdPanel()
{
    //dtor
}
const wxString& cbGcovSummaryEdPanel::GetFilename() const
{
    return shortName_;
}

const wxString& cbGcovSummaryEdPanel::GetShortName() const
{
    return shortName_;
}

const wxString& cbGcovSummaryEdPanel::GetTitle()
{
    return shortName_;
}
