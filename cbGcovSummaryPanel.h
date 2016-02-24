#ifndef CBGCOVSUMMARYPANEL_H
#define CBGCOVSUMMARYPANEL_H

#include <vector>
#include <wx/listctrl.h>

#include "editorbase.h"

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

class cbGcovSummaryPanel: public EditorBase
{
	public:

		cbGcovSummaryPanel(wxWindow* parent, const Summaries &summaries);
		virtual ~cbGcovSummaryPanel();

		//(*Declarations(cbGcovSummaryPanel)
		wxListCtrl* listCtrl;
		//*)

        virtual const wxString& GetFilename() const;

		virtual const wxString& GetShortName() const;
        virtual const wxString& GetTitle();


        virtual bool VisibleToTree() const { return false; }/// not visible in open files list

	protected:

		//(*Identifiers(cbGcovSummaryPanel)
		static const long ID_LISTCTRL;
		//*)

    private:
        static wxString shortName_;
        void OnColClick( wxListEvent& event );
        void InitGrid(bool doSort = false, bool sortAscending = true);
        const Summaries summaries_;


		//(*Handlers(cbGcovSummaryPanel)
		void ItemActivated(wxListEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
