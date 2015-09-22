#include "cbGcovConfigPanel.h"

//(*InternalHeaders(cbGcovConfigPanel)
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(cbGcovConfigPanel)
const long cbGcovConfigPanel::ID_CHECKBOX_DEMANGLE = wxNewId();
const long cbGcovConfigPanel::ID_CHECKBOX_FUNCTIONSUMMARIES = wxNewId();
const long cbGcovConfigPanel::ID_CHECKBOX_BRANCHPROBABILITIES = wxNewId();
//*)

#include "cbGcov.h"
#include "configmanager.h"
BEGIN_EVENT_TABLE(cbGcovConfigPanel, cbConfigurationPanel)
	//(*EventTable(cbGcovConfigPanel)
	//*)
END_EVENT_TABLE()

cbGcovConfigPanel::cbGcovConfigPanel(wxWindow* parent, cbGcov *plugin, wxWindowID id):
    plugin_(plugin)
{
	//(*Initialize(cbGcovConfigPanel)
	wxBoxSizer* BoxSizer2;
	wxBoxSizer* BoxSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer2 = new wxBoxSizer(wxVERTICAL);
	DemangleNamesCheckBox = new wxCheckBox(this, ID_CHECKBOX_DEMANGLE, _("Display demangled function names"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_DEMANGLE"));
	DemangleNamesCheckBox->SetValue(false);
	DemangleNamesCheckBox->SetToolTip(_("Display demangled function names in output."));
	BoxSizer2->Add(DemangleNamesCheckBox, 1, wxALL, 5);
	FunctionSummariesCheckBox = new wxCheckBox(this, ID_CHECKBOX_FUNCTIONSUMMARIES, _("Create funcion summaries"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_FUNCTIONSUMMARIES"));
	FunctionSummariesCheckBox->SetValue(false);
	FunctionSummariesCheckBox->SetToolTip(_("Output summaries for each function in addition to the file level summary."));
	BoxSizer2->Add(FunctionSummariesCheckBox, 1, wxALL, 5);
	BranchProbabilitiesCheckBox = new wxCheckBox(this, ID_CHECKBOX_BRANCHPROBABILITIES, _("Create branch frequencies"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_BRANCHPROBABILITIES"));
	BranchProbabilitiesCheckBox->SetValue(false);
	BranchProbabilitiesCheckBox->SetToolTip(_("Write branch frequencies to the output file, and write branch summary info to the standard output"));
	BoxSizer2->Add(BranchProbabilitiesCheckBox, 1, wxALL, 5);
	BoxSizer1->Add(BoxSizer2, 0, wxALL|wxALIGN_LEFT, 5);
	BoxSizer1->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	//*)

    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("cbGcov"));
    if(cfg)
    {
        DemangleNamesCheckBox->SetValue(cfg->ReadBool(_T("/DemangleNames"), true));
        FunctionSummariesCheckBox->SetValue(cfg->ReadBool(_T("/OutputFunctionSummaries"), true));
        BranchProbabilitiesCheckBox->SetValue(cfg->ReadBool(_T("/BranchProbabilities"), true));
    }
}

cbGcovConfigPanel::~cbGcovConfigPanel()
{
	//(*Destroy(cbGcovConfigPanel)
	//*)
}

void cbGcovConfigPanel::OnApply()
{
    // save any changes
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("cbGcov"));
    if(cfg)
    {
        cfg->Write(_T("/DemangleNames"), DemangleNamesCheckBox->GetValue());
        cfg->Write(_T("/OutputFunctionSummaries"), FunctionSummariesCheckBox->GetValue());
        cfg->Write(_T("/BranchProbabilities"), BranchProbabilitiesCheckBox->GetValue());
        plugin_->UpdateConfig();
    }
}

void cbGcovConfigPanel::OnCancel()
{
}

wxString cbGcovConfigPanel::GetTitle() const
{
    return _T("cbGcov");
}
wxString cbGcovConfigPanel::GetBitmapBaseName() const
{
    return _T("cbGvov");
}

