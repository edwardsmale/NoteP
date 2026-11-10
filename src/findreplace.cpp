#include "findreplace.h"
#include <wx/stc/stc.h>
#include <wx/display.h>

wxBEGIN_EVENT_TABLE(FindReplaceDialog, wxDialog)
    EVT_BUTTON(wxID_FIND, FindReplaceDialog::OnFind)
    EVT_BUTTON(wxID_REPLACE, FindReplaceDialog::OnReplace)
    EVT_BUTTON(wxID_REPLACE_ALL, FindReplaceDialog::OnReplaceAll)
    EVT_BUTTON(wxID_CLOSE, FindReplaceDialog::OnClose)
    EVT_SHOW(FindReplaceDialog::OnShow)
    EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY, FindReplaceDialog::OnPageChanged)
wxEND_EVENT_TABLE()

FindReplaceDialog::FindReplaceDialog(wxWindow* parent, wxStyledTextCtrl* tc)
    : wxDialog(parent, wxID_ANY, wxT("Find & Replace"), wxDefaultPosition, wxSize(400, 250),
               wxDEFAULT_DIALOG_STYLE & ~wxRESIZE_BORDER),
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

    matchCaseCheckbox = new wxCheckBox(findPanel, wxID_ANY, wxT("Match &case"));
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
    findButton = new wxButton(findPanel, wxID_FIND, wxT("&Find Next"));
    buttonSizer->Add(findButton, 0, wxRIGHT, 5);
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
    replaceWithText = new wxTextCtrl(replacePanel, wxID_ANY);
    replaceWithSizer->Add(replaceWithText, 1, wxEXPAND);
    replaceSizer->Add(replaceWithSizer, 0, wxEXPAND | wxBOTTOM, 10);

    replaceMatchCaseCheckbox = new wxCheckBox(replacePanel, wxID_ANY, wxT("Match &case"));
    replaceSizer->Add(replaceMatchCaseCheckbox, 0, wxBOTTOM, 10);

    wxBoxSizer* replaceButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    replaceButton = new wxButton(replacePanel, wxID_REPLACE, wxT("&Replace"));
    wxButton* replaceAllBtn = new wxButton(replacePanel, wxID_REPLACE_ALL, wxT("Replace &All"));
    replaceButtonSizer->Add(replaceButton, 0, wxRIGHT, 5);
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

    // Size the dialog to fit its tallest tab so no controls get squashed,
    // keeping the intended width, then lock that as the fixed size
    // (the dialog is not resizable).
    mainSizer->Fit(this);
    SetClientSize(400, GetClientSize().GetHeight());
    SetMinSize(GetSize());

    // Pressing Esc closes the dialog (maps to the Close button).
    SetEscapeId(wxID_CLOSE);

    // Position the dialog 120px to the right of its default (centred) spot.
    CentreOnParent();
    SetPosition(GetPosition() + wxPoint(120, 0));

    UpdateDefaultButton(notebook->GetSelection());
}

void FindReplaceDialog::UpdateDefaultButton(int page) {
    // Enter does Find Next on the Find tab, Replace on the Replace tab.
    if (page == 1) {
        replaceButton->SetDefault();
    } else {
        findButton->SetDefault();
    }
}

void FindReplaceDialog::OnPageChanged(wxBookCtrlEvent& event) {
    UpdateDefaultButton(event.GetSelection());
    event.Skip();
}

void FindReplaceDialog::ShowFindTab() {
    notebook->SetSelection(0);
    UpdateDefaultButton(0);
}

void FindReplaceDialog::ShowReplaceTab() {
    notebook->SetSelection(1);
    UpdateDefaultButton(1);
}

void FindReplaceDialog::OnShow(wxShowEvent& event) {
    if (event.IsShown()) {
        // Focus the Find box on whichever tab is currently active.
        wxTextCtrl* target = (notebook->GetSelection() == 1) ? replaceText : findText;
        if (target) {
            target->SetFocus();
            target->SelectAll();
        }
    }
    event.Skip();
}

void FindReplaceDialog::OnFind(wxCommandEvent&) {
    if (!findText || findText->GetValue().IsEmpty()) {
        return;
    }

    bool forward = forwardRadio->GetValue();
    bool matchCase = matchCaseCheckbox->GetValue();
    FindText(findText->GetValue(), matchCase, forward);
}

void FindReplaceDialog::OnReplace(wxCommandEvent&) {
    if (!textCtrl || !replaceText || replaceText->GetValue().IsEmpty()) {
        return;
    }

    wxString findStr = replaceText->GetValue();
    wxString replaceWith = replaceWithText->GetValue();
    bool matchCase = replaceMatchCaseCheckbox->GetValue();

    // If the current selection is a match, replace it before finding the next one.
    wxString selected = textCtrl->GetSelectedText();
    bool selectionMatches = matchCase ? selected.IsSameAs(findStr)
                                      : selected.IsSameAs(findStr, false);
    if (selectionMatches) {
        textCtrl->ReplaceSelection(replaceWith);
    }

    FindText(findStr, matchCase, true);
}

void FindReplaceDialog::OnReplaceAll(wxCommandEvent&) {
    if (!textCtrl || !replaceText || replaceText->GetValue().IsEmpty()) {
        return;
    }

    wxString findStr = replaceText->GetValue();
    wxString replaceWith = replaceWithText->GetValue();
    bool matchCase = replaceMatchCaseCheckbox->GetValue();

    int flags = 0;
    if (matchCase) {
        flags |= wxSTC_FIND_MATCHCASE;
    }

    int count = 0;
    int pos = 0;

    textCtrl->BeginUndoAction();
    while (true) {
        // Positions are byte offsets, so use the match end Scintilla reports
        // rather than the character length of the search string.
        int foundEnd = -1;
        int found = textCtrl->FindText(pos, textCtrl->GetLength(), findStr, flags, &foundEnd);
        if (found == -1) {
            break;
        }
        textCtrl->SetTargetStart(found);
        textCtrl->SetTargetEnd(foundEnd);
        textCtrl->ReplaceTarget(replaceWith);
        pos = textCtrl->GetTargetEnd();
        count++;
    }
    textCtrl->EndUndoAction();

    wxMessageBox(wxString::Format(wxT("%d replacement(s) made."), count),
                 wxT("Replace All"), wxOK | wxICON_INFORMATION, this);
}

void FindReplaceDialog::OnClose(wxCommandEvent&) {
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

    int lastPos = textCtrl->GetLastPosition();
    int result = -1;
    int resultEnd = -1;

    if (forward) {
        // Search from the end of the current selection so the current match is skipped.
        int startPos = textCtrl->GetSelectionEnd();
        result = textCtrl->FindText(startPos, lastPos, text, flags, &resultEnd);
        // Wrap around to the top once the end of the document is reached.
        if (result == -1) {
            result = textCtrl->FindText(0, lastPos, text, flags, &resultEnd);
        }
    } else {
        // Search from the start of the current selection so the current match is skipped.
        int startPos = textCtrl->GetSelectionStart();
        result = textCtrl->FindText(startPos, 0, text, flags, &resultEnd);
        // Wrap around to the bottom once the start of the document is reached.
        if (result == -1) {
            result = textCtrl->FindText(lastPos, 0, text, flags, &resultEnd);
        }
    }

    if (result != -1) {
        // Unfold the line if needed, select the match and scroll it into view.
        textCtrl->EnsureVisible(textCtrl->LineFromPosition(result));
        textCtrl->SetSelection(result, resultEnd);
        textCtrl->ScrollRange(resultEnd, result);
        MoveAwayFromMatch(result);
    }

    return result;
}

void FindReplaceDialog::MoveAwayFromMatch(int matchPos) {
    if (!IsShown()) {
        return;
    }

    // Screen rectangle of the line containing the match.
    int line = textCtrl->LineFromPosition(matchPos);
    int lineHeight = textCtrl->TextHeight(line);
    wxPoint linePos = textCtrl->ClientToScreen(textCtrl->PointFromPosition(matchPos));
    wxRect textRect = textCtrl->GetScreenRect();
    wxRect matchRect(textRect.GetLeft(), linePos.y, textRect.GetWidth(), lineHeight);

    wxRect dlgRect = GetScreenRect();
    if (!dlgRect.Intersects(matchRect)) {
        return;
    }

    // Put the dialog below the matched line if it fits on screen, otherwise above it.
    int displayIndex = wxDisplay::GetFromWindow(this);
    wxRect screen = wxDisplay(displayIndex == wxNOT_FOUND ? 0 : displayIndex).GetClientArea();
    const int gap = 10;

    int newY = matchRect.GetBottom() + gap;
    if (newY + dlgRect.GetHeight() > screen.GetBottom()) {
        newY = matchRect.GetTop() - gap - dlgRect.GetHeight();
    }
    newY = wxMax(newY, screen.GetTop());

    Move(dlgRect.GetLeft(), newY);
}
