#ifndef GCOVPROCESS_H
#define GCOVPROCESS_H

#include <wx/process.h>

class cbGcov;

/**
 * @brief Overrides wxProcess to capture gcov output
 *
 */
class GcovProcess : public wxProcess
{
public:
    GcovProcess(cbGcov *parent);
    virtual ~GcovProcess();
protected:
private:
    cbGcov *m_parent;
    unsigned int m_Id;
public:
    bool ReadProcessOutput();
    void SetId(unsigned int id);
};

#endif // GCOVPROCESS_H
