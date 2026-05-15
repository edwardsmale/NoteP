#pragma once

#include <wx/wx.h>
#include <wx/fileconf.h>

class Config {
public:
    Config();
    ~Config();

    void LoadWindowState(int& width, int& height, int& x, int& y);
    void SaveWindowState(int width, int height, int x, int y);

    void LoadFont(wxString& fontName, int& fontSize);
    void SaveFont(const wxString& fontName, int fontSize);

private:
    wxFileConfig* config;
};
