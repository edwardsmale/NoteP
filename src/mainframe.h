#pragma once

#include <wx/wx.h>
#include <wx/stc/stc.h>
#include <wx/print.h>
#include <wx/dnd.h>

class FindReplaceDialog;
class Config;

class FileDropTarget : public wxFileDropTarget {
public:
    FileDropTarget(class MainFrame* owner) : owner(owner) {}
    bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames) override;

private:
    class MainFrame* owner;
};

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title, const wxString& fileToOpen = wxT(""));
    ~MainFrame();

    void LoadFile(const wxString& filename);
    bool PromptSaveIfModified();

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
    wxMBConv* fileEncoding;
    bool fileBOM;
    bool fileEncodingOwned;
    wxFontEncoding fileEncodingType;

    void InitUI();
    void CreateMenuBar();
    void SetupStatusBar();
    void ConfigureTextCtrl();
    void UpdateStatusBar();
    void UpdateTitle();
    void SaveFile(const wxString& filename);
    wxMBConv* DetectFileEncoding(const wxString& filename, bool& hasBOM, int& bomSize, wxFontEncoding& encodingType);
    wxString GetEncodingName(wxFontEncoding encodingType, bool bom);

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
    void OnClearAll(wxCommandEvent& event);
    void OnCopyAll(wxCommandEvent& event);
    void OnCloseWithout(wxCommandEvent& event);

    void OnAbout(wxCommandEvent& event);

    void OnTextModified(wxStyledTextEvent& event);
    void OnTextUpdateUI(wxStyledTextEvent& event);
    void OnFrameKeyDown(wxKeyEvent& event);
    void OnMoving(wxMoveEvent& event);
    void OnClose(wxCloseEvent& event);

    wxDECLARE_EVENT_TABLE();
};
