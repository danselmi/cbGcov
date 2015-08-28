#ifndef CBGCOVCONFIGPANEL_H
#define CBGCOVCONFIGPANEL_H

#include <configurationpanel.h>

//(*Headers(cbGcovConfigPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxBoxSizer;
//*)
class cbGcov;

class cbGcovConfigPanel: public cbConfigurationPanel
{
	public:

		cbGcovConfigPanel(wxWindow* parent, cbGcov *plugin, wxWindowID id=wxID_ANY);
		virtual ~cbGcovConfigPanel();

		//(*Declarations(cbGcovConfigPanel)
		wxCheckBox* FunctionSummariesCheckBox;
		wxCheckBox* BranchProbabilitiesCheckBox;
		wxCheckBox* DemangleNamesCheckBox;
		//*)

        /// @return the panel's title.
        virtual wxString GetTitle() const;
        /// @return the panel's bitmap base name. You must supply two bitmaps: \<basename\>.png and \<basename\>-off.png...
        virtual wxString GetBitmapBaseName() const;
        /// Called when the user chooses to apply the configuration.
        virtual void OnApply();
        /// Called when the user chooses to cancel the configuration.
        virtual void OnCancel();
	protected:

		//(*Identifiers(cbGcovConfigPanel)
		static const long ID_CHECKBOX_DEMANGLE;
		static const long ID_CHECKBOX_FUNCTIONSUMMARIES;
		static const long ID_CHECKBOX_BRANCHPROBABILITIES;
		//*)

	private:
        cbGcov *plugin_;
		//(*Handlers(cbGcovConfigPanel)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
