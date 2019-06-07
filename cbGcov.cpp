#include <sdk.h> // Code::Blocks SDK
#include <configurationpanel.h>
#include <cbeditor.h>
#include <cbstyledtextctrl.h>
#include <logmanager.h>
#include <projectmanager.h>
#include <cbproject.h>
#include <editormanager.h>
#include <configmanager.h>
#include <cbcolourmanager.h>

#include <wx/xrc/xmlres.h>
#include <wx/ffile.h>
#include <wx/tokenzr.h>
#include <wx/process.h>
#include <wx/app.h>
#include <wx/timer.h>
#include <wx/datetime.h>
#include <wx/filefn.h>
#include <wx/cmdline.h>

#include <vector>

#include "cbGcov.h"
#include "GcovProcess.h"
#include "cbGcovConfigPanel.h"

#include "cbGcovSummaryPanel.h"

// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
    PluginRegistrant<cbGcov> reg(_T("cbGcov"));

    const int idDoShowGcov                  = wxNewId();
    const int idDoGcov                      = wxNewId();
    const int idDoGcovAnalyze               = wxNewId();
    const int idAddInstrumentationToProject = wxNewId();

    const int idGotoNextNotExecutedLine     = XRCID("CB_GCOV_ID_NEXT_LINE_NOT_EXECUTED");
    const int idGotoPrevNotExecutedLine     = XRCID("CB_GCOV_ID_PREV_LINE_NOT_EXECUTED");
    const int idGotoNextExecutedLine        = XRCID("CB_GCOV_ID_NEXT_LINE_EXECUTED");
    const int idGotoPrevExecutedLine        = XRCID("CB_GCOV_ID_PREV_LINE_EXECUTED");
}

const int cbGcovNaMarker = 10;
const int cbGcovOkMarker = 11;
const int cbGcovKoMarker = 12;

const int cbGcovMarginNumber = 4;

const int cbGcovNaStyle = 80;
const int cbGcovOkStyle = 81;
const int cbGcovKoStyle = 82;

struct cbGcovConfig
{
    bool demangleNames;
    bool outputFunctionSummaries;
    bool branchProbabilities;
};

// events handling
BEGIN_EVENT_TABLE(cbGcov, cbPlugin)
    // add any events you want to handle here
END_EVENT_TABLE()

/**
 * @brief Default constructor. Checks for resource file, initializes internal state.
 */
cbGcov::cbGcov()
//:m_pProcess(NULL)
//,JodDone(false)
{
    config_ = new cbGcovConfig;
    // Make sure our resources are available.
    // In the generated boilerplate code we have no resources but when
    // we add some, it will be nice that this code is in place already ;)
    if(!Manager::LoadResource(_T("cbGcov.zip")))
    {
        NotifyMissingFile(_T("cbGcov.zip"));
    }
    Initialize();
}

/**
 * @brief Empty default destructor.
 */
cbGcov::~cbGcov()
{
}

/**
 * @brief OnAttach handler. Registers plugin event handlers.
 */
void cbGcov::OnAttach()
{
    // do whatever initialization you need for your plugin
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be TRUE...
    // You should check for it in other functions, because if it
    // is FALSE, it means that the application did *not* "load"
    // (see: does not need) this plugin...

    ColourManager* cm = Manager::Get()->GetColourManager();
    if(cm)
    {
        cm->RegisterColour(_T("cbGcov"), _("no executable code"), _T("cbgcov_not_executable"), *wxBLACK );
        cm->RegisterColour(_T("cbGcov"), _("never executed"), _T("cbgcov_not_executed"), *wxRED );
        cm->RegisterColour(_T("cbGcov"), _("executed at least once"), _T("cbgcov_executed"), *wxGREEN );
    }
    UpdateConfig();

    Connect(idAddInstrumentationToProject, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(cbGcov::OnAddInstrumentationToProject));
    Connect(idAddInstrumentationToProject, wxEVT_UPDATE_UI,             wxUpdateUIEventHandler(cbGcov::OnUpdateAddInstrumentationToProject));
    Connect(idDoGcov,                      wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(cbGcov::OnDoGcov));
    Connect(idDoGcov,                      wxEVT_UPDATE_UI,             wxUpdateUIEventHandler(cbGcov::OnUpdateGcov));
    Connect(idDoGcovAnalyze,               wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(cbGcov::OnDoGcovWorkspace));
    Connect(idDoGcovAnalyze,               wxEVT_UPDATE_UI,             wxUpdateUIEventHandler(cbGcov::OnUpdateGcovWorkspace));

    Connect(wxEVT_END_PROCESS,   wxProcessEventHandler(cbGcov::OnGcovReturned), NULL, this);
    Connect(wxEVT_IDLE,          wxIdleEventHandler(cbGcov::OnIdle), NULL, this);

    Connect(idGotoNextNotExecutedLine, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(cbGcov::OnGotoNextNotExecutedLine));
    Connect(idGotoPrevNotExecutedLine, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(cbGcov::OnGotoPreviousNotExecutedLine));
    Connect(idGotoNextExecutedLine,    wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(cbGcov::OnGotoNextExecutedLine));
    Connect(idGotoPrevExecutedLine,    wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(cbGcov::OnGotoPreviousExecutedLine));
    Connect(idGotoNextNotExecutedLine, wxEVT_UPDATE_UI,             wxUpdateUIEventHandler(cbGcov::OnUpdateNextNotExecutedLine));
    Connect(idGotoPrevNotExecutedLine, wxEVT_UPDATE_UI,             wxUpdateUIEventHandler(cbGcov::OnUpdatePreviousNotExecutedLine));
    Connect(idGotoNextExecutedLine,    wxEVT_UPDATE_UI,             wxUpdateUIEventHandler(cbGcov::OnUpdateNextExecutedLine));
    Connect(idGotoPrevExecutedLine,    wxEVT_UPDATE_UI,             wxUpdateUIEventHandler(cbGcov::OnUpdatePreviousExecutedLine));

    Manager::Get()->RegisterEventSink(cbEVT_EDITOR_OPEN, new cbEventFunctor<cbGcov, CodeBlocksEvent>(this, &cbGcov::OnEditorOpen));
    Manager::Get()->RegisterEventSink(cbEVT_EDITOR_CLOSE, new cbEventFunctor<cbGcov, CodeBlocksEvent>(this, &cbGcov::OnEditorClose));
    Manager::Get()->RegisterEventSink(cbEVT_EDITOR_MODIFIED, new cbEventFunctor<cbGcov, CodeBlocksEvent>(this, &cbGcov::OnEditorModified));
    Manager::Get()->RegisterEventSink(cbEVT_CLEAN_PROJECT_STARTED, new cbEventFunctor<cbGcov, CodeBlocksEvent>(this, &cbGcov::OnCleanProject));
    Manager::Get()->RegisterEventSink(cbEVT_CLEAN_WORKSPACE_STARTED, new cbEventFunctor<cbGcov, CodeBlocksEvent>(this, &cbGcov::OnCleanWorkspace));
    Manager::Get()->RegisterEventSink(cbEVT_PROJECT_ACTIVATE, new cbEventFunctor<cbGcov, CodeBlocksEvent>(this, &cbGcov::OnProjectActivate));

}

/**
 * @brief OnRelease handler. Unregisters plugin handlers.
 */
void cbGcov::OnRelease(bool appShutDown)
{
    // do de-initialization for your plugin
    // if appShutDown is true, the plugin is unloaded because Code::Blocks is being shut down,
    // which means you must not use any of the SDK Managers
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be FALSE...


    Disconnect(idAddInstrumentationToProject, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(cbGcov::OnAddInstrumentationToProject));
    Disconnect(idAddInstrumentationToProject, wxEVT_UPDATE_UI,             wxUpdateUIEventHandler(cbGcov::OnUpdateAddInstrumentationToProject));
    Disconnect(idDoGcov,                      wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(cbGcov::OnDoGcov));
    Disconnect(idDoGcov,                      wxEVT_UPDATE_UI,             wxUpdateUIEventHandler(cbGcov::OnUpdateGcov));
    Disconnect(idDoGcovAnalyze,               wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(cbGcov::OnDoGcovWorkspace));
    Disconnect(idDoGcovAnalyze,               wxEVT_UPDATE_UI,             wxUpdateUIEventHandler(cbGcov::OnUpdateGcovWorkspace));

    Disconnect(wxEVT_END_PROCESS,   wxProcessEventHandler(cbGcov::OnGcovReturned), NULL, this);
    Disconnect(wxEVT_IDLE,          wxIdleEventHandler(cbGcov::OnIdle), NULL, this);

    Disconnect(idGotoNextNotExecutedLine, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(cbGcov::OnGotoNextNotExecutedLine));
    Disconnect(idGotoPrevNotExecutedLine, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(cbGcov::OnGotoPreviousNotExecutedLine));
    Disconnect(idGotoNextExecutedLine,    wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(cbGcov::OnGotoNextExecutedLine));
    Disconnect(idGotoPrevExecutedLine,    wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(cbGcov::OnGotoPreviousExecutedLine));
    Disconnect(idGotoNextNotExecutedLine, wxEVT_UPDATE_UI,             wxUpdateUIEventHandler(cbGcov::OnUpdateNextNotExecutedLine));
    Disconnect(idGotoPrevNotExecutedLine, wxEVT_UPDATE_UI,             wxUpdateUIEventHandler(cbGcov::OnUpdatePreviousNotExecutedLine));
    Disconnect(idGotoNextExecutedLine,    wxEVT_UPDATE_UI,             wxUpdateUIEventHandler(cbGcov::OnUpdateNextExecutedLine));
    Disconnect(idGotoPrevExecutedLine,    wxEVT_UPDATE_UI,             wxUpdateUIEventHandler(cbGcov::OnUpdatePreviousExecutedLine));
}

/**
 * @brief Not in use.
 */
int cbGcov::Configure()
{
    //create and display the configuration dialog for your plugin
    cbConfigurationDialog dlg(Manager::Get()->GetAppWindow(), wxID_ANY, _("cbGcov configuration"));
    cbConfigurationPanel* panel = GetConfigurationPanel(&dlg);
    if(panel)
    {
        dlg.AttachConfigurationPanel(panel);
        PlaceWindow(&dlg);
        return dlg.ShowModal() == wxID_OK ? 0 : -1;
    }
    return -1;
}

/**
 * @brief Adds menu items for plugin.
 */
void cbGcov::BuildMenu(wxMenuBar* menuBar)
{
    int projectmenupos = menuBar->FindMenu(_("Project"));
    if(projectmenupos != wxNOT_FOUND)
    {
        wxMenu *prjmenu = menuBar->GetMenu(projectmenupos);
        prjmenu->AppendSeparator();
        //prjmenu->Append(idDoShowGcov, _T("Show Coverage Data"));
        prjmenu->Append(idDoGcov, _("Run Gcov on project"));
        prjmenu->Append(idDoGcovAnalyze, _("Run Gcov on workspace"));
        prjmenu->Append(idAddInstrumentationToProject, _("Add Gcov instrumentation"), _("Add compiler/linker option to add gcov coverage instrumentation to project."));
    }

    int editMenuPos = menuBar->FindMenu(_("Edit"));
    if(editMenuPos != wxNOT_FOUND)
    {
        wxMenu *editMenu = menuBar->GetMenu(editMenuPos);
        wxMenu *gcovMenu = new wxMenu();
        gcovMenu->Append(idGotoNextNotExecutedLine,  _("next not executed line"), _("goto next not executed line"));
        gcovMenu->Append(idGotoPrevNotExecutedLine,  _("previous not executed line"), _("goto previous not executed line"));
        gcovMenu->Append(idGotoNextExecutedLine,     _("next executed line"), _("goto next executed line"));
        gcovMenu->Append(idGotoPrevExecutedLine,     _("previous executed line"), _("goto previous executed line"));
        editMenu->Append(wxID_ANY, _("cbGcov"), gcovMenu);
    }
}

/**
 * @brief
 */
void cbGcov::BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data)
{
    //Some library module is ready to display a pop-up menu.
    //Check the parameter \"type\" and see which module it is
    //and append any items you need in the menu...
    //TIP: for consistency, add a separator as the first item...
    NotImplemented(_T("cbGcov::BuildModuleMenu()"));
}

/**
 * @brief
 */
bool cbGcov::BuildToolBar(wxToolBar* toolBar)
{
    //The application is offering its toolbar for your plugin,
    //to add any toolbar items you want...
    //Append any items you need on the toolbar...
    m_pTbar = toolBar;
    if (!IsAttached() || !toolBar)
        return false;

    Manager::Get()->AddonToolBar(m_pTbar, _T("cb_gcov_toolbar"));
    m_pTbar->Realize();
    m_pTbar->SetInitialSize();

    return true;
}

cbConfigurationPanel* cbGcov::GetConfigurationPanel(wxWindow* parent)
{
    return new cbGcovConfigPanel(parent, this);
}



/**
 * @brief Initializes given styled text control.
 *
 * @param stc - styled text control to initialize.
 */
void cbGcov::InitTextCtrlForCovData(cbStyledTextCtrl *stc)
{
    if (!stc) return;

    // extra margin with coverage numbers per line
    stc->StyleSetForeground(cbGcovNaStyle, Manager::Get()->GetColourManager()->GetColour(wxT("cbgcov_not_executable")));
    stc->StyleSetForeground(cbGcovOkStyle, Manager::Get()->GetColourManager()->GetColour(wxT("cbgcov_executed")));
    stc->StyleSetForeground(cbGcovKoStyle, Manager::Get()->GetColourManager()->GetColour(wxT("cbgcov_not_executed")));

    stc->StyleSetBackground(cbGcovNaStyle, stc->StyleGetBackground(wxSCI_STYLE_LINENUMBER));
    stc->StyleSetBackground(cbGcovOkStyle, stc->StyleGetBackground(wxSCI_STYLE_LINENUMBER));
    stc->StyleSetBackground(cbGcovKoStyle, stc->StyleGetBackground(wxSCI_STYLE_LINENUMBER));

    // some color for the line:
    stc->MarkerDefine(cbGcovNaMarker,wxSCI_MARK_EMPTY, wxNullColour, Manager::Get()->GetColourManager()->GetColour(wxT("cbgcov_not_executable")));
    stc->MarkerDefine(cbGcovOkMarker,wxSCI_MARK_EMPTY, wxNullColour, Manager::Get()->GetColourManager()->GetColour(wxT("cbgcov_executed")));
    stc->MarkerDefine(cbGcovKoMarker,wxSCI_MARK_EMPTY, wxNullColour, Manager::Get()->GetColourManager()->GetColour(wxT("cbgcov_not_executed")));
    const int alpha = 12;
    stc->MarkerSetAlpha(cbGcovNaMarker, alpha);
    stc->MarkerSetAlpha(cbGcovOkMarker, alpha);
    stc->MarkerSetAlpha(cbGcovKoMarker, 2 * alpha);

    stc->AnnotationSetVisible(wxSCI_ANNOTATION_BOXED);
}

/**
 * @brief Removes coverage data display from given editor.
 *
 * @param ed - editor to remove coverage data.
 */
void cbGcov::ClearCovData(cbEditor *ed)
{
    cbStyledTextCtrl *stc = 0;
    if(!ed || (stc = ed->GetControl()) == 0) return;

    // do not show margin
    stc->SetMarginWidth(cbGcovMarginNumber, 0);

    stc->MarkerDeleteAll(cbGcovNaMarker);
    stc->MarkerDeleteAll(cbGcovKoMarker);
    stc->MarkerDeleteAll(cbGcovOkMarker);

    stc->AnnotationClearAll();
}

/**
 * @brief Display coverage data for a given editor.
 *
 * @param ed - editor to show coverage data
 * @param prj - NULL or project to which 'ed' belongs. Used to get working path.
 */
void cbGcov::ShowCovData(cbEditor *ed, cbProject* prj)
{
    wxString jobsWorkingdir;

    if(!ed) return;

    edLineExecInfos.erase(ed);

    // Either process a file belonging for a specific project, or use a global setting.
    if(!prj)
        jobsWorkingdir = m_JobsWorkingdir;
    else
        jobsWorkingdir = prj->GetBasePath();

    wxFileName basefilename(ed->GetFilename());
    wxFileName gcovfilename(jobsWorkingdir + basefilename.GetFullName() + _T(".gcov"));
//    wxFileName gcovfilename(ed->GetFilename() + _T(".gcov"));
    if(! gcovfilename.FileExists())
    {
        Log(basefilename.GetFullName() + _(": No coverage data (*.gcov) found. Please run gcov for source file"));
        return;
    }
    if(basefilename.GetModificationTime() > gcovfilename.GetModificationTime())
    {
        Log(basefilename.GetFullName() + _(": Source file is newer than coverage data (*.gcov). Please build and run your project"));
        return;
    }

    LineInformations lineInfos;
    GetLineInfos(gcovfilename, lineInfos);

    ShowCovData(ed, lineInfos);
}

void cbGcov::ShowCovData(cbEditor *ed, LineInformations &lineInfos)
{
    cbStyledTextCtrl* stc;

    if(!ed || (stc = ed->GetControl()) == 0)
        return;

    ClearCovData(ed);
    edLineExecInfos.erase(ed);
    ExecNotexecLines execLines;

    stc->SetMarginWidth(cbGcovMarginNumber, 60);
    stc->SetMarginType(cbGcovMarginNumber, wxSCI_MARGIN_TEXT);
    InitTextCtrlForCovData(stc);

    for(unsigned int l = 0 ; l < stc->GetLineCount() ; l++)
    {
        int calls = NoCode;
        LineInformations::iterator it = lineInfos.find(l + 1);
        if(it == lineInfos.end())
            continue;
        calls = it->second.executionCount;

        if(calls == NoCode)
        {
            stc->MarkerAdd(l, cbGcovNaMarker);
            stc->MarginSetText(l, _T("-"));
            stc->MarginSetStyle(l, cbGcovNaStyle);
        }
        else if(calls == 0)
        {
            execLines.NotexecLines.push_back(l);
            stc->MarkerAdd(l, cbGcovKoMarker);
            stc->MarginSetText(l, _T("0"));
            stc->MarginSetStyle(l, cbGcovKoStyle);
        }
        else
        {
            execLines.ExecLines.push_back(l);
            stc->MarkerAdd(l, cbGcovOkMarker);
            stc->MarginSetText(l, wxString::Format(_T("%d"), calls));
            stc->MarginSetStyle(l, cbGcovOkStyle);
        }

        wxString annotationStr;

        wxArrayString &functions = it->second.functionCalls;
        if(functions.GetCount())
        {
            annotationStr = _T("Functions summary:\n    ") + functions[0];
            for(size_t i = 1 ; i < functions.GetCount() ; ++i)
                annotationStr += _T("\n    ") + functions[i];
        }

        wxArrayString &branches = it->second.branchInfos;
        if(branches.GetCount())
        {
            if(!annotationStr.IsEmpty())
                annotationStr += _T("\n");
            annotationStr += _T("branches:\n    ") + branches[0];
            for(size_t i = 1; i < branches.GetCount() ; ++i)
                annotationStr +=_T("\n    ") + branches[i];
        }

        wxArrayString &callInfos = it->second.callInfos;
        if(callInfos.GetCount())
        {
            if(!annotationStr.IsEmpty())
                annotationStr += _T("\n");
            annotationStr += _T("call infos:\n    ") + callInfos[0];
            for(size_t i = 1; i < callInfos.GetCount() ; ++i)
                annotationStr +=_T("\n    ") + callInfos[i];
        }

        if(!annotationStr.IsEmpty())
            stc->AnnotationSetText(l, annotationStr);

    }
    edLineExecInfos[ed] = execLines;
}

/**
 * @brief Process gcov output for given project.
 *        Computes and displays basic statistics to the CB log. Currently that is
 *         per file, per project and per workspace executed vs total lines figures.
 *         These are taken from gcov process output (what it prints to stdout).
 *
 * @param prj - project to process.
 */
void cbGcov::GetStats(cbProject * prj)
{
    if(!prj)
        return;

    wxArrayString processedFiles;
    wxString jobsWorkingdir = prj->GetBasePath();

    for (FilesList::iterator it = prj->GetFilesList().begin(); it != prj->GetFilesList().end(); ++it)
    {
        ProjectFile* prjfile = *it;
        // also check header files and other non compiled source files
        //(inline code in class definition or template implementations)
        if(prjfile)
        {
            Log(_T("GcovStats"));
            const wxFileName basefilename(prjfile->file.GetFullPath());
            const wxFileName gcovfilename(jobsWorkingdir + basefilename.GetFullName() + _T(".gcov"));
            if(! gcovfilename.FileExists())
            {
                Log(basefilename.GetFullName() + _(": No coverage data (*.gcov) found. Please run gcov for source file"));
                continue;
            }
            if(basefilename.GetModificationTime() > gcovfilename.GetModificationTime())
            {
                Log(basefilename.GetFullName() + _(": Source file is newer than coverage data (*.gcov). Please build your project"));
                continue;
            }

            LineInformations lineInfos;
            m_LocalCodeLines = m_LocalCodeLinesCalled = m_LocalNonExecutableCodeLines = 0;
            GetLineInfos(gcovfilename, lineInfos);

            if(Manager::Get()->GetEditorManager() && Manager::Get()->GetEditorManager()->IsOpen(prjfile->file.GetFullPath()))
            {
                cbEditor *ed = Manager::Get()->GetEditorManager()->GetBuiltinEditor(prjfile->file.GetFullPath());
                if(ed)
                    ShowCovData(ed, lineInfos);
            }

            processedFiles.push_back(prjfile->file.GetFullPath());
        }
    }


    wxString str;
    str.Printf(_("Gcov summary: %zu files analyzed"), processedFiles.GetCount());
    Log(str);

    unsigned int total_codeLinesCalled = 0;
    unsigned int total_codeLines = 0;

    Summaries summaries;

    for(wxArrayString::iterator it = processedFiles.begin(); it < processedFiles.end(); ++it)
    {
        wxFileName fname(*it);
        str.Empty();
        for(Output_t::iterator mit = m_Output.begin(); mit != m_Output.end(); ++mit)
        {
            wxArrayString output = (*mit).second;
            for(int k = 0; k < output.size(); ++k)
            {
                /*
                File 'tests.cpp'
                Lines executed:92.20% of 141
                Branches executed:39.50% of 800
                Taken at least once:20.25% of 800
                Calls executed:29.64% of 1201
                Creating 'tests.cpp.gcov'*/

                if((output[k].Mid(0, 5) == _T("File ")) && (output[k].Find(fname.GetFullName()) != wxNOT_FOUND))
                {
                    gcovSummaryFileData summary;
                    summary.filename = fname.GetFullPath();
                    if(output[k+1].Mid(0, 15) == _T("Lines executed:"))
                    {
                        wxString s = output[k+1].Mid(15);
                        s = s.Mid(s.find_last_of(_T(" ")));
                        unsigned long _codeLines = 0;
                        s.ToULong(&_codeLines);
                        s = output[k+1].Mid(15);
                        s = s.Mid(0, s.find_first_of(_T("%")));
                        double code_percentage = 0;
                        s.ToDouble(&code_percentage);

                        str.Printf(_T("%s\n%s = %d, %s, %s, %s"), output[k].c_str(), output[k+1].c_str(), (int)(_codeLines * code_percentage / 100.0),
                                output[k+2].c_str(), output[k+3].c_str(), output[k+4].c_str());

                        total_codeLinesCalled += _codeLines * code_percentage / 100.0;
                        total_codeLines += _codeLines;

                        summary.totalCodeLines = _codeLines;
                        summary.totalCodeLinesCalled = _codeLines * code_percentage / 100.0;
                        summary.hasLines = true;
                    }
                    if(output[k+2].Mid(0, 18) == _T("Branches executed:"))
                    {
                        wxString s = output[k+2].Mid(18);
                        s = s.Mid(s.find_last_of(_T(" ")));
                        unsigned long _branches = 0;
                        s.ToULong(&_branches);
                        s = output[k+2].Mid(18);
                        s = s.Mid(0, s.find_first_of(_T("%")));
                        double branches_percentage = 0;
                        s.ToDouble(&branches_percentage);

                        summary.totalBranches = _branches;
                        summary.totalBranchesConditionEvaluated = _branches * branches_percentage / 100.0;
                        summary.hasBranches = true;

                        if(output[k+3].Mid(0, 20) == _T("Taken at least once:"))
                        {
                            wxString s = output[k+3].Mid(20);
                            s = s.Mid(0, s.find_first_of(_T("%")));
                            double branchesTaken_percentage = 0;
                            s.ToDouble(&branchesTaken_percentage);

                            summary.totalBranchesTaken = summary.totalBranches * branchesTaken_percentage / 100.0;
                            summary.hasBranchesTaken = true;
                        }
                    }
                    if(output[k+4].Mid(0, 15) == _T("Calls executed:"))
                    {
                        wxString s = output[k+4].Mid(15);
                        s = s.Mid(s.find_last_of(_T(" ")));
                        unsigned long _calls = 0;
                        s.ToULong(&_calls);
                        s = output[k+4].Mid(15);
                        s = s.Mid(0, s.find_first_of(_T("%")));
                        double calls_percentage = 0;
                        s.ToDouble(&calls_percentage);

                        summary.totalCalls = _calls;
                        summary.totalCallsExecuted = _calls * calls_percentage / 100.0;
                        summary.hasCalls = true;
                    }
                    summaries.push_back(summary);
                }
            }
        }
        if(!str.IsEmpty())
            Log(str);
    }
    if(total_codeLines)
    {
        str.Printf(_("Total %u of %u lines executed (%.1f%%)"), total_codeLinesCalled,
                total_codeLines, 100.0 * (float)total_codeLinesCalled / (float)total_codeLines);
    }
    else
    {
        str.Printf(_("Total 0 of 0 lines executed (0.0%%)"));
    }
    Log(str);
    m_CodeLines += total_codeLines;
    m_CodeLinesCalled += total_codeLinesCalled;

    new cbGcovSummaryPanel( (wxWindow*)Manager::Get()->GetEditorManager()->GetNotebook(), summaries);

    str.Printf(_("Workspace total: %u of %u lines executed (%.1f%%)"), m_CodeLinesCalled,
            m_CodeLines, 100.0 * (float)m_CodeLinesCalled / (float)m_CodeLines);
    Log(str);
    wxTimeSpan timeSpan = wxDateTime::UNow() - m_TimeSpan;
    str.Printf(_("cbGov finished (%u minutes, %u seconds)"), timeSpan.GetMinutes(), timeSpan.GetSeconds().GetLo() -
            60 * timeSpan.GetMinutes());
    Log(str);
}

/**
 * @brief Read a .gcov file to internal structure.
 *        The parsed input is used only for graphical display in the editor windows.
 *        The statistics printed out in CB console is derived from gcov process output
 *        to stdout.
 *
 * @param filename - .gcov full file name/path
 * @param LineInfos - internal structure to hold per file gcov info
 */
void cbGcov::GetLineInfos(wxFileName filename, LineInformations &lineInfos)
{
    lineInfos.clear();

    wxFFile file(filename.GetFullPath());
    if(!file.IsOpened())
    {
        Log(_("Failed opening: ") + filename.GetFullPath());
        return;
    }

    //wxString file_buffer;
    //file.ReadAll(&file_buffer);

    // Note: wxTextFile cannot open larger files.
    // wxFFile::ReadAll cannot read larger files to a wxString ?!
    // This is what seems to work always
    int len = file.Length();
    char* buff = new char[len+1];
    file.Read(buff, len);
    buff[len] = 0;
    wxString file_buffer(buff);
    delete [] buff;

    wxStringTokenizer tkz(file_buffer, wxT("\r\n"));
    wxString str;
    unsigned int lines = 0;
    while(tkz.HasMoreTokens())
    {
        str = tkz.GetNextToken();
        AddInfoFromLine(str, lineInfos);
        lines++;
    }

    str.Printf(_("%-40s: %u lines processed"), filename.GetFullPath().c_str(), lines);
    Log(str);
}

/**
 * @brief Parse a single line from a .gcov file
 *
 * @param line - a single line from .gcov file
 * @param LineInfos - internal structure to hold per file gcov info
 */
void cbGcov::AddInfoFromLine(wxString &line, LineInformations &lineInfos)
{
    // "\s*\\d+:\\s*\\d+:"
    // "\\s*-:\\s*\\d+:"   containing no code
    // "\s*#{5}:\s*\d+:"   never executed
    line.Trim(false);

    static unsigned int lastDetectedLine = 0;

    if(line[0] == _T('-') && line[1] == _T(':'))
    {
        line = line.Mid(2);
        line.Trim(false);
        unsigned long l = 0;
        line.ToULong(&l, 10);
        lineInfos[(unsigned int)l].executionCount = NoCode;
        lastDetectedLine = l;
        m_LocalNonExecutableCodeLines++;
    }
    else if(line.Mid(0, 6) == _T("#####:"))
    {
        line = line.Mid(6);
        line.Trim(false);

        unsigned long l = 0;
        if(!line.ToULong(&l))
            ;//return;

        lineInfos[(unsigned int)l].executionCount = 0;
        m_LocalCodeLines++;
        lastDetectedLine = l;
    }
    else if(line.Mid(0, 6) == _T("$$$$$:"))
    {
        //block
        return;
    }
    else if(line.StartsWith(_T("function")))
    {
        wxString str = line.Mid(8).Trim(false);
        lineInfos[lastDetectedLine].functionCalls.Add(str);
    }
    else if(line.StartsWith(_T("branch")))
    {
        wxString str = line.Mid(6).Trim(false);
        lineInfos[lastDetectedLine].branchInfos.Add(str);
    }
    else if(line.StartsWith(_T("call")))
    {
        wxString str = line.Mid(4).Trim(false);
        lineInfos[lastDetectedLine].callInfos.Add(str);
    }
    else
    {
        unsigned long calls = 0;
        if(!line.ToULong(&calls))
            ;//return;

        int pos = line.Find(_T(':'));
        if(pos == wxNOT_FOUND) return;
        line = line.Mid(pos + 1);
        line.Trim(false);
        unsigned long l = 0;
        if(!line.ToULong(&l, 10))
            ;//return;
        lineInfos[(unsigned int)l].executionCount = calls;
        m_LocalCodeLinesCalled++;
        m_LocalCodeLines++;
        lastDetectedLine = l;
    }
}


/**
 * @brief OnIdle handler. Used to read output from gcov processes.
 */
void cbGcov::OnIdle(wxIdleEvent& event)
{
    bool requestMore = false;
    if(m_pProcesses.size())
    {
        for(GcovProcesses_t::iterator it = m_pProcesses.begin(); it != m_pProcesses.end(); ++it)
            requestMore |= (*it).second->ReadProcessOutput();
    }

    if(requestMore)
        event.RequestMore();

    event.Skip();
}

/**
 * @brief On wxProcess terminate handler.
 *        Used to sequence parsing multiple projects in a workspace.
 */
void cbGcov::OnGcovReturned(wxProcessEvent & event)
{
    if(!m_pProcesses.size()) return;

    if(GcovProcess* process = m_pProcesses[event.GetPid()])
    {
        while(process->ReadProcessOutput())
            ;
        // delete process; // No need to delete
        m_pProcesses.erase(event.GetPid());
    }

    if(m_pProcesses.size() < m_ParallelProcessCount)
        StartGcov();
}

/**
 * @brief Receives gcov process output.
 *        This output is used to generate the statistics in GetStats()
 *
 * @param line - gcov process output to stdout
 * @param id - id that is assigned to a GcovProcess in StartGcovParallel()
 *
 * @see GetStats
 * @see StartGcovParallel
 */
void cbGcov::OnProcessGeneratedOutputLine(const wxString &line, unsigned int id)
{
    m_Output[id].Add(line);
}

/**
 * @brief Returns a gcov tool file name.
 */
const wxString cbGcov::GetGcovBinaryName()
{
    return _T("gcov");
}

/**
 * @brief Even handler for "Rung Gcov on workspace" menu item.
 *        Runs gcov on all projects in the workspace sequentially.
 *
 * @see OnDoGcov
 */
void cbGcov::OnDoGcovWorkspace(wxCommandEvent &event)
{
    ProjectsArray* prjs = Manager::Get()->GetProjectManager()->GetProjects();
    m_CodeLines = m_CodeLinesCalled = 0;

    if(m_pProcesses.size())
    {
        Log(_T("cbGcov already running"));
        return;
    }

    if(!prjs) return;

    Log(_T("Starting gcov on all projects in workspace"));

    Initialize();

    for(int i = 0; i < prjs->GetCount(); ++i)
    {
        if(prjs->Item(i))
        {
            m_Prjs.push_back(prjs->Item(i));
        }
    }
    WorkspaceNextProject();
}

/**
 * @brief Even handler for "Rung Gcov on project" menu item.
 *        Runs gcov on the active project.
 *
 * @see GcovProject
 * @see StartGcov
 */
void cbGcov::OnDoGcov(wxCommandEvent &event)
{
    if(m_pProcesses.size())
    {
        Log(_T("cbGcov already running"));
        return;
    }

    cbProject * prj = Manager::Get()->GetProjectManager()->GetActiveProject();
    if(!prj) return;

    Initialize();
    GcovProject(prj);
    StartGcov();
}

/**
 * @brief Launches gcov on the next project in workspace.
 *
 * @see GcovProject
 * @see StartGcov
 */
int cbGcov::WorkspaceNextProject()
{
    if(m_Prjs.size())
    {
        m_Prj = m_Prjs.back();
        m_Prjs.pop_back();
        GcovProject(m_Prj);
        StartGcov();
        return 1;
    }
    return 0;
}

/**
 * @brief Adds all files of given project to a commands list.
 *
 * @param prj - project to process.
 */
void cbGcov::GcovProject(cbProject* prj)
{
    if(!prj || !prj->GetBuildTarget(prj->GetActiveBuildTarget())) return;

    Log(_("Starting cbGcov on '") + prj->GetTitle() + _("'"));
    m_JobsWorkingdir = prj->GetBasePath();
    wxString sep = wxFILE_SEP_PATH;
    wxString objOutput = prj->GetBuildTarget(prj->GetActiveBuildTarget())->GetObjectOutput();

    for (FilesList::iterator it = prj->GetFilesList().begin(); it != prj->GetFilesList().end(); ++it)
    {
        ProjectFile* prjfile = *it;
        if(prjfile && prjfile->compile)
        {
            wxFileName objname(objOutput + sep + prjfile->GetObjName());
            wxString objdir = objname.GetPath();

            QuoteStringIfNeeded(objdir);
            wxString srcfile = prjfile->file.GetFullPath();
            m_JobsFileList.Add(srcfile);
            QuoteStringIfNeeded(srcfile);

            wxString cmd = GetGcovBinaryName() + _T(" -ac");
            cmd += config_ ?
                (wxString(config_->outputFunctionSummaries ? _T("f") : _T("")) +
                          wxString(config_->branchProbabilities ? _T("b") : _T("")) +
                          wxString(config_->demangleNames ? _T("m") : _T(""))) :
                wxString(_T("fb"));

            cmd += (objdir.IsEmpty() ? _T(" ") : _T(" -o ") + objdir + _T(" ")) + srcfile;

            m_Cmds.Add(cmd);
        }
    }
    m_Prj = prj;
}

/**
 * @brief Dispatches the commands list
 *        Starts gcov with the commands in the list if any are available.
 *        Else updates the editors with already processed statistics and checks
 *        if there is more projects to process in the workspace.
 */
void cbGcov::StartGcov()
{
    if(m_Cmds.GetCount())
        StartGcovParallel();
    else if(!m_pProcesses.size())    // Some processes may still be running even with 0 cmd count left
    {
        // gcov has fished running on all files of the project, update relevant editor windows
        GetStats(m_Prj);
        cbProject* prj = m_Prj;
        WorkspaceNextProject(); // Process next project in a workspace, if one is available
        //UpdateEditors(prj);     // if next project is processed by WorkspaceNextProject() then UpdateEditors() is done in parallel
    }
}

/**
 * @brief Launches a wxProcess for next items in the commands list.
 *        Number of processes is the same as the global 'compiler processes' cb setting.
 */
void cbGcov::StartGcovParallel()
{
    if(m_pProcesses.size() >= m_ParallelProcessCount) return;

    wxString curDir = wxGetCwd();
    wxSetWorkingDirectory(m_JobsWorkingdir);

    while(m_pProcesses.size() < m_ParallelProcessCount)
    {
        if(!m_Cmds.size()) break;

        wxString cmd = m_Cmds[0];
        m_Cmds.RemoveAt(0);

        GcovProcess* process = new GcovProcess(this);
        int pid = wxExecute(cmd, wxEXEC_ASYNC | wxEXEC_MAKE_GROUP_LEADER, process);
        if(pid == 0)
        {
            Log(_("Error calling gcov"));
            return; // if not returning here, then take care of infinite loop not taking place
        }

        Log(cmd);
        if (m_pProcesses.find(pid) != m_pProcesses.end() || m_Output.find(pid) != m_Output.end())
        {
            // pid already used by us?
        }

        m_pProcesses[pid] = process;
        m_Output[pid] = wxArrayString();
        process->SetId(pid);
    }

    wxSetWorkingDirectory(curDir);
}

/**
 * @brief Updates the CB editor windows for a given project.
 *        The project is already processed at this point.
 */
void cbGcov::UpdateEditors(cbProject* prj)
{
    // Update editors for given project only
    for (FilesList::iterator it = prj->GetFilesList().begin(); it != prj->GetFilesList().end(); ++it)
    {
        ProjectFile* prjfile = *it;
        if(!prjfile) continue;

        wxString srcfile = prjfile->file.GetFullPath();
        for(unsigned int i = 0 ; i < m_JobsFileList.GetCount() ; ++i)
            if(m_JobsFileList[i] == srcfile)
                if(cbEditor *ed = Manager::Get()->GetEditorManager()->GetBuiltinEditor(m_JobsFileList[i]))
                    ShowCovData(ed);
    }
}

/**
 * @brief OnEditorOpen handler. Updates a newly opened editor window with gcov stats.
 */
void cbGcov::OnEditorOpen(CodeBlocksEvent &event)
{
    EditorBase * eb = event.GetEditor();
    cbProject* prj = event.GetProject();  // this is always null?
    if(!eb || !eb->IsBuiltinEditor()) return;

    cbEditor *ed = Manager::Get()->GetEditorManager()->GetBuiltinEditor(eb);
    if(!ed) return;

    if(!prj && ed->GetProjectFile())
        prj = ed->GetProjectFile()->GetParentProject();  // this one too ? :(

    ShowCovData(ed, prj);
}

/**
 * @brief OnEditorClose handler. Clears gcov exec not exec data for ed
 */
void cbGcov::OnEditorClose(CodeBlocksEvent &event)
{
    EditorBase * eb = event.GetEditor();
    if(!eb || !eb->IsBuiltinEditor()) return;

    cbEditor *ed = Manager::Get()->GetEditorManager()->GetBuiltinEditor(eb);
    if(!ed) return;

    edLineExecInfos.erase(ed);
}

/**
 * @brief OnEditorModified handler. Clears any gcov stats upon first editor buffer modification.
 *        User must recompile,execute,run cbGcov to get the analysis results again.
 */
void cbGcov::OnEditorModified(CodeBlocksEvent &event)
{
    EditorBase * eb = event.GetEditor();
    if(!eb || !eb->IsBuiltinEditor()) return;

    cbEditor *ed = Manager::Get()->GetEditorManager()->GetBuiltinEditor(eb);
    if(!ed) return;

    edLineExecInfos.erase(ed);
    ClearCovData(ed);
}

/**
 * @brief OnCleanProject handler. Removes gcov related files from output directories.
 */
void cbGcov::OnCleanProject(CodeBlocksEvent &event)
{
    CleanProject(event.GetProject());
}

/**
 * @brief OnCleanWorkspace handler. Removes gcov related files from output directories.
 */
void cbGcov::OnCleanWorkspace(CodeBlocksEvent &event)
{
    ProjectsArray* prjs = Manager::Get()->GetProjectManager()->GetProjects();

    for(int i = 0; i < prjs->GetCount(); ++i)
    {
        if(prjs->Item(i))
            CleanProject(prjs->Item(i));
    }
}

/**
 * @brief Removes gcov related files from output directories for a given project.
 */
void cbGcov::CleanProject(cbProject* prj)
{
    if(!prj || !prj->GetBuildTarget(prj->GetActiveBuildTarget())) return;

    wxString sep = wxFILE_SEP_PATH;
    wxString objOutput = prj->GetBuildTarget(prj->GetActiveBuildTarget())->GetObjectOutput();
    for (FilesList::iterator it = prj->GetFilesList().begin(); it != prj->GetFilesList().end(); ++it)
    {
        ProjectFile* prjfile = *it;
        if(prjfile && prjfile->compile)
        {
            wxFileName file_src(prjfile->GetObjName());
            wxString file = (file_src.GetPath().size() ? file_src.GetPath() + sep : _T("")) +  file_src.GetName();
            wxString gcda_name = objOutput + sep + file + _T(".gcda");
            wxString gcno_name = objOutput + sep + file + _T(".gcno");
            wxRemoveFile(gcda_name);
            wxRemoveFile(gcno_name);
        }
    }
}

/**
 * @brief OnProjectActivate handler. Sets the working path to the active project path.
 */
void cbGcov::OnProjectActivate(CodeBlocksEvent &event)
{
    if(!IsRunning() && event.GetProject())
        m_JobsWorkingdir = event.GetProject()->GetBasePath();
}

/**
 * @brief Returns true, if cbGcov is currently running.
 */
int cbGcov::IsRunning()
{
    return m_Cmds.size() + m_pProcesses.size() + m_Prjs.size() > 0;
}

/**
 * @brief Log to CB console.
 */
void cbGcov::Log(wxString str)
{
    if(LogManager* LogMan = Manager::Get()->GetLogManager())
        LogMan->Log(str);
}

/**
 * @brief Initializes the cbGov class.
 */
void cbGcov::Initialize()
{
    m_ParallelProcessCount = Manager::Get()->GetConfigManager(_T("compiler"))->ReadInt(_T("/parallel_processes"), 1);
    m_Output.clear();
    m_Cmds.clear();
    m_JobsFileList.clear();
    m_Prjs.clear();
    m_Prj = 0;
    m_CodeLines = 0;
    m_CodeLinesCalled = 0;
    m_TimeSpan = wxDateTime::UNow();
}

void cbGcov::OnAddInstrumentationToProject(wxCommandEvent &event)
{
    cbProject * prj = Manager::Get()->GetProjectManager()->GetActiveProject();
    if(!prj) return;

    bool optionsChanged = false;

    const wxArrayString &compilerOptions = prj->GetCompilerOptions();
    bool hasProfileArcs = false;
    bool hasTestCoverage = false;

    for(size_t i = 0 ; i < compilerOptions.GetCount() ; ++i)
    {
        const wxString option = compilerOptions[i];
        if( option == _T("-fprofile-arcs"))
        {
            hasProfileArcs = true;
            continue;
        }
        if( option == _T("-ftest-coverage"))
        {
            hasTestCoverage = true;
            continue;
        }
    }
    if(!hasProfileArcs)
    {
        optionsChanged = true;
        prj->AddCompilerOption(_T("-fprofile-arcs"));
    }
    if(!hasTestCoverage)
    {
        optionsChanged = true;
        prj->AddCompilerOption(_T("-ftest-coverage"));
    }

    const wxArrayString &libs = prj->GetLinkLibs();
    bool needToAddgCovLib = true;
    for(size_t i = 0 ; i < libs.GetCount() ; ++i)
    {
        const wxString &lib = libs[i];
        if ( lib == _T("gcov") || lib == _T("libgcov") )
        {
            needToAddgCovLib = false;
            break;
        }
    }
    if(needToAddgCovLib)
    {
        optionsChanged = true;
        prj->AddLinkLib(_T("gcov"));
    }

    const wxArrayString &lnkOptions = prj->GetLinkerOptions();
    bool needToAddCoverageOption = true;
    for(size_t i = 0 ; i < lnkOptions.GetCount() ; ++i)
    {
        const wxString &option = lnkOptions[i];
        if ( option == _T("-coverage") )
        {
            needToAddCoverageOption = false;
            break;
        }
    }
    if(needToAddCoverageOption)
    {
        optionsChanged = true;
        prj->AddLinkerOption(_T("-coverage"));
    }


    if(optionsChanged)
        Log( _("Project options changed. Please rebuild your project"));
}

void cbGcov::OnUpdateAddInstrumentationToProject(wxUpdateUIEvent& event)
{
    cbProject * prj = Manager::Get()->GetProjectManager()->GetActiveProject();
    event.Enable(prj);
}

void cbGcov::OnUpdateGcov(wxUpdateUIEvent& event)
{
    cbProject * prj = Manager::Get()->GetProjectManager()->GetActiveProject();
    event.Enable((prj) && (m_pProcesses.size() == 0));
}

void cbGcov::OnUpdateGcovWorkspace(wxUpdateUIEvent& event)
{
    ProjectsArray* prjs = Manager::Get()->GetProjectManager()->GetProjects();
    event.Enable((m_pProcesses.size() == 0) && (prjs) && (prjs->GetCount()));
}

void cbGcov::UpdateConfig()
{
    if(!config_) return;

    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("cbGcov"));
    config_->demangleNames = cfg->ReadBool(_T("/DemangleNames"), true);
    config_->outputFunctionSummaries = cfg->ReadBool(_T("/OutputFunctionSummaries"), true);
    config_->branchProbabilities = cfg->ReadBool(_T("/BranchProbabilities"), true);
}

void cbGcov::OnGotoNextNotExecutedLine(wxCommandEvent &event)
{
    cbEditor *ed = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if(!ed) return;

    EditorExecNotexecLines::iterator itr = edLineExecInfos.find(ed);
    if(itr == edLineExecInfos.end()) return;

    if ( !(itr->second.NotexecLines.size()) ) return;

    cbStyledTextCtrl *stc = ed->GetControl();
    uint32_t currLine = stc->GetCurrentLine();
    for(auto it = itr->second.NotexecLines.begin(); it != itr->second.NotexecLines.end(); ++it)
    {
        if ( *it > currLine )
        {
            stc->GotoLine(*it);
            return;
        }
    }
}

void cbGcov::OnGotoPreviousNotExecutedLine(wxCommandEvent &event)
{
    cbEditor *ed = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if(!ed) return;

    EditorExecNotexecLines::iterator itr = edLineExecInfos.find(ed);
    if(itr == edLineExecInfos.end()) return;

    if ( !(itr->second.NotexecLines.size()) ) return;

    cbStyledTextCtrl *stc = ed->GetControl();
    uint32_t currLine = stc->GetCurrentLine();
    unsigned int target = itr->second.NotexecLines[0];
    for(auto it = itr->second.NotexecLines.begin(); it != itr->second.NotexecLines.end(); ++it)
    {
        if ( *it >= currLine )
        {
            stc->GotoLine(target);
            return;
        }
        target = *it;
    }
    stc->GotoLine(target);
}

void cbGcov::OnGotoNextExecutedLine(wxCommandEvent &event)
{
    cbEditor *ed = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if(!ed) return;

    EditorExecNotexecLines::iterator itr = edLineExecInfos.find(ed);
    if(itr == edLineExecInfos.end()) return;

    if ( !(itr->second.ExecLines.size()) ) return;

    cbStyledTextCtrl *stc = ed->GetControl();
    uint32_t currLine = stc->GetCurrentLine();
    for(auto it = itr->second.ExecLines.begin(); it != itr->second.ExecLines.end(); ++it)
    {
        if ( *it > currLine )
        {
            stc->GotoLine(*it);
            return;
        }
    }
}

void cbGcov::OnGotoPreviousExecutedLine(wxCommandEvent &event)
{
    cbEditor *ed = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if(!ed) return;

    EditorExecNotexecLines::iterator itr = edLineExecInfos.find(ed);
    if(itr == edLineExecInfos.end()) return;

    if ( !(itr->second.ExecLines.size()) ) return;

    cbStyledTextCtrl *stc = ed->GetControl();
    uint32_t currLine = stc->GetCurrentLine();
    unsigned int target = itr->second.ExecLines[0];
    for(auto it = itr->second.ExecLines.begin(); it != itr->second.ExecLines.end(); ++it)
    {
        if ( *it >= currLine )
        {
            stc->GotoLine(target);
            return;
        }
        target = *it;
    }
    stc->GotoLine(target);
}

void cbGcov::OnUpdateNextNotExecutedLine(wxUpdateUIEvent &event)
{
    event.Enable(false);
    cbEditor *ed = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if(!ed) return;

    EditorExecNotexecLines::iterator itr = edLineExecInfos.find(ed);
    if(itr == edLineExecInfos.end()) return;

    if ( size_t sz = itr->second.NotexecLines.size() )
        event.Enable(itr->second.NotexecLines[sz-1] > ed->GetControl()->GetCurrentLine());
}

void cbGcov::OnUpdatePreviousNotExecutedLine(wxUpdateUIEvent &event)
{
    event.Enable(false);
    cbEditor *ed = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if(!ed) return;

    EditorExecNotexecLines::iterator itr = edLineExecInfos.find(ed);
    if(itr == edLineExecInfos.end()) return;

    if ( size_t sz = itr->second.NotexecLines.size() )
        event.Enable(itr->second.NotexecLines[0] < ed->GetControl()->GetCurrentLine());
}

void cbGcov::OnUpdateNextExecutedLine(wxUpdateUIEvent &event)
{
    event.Enable(false);
    cbEditor *ed = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if(!ed) return;

    EditorExecNotexecLines::iterator itr = edLineExecInfos.find(ed);
    if(itr == edLineExecInfos.end()) return;

    if ( size_t sz = itr->second.ExecLines.size() )
        event.Enable(itr->second.ExecLines[sz-1] > ed->GetControl()->GetCurrentLine());
}

void cbGcov::OnUpdatePreviousExecutedLine(wxUpdateUIEvent &event)
{
    event.Enable(false);
    cbEditor *ed = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if(!ed) return;

    EditorExecNotexecLines::iterator itr = edLineExecInfos.find(ed);
    if(itr == edLineExecInfos.end()) return;

    if ( size_t sz = itr->second.ExecLines.size() )
        event.Enable(itr->second.ExecLines[0] < ed->GetControl()->GetCurrentLine());
}

