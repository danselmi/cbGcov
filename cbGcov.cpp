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

// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
  PluginRegistrant<cbGcov> reg(_T("cbGcov"));

  const int idDoShowGcov                  = wxNewId();
  const int idDoGcov                      = wxNewId();
  const int idDoGcovAnalyze               = wxNewId();
  const int idAddInstrumentationToProject = wxNewId();

}

const int cbGcovNaMarker = 10;
const int cbGcovOkMarker = 11;
const int cbGcovKoMarker = 12;

const int cbGcovMarginNumber = 4;

const int cbGcovNaStyle = 80;
const int cbGcovOkStyle = 81;
const int cbGcovKoStyle = 82;


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
    cm->RegisterColour(_("cbGcov"), _("no executable code"), wxT("cbgcov_not_executable"), *wxBLACK );
    cm->RegisterColour(_("cbGcov"), _("never executed"), wxT("cbgcov_not_executed"), *wxRED );
    cm->RegisterColour(_("cbGcov"), _("executed at least once"), wxT("cbgcov_executed"), *wxGREEN );
  }

  Connect(idAddInstrumentationToProject, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(cbGcov::OnAddInstrumentationToProject));
  Connect(idAddInstrumentationToProject, wxEVT_UPDATE_UI,             wxUpdateUIEventHandler(cbGcov::OnUpdateAddInstrumentationToProject));
  Connect(idDoGcov,                      wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(cbGcov::OnDoGcov));
  Connect(idDoGcov,                      wxEVT_UPDATE_UI,             wxUpdateUIEventHandler(cbGcov::OnUpdateGcov));
  Connect(idDoGcovAnalyze,               wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(cbGcov::OnDoGcovWorkspace));
  Connect(idDoGcovAnalyze,               wxEVT_UPDATE_UI,             wxUpdateUIEventHandler(cbGcov::OnUpdateGcovWorkspace));

  Connect(wxEVT_END_PROCESS,   wxProcessEventHandler(cbGcov::OnGcovReturned), NULL, this);
  Connect(wxEVT_IDLE,          wxIdleEventHandler(cbGcov::OnIdle), NULL, this);

  Manager::Get()->RegisterEventSink(cbEVT_EDITOR_OPEN, new cbEventFunctor<cbGcov, CodeBlocksEvent>(this, &cbGcov::OnEditorOpen));
  Manager::Get()->RegisterEventSink(cbEVT_EDITOR_MODIFIED, new cbEventFunctor<cbGcov, CodeBlocksEvent>(this, &cbGcov::OnEditorModified));
  Manager::Get()->RegisterEventSink(cbEVT_CLEAN_PROJECT_STARTED, new cbEventFunctor<cbGcov, CodeBlocksEvent>(this, &cbGcov::OnCleanProject));
  Manager::Get()->RegisterEventSink(cbEVT_CLEAN_WORKSPACE_STARTED, new cbEventFunctor<cbGcov, CodeBlocksEvent>(this, &cbGcov::OnCleanWorkspace));
  Manager::Get()->RegisterEventSink(cbEVT_PROJECT_ACTIVATE, new cbEventFunctor<cbGcov, CodeBlocksEvent>(this, &cbGcov::OnProjectActivate));

}

/**
 * @brief OnRelease handler. Deregisters plugin handlers.
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
}

/**
 * @brief Not in use.
 */
int cbGcov::Configure()
{
  //create and display the configuration dialog for your plugin
  cbConfigurationDialog dlg(Manager::Get()->GetAppWindow(), wxID_ANY, _("Your dialog title"));
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
    prjmenu->Append(idDoGcov, _T("Run Gcov on project"));
    prjmenu->Append(idDoGcovAnalyze, _T("Run Gcov on workspace"));
    prjmenu->Append(idAddInstrumentationToProject, _T("Add Gcov instrumentation"), _T("Add compiler/linker option to add gcov coverage instrumentation to project."));
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
  NotImplemented(_T("cbCov::BuildModuleMenu()"));
}

/**
 * @brief
 */
bool cbGcov::BuildToolBar(wxToolBar* toolBar)
{
  //The application is offering its toolbar for your plugin,
  //to add any toolbar items you want...
  //Append any items you need on the toolbar...
  NotImplemented(_T("Cov0::BuildToolBar()"));

  // return true if you add toolbar items
  return false;
}

/**
 * @brief Initializes given styled text control.
 *
 * @param stc - styled text control to initialize.
 */
void cbGcov::InitTextCtrlForCovData(cbStyledTextCtrl *stc)
{
  if (!stc)
  {
    return;
  }
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

}

/**
 * @brief Removes coverage data display from given editor.
 *
 * @param ed - editor to remove coverage data.
 */
void cbGcov::ClearCovData(cbEditor *ed)
{
  cbStyledTextCtrl *stc = 0;
  if(!ed || (stc = ed->GetControl()) == 0)
  {
    return;
  }
  // do not show margin
  stc->SetMarginWidth(cbGcovMarginNumber, 0);

  stc->MarkerDeleteAll(cbGcovNaMarker);
  stc->MarkerDeleteAll(cbGcovKoMarker);
  stc->MarkerDeleteAll(cbGcovOkMarker);
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

  if(!ed)
  {
    return;
  }

  // Either process a file belonging for a specific project, or use a global setting.
  if(!prj)
  {
    jobsWorkingdir = m_JobsWorkingdir;
  }
  else
  {
    jobsWorkingdir = prj->GetBasePath();
  }

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
    Log(basefilename.GetFullName() + _(": Source file is newer than coverage data (*.gcov). Please build your project"));
    return;
  }

  LineInfos_t LineInfos;
  GetLineInfos(gcovfilename, LineInfos);

  ShowCovData(ed, LineInfos);
}

void cbGcov::ShowCovData(cbEditor *ed, LineInfos_t &LineInfos)
{
  cbStyledTextCtrl* stc;

  if(!ed || (stc = ed->GetControl()) == 0)
  {
    return;
  }

  ClearCovData(ed);
  stc->SetMarginWidth(cbGcovMarginNumber, 60);
  stc->SetMarginType(cbGcovMarginNumber, wxSCI_MARGIN_TEXT);
  InitTextCtrlForCovData(stc);

  for(unsigned int l = 0 ; l < stc->GetLineCount() ; l++)
  {
    int calls = NoCode;
    std::map<unsigned int, int>::iterator it = LineInfos.find(l + 1);
    if(it == LineInfos.end())
      continue;
    calls = it->second;

    if(calls == NoCode)
    {
      stc->MarkerAdd(l, cbGcovNaMarker);
      stc->MarginSetText(l, _T("-"));
      stc->MarginSetStyle(l, cbGcovNaStyle);
    }
    else if(calls == 0)
    {
      stc->MarkerAdd(l, cbGcovKoMarker);
      stc->MarginSetText(l, _T("0"));
      stc->MarginSetStyle(l, cbGcovKoStyle);
    }
    else
    {
      stc->MarkerAdd(l, cbGcovOkMarker);
      stc->MarginSetText(l, wxString::Format(_T("%d"), calls));
      stc->MarginSetStyle(l, cbGcovOkStyle);
    }
  }
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
  {
    return;
  }
  GcovStats file_m_Stats;
  m_Stats.clear();
  wxString jobsWorkingdir = prj->GetBasePath();

  for (FilesList::iterator it = prj->GetFilesList().begin(); it != prj->GetFilesList().end(); ++it)
  {
    ProjectFile* prjfile = *it;
    // also check header files and other non compiled source files
    //(inline code in class definition or template implementations)
    if(prjfile)
    {
      Log(_("GcovStats"));
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

      LineInfos_t LineInfos;
      m_LocalCodeLines = m_LocalCodeLinesCalled = 0;
      GetLineInfos(gcovfilename, LineInfos);

      if(Manager::Get()->GetEditorManager() && Manager::Get()->GetEditorManager()->IsOpen(prjfile->file.GetFullPath()))
      {
        cbEditor *ed = Manager::Get()->GetEditorManager()->GetBuiltinEditor(prjfile->file.GetFullPath());
        if(ed)
            ShowCovData(ed, LineInfos);
      }

      file_m_Stats.Filename = basefilename.GetFullName();
      file_m_Stats.codeLines = m_LocalCodeLines;
      file_m_Stats.codeLinesCalled = m_LocalCodeLinesCalled;
      m_Stats.push_back(file_m_Stats);
    }
  }

  wxString str;
  int total_codeLinesCalled = 0;
  int total_codeLines = 0;

  str.Printf(_("Gcov summary: %d files analyzed"), m_Stats.size());
  Log(str);
  for(std::vector<GcovStats>::iterator it = m_Stats.begin(); it < m_Stats.end(); ++it)
  {
    wxFileName fname(it->Filename);
    str.Empty();
    for(Output_t::iterator mit = m_Output.begin(); mit != m_Output.end(); ++mit)
    {
      wxArrayString output = (*mit).second;
      for(int k = 0; k < output.size(); ++k)
      {
        if((output[k].Mid(0, 5) == _T("File ")) && (output[k].Find(fname.GetFullName()) != wxNOT_FOUND) &&
            (output[k+1].Mid(0, 15) == _T("Lines executed:")))
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
          break;
        }
      }
    }
    if(str.IsEmpty())
    {
      str.Printf(_("%-40s: %d of %d lines executed (%.1f%%) *approximate result given by cbGcov parser*"), it->Filename.c_str(), it->codeLinesCalled,
                 it->codeLines, 100 * (float)it->codeLinesCalled / (float)it->codeLines);
//      total_codeLinesCalled += it->codeLinesCalled;
//      total_codeLines += it->codeLines;
    }
    Log(str);
  }
  if(total_codeLines)
  {
    str.Printf(_("Total %d of %d lines executed (%.1f%%)"), total_codeLinesCalled,
               total_codeLines, 100.0 * (float)total_codeLinesCalled / (float)total_codeLines);
  }
  else
  {
    str.Printf(_("Total 0 of 0 lines executed (0.0%%)"));
  }
  Log(str);
  m_CodeLines += total_codeLines;
  m_CodeLinesCalled += total_codeLinesCalled;

  str.Printf(_("Workspace total: %d of %d lines executed (%.1f%%)"), m_CodeLinesCalled,
             m_CodeLines, 100.0 * (float)m_CodeLinesCalled / (float)m_CodeLines);
  Log(str);
  wxTimeSpan timeSpan = wxDateTime::UNow() - m_TimeSpan;
  str.Printf(_("cbGov finished (%d minutes, %d seconds)"), timeSpan.GetMinutes(), timeSpan.GetSeconds().GetLo() -
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
void cbGcov::GetLineInfos(wxFileName filename, LineInfos_t &LineInfos)
{
  LineInfos.clear();

  wxFFile file(filename.GetFullPath());
  if(!file.IsOpened())
  {
    Log(_("Failed opening: ") + filename.GetFullPath());
    return;
  }

  // Note: wxTextFile cannot open larger files.
  // wxFFile::ReadAll cannot read larger files to a wxString ?!
  // This is what seems to work always
  int lines = 0;
  int len = file.Length();
  char* buff = new char[len+1];
  file.Read(buff, len);
  buff[len] = 0;
  wxString file_buffer = wxString::FromAscii(buff);
  delete [] buff;

  wxStringTokenizer tkz(file_buffer, wxT("\r\n"));
  wxString str;
  while(tkz.HasMoreTokens())
  {
    str = tkz.GetNextToken();
    AddInfoFromLine(str, LineInfos);
    lines++;
  }

  str.Printf(_("%-40s: %d lines processed"), filename.GetFullPath().c_str(), lines);
  Log(str);
}

/**
 * @brief Parse a single line from a .gcov file
 *
 * @param line - a single line from .gcov file
 * @param LineInfos - internal structure to hold per file gcov info
 */
void cbGcov::AddInfoFromLine(wxString &line, LineInfos_t &LineInfos)
{
  // "\s*\\d+:\\s*\\d+:"
  // "\\s*-:\\s*\\d+:"   containing no code
  // "\s*#{5}:\s*\d+:"   never executed
  line.Trim(false);

  if(line[0] == _T('-') && line[1] == _T(':'))
  {
    line = line.Mid(2);
    line.Trim(false);
    unsigned long l = 0;
    line.ToULong(&l, 10);
    LineInfos[(unsigned int)l] = NoCode;
  }
  else if(line.Mid(0, 6) == _T("#####:"))
  {
    line = line.Mid(6);
    line.Trim(false);

    unsigned long l = 0;
    if(!line.ToULong(&l))
      ;//return;

    LineInfos[(unsigned int)l] = 0;
    m_LocalCodeLines++;

  }
  else if(line.Mid(0, 6) == _T("$$$$$:"))
  {
    //block
    return;
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
    LineInfos[(unsigned int)l] = calls;
    m_LocalCodeLinesCalled++;
    m_LocalCodeLines++;
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
    {
      requestMore |= (*it).second->ReadProcessOutput();
    }
  }
  if(requestMore)
  {
    event.RequestMore();
  }
  event.Skip();
}

/**
 * @brief On wxProcess terminate handler.
 *        Used to sequence parsing multiple projects in a workspace.
 */
void cbGcov::OnGcovReturned(wxProcessEvent & event)
{
  if(!m_pProcesses.size())
  {
    return;
  }
  GcovProcess* process = m_pProcesses[event.GetPid()];

  if(process)
  {
    while(process->ReadProcessOutput())
      ;
    // delete process; // No need to delete
    m_pProcesses.erase(event.GetPid());
  }

  if(m_pProcesses.size() < m_ParallelProcessCount)
  {
    StartGcov();
  }
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

  if(!prjs)
  {
    return;
  }

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
    Log(_("cbGcov already running"));
    return;
  }

  cbProject * prj = Manager::Get()->GetProjectManager()->GetActiveProject();
  if(!prj)
  {
    return;
  }

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
  if(!prj || !prj->GetBuildTarget(prj->GetActiveBuildTarget()))
  {
    return;
  }
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

      wxString cmd = GetGcovBinaryName() + _T(" -abf") +
                     (objdir.IsEmpty() ? _T(" ") : _T(" -o ") + objdir + _T(" ")) +
                     srcfile;

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
  {
    StartGcovParallel();
  }
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
    if(m_pProcesses.size() >= m_ParallelProcessCount)
    {
        return;
    }

    wxString curDir = wxGetCwd();
    wxSetWorkingDirectory(m_JobsWorkingdir);

    while(m_pProcesses.size() < m_ParallelProcessCount)
    {
        if(!m_Cmds.size())
        {
            break;
        }
        wxString cmd = m_Cmds[0];
        m_Cmds.RemoveAt(0);

        GcovProcess* process = new GcovProcess(this);
        int pid = wxExecute(cmd, wxEXEC_ASYNC | wxEXEC_MAKE_GROUP_LEADER, process);
        if(pid == 0)
        {
            Log(_("Error calling gcov"));
            return; // if not returning here, then take care of infinite loop not taking place
        }
        else
        {
            Log(cmd);
            if (m_pProcesses.find(pid) != m_pProcesses.end() || m_Output.find(pid) != m_Output.end())
            {
                // pid already used by us?
            }

            m_pProcesses[pid] = process;
            m_Output[pid] = wxArrayString();
            process->SetId(pid);
        }
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
    if(prjfile)
    {
      wxString srcfile = prjfile->file.GetFullPath();
      for(unsigned int i = 0 ; i < m_JobsFileList.GetCount() ; ++i)
      {
        if(m_JobsFileList[i] == srcfile)
        {
          cbEditor *ed = Manager::Get()->GetEditorManager()->GetBuiltinEditor(m_JobsFileList[i]);
          if(ed)
          {
            ShowCovData(ed);
          }
        }
      }
    }
  }
}

/**
 * @brief OnEditorOpen handler. Updates a newly opened editor window with gcov stats.
 */
void cbGcov::OnEditorOpen(CodeBlocksEvent &event)
{
  EditorBase * eb = event.GetEditor();
  cbProject* prj = event.GetProject();  // this is always null?
  if(!eb || !eb->IsBuiltinEditor())
  {
    return;
  }

  cbEditor *ed = Manager::Get()->GetEditorManager()->GetBuiltinEditor(eb);
  if(!ed)
  {
    return;
  }
  if(!prj && ed->GetProjectFile())
  {
    prj = ed->GetProjectFile()->GetParentProject();  // this one too ? :(
  }

  ShowCovData(ed, prj);
}

/**
 * @brief OnEditorModified handler. Clears any gcov stats upon first editor buffer modification.
 *        User must recompile,execute,run cbGcov to get the analysis results again.
 */
void cbGcov::OnEditorModified(CodeBlocksEvent &event)
{
  EditorBase * eb = event.GetEditor();
  if(!eb || !eb->IsBuiltinEditor())return;

  cbEditor *ed = Manager::Get()->GetEditorManager()->GetBuiltinEditor(eb);
  if(!ed)
  {
    return;
  }

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
    {
      CleanProject(prjs->Item(i));
    }
  }
}

/**
 * @brief Removes gcov related files from output directories for a given project.
 */
void cbGcov::CleanProject(cbProject* prj)
{
  if(!prj || !prj->GetBuildTarget(prj->GetActiveBuildTarget()))
  {
    return;
  }
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
  {
    m_JobsWorkingdir = event.GetProject()->GetBasePath();
  }
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
  {
    LogMan->Log(str);
  }
}

/**
 * @brief Initializes the cbGov class.
 */
void cbGcov::Initialize()
{
  m_ParallelProcessCount = Manager::Get()->GetConfigManager(_T("compiler"))->ReadInt(_T("/parallel_processes"), 1);
  m_Output.clear();
  m_Stats.clear();
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

