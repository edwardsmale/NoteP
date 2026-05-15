#include "config.h"
#include <wx/stdpaths.h>

Config::Config() {
    wxString configPath = wxStandardPaths::Get().GetUserDataDir();
    wxFileName::Mkdir(configPath, 0700, wxPATH_MKDIR_FULL);
    config = new wxFileConfig(wxT("NoteP"), wxT(""),
                             configPath + wxFILE_SEP_PATH + wxT("config"));
}

Config::~Config() {
    if (config) {
        config->Flush();
        delete config;
    }
}

void Config::LoadWindowState(int& width, int& height, int& x, int& y) {
    config->SetPath(wxT("/Window"));
    width = config->Read(wxT("Width"), 800L);
    height = config->Read(wxT("Height"), 600L);
    x = config->Read(wxT("X"), 50L);
    y = config->Read(wxT("Y"), 50L);
}

void Config::SaveWindowState(int width, int height, int x, int y) {
    config->SetPath(wxT("/Window"));
    config->Write(wxT("Width"), width);
    config->Write(wxT("Height"), height);
    config->Write(wxT("X"), x);
    config->Write(wxT("Y"), y);
    config->Flush();
}

void Config::LoadFont(wxString& fontName, int& fontSize) {
    config->SetPath(wxT("/Font"));
    fontName = config->Read(wxT("Name"), wxT("Consolas"));
    fontSize = config->Read(wxT("Size"), 10L);
}

void Config::SaveFont(const wxString& fontName, int fontSize) {
    config->SetPath(wxT("/Font"));
    config->Write(wxT("Name"), fontName);
    config->Write(wxT("Size"), fontSize);
    config->Flush();
}
