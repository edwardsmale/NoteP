#include "app.h"
#include "mainframe.h"

bool NotepApp::OnInit() {
    wxString fileToOpen = wxT("");

    // Check if a file path was provided as a command-line argument
    if (argc > 1) {
        fileToOpen = argv[1];
    }

    MainFrame* frame = new MainFrame(wxT("NoteP"), fileToOpen);
    frame->Show();
    return true;
}
