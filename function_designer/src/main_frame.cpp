/********************************************************************************
 * Copyright (c) 2026 ZF Friedrichshafen AG
 * 
 * See the NOTICE file(s) distributed with this work for additional
 * information regarding copyright ownership.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors:
 *   Thomas Pfleiderer - initial function designer
 ********************************************************************************/
 
#include "main_frame.h"
#include "json_signal_parser.h"
#include "meta_model.h"
#include "supervision_editor.h"
#include "validate_function.h"

#include <wx/config.h>
#include <wx/dirdlg.h>
#include <wx/file.h>
#include <wx/filedlg.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/listctrl.h>
#include <wx/notebook.h>
#include <wx/stdpaths.h>

#include <algorithm>
#include <functional>
#include <vector>
#include <regex>

namespace afd 
{
    namespace 
    {
        constexpr const char* kMetaModelFileName = "autoapiframework_meta_model.yaml";
        constexpr const char* kMetaModelFileConfigKey = "/Paths/MetaModelFile";
        constexpr const char* kMetaModelDirectoryConfigKey = "/Paths/MetaModelDirectory";
        constexpr const char* kVspecDirectoryConfigKey = "/Paths/VSpecDirectory";
        constexpr const char* kCreateApiLanguageConfigKey = "/CreateAPI/Language";
        constexpr const char* kDisableSyntaxCheckBeforeSavingConfigKey = "/Settings/DisableSyntaxCheckBeforeSaving";
        constexpr const char* kLogoFileName = "autoapiframework_logo.png";
        constexpr const char* kLogoFileNameLarge = "autoapiframework_logo_large.png";		
        constexpr const char* kIconFileName = "autoapiframework_icon.png";
        constexpr const char* kHelpUrl = "https://eclipse-autoapiframework.github.io/autoapiframework/main/";      
        constexpr int kFirstColumnWidth = 25;
        constexpr int kMinColumnWidth = 90;
        constexpr int kMaxColumnWidth = 320;

        wxString ToWx(const std::string& text) { return wxString::FromUTF8(text.c_str()); }

        std::string ToStd(const wxString& text) { return std::string(text.utf8_str()); }

        bool IsSyntaxCheckDisabledBeforeSaving()
        {
            bool disabled = false;
            wxConfigBase::Get()->Read(kDisableSyntaxCheckBeforeSavingConfigKey, &disabled, false);
            return disabled;
        }

        wxString FindRuntimeFile(const char* fileName)
        {
            wxFileName executable(wxStandardPaths::Get().GetExecutablePath());
            const wxString executableDirectory = executable.GetPath();
            const wxString candidates[] = {
                wxFileName(executableDirectory, fileName).GetFullPath(),
                wxFileName(wxGetCwd(), fileName).GetFullPath(),
                wxFileName(wxGetCwd() + "/..", fileName).GetFullPath(),
                wxFileName(wxGetCwd() + "/function_designer", fileName).GetFullPath()};

            for (const wxString& candidate : candidates)
            {
                if (wxFileName::FileExists(candidate))
                {
                    return candidate;
                }
            }
            return wxEmptyString;
        }

        /// Single line preview of a possibly multi line scalar.
        wxString OneLine(const std::string& text) 
        {
            wxString value = ToWx(text);
            value.Replace("\n", " ");
            value.Replace("\t", " ");
            return value;
        }

        wxString CellValue(const YamlNodePtr& entry, const std::string& key) 
        {
            if (!entry || !entry->IsMap()) 
            {
                return wxEmptyString;
            }
            YamlNodePtr value = entry->Find(key);
            if (!value && key == kQualityCodeKey)
            {
                const YamlNodePtr companion = entry->Find(kRuntimeCompanionKey);
                value = companion ? companion->Find(key) : nullptr;
                if (value && value->IsMap())
                {
                    return OneLine(value->ScalarOf(kQualityNameKey));
                }
            }
            if (!value) 
            {
                return wxEmptyString;
            }
            if (key == kExecutionResultKey && value->IsMap())
            {
                return OneLine(value->ScalarOf(kExecutionResultTypeKey));
            }
            if (key == kSupervisionKey && value->IsMap())
            {
                return OneLine(value->ScalarOf(kSupervisionRequiredKey));
            }
            if (value->IsScalar()) 
            {
                return OneLine(value->GetScalar());
            }
            if (value->IsSequence()) 
            {
                wxString joined;
                for (const YamlNodePtr& item : value->GetSequence()) 
                {
                    if (item && item->IsScalar()) 
                    {
                        if (!joined.empty()) 
                        {
                            joined += ", ";
                        }
                        joined += OneLine(item->GetScalar());
                    }
                }
                return joined;
            }
            return "<map>";
        }

        wxListCtrl* CreateReportList(wxWindow* parent) 
        {
            return new wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
        }

        void AutoSizeColumns(wxListCtrl* list) 
        {
            for (int column = 0; column < list->GetColumnCount(); ++column) 
            {
                list->SetColumnWidth(column, wxLIST_AUTOSIZE);
                const int contentWidth = list->GetColumnWidth(column);
                list->SetColumnWidth(column, wxLIST_AUTOSIZE_USEHEADER);
                int width = std::max(contentWidth, list->GetColumnWidth(column));
                if(column == 0)
                {
                    width = std::max(kFirstColumnWidth, std::min(width, kMaxColumnWidth));
                }
                else
                {
                    width = std::max(kMinColumnWidth, std::min(width, kMaxColumnWidth));
                }
                list->SetColumnWidth(column, width);
            }
        }
    } // namespace

    wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
        EVT_BUTTON(MainFrame::ID_NewSpecification, MainFrame::OnNewSpecification)
        EVT_BUTTON(MainFrame::ID_OpenSpecification, MainFrame::OnOpenSpecification)
        EVT_BUTTON(MainFrame::ID_SaveSpecification, MainFrame::OnSaveSpecification)
        EVT_BUTTON(MainFrame::ID_SaveSpecificationAs, MainFrame::OnSaveSpecificationAs)
        EVT_BUTTON(MainFrame::ID_OpenMetaModel, MainFrame::OnOpenMetaModel)
        EVT_BUTTON(MainFrame::ID_ShowMetaModel, MainFrame::OnShowMetaModel)
        EVT_BUTTON(MainFrame::ID_Settings, MainFrame::OnSettings)
        EVT_BUTTON(MainFrame::ID_AddVWithVspecFile, MainFrame::OnAddWithVspecFile)
        EVT_BUTTON(MainFrame::ID_Add, MainFrame::OnAdd)
        EVT_BUTTON(MainFrame::ID_Delete, MainFrame::OnDelete)
        EVT_BUTTON(MainFrame::ID_Edit, MainFrame::OnEdit)
        EVT_BUTTON(MainFrame::ID_Validation, MainFrame::OnValidation)
        EVT_BUTTON(MainFrame::ID_Show, MainFrame::OnShow)
        EVT_BUTTON(MainFrame::ID_CreateAPI, MainFrame::OnCreateAPI)
        EVT_MENU(MainFrame::ID_NewSpecification, MainFrame::OnNewSpecification)
        EVT_MENU(MainFrame::ID_OpenSpecification, MainFrame::OnOpenSpecification)
        EVT_MENU(MainFrame::ID_SaveSpecificationAs, MainFrame::OnSaveSpecificationAs)
        EVT_MENU(MainFrame::ID_OpenMetaModel, MainFrame::OnOpenMetaModel)
        EVT_MENU(MainFrame::ID_ShowMetaModel, MainFrame::OnShowMetaModel)
        EVT_MENU(MainFrame::ID_Settings, MainFrame::OnSettings)
        EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
        EVT_MENU(MainFrame::ID_GetVssToolsVersion, MainFrame::OnGetVssToolVersion)
        EVT_MENU(MainFrame::ID_QuickTips, MainFrame::OnQuickTips)
        EVT_MENU(wxID_HELP, MainFrame::OnHelp)
        EVT_MENU(wxID_EXIT, MainFrame::OnExit)
    wxEND_EVENT_TABLE()

    MainFrame::MainFrame()
        : wxFrame(nullptr, wxID_ANY, kApplicationName, wxDefaultPosition, wxSize(1200, 750)) 
    {
#ifdef __WXMSW__
        SetIcon(wxIcon("IDI_APP_ICON", wxBITMAP_TYPE_ICO_RESOURCE));
#endif
#ifdef __WXGTK__
        wxIcon appIcon(FindRuntimeFile(kIconFileName), wxBITMAP_TYPE_PNG);
        if (appIcon.IsOk())
        {
            SetIcon(appIcon);
        }
#endif
        BuildUi();
        AutoLoadMetaModel();
        UpdateMetaModelButtonStates();
        UpdateTitleAndStatus();
    }

    void MainFrame::BuildUi() 
    {
        wxMenu* fileMenu = new wxMenu();
        fileMenu->Append(ID_NewSpecification, "&New function specification\tCtrl-N");
        fileMenu->Append(ID_OpenSpecification, "&Read function specification...\tCtrl-O");
        fileMenu->Append(ID_SaveSpecification, "&Save function specification\tCtrl-S");
        fileMenu->Enable(ID_SaveSpecification, false);        
        fileMenu->Append(ID_SaveSpecificationAs, "&Write function specification as...\tCtrl-W");
        fileMenu->Enable(ID_SaveSpecificationAs, false);
        fileMenu->AppendSeparator();
        fileMenu->Append(ID_OpenMetaModel, "Load &meta model...");
        fileMenu->Append(ID_ShowMetaModel, "Show meta model");
        fileMenu->AppendSeparator();
        fileMenu->Append(wxID_EXIT);

        wxMenu* settingsMenu = new wxMenu();        
        settingsMenu->Append(ID_Settings, "&Settings...");

        wxMenu* helpMenu = new wxMenu();
        helpMenu->Append(wxID_ABOUT);
        helpMenu->Append(ID_GetVssToolsVersion, "Get vss-tools version");
        helpMenu->Append(ID_QuickTips, "Quick Tips...");
        helpMenu->Append(wxID_HELP, "&Help...");

        wxMenuBar* menuBar = new wxMenuBar();
        menuBar->Append(fileMenu, "&File");
        menuBar->Append(settingsMenu, "&Settings...");
        menuBar->Append(helpMenu, "&Help");
        SetMenuBar(menuBar);

        CreateStatusBar(2);
        int widths[] = { -2, -1 }; // first field gets 2/3, second gets 1/3
        GetStatusBar()->SetStatusWidths(2, widths);

        wxPanel* panel = new wxPanel(this);
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

        wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
        m_newButton = new wxButton(panel, ID_NewSpecification, "New");
        m_readButton = new wxButton(panel, ID_OpenSpecification, "Read .afs file");
        m_saveButton = new wxButton(panel, ID_SaveSpecification, "Save");
        m_saveButton->Enable(false);
        m_saveAsButton = new wxButton(panel, ID_SaveSpecificationAs, "Save as...");
        m_saveAsButton->Enable(false);
        m_metaButton = new wxButton(panel, ID_OpenMetaModel, "Load meta model...");
        m_showMetaModelButton = new wxButton(panel, ID_ShowMetaModel, "Show meta model");
        m_settingsButton = new wxButton(panel, ID_Settings, "Settings...");
        m_metaModelLabel = new wxStaticText(panel, wxID_ANY, "Meta model: <not loaded>");

        buttonSizer->Add(m_newButton, 0, wxALL, 5);
        buttonSizer->Add(m_readButton, 0, wxALL, 5);
        buttonSizer->Add(m_saveButton, 0, wxALL, 5);
        buttonSizer->Add(m_saveAsButton, 0, wxALL, 5);
        buttonSizer->Add(m_metaButton, 0, wxALL, 5);
        buttonSizer->Add(m_showMetaModelButton, 0, wxALL, 5);
        buttonSizer->Add(m_settingsButton, 0, wxALL, 5);
        buttonSizer->AddStretchSpacer();
        buttonSizer->Add(m_metaModelLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
        wxImage logoImage(FindRuntimeFile(kLogoFileName), wxBITMAP_TYPE_PNG);
        if (logoImage.IsOk())
        {
            const int logoHeight = 55;
            const int logoWidth = logoImage.GetWidth() * logoHeight / logoImage.GetHeight();
            logoImage = logoImage.Scale(logoWidth, logoHeight, wxIMAGE_QUALITY_HIGH);
            buttonSizer->Add(new wxStaticBitmap(panel, wxID_ANY, wxBitmap(logoImage)), 0,
                            wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 8);
        }
        mainSizer->Add(buttonSizer, 0, wxEXPAND);

        wxBoxSizer* editButtonSizer = new wxBoxSizer(wxHORIZONTAL);
        m_addWithVspecFileButton = new wxButton(panel, ID_AddVWithVspecFile, "Add by vspec file");
        m_addWithVspecFileButton->Enable(false);
        m_addButton = new wxButton(panel, ID_Add, "Add");
        m_addButton->Enable(false);
        m_deleteButton = new wxButton(panel, ID_Delete, "Delete");
        m_deleteButton->Enable(false);
        m_editButton = new wxButton(panel, ID_Edit, "Edit");
        m_editButton->Enable(false);
        m_validationButton = new wxButton(panel, ID_Validation, "Syntax check");
        m_validationButton->Enable(false);
        m_showButton = new wxButton(panel, ID_Show, "Show");
        m_showButton->Enable(false);
        m_createAPIButton = new wxButton(panel, ID_CreateAPI, "Create API");
        m_createAPIButton->Enable(false);
        editButtonSizer->Add(m_addWithVspecFileButton, 0, wxALL, 5);
        editButtonSizer->Add(m_addButton, 0, wxALL, 5);
        editButtonSizer->Add(m_deleteButton, 0, wxALL, 5);
        editButtonSizer->Add(m_editButton, 0, wxALL, 5);
        editButtonSizer->Add(m_validationButton, 0, wxALL, 5);
        editButtonSizer->Add(m_showButton, 0, wxALL, 5);
        editButtonSizer->Add(m_createAPIButton, 0, wxALL, 5);
        mainSizer->Add(editButtonSizer, 0, wxEXPAND);

        m_notebook = new wxNotebook(panel, wxID_ANY);

        wxPanel* attributePage = new wxPanel(m_notebook);
        m_attributeList = CreateReportList(attributePage);
        m_nameText = new wxTextCtrl(attributePage, wxID_ANY);
        m_versionText = new wxTextCtrl(attributePage, wxID_ANY);
        m_descriptionText = new wxTextCtrl(attributePage, wxID_ANY, wxEmptyString, wxDefaultPosition,
                                        wxSize(-1, 140), wxTE_MULTILINE | wxTE_BESTWRAP);
        m_nameText->Bind(wxEVT_TEXT, [this](wxCommandEvent&)
        {
            m_specification.SetName(ToStd(m_nameText->GetValue()));
        });
        m_versionText->Bind(wxEVT_TEXT, [this](wxCommandEvent&)
        {
            m_specification.SetVersion(ToStd(m_versionText->GetValue()));
        });
        m_descriptionText->Bind(wxEVT_TEXT, [this](wxCommandEvent&)
        {
            m_specification.SetDescription(ToStd(m_descriptionText->GetValue()));
        });
        wxBoxSizer* attributeSizer = new wxBoxSizer(wxVERTICAL);
        wxFlexGridSizer* editableAttributes = new wxFlexGridSizer(2, 5, 5);
        editableAttributes->AddGrowableCol(1, 1);
        editableAttributes->Add(new wxStaticText(attributePage, wxID_ANY, "Name"), 0,
                                wxALIGN_CENTER_VERTICAL);
        editableAttributes->Add(m_nameText, 1, wxEXPAND);
        editableAttributes->Add(new wxStaticText(attributePage, wxID_ANY, "Version"), 0,
                                wxALIGN_CENTER_VERTICAL);
        editableAttributes->Add(m_versionText, 1, wxEXPAND);
        attributeSizer->Add(editableAttributes, 0, wxEXPAND | wxALL, 5);
        attributeSizer->Add(new wxStaticText(attributePage, wxID_ANY, "Attributes (read only)"), 0, wxLEFT | wxTOP, 5);
        attributeSizer->Add(m_attributeList, 1, wxEXPAND | wxALL, 5);
        attributeSizer->Add(new wxStaticText(attributePage, wxID_ANY, "Description"), 0, wxLEFT, 5);
        attributeSizer->Add(m_descriptionText, 0, wxEXPAND | wxALL, 5);
        attributePage->SetSizer(attributeSizer);

        auto makeListPage = [this](wxListCtrl** target) 
        {
            wxPanel* page = new wxPanel(m_notebook);
            *target = CreateReportList(page);
            wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(*target, 1, wxEXPAND | wxALL, 5);
            page->SetSizer(sizer);
            return page;
        };

        wxPanel* signalPage = makeListPage(&m_signalList);
        wxPanel* parameterPage = makeListPage(&m_parameterList);
        wxPanel* schedulingPage = makeListPage(&m_schedulingList);
        wxPanel* errorPage = makeListPage(&m_errorList);
        auto bindEditButtonState = [this](wxListCtrl* list)
        {
            list->Bind(wxEVT_LIST_ITEM_SELECTED, [this](wxListEvent&)
            {
                UpdateCollectionButtonStates();
            });
            list->Bind(wxEVT_LIST_ITEM_DESELECTED, [this](wxListEvent&)
            {
                UpdateCollectionButtonStates();
            });
            list->Bind(wxEVT_LIST_ITEM_ACTIVATED, [this, list](wxListEvent& event)
            {
                if (list->GetSelectedItemCount() == 1)
                {
                    wxCommandEvent editEvent(wxEVT_BUTTON, ID_Edit);
                    OnEdit(editEvent);
                }
                else
                {
                    event.Skip();
                }
            });
        };
        bindEditButtonState(m_signalList);
        bindEditButtonState(m_parameterList);
        bindEditButtonState(m_schedulingList);
        bindEditButtonState(m_errorList);

        m_notebook->AddPage(attributePage, "Attributes", true);
        m_notebook->AddPage(signalPage, "Signal collection");
        m_notebook->AddPage(parameterPage, "Parameter collection");
        m_notebook->AddPage(schedulingPage, "Scheduling collection");
        m_notebook->AddPage(errorPage, "Error collection");
        m_notebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, [this](wxBookCtrlEvent& event)
        {
            UpdateCollectionButtonStates();
            event.Skip();
        });

        mainSizer->Add(m_notebook, 1, wxEXPAND | wxALL, 5);
        panel->SetSizer(mainSizer);

        m_attributeList->AppendColumn("Attribute", wxLIST_FORMAT_LEFT, 260);
        m_attributeList->AppendColumn("Value", wxLIST_FORMAT_LEFT, 600);
    }

    bool MainFrame::AutoLoadMetaModel() 
    {
        wxFileName executable(wxStandardPaths::Get().GetExecutablePath());
        const wxString exeDir = executable.GetPath();

        wxString configuredMetaModelFile;
        wxConfigBase::Get()->Read(kMetaModelFileConfigKey, &configuredMetaModelFile);
        if (!configuredMetaModelFile.empty() && wxFileName::FileExists(configuredMetaModelFile))
        {
            if (LoadMetaModel(configuredMetaModelFile, true) && m_metaModel.IsLoaded())
            {
                return true;
            }
        }

        wxString configuredDirectory;
        wxConfigBase::Get()->Read(kMetaModelDirectoryConfigKey, &configuredDirectory);
        if (!configuredDirectory.empty())
        {
            const wxString configuredPath = wxFileName(configuredDirectory, kMetaModelFileName).GetFullPath();
            if (wxFileName::FileExists(configuredPath))
            {
                if (m_metaModel.IsLoaded())
                {                    
                    return true;
                }
            }
        }

        const wxString candidates[] = 
        {
            wxFileName(exeDir, kMetaModelFileName).GetFullPath(),
            wxFileName(exeDir + "/..", kMetaModelFileName).GetFullPath(),
            wxFileName(exeDir + "/../..", kMetaModelFileName).GetFullPath(),
            wxFileName(exeDir + "/../../..", kMetaModelFileName).GetFullPath(),
            wxFileName(wxGetCwd(), kMetaModelFileName).GetFullPath(),
            wxFileName(wxGetCwd() + "/..", kMetaModelFileName).GetFullPath(),
            wxFileName(wxGetCwd() + "/function_designer", kMetaModelFileName).GetFullPath()
        };

        for (const wxString& candidate : candidates) 
        {
            if (wxFileName::FileExists(candidate)) 
            {
                if (!LoadMetaModel(candidate, true))
                {
                    continue;
                }
                if (m_metaModel.IsLoaded()) 
                {
                    return true;
                }
            }
        }

        return false;
    }

    void MainFrame::OnNewSpecification(wxCommandEvent&)
    {
        if (m_specification.IsLoaded())
        {
            return;
        }

        m_specification.New(m_metaModel.GetName(), m_metaModel.GetVersion());
        m_contentChanged = true;
        m_newButton->Enable(false);
        GetMenuBar()->Enable(ID_NewSpecification, false);
        m_saveAsButton->Enable(true);
        GetMenuBar()->Enable(ID_SaveSpecification, m_contentChanged);          
        GetMenuBar()->Enable(ID_SaveSpecificationAs, true);		
        m_showButton->Enable(true);
        GetMenuBar()->Enable(ID_ShowMetaModel, true);			
        m_validationButton->Enable(true);        
		m_createAPIButton->Enable(true);
        RefreshAll();
        UpdateTitleAndStatus();
    }

    void MainFrame::OnOpenSpecification(wxCommandEvent&) 
    {
        wxFileDialog dialog(this, "Read function specification", wxEmptyString, wxEmptyString,
                            "Function specification (*.afs;*.afs)|*.afs;*.afs|"
                            "Function specification (*.afs.yaml;*.afs.yaml)|*.afs.yaml;*.afs.yaml|"
                            "YAML files (*.yaml;*.yml)|*.yaml;*.yml|All files (*.*)|*.*",
                            wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dialog.ShowModal() == wxID_CANCEL) 
        {
            return;
        }
        LoadSpecification(dialog.GetPath());
    }

    void MainFrame::OnSaveSpecification(wxCommandEvent& e)
    {
        if (!m_specification.IsLoaded())
        {
            return;
        }
        if (!IsSyntaxCheckDisabledBeforeSaving() && !SyntaxCheckIsOK(true, "Not saved! "))
        {
            return;
        }

        const wxString path = ToWx(m_specification.GetSourcePath());
        if (path.IsEmpty())
        {
            OnSaveSpecificationAs(e);
            return;
        }
        
        std::string error;
        if (!m_specification.Save(ToStd(path), error))
        {
            wxMessageBox("Could not write file:\n\n" + ToWx(error), kApplicationName,
                        wxOK | wxICON_ERROR, this);
            return;
        }
        SetStatusText("Written: " + path, 0);
    }

    void MainFrame::OnSaveSpecificationAs(wxCommandEvent&) 
    {
        if (!m_specification.IsLoaded()) 
        {
            wxMessageBox("Read a function specification first.", kApplicationName,
                        wxOK | wxICON_INFORMATION, this);
            return;
        }
        if (!IsSyntaxCheckDisabledBeforeSaving() && !SyntaxCheckIsOK(true))
        {
            return;
        }

        wxFileName source(ToWx(m_specification.GetSourcePath()));
        wxString suggested = source.GetFullName();
        const wxString suffix = ".afs";
        if(suggested.IsEmpty())
        {
            suggested = "untitled" + suffix;
        }
        else if (suggested.EndsWith(".afs") || suggested.EndsWith(".afs")) 
        {
            suggested = suggested.Left(suggested.length() - suffix.length()) + "_copy" + suffix;
        } 
        else if (suggested.EndsWith(".afs.yaml") || suggested.EndsWith(".afs.yaml")) 
        {
            suggested = suggested.Left(suggested.length() - wxString(".afs.yaml").length()) + "_copy" + suffix;
        } 
        else 
        {
            suggested += "_copy" + suffix;
        }

        wxFileDialog dialog(this, "Write function specification as", source.GetPath(), suggested,
                            "Function specification (*.afs)|*.afs|All files (*.*)|*.*",
                            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dialog.ShowModal() == wxID_CANCEL) 
        {
            return;
        }

        std::string error;
        if (!m_specification.Save(ToStd(dialog.GetPath()), error)) 
        {
            wxMessageBox("Could not write file:\n\n" + ToWx(error), kApplicationName,
                        wxOK | wxICON_ERROR, this);
            return;
        }
        SetStatusText("Written: " + dialog.GetPath(), 0);
    }

    void MainFrame::OnOpenMetaModel(wxCommandEvent&) 
    {
        auto isLoaded = m_metaModel.IsLoaded();
        wxString configuredMetaModelFile;
        wxConfigBase::Get()->Read(kMetaModelFileConfigKey, &configuredMetaModelFile);
        wxFileName configuredFile(configuredMetaModelFile);
        const wxString initialDirectory = configuredFile.IsOk() ? configuredFile.GetPath() : wxString();
        const wxString initialFileName = configuredFile.IsOk() ? configuredFile.GetFullName() : wxString(kMetaModelFileName);
        wxFileDialog dialog(this, "Load meta model", initialDirectory, initialFileName,
                            "YAML files (*.yaml;*.yml)|*.yaml;*.yml|All files (*.*)|*.*",
                            wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dialog.ShowModal() == wxID_CANCEL) 
        {
            return;
        }
        if (LoadMetaModel(dialog.GetPath(), true))
        {
            if (m_metaModel.IsLoaded())
            {
                wxConfigBase::Get()->Write(kMetaModelFileConfigKey, dialog.GetPath());
                wxConfigBase::Get()->Flush();
                if (isLoaded)
                {
                    wxMessageBox("Meta model reloaded successfully.", kApplicationName,
                                wxOK | wxICON_INFORMATION, this);
                }
            }
            SetStatusText(m_metaModel.IsLoaded() ? "Meta model loaded" : "No meta model", 1);

            UpdateMetaModelButtonStates();
            RefreshAll();
        }
    }

    void MainFrame::OnSettings(wxCommandEvent&)
    {
        wxConfigBase* config = wxConfigBase::Get();
        wxString metaModelFile = kMetaModelFileName;
        wxString vspecDirectory;
        bool disableSyntaxCheckBeforeSaving = false;
        config->Read(kMetaModelFileConfigKey, &metaModelFile);
        config->Read(kVspecDirectoryConfigKey, &vspecDirectory);
        config->Read(kDisableSyntaxCheckBeforeSavingConfigKey, &disableSyntaxCheckBeforeSaving, false);

        MetaModel metaModel;
        std::string metaModelVersion = "unknown";
        std::string error;
        if (metaModel.Load(ToStd(metaModelFile), error)) 
        {
            metaModelVersion = metaModel.GetVersion();
        }

        wxDialog dialog(this, wxID_ANY, "Settings", wxDefaultPosition, wxDefaultSize,
                        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
        wxBoxSizer* dialogSizer = new wxBoxSizer(wxVERTICAL);
        wxFlexGridSizer* fields = new wxFlexGridSizer(4, 8, 8);
        fields->AddGrowableCol(1, 1);

        wxTextCtrl* metaModelControl = new wxTextCtrl(&dialog, wxID_ANY, metaModelFile);
        wxTextCtrl* vspecControl = new wxTextCtrl(&dialog, wxID_ANY, vspecDirectory);
        wxCheckBox* disableSyntaxCheckControl = new wxCheckBox(
            &dialog, wxID_ANY, "Syntax check disabled before saving the file.");
        disableSyntaxCheckControl->SetValue(disableSyntaxCheckBeforeSaving);
        wxStaticText* metaModelVersionLabel = new wxStaticText(&dialog, wxID_ANY, metaModelVersion);    
        
        wxString vspecVersion = ToWx(GetGitTagAndVersion("VSS version: ", vspecDirectory));
        wxStaticText* vspecVersionLabel = new wxStaticText(&dialog, wxID_ANY, vspecVersion.IsEmpty() ? "Version: unknown" : vspecVersion);

        const auto addField = [&dialog, fields](const wxString& label, wxTextCtrl* control, bool isFile,
                                               wxStaticText* versionLabel,
                                               const std::function<wxString(const wxString&)>& getVersion)
        {
            fields->Add(new wxStaticText(&dialog, wxID_ANY, label),
                        0, wxALIGN_CENTER_VERTICAL);    
            fields->Add(control, 1, wxEXPAND);
            wxButton* browseButton = new wxButton(&dialog, wxID_ANY, "Browse...");
            browseButton->Bind(wxEVT_BUTTON, [&dialog, control, isFile, versionLabel, getVersion](wxCommandEvent&)
            {
                wxString selectedPath;
                if (isFile)
                {
                    wxFileName current(control->GetValue());
                    wxFileDialog picker(&dialog, "Select meta model", current.GetPath(), current.GetFullName(),
                                        "YAML files (*.yaml;*.yml)|*.yaml;*.yml|All files (*.*)|*.*",
                                        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
                    if (picker.ShowModal() == wxID_OK)
                    {
                        selectedPath = picker.GetPath();
                    }
                }
                else
                {
                    wxDirDialog picker(&dialog, "Select folder", control->GetValue(),
                                       wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
                    if (picker.ShowModal() == wxID_OK)
                    {
                        selectedPath = picker.GetPath();
                    }
                }

                if (!selectedPath.empty())
                {
                    control->SetValue(selectedPath);
                    versionLabel->SetLabel(getVersion(selectedPath));
                    dialog.Fit();
                }
            });
            fields->Add(browseButton);
            fields->AddSpacer(1);
            if (versionLabel)
            {
                fields->AddSpacer(1);
                fields->Add(versionLabel, 0, wxALIGN_LEFT | wxBOTTOM, 4);
                fields->AddSpacer(1);
                fields->AddSpacer(1);
            }
        };

        addField("Meta model file", metaModelControl, true, metaModelVersionLabel,
                 [](const wxString& path)
                 {
                     MetaModel selectedMetaModel;
                     std::string loadError;
                     return selectedMetaModel.Load(ToStd(path), loadError)
                         ? ToWx(selectedMetaModel.GetVersion()) : "unknown";
                 });
        addField("COVESA VSS folder", vspecControl, false, vspecVersionLabel,
                 [this](const wxString& path)
                 {
                     const wxString version = ToWx(GetGitTagAndVersion("VSS version: ", path));
                     return version.IsEmpty() ? wxString("Version: unknown") : version;
                 });

        dialogSizer->Add(fields, 1, wxEXPAND | wxALL, 12);
        dialogSizer->Add(disableSyntaxCheckControl, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
        dialogSizer->Add(dialog.CreateSeparatedButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 8);
        dialog.SetSizerAndFit(dialogSizer);
        dialog.SetMinSize(wxSize(700, -1));
        dialog.CentreOnParent();
        if (dialog.ShowModal() != wxID_OK)
        {
            return;
        }

        config->Write(kMetaModelFileConfigKey, metaModelControl->GetValue());
        config->Write(kVspecDirectoryConfigKey, vspecControl->GetValue());
        config->Write(kDisableSyntaxCheckBeforeSavingConfigKey, disableSyntaxCheckControl->GetValue());
        config->Flush();
        SetStatusText("Global settings updated", 0);
        if (metaModelFile.CompareTo(metaModelControl->GetValue()) != 0)
        {
            m_metaButton->Enable(true); 
        }
    }

    void MainFrame::OnShowMetaModel(wxCommandEvent&)
    {
        if (!m_metaModel.IsLoaded())
        {
            return;
        }

        wxDialog dialog(this, wxID_ANY, "Meta model", wxDefaultPosition,
                        wxSize(900, 650), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(new wxTextCtrl(&dialog, wxID_ANY, ToWx(m_metaModel.GetFileContent()),
                                 wxDefaultPosition, wxDefaultSize,
                                 wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP),
                   1, wxEXPAND | wxALL, 8);
        sizer->Add(dialog.CreateSeparatedButtonSizer(wxOK), 0, wxEXPAND | wxALL, 8);
        dialog.SetSizer(sizer);
        dialog.CentreOnParent();
        dialog.ShowModal();
    }

    void MainFrame::OnAddWithVspecFile(wxCommandEvent&)
    {
        if (!IsVspecAvailable())
        {
            wxMessageBox(GetVspecUnavailableMessage(), kApplicationName, wxOK | wxICON_WARNING, this);
            return;
        }

        wxString vspecDirectory;
        wxConfigBase::Get()->Read(kVspecDirectoryConfigKey, &vspecDirectory);

        wxFileDialog sourceDialog(this, "Select vspec file", vspecDirectory, wxEmptyString,
                                  "vspec files (*.vspec)|*.vspec|All files (*.*)|*.*",
                                  wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (sourceDialog.ShowModal() != wxID_OK)
        {
            return;
        }

        const wxString tempJsonPath = wxFileName::CreateTempFileName("acd_vspec_");
        if (tempJsonPath.empty())
        {
            wxMessageBox("Could not create a temporary file for the vspec conversion.",
                         kApplicationName, wxOK | wxICON_ERROR, this);
            return;
        }

        const wxString result = RunVspec2Json(sourceDialog.GetPath(), tempJsonPath);
        if (result.StartsWith("ERROR:"))
        {
            wxMessageBox(result, kApplicationName, wxOK | wxICON_ERROR, this);
            wxRemoveFile(tempJsonPath);
            return;
        }

        wxFile jsonFile(tempJsonPath);
        wxString jsonContent;
        const bool readOk = jsonFile.IsOpened() && jsonFile.ReadAll(&jsonContent);
        jsonFile.Close();
        wxRemoveFile(tempJsonPath);
        if (!readOk)
        {
            wxMessageBox("Could not read the converted VSPEC JSON file.", kApplicationName,
                         wxOK | wxICON_ERROR, this);
            return;
        }

        JsonValue root;
        std::string parseError;
        if (!JsonParser().Parse(ToStd(jsonContent), root, parseError))
        {
            wxMessageBox("Could not parse VSPEC JSON:\n\n" + ToWx(parseError), kApplicationName,
                         wxOK | wxICON_ERROR, this);
            return;
        }

        std::vector<VssSignal> signals;
        FlattenVssTree(root, "", signals);
        if (signals.empty())
        {
            wxMessageBox("No signals found in the converted VSPEC JSON.", kApplicationName,
                         wxOK | wxICON_INFORMATION, this);
            return;
        }

        SelectSignalsDialog selectDialog(this, signals);
        if (selectDialog.ShowModal() != wxID_OK)
        {
            return;
        }

        const std::vector<VssSignal> selected = selectDialog.GetSelectedSignals();
        if (selected.empty())
        {
            return;
        }

        const std::vector<std::string> fields = m_metaModel.ColumnsFor(kDataInterfaceTypeKey, {});
        std::size_t added = 0;
        for (const VssSignal& signal : selected)
        {
            YamlNodePtr item = YamlNode::MakeMap();
            for (const std::string& field : fields)
            {
                if (field == kQualityCodeKey)
                {
                    continue;
                }
                std::string value;
                if (field == FunctionSpecification::kNamePathKey)
                {
                    value = signal.path;
                }
                else if (field == kCovesaType)
                {
                    value = signal.type;
                }                
                else if (field == kMetaModelDataType)
                {
                    value = signal.dataType;
                }
                else if (field == kCovesaDescription)
                {
                    value = signal.description;
                }
                else if (field == kCovesaUnit)
                {
                    value = signal.unit;
                }
                else if (field == kCovesaComment)
                {
                    value = signal.comment;
                }
                else if (field == kCovesaMin)
                {
                    value = signal.min;
                }
                else if (field == kCovesaMax)
                {
                    value = signal.max;
                }
                else if (field == kCovesaAllowed)
                {
                    value = signal.allowed;
                }
                else if (field == kMetaModelDefaultValue)
                {
                    value = signal.defaultValue;
                }
                else if (field == kCovesaUuid)
                {
                    value = signal.uuid;
                }
                else if (field == kCovesaArraySize)
                {
                    value = signal.arraySize;
                }
                item->Set(field, YamlNode::MakeScalar(value));
            }
            if (m_specification.AddCollectionItem(FunctionSpecification::kDataInterfacesKey, std::move(item)))
            {
                ++added;
            }
        }

        if (added > 0)
        {
            m_contentChanged = true;
            RefreshAll();
            SetStatusText(wxString::Format("Added %zu signal(s)", added), 0);
        }
    }

    void MainFrame::OnAdd(wxCommandEvent&)
    {
        const char* collectionKey = GetSelectedCollectionKey();
        if (!collectionKey)
        {
            return;
        }

        const int selectedTab = m_notebook->GetSelection();
        const std::string interfaceType = selectedTab == 1 ? "Data" :
                                          selectedTab == 2 ? "Parameter" :
                                          selectedTab == 3 ? "Scheduling" : "Error";
        const std::vector<std::string> fields = m_metaModel.ColumnsFor(interfaceType, {});
        if (fields.empty())
        {
            return;
        }

        wxString collectionName = m_notebook->GetPageText(m_notebook->GetSelection());
        collectionName = collectionName.BeforeFirst('(').Trim();
        collectionName.Replace(" collection", "");
        collectionName.MakeLower();
        wxDialog dialog(this, wxID_ANY, "Add " + collectionName + " item",
                wxDefaultPosition, wxDefaultSize,
                wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        wxFlexGridSizer* fieldSizer = new wxFlexGridSizer(2, 6, 8);
        fieldSizer->AddGrowableCol(1, 1);
        std::vector<std::pair<std::string, wxTextCtrl*>> controls;
        for (const std::string& field : fields)
        {
            const bool isSupervisionRequired =
                interfaceType == kSchedulingInterfaceTypeKey && field == kSupervisionKey;
            const std::string fieldName = isSupervisionRequired ? kSupervisionRequiredKey : field;
            fieldSizer->Add(new wxStaticText(&dialog, wxID_ANY, ToWx(fieldName)), 0,
                            wxALIGN_CENTER_VERTICAL);

            wxTextCtrl* control = nullptr;
            if (field == kDescriptionKey)
            {
                control = new wxTextCtrl(
                    &dialog,
                    wxID_ANY,
                    "",
                    wxDefaultPosition,
                    wxSize(-1, 100),          // height of 100 px
                    wxTE_MULTILINE);
            }
            else
            {
                control = new wxTextCtrl(&dialog, wxID_ANY);
            }

            if (interfaceType == kSchedulingInterfaceTypeKey && field == kExecutionResultKey)
            {
                if (const YamlNodePtr* type = m_metaModel.FindInterfaceType(interfaceType))
                {
                    const YamlNodePtr properties = (*type)->Find(kPropertiesKey);
                    const YamlNodePtr result = properties ? properties->Find(field) : nullptr;
                    if (result)
                    {
                        control->SetValue(ToWx(result->ScalarOf(kEnumRefKey)));
                    }
                }
                control->SetEditable(false);
            }
            else if (isSupervisionRequired)
            {
                control->SetValue("false");
                control->SetEditable(false);
            }

            fieldSizer->Add(control, 1, wxEXPAND);
            controls.emplace_back(fieldName, control);
        }

        sizer->Add(fieldSizer, 1, wxEXPAND | wxALL, 12);
        sizer->Add(dialog.CreateSeparatedButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 8);
        dialog.SetSizerAndFit(sizer);
        dialog.SetMinSize(wxSize(450, -1));
        dialog.CentreOnParent();
        if (dialog.ShowModal() != wxID_OK)
        {
            return;
        }

        YamlNodePtr item = YamlNode::MakeMap();
        for (const auto& control : controls)
        {
            if (interfaceType == kSchedulingInterfaceTypeKey && control.first == kExecutionResultKey)
            {
                YamlNodePtr result = YamlNode::MakeMap();
                result->Set(kExecutionResultTypeKey, YamlNode::MakeScalar(ToStd(control.second->GetValue())));
                item->Set(control.first, std::move(result));
                continue;
            }
            if (interfaceType == kSchedulingInterfaceTypeKey && control.first == kSupervisionRequiredKey)
            {
                YamlNodePtr supervision = YamlNode::MakeMap();
                supervision->Set(kSupervisionRequiredKey, YamlNode::MakeScalar(ToStd(control.second->GetValue())));
                item->Set(kSupervisionKey, std::move(supervision));
                continue;
            }
            if (interfaceType == kDataInterfaceTypeKey && control.first == kQualityCodeKey)
            {
                const std::string name = ToStd(control.second->GetValue());
                if (!name.empty())
                {
                    YamlNodePtr quality = YamlNode::MakeMap();
                    quality->Set(kNameKey, YamlNode::MakeScalar(name));
                    quality->Set(kDatatypeKey, YamlNode::MakeScalar(kEnumKey));
                    quality->Set(kEnumRefKey, YamlNode::MakeScalar(kDataQualityKey));
                    YamlNodePtr companion = YamlNode::MakeMap();
                    companion->Set(kQualityCodeKey , std::move(quality));
                    item->Set(kRuntimeCompanionKey, std::move(companion));
                }
                continue;
            }
            item->Set(control.first, YamlNode::MakeScalar(ToStd(control.second->GetValue())));
        }
        if (m_specification.AddCollectionItem(collectionKey, std::move(item)))
        {
            m_contentChanged = true;
            RefreshAll();
            SetStatusText("Added item", 0);
        }
    }

    void MainFrame::OnDelete(wxCommandEvent&)
    {
        wxListCtrl* selectedList = GetSelectedCollectionList();
        const char* collectionKey = GetSelectedCollectionKey();
        if (!selectedList || !collectionKey)
        {
            return;
        }

        const long selectedRow = selectedList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (selectedRow < 0)
        {
            return;
        }

        if (m_specification.RemoveCollectionItem(collectionKey, static_cast<std::size_t>(selectedRow)))
        {
            m_contentChanged = true;
            RefreshAll();
            SetStatusText(wxString::Format("Deleted item %ld", selectedRow + 1), 0);
        }
    }

    void MainFrame::OnEdit(wxCommandEvent&) 
    {
        wxListCtrl* selectedList = GetSelectedCollectionList();
        const char* collectionKey = GetSelectedCollectionKey();
        const long selectedRow = selectedList
            ? selectedList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)
            : -1;
        if (!collectionKey || selectedRow < 0)
        {
            return;
        }

        const std::vector<YamlNodePtr> entries = m_specification.GetCollection(collectionKey);
        if (static_cast<std::size_t>(selectedRow) >= entries.size() || !entries[selectedRow] ||
            !entries[selectedRow]->IsMap())
        {
            return;
        }

        CompleteEntryFromMetaModel(GetSelectedInterfaceType(), entries[selectedRow]);

        struct EditableValue
        {
            YamlNodePtr node;
            wxTextCtrl* control;
        };

        wxString collectionName = m_notebook->GetPageText(m_notebook->GetSelection());
        collectionName = collectionName.BeforeFirst('(').Trim();
        collectionName.Replace(" collection", "");
        collectionName.MakeLower();
        wxDialog dialog(this, wxID_ANY, "Edit " + collectionName + " item",
                wxDefaultPosition, wxDefaultSize,
                wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        wxFlexGridSizer* fields = new wxFlexGridSizer(2, 6, 8);
        fields->AddGrowableCol(1, 1);
        std::vector<EditableValue> editableValues;
        YamlNodePtr editedSupervision;
        wxTextCtrl* supervisionRequiredControl = nullptr;

        const auto addField =
            [&fields, &editableValues, &dialog, &supervisionRequiredControl]
            (const wxString& name, const YamlNodePtr& node, bool readOnly)
        {
            if (!node || !node->IsScalar())
            {
                return;
            }

            fields->Add(new wxStaticText(&dialog, wxID_ANY, name), 0, wxALIGN_CENTER_VERTICAL);

            wxTextCtrl* control = nullptr;
            if (name.CmpNoCase(kDescriptionKey) == 0)
            {
                control = new wxTextCtrl(
                    &dialog,
                    wxID_ANY,
                    ToWx(node->GetScalar()),
                    wxDefaultPosition,
                    wxSize(-1, 100),
                    wxTE_MULTILINE);
            }
            else
            {
                control = new wxTextCtrl(&dialog, wxID_ANY, ToWx(node->GetScalar()));
            }

            if (readOnly)
            {
                control->SetEditable(false);
            }
            fields->Add(control, 1, wxEXPAND);
            if (name == ToWx(kSupervisionRequiredPath))
            {
                supervisionRequiredControl = control;
            }
            if (!readOnly)
            {
                editableValues.push_back({node, control});
            }
        };

        for (const auto& entry : entries[selectedRow]->GetMap())
        {
            if (entry.first == kRuntimeCompanionKey && entry.second && entry.second->IsMap())
            {
                for (const auto& companion : entry.second->GetMap())
                {
                    if (companion.second && companion.second->IsMap())
                    {
                        for (const auto& field : companion.second->GetMap())
                        {
                            addField(ToWx(companion.first + "." + field.first), field.second, false);
                        }
                    }
                }
            }
            else if (entry.second && entry.second->IsMap())
            {
                for (const auto& nested : entry.second->GetMap())
                {
                    const std::string fieldPath = entry.first + "." + nested.first;
                    const bool readOnly = collectionKey == FunctionSpecification::kSchedulingKey &&
                        (fieldPath == kExecutionResultTypeFieldPath || fieldPath == kSupervisionRequiredPath);
                    addField(ToWx(fieldPath), nested.second, readOnly);
                }
            }
            else
            {
                addField(ToWx(entry.first), entry.second, false);
            }
        }

        if (editableValues.empty())
        {
            return;
        }

        wxButton* editSupervisionButton = nullptr;
        if (collectionKey == FunctionSpecification::kSchedulingKey)
        {
            editSupervisionButton = new wxButton(&dialog, wxID_ANY, "Edit Supervision");
            editSupervisionButton->Bind(wxEVT_BUTTON,
                [&dialog, &entries, selectedRow, &editedSupervision, supervisionRequiredControl]
                (wxCommandEvent&)
                {
                    const YamlNodePtr existingSupervision = editedSupervision
                        ? editedSupervision
                        : entries[selectedRow]->Find(kSupervisionKey);
                    wxDialog supervisionDialog(&dialog, wxID_ANY, "Edit Supervision",
                                               wxDefaultPosition, wxDefaultSize,
                                               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
                    wxBoxSizer* supervisionSizer = new wxBoxSizer(wxVERTICAL);
                    SupervisionEditor editor(supervisionDialog, *supervisionSizer,
                                             existingSupervision);
                    supervisionSizer->Add(
                        supervisionDialog.CreateSeparatedButtonSizer(wxOK | wxCANCEL),
                        0, wxEXPAND | wxALL, 8);
                    supervisionDialog.SetSizerAndFit(supervisionSizer);
                    supervisionDialog.SetMinSize(wxSize(500, -1));
                    supervisionDialog.CentreOnParent();
                    if (supervisionDialog.ShowModal() == wxID_OK)
                    {
                        editedSupervision = editor.Build();
                        if (supervisionRequiredControl)
                        {
                            supervisionRequiredControl->SetValue(
                                ToWx(editedSupervision->ScalarOf(kSupervisionRequiredKey)));
                        }
                        dialog.Layout();
                        dialog.Fit();
                    }
                });
        }

        sizer->Add(fields, 1, wxEXPAND | wxALL, 12);
        if (editSupervisionButton)
        {
            wxBoxSizer* actionSizer = new wxBoxSizer(wxHORIZONTAL);
            actionSizer->Add(editSupervisionButton, 0, wxALIGN_CENTER_VERTICAL);
            actionSizer->AddStretchSpacer();
            actionSizer->Add(dialog.CreateStdDialogButtonSizer(wxOK | wxCANCEL),
                             0, wxALIGN_CENTER_VERTICAL);
            sizer->Add(actionSizer, 0, wxEXPAND | wxALL, 8);
        }
        else
        {
            sizer->Add(dialog.CreateSeparatedButtonSizer(wxOK | wxCANCEL),
                       0, wxEXPAND | wxALL, 8);
        }
        dialog.SetSizerAndFit(sizer);
        dialog.SetMinSize(wxSize(450, -1));
        dialog.CentreOnParent();
        if (dialog.ShowModal() != wxID_OK)
        {
            return;
        }

        m_contentChanged = true;        
        for (const EditableValue& value : editableValues)
        {
            value.node->SetScalar(ToStd(value.control->GetValue()));
        }
        if (editedSupervision)
        {
            entries[selectedRow]->Set(kSupervisionKey, std::move(editedSupervision));
        }
        RefreshAll();
        SetStatusText(wxString::Format("Edited item %ld", selectedRow + 1), 0);
    }

    void MainFrame::OnValidation(wxCommandEvent&)
    {                        
        SyntaxCheckIsOK(false); 
    }

    bool MainFrame::SyntaxCheckIsOK(bool doNotShowOnSuccess, const std::string& contextMessage)
    {
        if (!m_specification.IsLoaded())
        {
            return false;
        }

        std::string content = m_specification.ToText();
        std::string error;

        ValidateFunction validateFunction;
        if (validateFunction.SyntaxCheckIsOK(m_metaModel, content, error))
        {
            if (!doNotShowOnSuccess)
            {
                wxMessageBox(contextMessage + "No syntax errors found.",
                            kApplicationName,
                            wxOK | wxICON_INFORMATION, this);
            }
            return true;
        }
        else
        {
            wxMessageBox(contextMessage + "Syntax error found:\n\n" + ToWx(error),
                        kApplicationName,
                        wxOK | wxICON_ERROR, this);
        }  
        return false;
    }

    void MainFrame::OnShow(wxCommandEvent&)
    {
        if (!m_specification.IsLoaded())
        {
            return;
        }

        wxDialog dialog(this, wxID_ANY, "Function specification", wxDefaultPosition,
                        wxSize(900, 650), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(new wxTextCtrl(&dialog, wxID_ANY, ToWx(m_specification.ToText()),
                                wxDefaultPosition, wxDefaultSize,
                                wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP),
                   1, wxEXPAND | wxALL, 8);
        wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
        wxButton* syntaxCheckButton = new wxButton(&dialog, wxID_ANY, "Syntax check");
        syntaxCheckButton->Bind(wxEVT_BUTTON, &MainFrame::OnValidation, this);
        buttonSizer->Add(syntaxCheckButton);
        buttonSizer->AddStretchSpacer();
        buttonSizer->Add(dialog.CreateButtonSizer(wxOK));
        sizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 8);
        dialog.SetSizer(sizer);
        dialog.CentreOnParent();
        dialog.ShowModal();
    }

    void MainFrame::OnCreateAPI(wxCommandEvent&)
    {
        const wxArrayString languages = {"C", "C++", "Rust"};
        wxSingleChoiceDialog dialog(this, "Select target language", "Create API", languages);

        wxString lastLanguage;
        wxConfigBase::Get()->Read(kCreateApiLanguageConfigKey, &lastLanguage);
        const int lastIndex = languages.Index(lastLanguage);
        if (lastIndex != wxNOT_FOUND)
        {
            dialog.SetSelection(lastIndex);
        }

        if (dialog.ShowModal() != wxID_OK)
        {
            return;
        }

        wxConfigBase::Get()->Write(kCreateApiLanguageConfigKey, dialog.GetStringSelection());
        wxConfigBase::Get()->Flush();

        wxMessageBox("Create API for " + dialog.GetStringSelection() + ":\n\nNOT IMPLEMENTED",
                     kApplicationName,
                     wxOK | wxICON_INFORMATION, this);
    }

    void MainFrame::OnAbout(wxCommandEvent&) 
    {
        wxDialog dialog(this, wxID_ANY, "About " + wxString(kApplicationName), wxDefaultPosition,
                        wxDefaultSize, wxDEFAULT_DIALOG_STYLE);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        wxImage logoImage(FindRuntimeFile(kLogoFileNameLarge), wxBITMAP_TYPE_PNG);
        if (logoImage.IsOk())
        {
            sizer->Add(new wxStaticBitmap(&dialog, wxID_ANY, wxBitmap(logoImage)), 0,
                       wxALIGN_CENTER_HORIZONTAL | wxALL, 12);
        }
        sizer->Add(new wxStaticText(&dialog, wxID_ANY, wxString(kApplicationName) + "\n\n"
                    "Reads, displays and writes Eclipse autoapiframework function\n"
                    "specifications (*.afs, YAML content) based on the framework meta model.",
                    wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL),
                   0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT | wxBOTTOM, 12);
        sizer->Add(dialog.CreateSeparatedButtonSizer(wxOK), 0, wxEXPAND | wxALL, 8);
        dialog.SetSizerAndFit(sizer);
        dialog.CentreOnParent();
        dialog.ShowModal();
    }

    void MainFrame::OnGetVssToolVersion(wxCommandEvent&)
    {
        auto version = GetVssToolsVersion();
        wxMessageBox("Recommended vss-tools version: 6.1\n\nFound: " + version,
                    kApplicationName,
                    wxOK | wxICON_INFORMATION, this);
    }

    void MainFrame::OnQuickTips(wxCommandEvent&)
    {
        std::string title = "";
        wxMessageBox( title + "\nThe automatic syntax check performed before saving a file to disk can be disabled.\n" +
                     "\nAdd 'null' to mandatory properties so that they are accepted by the syntax check.\n"         
                     "\nThe supervision node on the Scheduling collection is editable in the 'Edit scheduling item' dialog.\n" +
                     "\nAPI creation is not implemented yet.",
                     kApplicationName,
                     wxOK | wxICON_INFORMATION, this);
    } 

    void MainFrame::OnHelp(wxCommandEvent&) { wxLaunchDefaultBrowser(kHelpUrl); }

    void MainFrame::OnExit(wxCommandEvent&) { Close(true); }

    void MainFrame::LoadSpecification(const wxString& path) 
    {
        std::string error;
        FunctionSpecification specification;
        if (!specification.Load(m_metaModel.GetVersion(), ToStd(path), error)) 
        {
            wxMessageBox("Could not read function specification:\n\n" + ToWx(error),
                        kApplicationName, wxOK | wxICON_ERROR, this);
            return;
        }
        m_contentChanged = false;
        m_specification = std::move(specification);
        m_newButton->Enable(false);
        GetMenuBar()->Enable(ID_NewSpecification, false);
        m_saveButton->Enable(m_saveButton);
        m_saveAsButton->Enable(true);
        GetMenuBar()->Enable(ID_SaveSpecification, m_contentChanged);	        
        GetMenuBar()->Enable(ID_SaveSpecificationAs, true);				
        m_showButton->Enable(true);
        GetMenuBar()->Enable(ID_ShowMetaModel, true);				
        m_validationButton->Enable(true);        
		m_createAPIButton->Enable(true);
        RefreshAll();
        UpdateTitleAndStatus();
    }

    bool MainFrame::LoadMetaModel(const wxString& path, bool reportErrors) 
    {
        std::string error;
        if (!m_metaModel.Load(ToStd(path), error)) 
        {
            if (reportErrors) 
            {
                wxMessageBox("Could not read meta model:\n\n" + ToWx(error), kApplicationName,
                            wxOK | wxICON_ERROR, this);
            }
            return false;
        }
        m_metaModelLabel->SetLabel("Meta model: " + ToWx(m_metaModel.GetName()) + " " +
                                ToWx(m_metaModel.GetVersion()));
        m_metaModelLabel->GetParent()->Layout();
        return true;
    }

    void MainFrame::RefreshAll() 
    {
        GetMenuBar()->Enable(ID_SaveSpecification, m_contentChanged);	        
        m_saveButton->Enable(m_contentChanged);  
        FillAttributes();
        FillCollection(m_signalList, FunctionSpecification::kDataInterfacesKey, kDataInterfaceTypeKey);
        FillCollection(m_parameterList, FunctionSpecification::kParametersKey, kParameterInterfaceTypeKey);
        FillCollection(m_schedulingList, FunctionSpecification::kSchedulingKey, kSchedulingInterfaceTypeKey);
        FillCollection(m_errorList, FunctionSpecification::kErrorsKey, kErrorInterfaceTypeKey);

        m_notebook->SetPageText(1, wxString::Format(
            "Signal collection (%d)",
            static_cast<int>(m_specification.GetCollection(FunctionSpecification::kDataInterfacesKey).size())));
        m_notebook->SetPageText(2, wxString::Format(
            "Parameter collection (%d)",
            static_cast<int>(m_specification.GetCollection(FunctionSpecification::kParametersKey).size())));
        m_notebook->SetPageText(3, wxString::Format(
            "Scheduling collection (%d)",
            static_cast<int>(m_specification.GetCollection(FunctionSpecification::kSchedulingKey).size())));
        m_notebook->SetPageText(4, wxString::Format(
            "Error collection (%d)",
            static_cast<int>(m_specification.GetCollection(FunctionSpecification::kErrorsKey).size())));
        UpdateCollectionButtonStates();
    }

    void MainFrame::FillAttributes() 
    {
        m_attributeList->DeleteAllItems();
        long row = 0;
        for (const auto& attribute : m_specification.GetAttributes()) 
        {
            if (attribute.first == kNameKey || attribute.first == kVersionKey  ||
                attribute.first == kDescriptionKey)
            {
                continue;
            }
            m_attributeList->InsertItem(row, ToWx(attribute.first));
            m_attributeList->SetItem(row, 1, OneLine(attribute.second));
            ++row;
        }
        m_nameText->SetValue(ToWx(m_specification.GetName()));
        m_versionText->SetValue(ToWx(m_specification.GetVersion()));
        m_descriptionText->SetValue(ToWx(m_specification.GetDescription()));
    }

    void MainFrame::FillCollection(wxListCtrl* list, const std::string& collectionKey,
                                const std::string& interfaceTypeName) 
    {
        const std::vector<YamlNodePtr> entries = m_specification.GetCollection(collectionKey);
        const std::vector<std::string> columns = m_metaModel.ColumnsFor(interfaceTypeName, entries);

        list->ClearAll();
        list->AppendColumn("#", wxLIST_FORMAT_RIGHT, kFirstColumnWidth);
        for (const std::string& column : columns) 
        {
            list->AppendColumn(ToWx(column), wxLIST_FORMAT_LEFT, kMinColumnWidth);
        }

        long row = 0;
        for (const YamlNodePtr& entry : entries) 
        {
            list->InsertItem(row, wxString::Format("%ld", row + 1));
            for (std::size_t column = 0; column < columns.size(); ++column) 
            {
                list->SetItem(row, static_cast<int>(column) + 1, CellValue(entry, columns[column]));
            }
            ++row;
        }
        AutoSizeColumns(list);
    }

    wxListCtrl* MainFrame::GetSelectedCollectionList() const
    {
        switch (m_notebook->GetSelection())
        {
        case 1:
            return m_signalList;
        case 2:
            return m_parameterList;
        case 3:
            return m_schedulingList;
        case 4:
            return m_errorList;
        default:
            return nullptr;
        }
    }

    const char* MainFrame::GetSelectedCollectionKey() const
    {
        switch (m_notebook->GetSelection())
        {
        case 1:
            return FunctionSpecification::kDataInterfacesKey;
        case 2:
            return FunctionSpecification::kParametersKey;
        case 3:
            return FunctionSpecification::kSchedulingKey;
        case 4:
            return FunctionSpecification::kErrorsKey;
        default:
            return nullptr;
        }
    }

    std::string MainFrame::GetSelectedInterfaceType() const
    {
        switch (m_notebook->GetSelection())
        {
        case 1:
            return kDataInterfaceTypeKey;
        case 2:
            return kParameterInterfaceTypeKey;
        case 3:
            return kSchedulingInterfaceTypeKey;
        case 4:
            return kErrorInterfaceTypeKey;
        default:
            return std::string();
        }
    }

    void MainFrame::CompleteEntryFromMetaModel(const std::string& interfaceType,
                                               const YamlNodePtr& entry) const
    {
        if (interfaceType.empty() || !entry || !entry->IsMap())
        {
            return;
        }

        for (const std::string& field : m_metaModel.ColumnsFor(interfaceType, {}))
        {
            if (interfaceType == kSchedulingInterfaceTypeKey && field == kExecutionResultKey)
            {
                YamlNodePtr result = entry->Find(field);
                if (!result || !result->IsMap())
                {
                    result = YamlNode::MakeMap();
                    entry->Set(field, result);
                }
                if (!result->Find(kExecutionResultTypeKey))
                {
                    std::string enumReference;
                    if (const YamlNodePtr* type = m_metaModel.FindInterfaceType(interfaceType))
                    {
                        const YamlNodePtr properties = (*type)->Find(kPropertiesKey);
                        const YamlNodePtr definition = properties ? properties->Find(field) : nullptr;
                        if (definition)
                        {
                            enumReference = definition->ScalarOf(kEnumRefKey);
                        }
                    }
                    result->Set(kExecutionResultTypeKey, YamlNode::MakeScalar(enumReference));
                }
                continue;
            }

            if (interfaceType == kSchedulingInterfaceTypeKey && field == kSupervisionKey)
            {
                YamlNodePtr supervision = entry->Find(field);
                if (!supervision || !supervision->IsMap())
                {
                    supervision = YamlNode::MakeMap();
                    entry->Set(field, supervision);
                }
                if (!supervision->Find(kSupervisionRequiredKey))
                {
                    supervision->Set(kSupervisionRequiredKey, YamlNode::MakeScalar("false"));
                }
                continue;
            }

            if (interfaceType == kDataInterfaceTypeKey && field == kQualityCodeKey)
            {
                YamlNodePtr companion = entry->Find(kRuntimeCompanionKey);
                if (!companion || !companion->IsMap())
                {
                    companion = YamlNode::MakeMap();
                    entry->Set(kRuntimeCompanionKey, companion);
                }
                YamlNodePtr quality = companion->Find(kQualityCodeKey);
                if (!quality || !quality->IsMap())
                {
                    quality = YamlNode::MakeMap();
                    quality->Set(kNameKey, YamlNode::MakeScalar(""));
                    quality->Set(kDatatypeKey, YamlNode::MakeScalar(kEnumKey));
                    quality->Set(kEnumRefKey, YamlNode::MakeScalar(kDataQualityKey));
                    companion->Set(kQualityCodeKey, quality);
                }
                continue;
            }

            if (!entry->Find(field))
            {
                entry->Set(field, YamlNode::MakeScalar(""));
            }
        }
    }

    void MainFrame::UpdateCollectionButtonStates()
    {
        wxListCtrl* selectedList = GetSelectedCollectionList();
        const bool hasSelectedItem = selectedList && selectedList->GetSelectedItemCount() > 0;
        m_addWithVspecFileButton->Enable(m_notebook->GetSelection() == 1);
        m_addButton->Enable(selectedList != nullptr);
        m_deleteButton->Enable(hasSelectedItem);
        m_editButton->Enable(hasSelectedItem);
    }

    void MainFrame::UpdateMetaModelButtonStates()
    {
        const bool metaModelLoaded = m_metaModel.IsLoaded();
        const bool specificationLoaded = m_specification.IsLoaded();     
        m_newButton->Enable(metaModelLoaded && !specificationLoaded);
        GetMenuBar()->Enable(ID_NewSpecification, metaModelLoaded && !specificationLoaded);		
        m_readButton->Enable(metaModelLoaded);
        GetMenuBar()->Enable(ID_OpenSpecification, metaModelLoaded);				
        m_metaButton->Enable(!metaModelLoaded);
        m_showMetaModelButton->Enable(metaModelLoaded);
        GetMenuBar()->Enable(ID_ShowMetaModel, metaModelLoaded);

    }

    void MainFrame::UpdateTitleAndStatus() 
    {
        if (!m_specification.IsLoaded()) 
        {
            SetTitle(kApplicationName);
            SetStatusText("No function specification loaded", 0);
            SetStatusText(m_metaModel.IsLoaded() ? "Meta model loaded" : "No meta model", 1);
            return;
        }
        SetTitle(wxString(kApplicationName) + " - " + ToWx(m_specification.GetName()) + " [" +
                wxFileName(ToWx(m_specification.GetSourcePath())).GetFullName() + "]");
        SetStatusText("Read: " + ToWx(m_specification.GetSourcePath()), 0);
        SetStatusText("Meta model reference: " + ToWx(m_specification.GetMetaModelName()) + " " +
                        ToWx(m_specification.GetMetaModelVersion()),
                    1);
    }

    std::string MainFrame::GetGitTagAndVersion(const std::string& prefix, const wxString& repoDir)
    {
        std::string version = "";
        wxString gitVersion = GetGitVersion(repoDir);
        wxString gitTag = GetGitTag(repoDir);
        if(!gitTag.empty() && !gitVersion.empty() && gitTag != "Unknown" && gitVersion != "Unknown")
        {
            version = prefix + gitTag + " (Commit " + gitVersion + ")";
        }
        return version;
    }

    wxString MainFrame::GetGitVersion(const wxString& repoDir)
    {
        wxArrayString output, errors;
        wxString cmd = wxString::Format("git -C \"%s\" describe --tags", repoDir);

        long rc = wxExecute(cmd, output, errors, wxEXEC_SYNC);
        if (rc != 0 || output.IsEmpty())
        {
            return "Unknown";
        }

        return output[0];
    }

    wxString MainFrame::GetGitTag(const wxString& repoDir)
    {
        wxArrayString output, errors;
        wxString cmd = wxString::Format("git -C \"%s\" describe --tags --abbrev=0", repoDir);

        long rc = wxExecute(cmd, output, errors, wxEXEC_SYNC);
        if (rc != 0 || output.IsEmpty())
        {
            return "Unknown";
        }

        return output[0];
    }

    wxString MainFrame::GetVspecExecutable()
    {
        return wxString::FromUTF8(AFD_VSPEC_EXECUTABLE);
    }

    bool MainFrame::IsVspecAvailable()
    {
        const wxString executable = GetVspecExecutable();
        return !executable.IsEmpty() && wxFileName::FileExists(executable);
    }

    wxString MainFrame::GetVspecUnavailableMessage()
    {
        const wxString executable = GetVspecExecutable();
        if (executable.IsEmpty())
        {
            return "vss-tools was not installed when this application was built, "
                   "so vspec files cannot be imported.\n\n"
                   "Install vss-tools and re-run CMake to enable this feature.";
        }

        return "vss-tools is not installed on this machine, so vspec files cannot be imported.\n\n"
               "The vspec executable configured at build time is not present:\n" + executable +
               "\n\nInstall vss-tools at that location, or rebuild the application on this machine.";
    }

    wxString MainFrame::RunVspec2Json(const wxString& vspecFile, const wxString& outputFile)
    {
        wxArrayString output, errors;
        const wxString executable = GetVspecExecutable();
        if (!IsVspecAvailable())
        {
            return "ERROR: " + GetVspecUnavailableMessage();
        }

        wxString cmd = wxString::Format(
            "\"%s\" export json --vspec \"%s\" --output \"%s\"",
            executable,
            vspecFile,
            outputFile);

        long rc = wxExecute(cmd, output, errors, wxEXEC_SYNC);
        if (rc != 0)
        {
            wxString err;
            for (const auto& e : errors)
            {
                err += e + "\n";
            }

            return err.empty()
                ? wxString::Format("ERROR: VSPEC conversion failed with exit code %ld.", rc)
                : "ERROR: " + err;
        }

        return outputFile;
    }

    std::string  MainFrame::GetVssToolsVersion()
    {
    #ifdef _WIN32
        const char* cmd = getenv("VIRTUAL_ENV") ? "%USERPROFILE%\\venvs\\vss-tools\\Scripts\\pip.exe show vss-tools 2>NUL" : "pip show vss-tools 2>NUL";
        FILE* pipe = _popen(cmd, "r");
    #else

        const char* cmd = getenv("VIRTUAL_ENV") ? "$HOME/venvs/vss-tools/bin/pip show vss-tools 2>/dev/null" : "pip show vss-tools 2>/dev/null";
        FILE* pipe = popen(cmd, "r");
    #endif

        if (!pipe)
            return "";

        char buffer[256];
        std::string output;

        while (fgets(buffer, sizeof(buffer), pipe))
            output += buffer;

    #ifdef _WIN32
        _pclose(pipe);
    #else
        pclose(pipe);
    #endif

        std::regex versionRegex(R"(Version:\s*([^\r\n]+))");
        std::smatch match;

        if (std::regex_search(output, match, versionRegex))
            return match[1];

        return "";
    }

    void  MainFrame::ShowVssToolsVersion()
    {
        auto version = GetVssToolsVersion();
        if (version.find("6.1") == std::string::npos) 
        {
            wxMessageBox("vss-tools 6.1 required, found: " + version,
                    kApplicationName,
                    wxOK | wxICON_ERROR, this);     
        }
    }
} // namespace afd
