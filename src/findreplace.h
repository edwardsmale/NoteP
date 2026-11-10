#pragma once

#include <wx/wx.h>
#include <wx/stc/stc.h>
#include <wx/notebook.h>

class FindReplaceDialog : public wxDialog {
public:
    FindReplaceDialog(wxWindow* parent, wxStyledTextCtrl* textCtrl);

    void ShowFindTab();
    void ShowReplaceTab();

    // Repeats the last search (F3). Returns false if nothing has been searched for yet.
    bool FindNext();

    // Remembers the selected text (or the word at the caret) as the search text
    // and finds its next occurrence (Ctrl+F3).
    void FindSelection();

private:
    // The criteria of the last search, used by FindNext.
    wxString searchText;
    bool searchMatchCase;
    bool searchForward;

    void RememberSearch(const wxString& text, bool matchCase, bool forward);
    void OnFindNextKey(wxCommandEvent& event);
    void OnFindSelectionKey(wxCommandEvent& event);

    wxStyledTextCtrl* textCtrl;
    wxNotebook* notebook;
    wxTextCtrl* findText;
    wxTextCtrl* replaceText;
    wxTextCtrl* replaceWithText;
    wxCheckBox* matchCaseCheckbox;
    wxCheckBox* replaceMatchCaseCheckbox;
    wxRadioButton* forwardRadio;
    wxRadioButton* backwardRadio;
    wxButton* findButton;
    wxButton* replaceButton;
    int lastFoundPos;

    void OnFind(wxCommandEvent& event);
    void OnReplace(wxCommandEvent& event);
    void OnReplaceAll(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);
    void OnShow(wxShowEvent& event);
    void OnPageChanged(wxBookCtrlEvent& event);

    // Makes Enter trigger the right action for the given tab (0 = Find, 1 = Replace).
    void UpdateDefaultButton(int page);

    int FindText(const wxString& text, bool matchCase, bool forward);

    // Moves the dialog out of the way if it covers the line containing matchPos.
    void MoveAwayFromMatch(int matchPos);

    wxDECLARE_EVENT_TABLE();
};
