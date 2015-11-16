#include "cbGcovSummaryPanel.h"
#include "editormanager.h"

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

wxString cbGcovSummaryPanel::shortName_(_("cbGcov summary"));


cbGcovSummaryPanel::cbGcovSummaryPanel(wxWindow* parent, const Summaries &summaries):
    EditorBase(parent, _("cbGcov summary"))
{
	//(*Initialize(cbGcovSummaryPanel)
	wxBoxSizer* BoxSizer1;

	//Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	listCtrl = new wxListCtrl(this, ID_LISTCTRL, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_LISTCTRL"));
	BoxSizer1->Add(listCtrl, 1, wxALL|wxEXPAND, 5);
	SetSizer(BoxSizer1);

	Connect(ID_LISTCTRL,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&cbGcovSummaryPanel::ItemActivated);
	//*)
	Layout();

    wxListItem col0;
    col0.SetId(0);
    col0.SetAlign(wxLIST_FORMAT_LEFT);
    col0.SetText( _("Filename") );
    col0.SetWidth(400);
    listCtrl->InsertColumn(0, col0);

    wxListItem col1;
    col1.SetId(1);
    col1.SetAlign(wxLIST_FORMAT_RIGHT);
    col1.SetText( _("Executed lines") );
    col1.SetWidth(100);
    listCtrl->InsertColumn(1, col1);

    wxListItem col2;
    col2.SetId(2);
    col2.SetAlign(wxLIST_FORMAT_RIGHT);
    col2.SetText( _("Line Coverage [%]") );
    col2.SetWidth(100);
    listCtrl->InsertColumn(2, col2);

    wxListItem col3;
    col3.SetId(3);
    col3.SetAlign(wxLIST_FORMAT_RIGHT);
    col3.SetText( _("branches evaluated") );
    col3.SetWidth(100);
    listCtrl->InsertColumn(3, col3);

    wxListItem col4;
    col4.SetId(4);
    col4.SetAlign(wxLIST_FORMAT_RIGHT);
    col4.SetText( _("branches taken") );
    col4.SetWidth(100);
    listCtrl->InsertColumn(4, col4);

    wxListItem col5;
    col5.SetId(5);
    col5.SetAlign(wxLIST_FORMAT_RIGHT);
    col5.SetText( _("Branch Coverage [%]") );
    col5.SetWidth(100);
    listCtrl->InsertColumn(5, col5);

    wxListItem col6;
    col6.SetId(6);
    col6.SetAlign(wxLIST_FORMAT_RIGHT);
    col6.SetText( _("Total calls") );
    col6.SetWidth(100);
    listCtrl->InsertColumn(6, col6);

    wxListItem col7;
    col7.SetId(7);
    col7.SetAlign(wxLIST_FORMAT_RIGHT);
    col7.SetText( _("Coverage [%]") );
    col7.SetWidth(100);
    listCtrl->InsertColumn(7, col7);

    for(size_t n = 0 ; n < summaries.size() ; ++n)
    {
        const gcovSummaryFileData data = summaries[n];
        wxListItem item;
        item.SetId(n);
        item.SetText( data.filename );
        listCtrl->InsertItem( item );

        listCtrl->SetItem(n, 0, data.filename);
        if(data.hasLines)
        {
            listCtrl->SetItem(n, 1, wxString::Format(_T("%d / %d"), data.totalCodeLinesCalled, data.totalCodeLines));

            float cov = 100.0*data.totalCodeLinesCalled/data.totalCodeLines;
            long id = listCtrl->SetItem(n, 2, wxString::Format(_T("%.2f"), cov));
            //listCtrl->SetItemBackgroundColour(2, *wxRED);
//            wxListItem item;
//            item.SetId(n);
//            item.SetColumn(2);
//            item.SetMask(wxLIST_MASK_TEXT);
//            item.SetText(wxString::Format(_T("%.2f"), cov));
//            if(cov < 50)
//                item.SetBackgroundColour(*wxRED);
//            else if(cov < 80)
//                item.SetBackgroundColour(wxColour(0xff,0xff,0x00));
//            else
//                item.SetBackgroundColour(*wxGREEN);
//            listCtrl->SetItem(item);
        }
        if(data.hasBranches)
        {
            listCtrl->SetItem(n, 3, wxString::Format(_T("%d / %d"), data.totalBranchesConditionEvaluated, data.totalBranches));
            if(data.hasBranchesTaken)
            {
                listCtrl->SetItem(n, 4, wxString::Format(_T("%d"), data.totalBranchesTaken, data.totalBranches));
            }
            listCtrl->SetItem(n, 5, wxString::Format(_T("%.2f"), 100.0*data.totalBranchesConditionEvaluated/data.totalBranches));
        }
        if(data.hasCalls)
        {
            listCtrl->SetItem(n, 6, wxString::Format(_T("%d/ %d"), data.totalCallsExecuted, data.totalCalls));
            listCtrl->SetItem(n, 7, wxString::Format(_T("%.2f"), 100.0*data.totalCallsExecuted/data.totalCalls));
        }
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
