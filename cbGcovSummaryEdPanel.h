#ifndef CBGCOVSUMMARYEDPANEL_H
#define CBGCOVSUMMARYEDPANEL_H

#include "editorbase.h"
#include "cbGcovSummaryPanel.h"

class cbGcovSummaryEdPanel : public EditorBase
{
    public:
        cbGcovSummaryEdPanel(wxWindow* parent, const Summaries &summaries, const wxString& filename);
        virtual ~cbGcovSummaryEdPanel();

        virtual const wxString& GetFilename() const;

		virtual const wxString& GetShortName() const;
        virtual const wxString& GetTitle();
        virtual bool VisibleToTree() const { return false; }
        virtual bool IsReadOnly() const { return true; }
    protected:

    private:
        static wxString shortName_;
};

#endif
