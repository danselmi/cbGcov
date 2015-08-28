#include "GcovProcess.h"

#include "cbGcov.h"
#include <wx/txtstrm.h>


GcovProcess::GcovProcess(cbGcov *parent):
    wxProcess(parent)
{
    //ctor
    wxASSERT(parent);
    m_parent = parent;

    //turn on redirect
    Redirect();
    m_Id = 0;
}

GcovProcess::~GcovProcess()
{
    //dtor
}

/**
 * @brief Sets identifier for the process
 *
 */
void GcovProcess::SetId(unsigned int id)
{
    m_Id = id;
}

/**
 * @brief Reads the process stdoutput
 *
 */
bool GcovProcess::ReadProcessOutput()
{

    //wxProcess::GetInputStream() will capture stdout!
    if (IsInputAvailable())
    {
        wxTextInputStream ts(*GetInputStream());
        wxString line = ts.ReadLine();

        if(line.Length())
        {
            m_parent->OnProcessGeneratedOutputLine(line,m_Id);
        }
        return true;
    }
    return false;
}
