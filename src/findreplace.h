#pragma once

#include <wx/wx.h>
#include <wx/stc/stc.h>
#include <wx/notebook.h>

class FindReplaceDialog : public wxDialog {
public:
    FindReplaceDialog(wxWindow* parent, wxStyledTextCtrl* textCtrl);

private:
    wxStyledTextCtrl* textCtrl;
    wxNotebook* notebook;
    wxTextCtrl* findText;
    wxTextCtrl* replaceText;
    wxCheckBox* matchCaseCheckbox;
    wxRadioButton* forwardRadio;
    wxRadioButton* backwardRadio;
    int lastFoundPos;

    void OnFind(wxCommandEvent& event);
    void OnFindAll(wxCommandEvent& event);
    void OnReplace(wxCommandEvent& event);
    void OnReplaceAll(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);

    int FindText(const wxString& text, bool matchCase, bool forward);

    wxDECLARE_EVENT_TABLE();
};
