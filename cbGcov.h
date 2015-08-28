/***************************************************************
 * Name:      cbGcov
 * Purpose:   Code::Blocks plugin
 * Author:    danselmi ()
 * Created:   2010-04-11
 * Copyright: danselmi
 * License:   GPL
 **************************************************************/

#ifndef CBGCOV_H_INCLUDED
#define CBGCOV_H_INCLUDED

// For compilers that support precompilation, includes <wx/wx.h>
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <map>

#include <wx/process.h>
#include <cbplugin.h> // for "class cbPlugin"

class cbStyledTextCtrl;
class GcovProcess;
struct cbGcovConfig;

const int NoCode = -1;

/**
* @brief Data container for internal cbGcov analysis.
*
*/
struct GcovStats
{
    wxString    Filename;
    int         codeLines;
    int         codeLinesCalled;
};
struct cbGcovConfig;
/**
* @brief Implements main functionality of the plugin.
*
*/
class cbGcov : public cbPlugin
{
public:
    /** Constructor. */
    cbGcov();
    /** Destructor. */
    virtual ~cbGcov();

    /** Invoke configuration dialog. */
    virtual int Configure();

    /** Return the plugin's configuration priority.
      * This is a number (default is 50) that is used to sort plugins
      * in configuration dialogs. Lower numbers mean the plugin's
      * configuration is put higher in the list.
      */
    virtual int GetConfigurationPriority() const { return 50; }

    /** Return the configuration group for this plugin. Default is cgUnknown.
      * Notice that you can logically OR more than one configuration groups,
      * so you could set it, for example, as "cgCompiler | cgContribPlugin".
      */
    virtual int GetConfigurationGroup() const { return cgUnknown; }

    /** Return plugin's configuration panel.
      * @param parent The parent window.
      * @return A pointer to the plugin's cbConfigurationPanel. It is deleted by the caller.
      */
    virtual cbConfigurationPanel* GetConfigurationPanel(wxWindow* parent);

    /** Return plugin's configuration panel for projects.
      * The panel returned from this function will be added in the project's
      * configuration dialog.
      * @param parent The parent window.
      * @param project The project that is being edited.
      * @return A pointer to the plugin's cbConfigurationPanel. It is deleted by the caller.
      */
    virtual cbConfigurationPanel* GetProjectConfigurationPanel(wxWindow* parent, cbProject* project){ return 0; }

    /** This method is called by Code::Blocks and is used by the plugin
      * to add any menu items it needs on Code::Blocks's menu bar.\n
      * It is a pure virtual method that needs to be implemented by all
      * plugins. If the plugin does not need to add items on the menu,
      * just do nothing ;)
      * @param menuBar the wxMenuBar to create items in
      */
    virtual void BuildMenu(wxMenuBar* menuBar);

    /** This method is called by Code::Blocks core modules (EditorManager,
      * ProjectManager etc) and is used by the plugin to add any menu
      * items it needs in the module's popup menu. For example, when
      * the user right-clicks on a project file in the project tree,
      * ProjectManager prepares a popup menu to display with context
      * sensitive options for that file. Before it displays this popup
      * menu, it asks all attached plugins (by asking PluginManager to call
      * this method), if they need to add any entries
      * in that menu. This method is called.\n
      * If the plugin does not need to add items in the menu,
      * just do nothing ;)
      * @param type the module that's preparing a popup menu
      * @param menu pointer to the popup menu
      * @param data pointer to FileTreeData object (to access/modify the file tree)
      */
    virtual void BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data = 0);

    /** This method is called by Code::Blocks and is used by the plugin
      * to add any toolbar items it needs on Code::Blocks's toolbar.\n
      * It is a pure virtual method that needs to be implemented by all
      * plugins. If the plugin does not need to add items on the toolbar,
      * just do nothing ;)
      * @param toolBar the wxToolBar to create items on
      * @return The plugin should return true if it needed the toolbar, false if not
      */
    virtual bool BuildToolBar(wxToolBar* toolBar);
protected:
    /** Any descendent plugin should override this virtual method and
      * perform any necessary initialization. This method is called by
      * Code::Blocks (PluginManager actually) when the plugin has been
      * loaded and should attach in Code::Blocks. When Code::Blocks
      * starts up, it finds and <em>loads</em> all plugins but <em>does
      * not</em> activate (attaches) them. It then activates all plugins
      * that the user has selected to be activated on start-up.\n
      * This means that a plugin might be loaded but <b>not</b> activated...\n
      * Think of this method as the actual constructor...
      */
    virtual void OnAttach();

    /** Any descendent plugin should override this virtual method and
      * perform any necessary de-initialization. This method is called by
      * Code::Blocks (PluginManager actually) when the plugin has been
      * loaded, attached and should de-attach from Code::Blocks.\n
      * Think of this method as the actual destructor...
      * @param appShutDown If true, the application is shutting down. In this
      *         case *don't* use Manager::Get()->Get...() functions or the
      *         behaviour is undefined...
      */
    virtual void OnRelease(bool appShutDown);

private:
    struct LineInfo {int executionCount;
                     wxArrayString functionCalls;
                     wxArrayString branchInfos;
                     wxArrayString callInfos;};
    typedef std::map<unsigned int, LineInfo> LineInfos;
    typedef std::map<int, wxArrayString> Output_t;           /**< wxProcessEvent::GetPid(), Log of process output */
    typedef std::map<int, GcovProcess*> GcovProcesses_t;     /**< wxProcessEvent::GetPid(), GcovProcess* */

    unsigned int m_LocalCodeLines;          /**< lines of code per file */
    unsigned int m_LocalCodeLinesCalled;    /**< lines of executed code per file */
    unsigned int m_CodeLines;               /**< lines of code per workspace */
    unsigned int m_CodeLinesCalled;         /**< lines of executed code per workspace */
    unsigned int m_ParallelProcessCount;    /**< number of processes for parallel execution */
    Output_t m_Output;                      /**< captured gcov process output */
    std::vector<GcovStats> m_Stats;         /**< statistics computed by cbGcov, not usable for now */
    std::vector<cbProject*> m_Prjs;         /**< projects in queue */
    cbProject* m_Prj;                       /**< currently processed project */
    GcovProcesses_t m_pProcesses;           /**< currently running gcov processes */
    wxString m_JobsWorkingdir;
    wxArrayString m_Cmds;                   /**< array of commands to invode gcov for each file*/
    wxArrayString m_JobsFileList;           /**< files that gcov was run against */
    wxDateTime m_TimeSpan;                  /**< total cbGcov execution time */

    void Log(wxString str);
    void InitTextCtrlForCovData(cbStyledTextCtrl *stc);
    void ShowCovData(cbEditor *ed, cbProject* prj = 0);
    void ShowCovData(cbEditor *ed, LineInfos &lineInfos);
    void ClearCovData(cbEditor *ed);
    void UpdateEditors(cbProject*);

    void GetLineInfos(wxFileName filename, LineInfos &lineInfos);
    void AddInfoFromLine(wxString &line, LineInfos &lineInfos);

    void GcovProject(cbProject*);
    void GetStats(cbProject*);

    void Initialize();
    int IsRunning();
    int WorkspaceNextProject();

    const wxString GetGcovBinaryName();
    void StartGcov();
    void StartGcovParallel();
    void CleanProject(cbProject* prj);

    void OnAddInstrumentationToProject(wxCommandEvent &event);
    void OnUpdateAddInstrumentationToProject(wxUpdateUIEvent& event);
    void OnDoGcov(wxCommandEvent &event);
    void OnUpdateGcov(wxUpdateUIEvent& event);
    void OnDoGcovWorkspace(wxCommandEvent &event);
    void OnUpdateGcovWorkspace(wxUpdateUIEvent& event);

    void OnEditorOpen(CodeBlocksEvent &event);
    void OnEditorModified(CodeBlocksEvent &event);
    void OnCleanProject(CodeBlocksEvent &event);
    void OnCleanWorkspace(CodeBlocksEvent &event);
    void OnProjectActivate(CodeBlocksEvent &event);
    void OnGcovReturned(wxProcessEvent &event);
    void OnIdle(wxIdleEvent& event);

    DECLARE_EVENT_TABLE();
    cbGcovConfig *config_;
public:
    void OnProcessGeneratedOutputLine(const wxString &line, unsigned int id);

    void UpdateConfig();

};

#endif // CBGCOV_H_INCLUDED
