#ifndef CBGCOVSUMMARYPANEL_H
#define CBGCOVSUMMARYPANEL_H

#include <vector>

//(*Headers(cbGcovSummaryPanel)
#include <wx/panel.h>
class wxListCtrl;
class wxBoxSizer;
//*)

struct gcovSummaryFileData
{
    wxString filename;
    int totalCodeLines;
    int totalCodeLinesCalled;
    bool hasLines;
    int totalBranches;
    int totalBranchesConditionEvaluated;
    int totalBranchesTaken;
    bool hasBranches;
    bool hasBranchesTaken;
    int totalCalls;
    int totalCallsExecuted;
    bool hasCalls;
    gcovSummaryFileData():
        hasLines(false),
        hasBranches(false),
        hasBranchesTaken(false),
        hasCalls(false){}
};
typedef std::vector<gcovSummaryFileData> Summaries;

class cbGcovSummaryPanel: public wxPanel
{
	public:

		cbGcovSummaryPanel(wxWindow* parent, const Summaries &summaries, wxWindowID id=wxID_ANY);
		virtual ~cbGcovSummaryPanel();

		//(*Declarations(cbGcovSummaryPanel)
		wxListCtrl* listCtrl;
		//*)


	protected:

		//(*Identifiers(cbGcovSummaryPanel)
		static const long ID_LISTCTRL;
		//*)

	private:

		//(*Handlers(cbGcovSummaryPanel)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
