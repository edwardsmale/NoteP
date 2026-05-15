#include "findreplace.h"
#include <wx/stc/stc.h>

wxBEGIN_EVENT_TABLE(FindReplaceDialog, wxDialog)
    EVT_BUTTON(wxID_FIND, FindReplaceDialog::OnFind)
    EVT_BUTTON(wxID_REPLACE, FindReplaceDialog::OnReplace)
    EVT_BUTTON(wxID_CLOSE, FindReplaceDialog::OnClose)
wxEND_EVENT_TABLE()

FindReplaceDialog::FindReplaceDialog(wxWindow* parent, wxStyledTextCtrl* tc)
    : wxDialog(parent, wxID_ANY, wxT("Find & Replace"), wxDefaultPosition, wxSize(400, 250),
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      textCtrl(tc), lastFoundPos(0) {

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    notebook = new wxNotebook(this, wxID_ANY);

    // Find tab
    wxPanel* findPanel = new wxPanel(notebook);
    wxBoxSizer* findSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* findTextSizer = new wxBoxSizer(wxHORIZONTAL);
    findTextSizer->Add(new wxStaticText(findPanel, wxID_ANY, wxT("Find:")), 0,
                       wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    findText = new wxTextCtrl(findPanel, wxID_ANY);
    findTextSizer->Add(findText, 1, wxEXPAND);
    findSizer->Add(findTextSizer, 0, wxEXPAND | wxBOTTOM, 10);

    matchCaseCheckbox = new wxCheckBox(findPanel, wxID_ANY, wxT("Match Case"));
    findSizer->Add(matchCaseCheckbox, 0, wxBOTTOM, 10);

    wxBoxSizer* directionSizer = new wxBoxSizer(wxHORIZONTAL);
    directionSizer->Add(new wxStaticText(findPanel, wxID_ANY, wxT("Direction:")), 0,
                        wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    forwardRadio = new wxRadioButton(findPanel, wxID_ANY, wxT("Forward"), wxDefaultPosition,
                                      wxDefaultSize, wxRB_GROUP);
    forwardRadio->SetValue(true);
    directionSizer->Add(forwardRadio, 0, wxRIGHT, 20);
    backwardRadio = new wxRadioButton(findPanel, wxID_ANY, wxT("Backward"));
    directionSizer->Add(backwardRadio, 0);
    findSizer->Add(directionSizer, 0, wxBOTTOM, 10);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* findBtn = new wxButton(findPanel, wxID_FIND, wxT("Find Next"));
    wxButton* findAllBtn = new wxButton(findPanel, wxID_ANY, wxT("Find All"));
    buttonSizer->Add(findBtn, 0, wxRIGHT, 5);
    buttonSizer->Add(findAllBtn, 0, wxRIGHT, 5);
    findSizer->Add(buttonSizer, 0, wxBOTTOM, 10);

    findPanel->SetSizer(findSizer);
    notebook->AddPage(findPanel, wxT("Find"));

    // Replace tab
    wxPanel* replacePanel = new wxPanel(notebook);
    wxBoxSizer* replaceSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* findTextSizer2 = new wxBoxSizer(wxHORIZONTAL);
    findTextSizer2->Add(new wxStaticText(replacePanel, wxID_ANY, wxT("Find:")), 0,
                        wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    replaceText = new wxTextCtrl(replacePanel, wxID_ANY);
    findTextSizer2->Add(replaceText, 1, wxEXPAND);
    replaceSizer->Add(findTextSizer2, 0, wxEXPAND | wxBOTTOM, 10);

    wxBoxSizer* replaceWithSizer = new wxBoxSizer(wxHORIZONTAL);
    replaceWithSizer->Add(new wxStaticText(replacePanel, wxID_ANY, wxT("Replace with:")), 0,
                          wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    wxTextCtrl* replaceWithText = new wxTextCtrl(replacePanel, wxID_ANY);
    replaceWithSizer->Add(replaceWithText, 1, wxEXPAND);
    replaceSizer->Add(replaceWithSizer, 0, wxEXPAND | wxBOTTOM, 10);

    wxBoxSizer* replaceButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* replaceBtn = new wxButton(replacePanel, wxID_REPLACE, wxT("Replace"));
    wxButton* replaceAllBtn = new wxButton(replacePanel, wxID_ANY, wxT("Replace All"));
    replaceButtonSizer->Add(replaceBtn, 0, wxRIGHT, 5);
    replaceButtonSizer->Add(replaceAllBtn, 0, wxRIGHT, 5);
    replaceSizer->Add(replaceButtonSizer, 0, wxBOTTOM, 10);

    replacePanel->SetSizer(replaceSizer);
    notebook->AddPage(replacePanel, wxT("Replace"));

    mainSizer->Add(notebook, 1, wxEXPAND | wxALL, 10);

    wxBoxSizer* closeSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* closeBtn = new wxButton(this, wxID_CLOSE, wxT("Close"));
    closeSizer->Add(closeBtn, 0);
    mainSizer->Add(closeSizer, 0, wxALIGN_RIGHT | wxALL, 10);

    SetSizer(mainSizer);
}

void FindReplaceDialog::OnFind(wxCommandEvent& event) {
    if (!findText || findText->GetValue().IsEmpty()) {
        return;
    }

    bool forward = forwardRadio->GetValue();
    bool matchCase = matchCaseCheckbox->GetValue();
    FindText(findText->GetValue(), matchCase, forward);
}

void FindReplaceDialog::OnFindAll(wxCommandEvent& event) {
    // Placeholder for find all functionality
}

void FindReplaceDialog::OnReplace(wxCommandEvent& event) {
    // Placeholder for replace functionality
}

void FindReplaceDialog::OnReplaceAll(wxCommandEvent& event) {
    // Placeholder for replace all functionality
}

void FindReplaceDialog::OnClose(wxCommandEvent& event) {
    EndModal(wxID_CLOSE);
}

int FindReplaceDialog::FindText(const wxString& text, bool matchCase, bool forward) {
    if (!textCtrl || text.IsEmpty()) {
        return -1;
    }

    int flags = 0;
    if (matchCase) {
        flags |= wxSTC_FIND_MATCHCASE;
    }

    int currentPos = textCtrl->GetCurrentPos();
    int result = -1;

    if (forward) {
        result = textCtrl->FindText(currentPos, textCtrl->GetLastPosition(), text, flags);
    } else {
        result = textCtrl->FindText(currentPos, 0, text, flags);
    }

    if (result != -1) {
        textCtrl->SetSelection(result, result + text.Len());
        textCtrl->EnsureVisible(textCtrl->LineFromPosition(result));
    }

    return result;
}
