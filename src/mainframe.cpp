#include "mainframe.h"
#include "findreplace.h"
#include "config.h"
#include <wx/stc/stc.h>
#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/fontdlg.h>
#include <wx/tokenzr.h>
#include <wx/stdpaths.h>
#include <fstream>
#include <sstream>
#ifdef __WXMSW__
#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif

// Create a simple Notepad-like icon
wxIcon CreateNotepadIcon() {
    wxBitmap bitmap(16, 16);
    wxMemoryDC dc(bitmap);

    // Fill with light gray background
    dc.SetBrush(wxBrush(*wxWHITE));
    dc.SetPen(wxPen(*wxBLACK, 1));
    dc.DrawRectangle(1, 1, 14, 14);

    // Draw lines to represent text (like notepad)
    dc.SetPen(wxPen(*wxBLACK, 1));
    dc.DrawLine(3, 4, 13, 4);
    dc.DrawLine(3, 6, 13, 6);
    dc.DrawLine(3, 8, 13, 8);
    dc.DrawLine(3, 10, 10, 10);

    dc.SelectObject(wxNullBitmap);

    wxIcon icon;
    icon.CopyFromBitmap(bitmap);
    return icon;
}

class TextPrintout : public wxPrintout {
public:
    TextPrintout(const wxString& title, const wxString& text)
        : wxPrintout(title), text(text) {
    }

    bool OnPrintPage(int) override {
        wxDC* dc = GetDC();
        if (!dc) return false;

        int pageWidth, pageHeight;
        GetPageSizePixels(&pageWidth, &pageHeight);

        // Get printer DPI and scale font accordingly
        int printerPPI_X, printerPPI_Y;
        GetPPIPrinter(&printerPPI_X, &printerPPI_Y);

        // Use a large base font size (72pt) and scale by printer DPI
        // Typical screen is 96 DPI, printer is 300+ DPI
        int scaledFontSize = 72;
        wxFont font(scaledFontSize, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        dc->SetFont(font);

        // Draw the text at the top of the page with margins
        int margin = 500;
        int y = margin;
        dc->DrawText(text, margin, y);

        return true;
    }

    bool OnBeginDocument(int startPage, int endPage) override {
        return wxPrintout::OnBeginDocument(startPage, endPage);
    }

    void GetPageInfo(int* minPage, int* maxPage, int* selPageFrom, int* selPageTo) override {
        *minPage = 1;
        *maxPage = 1;
        *selPageFrom = 1;
        *selPageTo = 1;
    }

private:
    wxString text;
};


enum {
    ID_NEW = 1,
    ID_NEWWINDOW,
    ID_OPEN,
    ID_SAVE,
    ID_SAVEAS,
    ID_PAGESETUP,
    ID_PRINT,
    ID_EXIT,
    ID_UNDO,
    ID_REDO,
    ID_CUT,
    ID_COPY,
    ID_PASTE,
    ID_SELECTALL,
    ID_FIND,
    ID_REPLACE,
    ID_FONT,
    ID_WORDWRAP,
    ID_LINENUMBERS,
    ID_CLEARALL,
    ID_COPYALL,
    ID_ABOUT,
};

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(ID_NEW, MainFrame::OnNew)
    EVT_MENU(ID_NEWWINDOW, MainFrame::OnNewWindow)
    EVT_MENU(ID_OPEN, MainFrame::OnOpen)
    EVT_MENU(ID_SAVE, MainFrame::OnSave)
    EVT_MENU(ID_SAVEAS, MainFrame::OnSaveAs)
    EVT_MENU(ID_PAGESETUP, MainFrame::OnPageSetup)
    EVT_MENU(ID_PRINT, MainFrame::OnPrint)
    EVT_MENU(ID_EXIT, MainFrame::OnExit)
    EVT_MENU(ID_UNDO, MainFrame::OnUndo)
    EVT_MENU(ID_REDO, MainFrame::OnRedo)
    EVT_MENU(ID_CUT, MainFrame::OnCut)
    EVT_MENU(ID_COPY, MainFrame::OnCopy)
    EVT_MENU(ID_PASTE, MainFrame::OnPaste)
    EVT_MENU(ID_SELECTALL, MainFrame::OnSelectAll)
    EVT_MENU(ID_FIND, MainFrame::OnFind)
    EVT_MENU(ID_REPLACE, MainFrame::OnReplace)
    EVT_MENU(ID_FONT, MainFrame::OnFont)
    EVT_MENU(ID_WORDWRAP, MainFrame::OnWordWrap)
    EVT_MENU(ID_LINENUMBERS, MainFrame::OnLineNumbers)
    EVT_MENU(ID_CLEARALL, MainFrame::OnClearAll)
    EVT_MENU(ID_COPYALL, MainFrame::OnCopyAll)
    EVT_MENU(ID_ABOUT, MainFrame::OnAbout)
    EVT_STC_MODIFIED(wxID_ANY, MainFrame::OnTextModified)
    EVT_STC_UPDATEUI(wxID_ANY, MainFrame::OnTextUpdateUI)
    EVT_KEY_DOWN(MainFrame::OnFrameKeyDown)
    EVT_MOVING(MainFrame::OnMoving)
    EVT_CLOSE(MainFrame::OnClose)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(const wxString& title, const wxString& fileToOpen)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)),
      isModified(false), findReplaceDialog(NULL), currentFile(wxT("")), wordWrapEnabled(false),
      lineNumbersEnabled(false), fileEncoding(NULL), fileBOM(false), fileEncodingOwned(false), fileEncodingType(wxFONTENCODING_UTF8) {

    // Initialize with default font
    currentFont = wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Consolas"));

    config = new Config();
    InitUI();
    CreateMenuBar();
    SetupStatusBar();
    ConfigureTextCtrl();

    // Set the Notepad icon
    wxIcon icon;
    wxString exePath = wxStandardPaths::Get().GetExecutablePath();
    wxString exeDir = exePath.BeforeLast(wxFILE_SEP_PATH);

    // Try multiple locations for the icon
    wxArrayString iconPaths;
    iconPaths.Add(exeDir + wxFILE_SEP_PATH + wxT("icon.ico"));  // Same dir as exe
    iconPaths.Add(exeDir + wxFILE_SEP_PATH + wxT("..") + wxFILE_SEP_PATH + wxT("..") + wxFILE_SEP_PATH + wxT("icon.ico"));  // Project root

    bool iconLoaded = false;
    for (size_t i = 0; i < iconPaths.GetCount(); i++) {
        if (wxFileExists(iconPaths[i])) {
            icon.LoadFile(iconPaths[i], wxBITMAP_TYPE_ICO);
            SetIcon(icon);
            iconLoaded = true;
            break;
        }
    }

    if (!iconLoaded) {
        // Fallback to programmatic icon if file not found
        SetIcon(CreateNotepadIcon());
    }

    // Set window to 800x600 on load
    SetSize(800, 600);
    int width, height, x, y;
    config->LoadWindowState(width, height, x, y);
    SetPosition(wxPoint(x, y));

    wxString fontName;
    int fontSize;
    config->LoadFont(fontName, fontSize);

    // Ensure we have valid font values
    if (!fontName.IsEmpty() && fontSize > 0) {
        currentFont = wxFont(fontSize, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, fontName);
    }

    textCtrl->StyleSetFont(wxSTC_STYLE_DEFAULT, currentFont);

    // Open file if provided as argument
    if (!fileToOpen.IsEmpty() && wxFileExists(fileToOpen)) {
        LoadFile(fileToOpen);
    }

#ifdef __WXMSW__
    // Disable rounded corners on Windows 11
    HWND hwnd = (HWND)GetHandle();
    if (hwnd) {
        enum DWMWINDOWATTRIBUTE {
            DWMWA_WINDOW_CORNER_PREFERENCE = 33
        };
        enum DWM_WINDOW_CORNER_PREFERENCE {
            DWMWCP_DEFAULT = 0,
            DWMWCP_DONOTROUND = 1,
            DWMWCP_ROUND = 2,
            DWMWCP_ROUNDSMALL = 3
        };

        DWM_WINDOW_CORNER_PREFERENCE cornerPreference = DWMWCP_DONOTROUND;
        DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &cornerPreference, sizeof(cornerPreference));
    }
#endif
}

MainFrame::~MainFrame() {
    if (fileEncoding && fileEncodingOwned) {
        delete fileEncoding;
    }
    if (config) {
        int width, height, x, y;
        GetSize(&width, &height);
        GetPosition(&x, &y);
        config->SaveWindowState(width, height, x, y);

        config->SaveFont(currentFont.GetFaceName(), currentFont.GetPointSize());

        delete config;
    }
    if (findReplaceDialog) {
        findReplaceDialog->Destroy();
    }
}

void MainFrame::InitUI() {
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    textCtrl = new wxStyledTextCtrl(this);

    // Bind right-click context menu
    textCtrl->Bind(wxEVT_RIGHT_UP, [this](wxMouseEvent&) {
        wxMenu contextMenu;
        contextMenu.Append(ID_CUT, wxT("Cu&t\tCtrl+X"));
        contextMenu.Append(ID_COPY, wxT("&Copy\tCtrl+C"));
        contextMenu.Append(ID_PASTE, wxT("&Paste\tCtrl+V"));
        contextMenu.AppendSeparator();
        contextMenu.Append(ID_SELECTALL, wxT("Select &All\tCtrl+A"));
        contextMenu.AppendSeparator();
        contextMenu.Append(ID_CLEARALL, wxT("C&lear all"));
        contextMenu.Append(ID_COPYALL, wxT("C&opy all"));
        PopupMenu(&contextMenu);
    });

    // Bind F2 and F5 key event handlers to text control
    textCtrl->Bind(wxEVT_KEY_DOWN, [this](wxKeyEvent& event) {
        if (event.GetKeyCode() == WXK_F2) {
            // F2: Reload file from disk
            if (currentFile.IsEmpty()) {
                wxMessageBox(wxT("No file is currently open."), wxT("Reload"), wxOK | wxICON_INFORMATION);
                return;
            }

            if (isModified) {
                wxMessageDialog dlg(this,
                    wxT("The current file has been modified. Reload and discard changes?"),
                    wxT("Reload File"),
                    wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
                if (dlg.ShowModal() != wxID_YES) {
                    return;
                }
            }

            // Read new file content and detect encoding
            std::ifstream file(currentFile.ToStdString(), std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                wxMessageBox(wxT("Could not open file"), wxT("Error"), wxOK | wxICON_ERROR);
                return;
            }

            std::streamsize fileSize = file.tellg();
            file.seekg(0, std::ios::beg);
            file.rdbuf()->pubsetbuf(nullptr, 256 * 1024);

            std::string newContentBuffer(fileSize, '\0');
            file.read(&newContentBuffer[0], fileSize);
            file.close();

            // Detect encoding for comparison
            int newBomSize = 0;
            bool newHasBOM = false;
            wxFontEncoding newEncodingType;
            wxMBConv* newEncoding = DetectFileEncoding(currentFile, newHasBOM, newBomSize, newEncodingType);

            // Convert new content to wxString for comparison
            wxString newContent;
            if (newBomSize > 0 && fileSize > static_cast<std::streamsize>(newBomSize)) {
                const char* contentStart = newContentBuffer.c_str() + newBomSize;
                size_t contentSize = fileSize - newBomSize;
                newContent = wxString(contentStart, *newEncoding, contentSize);
            } else {
                newContent = wxString(newContentBuffer.c_str(), *newEncoding, fileSize);
            }
            delete newEncoding;

            wxString currentContent = textCtrl->GetValue();

            // Check if content is identical
            if (newContent == currentContent) {
                return;  // No changes, do nothing
            }

            // For large file append optimization, compare prefixes of current content
            if (newContent.size() > currentContent.size() &&
                newContent.substr(0, currentContent.size()) == currentContent) {
                // Just append the new content - optimized for speed
                wxString appendedText = newContent.substr(currentContent.size());

                // Disable updates during append for speed
                textCtrl->BeginUndoAction();
                int insertPos = textCtrl->GetLength();
                textCtrl->InsertText(insertPos, appendedText);
                textCtrl->EndUndoAction();

                // Move cursor to end to show new content
                textCtrl->SetCurrentPos(insertPos + appendedText.size());
                textCtrl->SetAnchor(insertPos + appendedText.size());

                isModified = false;
                return;
            }

            // Full reload needed - content has changed or was truncated
            int firstVisibleLine = textCtrl->GetFirstVisibleLine();
            int currentPos = textCtrl->GetCurrentPos();

            LoadFile(currentFile);

            // Restore scroll position and cursor
            if (firstVisibleLine >= 0) {
                textCtrl->SetFirstVisibleLine(firstVisibleLine);
            }
            if (currentPos >= 0 && currentPos <= (int)textCtrl->GetLength()) {
                textCtrl->SetCurrentPos(currentPos);
                textCtrl->SetAnchor(currentPos);
            }

            return;
        }

        if (event.GetKeyCode() == WXK_F5) {
            wxDateTime now = wxDateTime::Now();

            // Get Windows locale to determine date/time format
            wxLocale locale(wxLANGUAGE_DEFAULT);
            int lang = locale.GetLanguage();

            // Determine format based on locale
            wxString dateStr, timeStr;

            // Use 24-hour or 12-hour time based on locale
            bool use12Hour = (lang == wxLANGUAGE_ENGLISH_US);

            if (use12Hour) {
                timeStr = now.Format(wxT("%I:%M %p"));
            } else {
                timeStr = now.Format(wxT("%H:%M"));
            }

            // Use MM/DD/YYYY for US, DD/MM/YYYY for others
            if (lang == wxLANGUAGE_ENGLISH_US) {
                dateStr = now.Format(wxT("%m/%d/%Y"));
            } else {
                dateStr = now.Format(wxT("%d/%m/%Y"));
            }

            wxString dateTime = timeStr + wxT(" ") + dateStr;

            int currentPos = textCtrl->GetCurrentPos();
            textCtrl->InsertText(currentPos, dateTime);
            // Clear selection and position cursor after inserted text
            textCtrl->SetSelection(currentPos + dateTime.Length(), currentPos + dateTime.Length());
            textCtrl->SetCurrentPos(currentPos + dateTime.Length());

            isModified = true;
            UpdateTitle();
            return;
        }
        event.Skip();
    });

    sizer->Add(textCtrl, 1, wxEXPAND);
    SetSizer(sizer);
}

void MainFrame::CreateMenuBar() {
    wxMenuBar* menuBar = new wxMenuBar();

    // File menu
    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(ID_NEW, wxT("&New\tCtrl+N"), wxT("Create a new file"));
    fileMenu->Append(ID_NEWWINDOW, wxT("New &Window\tCtrl+Shift+N"), wxT("Create a new window"));
    fileMenu->Append(ID_OPEN, wxT("&Open...\tCtrl+O"), wxT("Open a file"));
    fileMenu->Append(ID_SAVE, wxT("&Save\tCtrl+S"), wxT("Save the current file"));
    fileMenu->Append(ID_SAVEAS, wxT("Save &As...\tCtrl+Shift+S"), wxT("Save file with a new name"));
    fileMenu->AppendSeparator();
    fileMenu->Append(ID_PAGESETUP, wxT("Page Set&up..."), wxT("Setup page printing options"));
    fileMenu->Append(ID_PRINT, wxT("&Print...\tCtrl+P"), wxT("Print the document"));
    fileMenu->AppendSeparator();
    fileMenu->Append(ID_EXIT, wxT("E&xit"), wxT("Exit the application"));
    menuBar->Append(fileMenu, wxT("&File"));

    // Edit menu
    wxMenu* editMenu = new wxMenu();
    editMenu->Append(ID_UNDO, wxT("&Undo\tCtrl+Z"), wxT("Undo the last action"));
    editMenu->Append(ID_REDO, wxT("&Redo\tCtrl+Y"), wxT("Redo the last undone action"));
    editMenu->AppendSeparator();
    editMenu->Append(ID_CUT, wxT("Cu&t\tCtrl+X"), wxT("Cut selected text"));
    editMenu->Append(ID_COPY, wxT("&Copy\tCtrl+C"), wxT("Copy selected text"));
    editMenu->Append(ID_PASTE, wxT("&Paste\tCtrl+V"), wxT("Paste text from clipboard"));
    editMenu->AppendSeparator();
    editMenu->Append(ID_SELECTALL, wxT("Select &All\tCtrl+A"), wxT("Select all text"));
    editMenu->AppendSeparator();
    editMenu->Append(ID_FIND, wxT("&Find\tCtrl+F"), wxT("Find text"));
    editMenu->Append(ID_REPLACE, wxT("&Replace\tCtrl+H"), wxT("Find and replace text"));
    menuBar->Append(editMenu, wxT("&Edit"));

    // Format menu
    wxMenu* formatMenu = new wxMenu();
    formatMenu->Append(ID_FONT, wxT("&Font..."), wxT("Change font"));
    formatMenu->AppendCheckItem(ID_WORDWRAP, wxT("&Word Wrap"), wxT("Toggle word wrap"));
    menuBar->Append(formatMenu, wxT("F&ormat"));

    // View menu
    wxMenu* viewMenu = new wxMenu();
    viewMenu->AppendCheckItem(ID_LINENUMBERS, wxT("&Line Numbers"), wxT("Toggle line numbers"));
    menuBar->Append(viewMenu, wxT("&View"));

    // Help menu
    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(ID_ABOUT, wxT("&About"), wxT("About NoteP"));
    menuBar->Append(helpMenu, wxT("&Help"));

    SetMenuBar(menuBar);
}

void MainFrame::SetupStatusBar() {
    statusBar = CreateStatusBar(2);
    SetStatusText(wxT("Ready"), 0);
    SetStatusText(wxT("Line: 1  Column: 1"), 1);
}

void MainFrame::ConfigureTextCtrl() {
    textCtrl->SetLexer(wxSTC_LEX_NULL);

    // Setup line numbers (hidden by default)
    textCtrl->SetMarginType(0, wxSTC_MARGIN_NUMBER);
    textCtrl->SetMarginWidth(0, 0);
    textCtrl->StyleSetForeground(wxSTC_STYLE_LINENUMBER, *wxBLACK);
    textCtrl->StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColour(200, 200, 200));

    // Enable folding margin
    textCtrl->SetMarginType(1, wxSTC_MARGIN_SYMBOL);
    textCtrl->SetMarginMask(1, wxSTC_MASK_FOLDERS);
    textCtrl->SetMarginWidth(1, 0);

    // Set font
    wxFont font(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Consolas"));
    textCtrl->StyleSetFont(wxSTC_STYLE_DEFAULT, font);
    textCtrl->StyleSetForeground(wxSTC_STYLE_DEFAULT, *wxBLACK);
    textCtrl->StyleSetBackground(wxSTC_STYLE_DEFAULT, *wxWHITE);
    textCtrl->SetCaretForeground(*wxBLACK);

    // Set tab width
    textCtrl->SetTabWidth(4);
    textCtrl->SetUseTabs(false);

    // Performance optimizations for large files
    textCtrl->SetScrollWidthTracking(false);
    textCtrl->SetIdleStyling(wxSTC_IDLESTYLING_NONE);
    textCtrl->SetLayoutCache(wxSTC_CACHE_PAGE);

    // Disable expensive visual features
    textCtrl->SetViewWhiteSpace(wxSTC_WS_INVISIBLE);
    textCtrl->SetEdgeMode(wxSTC_EDGE_NONE);

    // Configure visual features
    textCtrl->SetCaretPeriod(500);  // Enable caret blinking (500ms period)
    textCtrl->SetCaretWidth(1);     // Thin caret
    textCtrl->SetMultipleSelection(false);  // Simpler selection handling

    // Optimize scrolling performance
    textCtrl->SetScrollWidth(1);
    textCtrl->SetEndAtLastLine(false);

    // Reduce visual updates and rendering
    textCtrl->SetMarginSensitive(0, false);
    textCtrl->SetMarginSensitive(1, false);

    textCtrl->SetFocus();
}

void MainFrame::UpdateStatusBar() {
    long line = textCtrl->GetCurrentLine() + 1;
    long col = textCtrl->GetColumn(textCtrl->GetCurrentPos()) + 1;
    wxString encoding = GetEncodingName(fileEncodingType, fileBOM);
    wxString status;
    status.Printf(wxT("%s | Line: %ld  Column: %ld"), encoding.c_str(), line, col);
    SetStatusText(status, 1);
}

void MainFrame::UpdateTitle() {
    wxString title;

    if (!currentFile.IsEmpty()) {
        wxFileName fn(currentFile);
        title = fn.GetFullName() + wxT(" - NoteP");
    } else {
        title = wxT("NoteP");
    }

    if (isModified) {
        title = wxT("*") + title;
    }
    SetTitle(title);
}

wxMBConv* MainFrame::DetectFileEncoding(const wxString& filename, bool& hasBOM, int& bomSize, wxFontEncoding& encodingType) {
    bomSize = 0;
    hasBOM = false;
    encodingType = wxFONTENCODING_UTF8;

    std::ifstream file(filename.ToStdString(), std::ios::binary);
    if (!file.is_open()) {
        return &wxConvUTF8;
    }

    unsigned char bom[4] = {0};
    file.read((char*)bom, 4);
    file.close();

    // Check for UTF-8 BOM (EF BB BF)
    if (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {
        hasBOM = true;
        bomSize = 3;
        encodingType = wxFONTENCODING_UTF8;
        return &wxConvUTF8;
    }

    // Check for UTF-16LE BOM (FF FE) - but not UTF-32LE (FF FE 00 00)
    if (bom[0] == 0xFF && bom[1] == 0xFE && !(bom[2] == 0x00 && bom[3] == 0x00)) {
        hasBOM = true;
        bomSize = 2;
        encodingType = wxFONTENCODING_UTF16LE;
        return new wxCSConv(wxFONTENCODING_UTF16LE);
    }

    // Check for UTF-16BE BOM (FE FF) - but not UTF-32BE (00 00 FE FF)
    if (bom[0] == 0xFE && bom[1] == 0xFF && !(bom[2] == 0x00 && bom[3] == 0x00)) {
        hasBOM = true;
        bomSize = 2;
        encodingType = wxFONTENCODING_UTF16BE;
        return new wxCSConv(wxFONTENCODING_UTF16BE);
    }

    // Check for UTF-32LE BOM (FF FE 00 00)
    if (bom[0] == 0xFF && bom[1] == 0xFE && bom[2] == 0x00 && bom[3] == 0x00) {
        hasBOM = true;
        bomSize = 4;
        encodingType = wxFONTENCODING_UTF32LE;
        return new wxCSConv(wxFONTENCODING_UTF32LE);
    }

    // Check for UTF-32BE BOM (00 00 FE FF)
    if (bom[0] == 0x00 && bom[1] == 0x00 && bom[2] == 0xFE && bom[3] == 0xFF) {
        hasBOM = true;
        bomSize = 4;
        encodingType = wxFONTENCODING_UTF32BE;
        return new wxCSConv(wxFONTENCODING_UTF32BE);
    }

    // No BOM found - default to UTF-8 without BOM
    return &wxConvUTF8;
}

wxString MainFrame::GetEncodingName(wxFontEncoding encodingType, bool bom) {
    wxString name;

    switch (encodingType) {
        case wxFONTENCODING_UTF16LE:
            name = wxT("UTF-16LE");
            break;
        case wxFONTENCODING_UTF16BE:
            name = wxT("UTF-16BE");
            break;
        case wxFONTENCODING_UTF32LE:
            name = wxT("UTF-32LE");
            break;
        case wxFONTENCODING_UTF32BE:
            name = wxT("UTF-32BE");
            break;
        default:
            name = wxT("UTF-8");
            break;
    }

    if (bom) {
        name += wxT(" BOM");
    }

    return name;
}

void MainFrame::LoadFile(const wxString& filename) {
    std::ifstream file(filename.ToStdString(), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        wxMessageBox(wxT("Could not open file"), wxT("Error"), wxOK | wxICON_ERROR);
        return;
    }

    // Check file size
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Disable undo for very large files to save memory
    bool largeFile = (fileSize > 10 * 1024 * 1024);
    if (largeFile) {
        textCtrl->SetUndoCollection(false);
    }

    // Detect file encoding
    int bomSize = 0;
    wxMBConv* encoding = DetectFileEncoding(filename, fileBOM, bomSize, fileEncodingType);

    // Clean up old encoding if it exists and was owned
    if (fileEncoding && fileEncodingOwned) {
        delete fileEncoding;
    }
    fileEncoding = encoding;
    fileEncodingOwned = (encoding != &wxConvUTF8);

    // Optimize file reading with larger buffer
    file.rdbuf()->pubsetbuf(nullptr, 256 * 1024);  // 256KB buffer

    // Fast binary read directly into string
    std::string buffer(fileSize, '\0');
    file.read(&buffer[0], fileSize);
    file.close();

    // Show busy cursor and disable updates for large files
    wxBusyCursor wait;
    textCtrl->BeginUndoAction();

    // Convert buffer to wxString using detected encoding, skipping BOM if present
    wxString content;
    if (bomSize > 0 && fileSize > static_cast<std::streamsize>(bomSize)) {
        // Skip BOM bytes and convert remaining content
        const char* contentStart = buffer.c_str() + bomSize;
        size_t contentSize = fileSize - bomSize;
        content = wxString(contentStart, *fileEncoding, contentSize);
    } else {
        // No BOM, convert entire buffer
        content = wxString(buffer.c_str(), *fileEncoding, fileSize);
    }

    textCtrl->SetText(content);
    textCtrl->EndUndoAction();

    // Re-enable undo after loading
    if (largeFile) {
        textCtrl->SetUndoCollection(true);
    }

    // Move to top and clear any selection
    textCtrl->SetCurrentPos(0);
    textCtrl->SetAnchor(0);

    currentFile = filename;
    isModified = false;
    UpdateTitle();
    UpdateStatusBar();
}

void MainFrame::SaveFile(const wxString& filename) {
    std::ofstream file(filename.ToStdString(), std::ios::binary);
    if (!file.is_open()) {
        wxMessageBox(wxT("Could not save file"), wxT("Error"), wxOK | wxICON_ERROR);
        return;
    }

    wxString content = textCtrl->GetValue();
    wxMBConv* encoding = fileEncoding ? fileEncoding : &wxConvUTF8;
    wxFontEncoding encodingType = fileEncoding ? fileEncodingType : wxFONTENCODING_UTF8;

    // Write BOM if the original file had one
    if (fileBOM) {
        switch (encodingType) {
            case wxFONTENCODING_UTF8:
                // UTF-8 BOM: EF BB BF
                file.write("\xef\xbb\xbf", 3);
                break;
            case wxFONTENCODING_UTF16LE:
                // UTF-16LE BOM: FF FE
                file.write("\xff\xfe", 2);
                break;
            case wxFONTENCODING_UTF16BE:
                // UTF-16BE BOM: FE FF
                file.write("\xfe\xff", 2);
                break;
            case wxFONTENCODING_UTF32LE:
                // UTF-32LE BOM: FF FE 00 00
                file.write("\xff\xfe\x00\x00", 4);
                break;
            case wxFONTENCODING_UTF32BE:
                // UTF-32BE BOM: 00 00 FE FF
                file.write("\x00\x00\xfe\xff", 4);
                break;
            default:
                break;
        }
    }

    // Convert content using the file's encoding and write to file
    wxCharBuffer encoded = encoding->cWC2MB(content.wc_str());
    if (encoded) {
        file.write(encoded, strlen(encoded));
    }

    file.close();

    currentFile = filename;
    isModified = false;
    UpdateTitle();
}

bool MainFrame::PromptSaveIfModified() {
    if (!isModified) {
        return true;
    }

    wxString message = wxT("Do you want to save changes to ");
    if (currentFile.IsEmpty()) {
        message += wxT("Untitled");
    } else {
        wxFileName fn(currentFile);
        message += fn.GetFullName();
    }
    message += wxT("?");

    int result = wxMessageBox(message, wxT("Save changes?"), wxYES_NO | wxCANCEL | wxICON_QUESTION);
    if (result == wxYES) {
        if (currentFile.IsEmpty()) {
            wxFileDialog dialog(this, wxT("Save File"), wxT(""), wxT(""),
                              wxT("Text files (*.txt)|*.txt|All files (*.*)|*.*"),
                              wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
            if (dialog.ShowModal() != wxID_CANCEL) {
                SaveFile(dialog.GetPath());
                return true;
            }
            return false;
        } else {
            SaveFile(currentFile);
            return true;
        }
    }
    return result != wxCANCEL;
}

void MainFrame::OnNew(wxCommandEvent&) {
    if (!PromptSaveIfModified()) {
        return;
    }

    textCtrl->ClearAll();
    currentFile = wxT("");
    isModified = false;
    UpdateTitle();
    UpdateStatusBar();
}

void MainFrame::OnNewWindow(wxCommandEvent&) {
    MainFrame* frame = new MainFrame(wxT("NoteP - Text Editor"));
    frame->Show();
}

void MainFrame::OnOpen(wxCommandEvent&) {
    if (!PromptSaveIfModified()) {
        return;
    }

    wxFileDialog dialog(this, wxT("Open File"), wxT(""), wxT(""),
                       wxT("Text files (*.txt)|*.txt|All files (*.*)|*.*"),
                       wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dialog.ShowModal() != wxID_CANCEL) {
        LoadFile(dialog.GetPath());
    }
}

void MainFrame::OnSave(wxCommandEvent&) {
    if (currentFile.IsEmpty()) {
        wxFileDialog dialog(this, wxT("Save File"), wxT(""), wxT(""),
                          wxT("Text files (*.txt)|*.txt|All files (*.*)|*.*"),
                          wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dialog.ShowModal() != wxID_CANCEL) {
            SaveFile(dialog.GetPath());
        }
    } else {
        SaveFile(currentFile);
    }
}

void MainFrame::OnSaveAs(wxCommandEvent&) {
    wxFileDialog dialog(this, wxT("Save File As"), wxT(""), wxT(""),
                       wxT("Text files (*.txt)|*.txt|All files (*.*)|*.*"),
                       wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() != wxID_CANCEL) {
        SaveFile(dialog.GetPath());
    }
}

void MainFrame::OnPageSetup(wxCommandEvent&) {
    pageSetupData.SetPrintData(printData);
    wxPageSetupDialog pageSetupDialog(this, &pageSetupData);

    if (pageSetupDialog.ShowModal() == wxID_OK) {
        pageSetupData = pageSetupDialog.GetPageSetupData();
        printData = pageSetupData.GetPrintData();
    }
}

void MainFrame::OnPrint(wxCommandEvent&) {
    wxPrintDialogData printDialogData(printData);
    wxPrinter printer(&printDialogData);
    TextPrintout printout(wxT("NoteP Document"), textCtrl->GetValue());

    if (!printer.Print(this, &printout, true)) {
        if (printer.GetLastError() == wxPRINTER_ERROR) {
            wxMessageBox(wxT("There was a problem printing.\nPerhaps your current printer is not set correctly?"),
                        wxT("Printing"), wxOK | wxICON_ERROR);
        }
    } else {
        printData = printer.GetPrintDialogData().GetPrintData();
    }
}

void MainFrame::OnExit(wxCommandEvent&) {
    Close();
}

void MainFrame::OnUndo(wxCommandEvent&) {
    if (textCtrl->CanUndo()) {
        textCtrl->Undo();
    }
}

void MainFrame::OnRedo(wxCommandEvent&) {
    if (textCtrl->CanRedo()) {
        textCtrl->Redo();
    }
}

void MainFrame::OnCut(wxCommandEvent&) {
    textCtrl->Cut();
}

void MainFrame::OnCopy(wxCommandEvent&) {
    textCtrl->Copy();
}

void MainFrame::OnPaste(wxCommandEvent&) {
    textCtrl->Paste();
}

void MainFrame::OnSelectAll(wxCommandEvent&) {
    textCtrl->SelectAll();
}

void MainFrame::OnFind(wxCommandEvent&) {
    if (!findReplaceDialog) {
        findReplaceDialog = new FindReplaceDialog(this, textCtrl);
    }
    findReplaceDialog->ShowModal();
}

void MainFrame::OnReplace(wxCommandEvent&) {
    if (!findReplaceDialog) {
        findReplaceDialog = new FindReplaceDialog(this, textCtrl);
    }
    findReplaceDialog->ShowModal();
}

void MainFrame::OnFont(wxCommandEvent&) {
    wxFontData fontData;
    fontData.SetInitialFont(currentFont);

    wxFontDialog fontDialog(this, fontData);
    if (fontDialog.ShowModal() == wxID_OK) {
        currentFont = fontDialog.GetFontData().GetChosenFont();
        textCtrl->StyleSetFont(wxSTC_STYLE_DEFAULT, currentFont);
        config->SaveFont(currentFont.GetFaceName(), currentFont.GetPointSize());
    }
}

void MainFrame::OnWordWrap(wxCommandEvent&) {
    wordWrapEnabled = !wordWrapEnabled;
    textCtrl->SetWrapMode(wordWrapEnabled ? wxSTC_WRAP_WORD : wxSTC_WRAP_NONE);
}

void MainFrame::OnLineNumbers(wxCommandEvent&) {
    lineNumbersEnabled = !lineNumbersEnabled;
    textCtrl->SetMarginWidth(0, lineNumbersEnabled ? 40 : 0);
}

void MainFrame::OnClearAll(wxCommandEvent&) {
    textCtrl->ClearAll();
    isModified = true;
    UpdateTitle();
    UpdateStatusBar();
}

void MainFrame::OnCopyAll(wxCommandEvent&) {
    textCtrl->SelectAll();
    textCtrl->Copy();
}

void MainFrame::OnAbout(wxCommandEvent&) {
    wxMessageBox(wxT("NoteP - A Simple Text Editor\nVersion 1.0"),
                wxT("About NoteP"),
                wxOK | wxICON_INFORMATION);
}

void MainFrame::OnTextModified(wxStyledTextEvent&) {
    isModified = true;
    UpdateTitle();
}

void MainFrame::OnTextUpdateUI(wxStyledTextEvent&) {
    UpdateStatusBar();
}

void MainFrame::OnFrameKeyDown(wxKeyEvent& event) {
    event.Skip();
}

void MainFrame::OnMoving(wxMoveEvent& event) {
    // If window is maximized and being dragged, restore it
    if (IsMaximized()) {
        wxPoint pos = event.GetPosition();
        wxSize size = GetSize();

        // Check if being dragged from top (title bar area)
        wxPoint mousePos = wxGetMousePosition();
        int titleBarHeight = 30; // Approximate title bar height

        // If mouse is near the top and window is being moved, restore and reposition
        if (mousePos.y < pos.y + titleBarHeight) {
            // Get stored window size from config or use default
            int storedWidth = 800, storedHeight = 600;
            config->LoadWindowState(storedWidth, storedHeight, storedWidth, storedHeight);

            // Restore the window
            Maximize(false);

            // Position window so cursor stays at same relative position in title bar
            int newX = mousePos.x - (storedWidth / 2);
            int newY = mousePos.y - 15; // Approximate title bar center

            SetPosition(wxPoint(newX, newY));
        }
    }

    event.Skip();
}

void MainFrame::OnClose(wxCloseEvent& event) {
    if (!PromptSaveIfModified()) {
        event.Veto();
        return;
    }
    event.Skip();
}
