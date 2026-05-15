#pragma once

#include <wx/wx.h>
#include <wx/stc/stc.h>
#include <wx/print.h>

class FindReplaceDialog;
class Config;

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title, const wxString& fileToOpen = wxT(""));
    ~MainFrame();

private:
    wxStyledTextCtrl* textCtrl;
    wxStatusBar* statusBar;
    wxString currentFile;
    bool isModified;
    FindReplaceDialog* findReplaceDialog;
    Config* config;
    wxPrintData printData;
    wxPageSetupDialogData pageSetupData;
    bool wordWrapEnabled;
    bool lineNumbersEnabled;
    wxFont currentFont;

    void InitUI();
    void CreateMenuBar();
    void SetupStatusBar();
    void ConfigureTextCtrl();
    void UpdateStatusBar();
    void UpdateTitle();
    void LoadFile(const wxString& filename);
    void SaveFile(const wxString& filename);

    void OnNew(wxCommandEvent& event);
    void OnNewWindow(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnPageSetup(wxCommandEvent& event);
    void OnPrint(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);

    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);
    void OnCut(wxCommandEvent& event);
    void OnCopy(wxCommandEvent& event);
    void OnPaste(wxCommandEvent& event);
    void OnSelectAll(wxCommandEvent& event);
    void OnFind(wxCommandEvent& event);
    void OnReplace(wxCommandEvent& event);

    void OnFont(wxCommandEvent& event);
    void OnWordWrap(wxCommandEvent& event);
    void OnLineNumbers(wxCommandEvent& event);

    void OnAbout(wxCommandEvent& event);

    void OnTextModified(wxStyledTextEvent& event);
    void OnTextUpdateUI(wxStyledTextEvent& event);
    void OnClose(wxCloseEvent& event);

    bool PromptSaveIfModified();

    wxDECLARE_EVENT_TABLE();
};
