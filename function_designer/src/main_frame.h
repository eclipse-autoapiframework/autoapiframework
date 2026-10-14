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
 
#ifndef AFD_MAIN_FRAME_H
#define AFD_MAIN_FRAME_H

#include <wx/wx.h>

#include <string>

#include "function_specification.h"
#include "meta_model.h"

class wxListCtrl;
class wxNotebook;

namespace afd 
{
    static constexpr const char* kApplicationName = "AutoAPI Function Designer - Preview 0.1";

    /// Main window of the AutoAPI Function Designer application.
    class MainFrame : public wxFrame 
    {
    public:
        MainFrame();

    private:
        enum 
        {
            ID_NewSpecification = wxID_HIGHEST + 1,
            ID_OpenSpecification,
            ID_SaveSpecification,
            ID_SaveSpecificationAs,
            ID_OpenMetaModel,
            ID_ShowMetaModel,
            ID_Settings,
            ID_GetVssToolsVersion,
            ID_QuickTips,            
            ID_AddVWithVspecFile,
            ID_Add,
            ID_Delete,
            ID_Edit,
            ID_Validation,
            ID_Show,
            ID_CreateAPI
        };

        void BuildUi();
        bool AutoLoadMetaModel();
        void UpdateMetaModelButtonStates();

        void OnNewSpecification(wxCommandEvent& event);
        void OnOpenSpecification(wxCommandEvent& event);
        void OnSaveSpecification(wxCommandEvent& event);
        void OnSaveSpecificationAs(wxCommandEvent& event);
        void OnOpenMetaModel(wxCommandEvent& event);
        void OnShowMetaModel(wxCommandEvent& event);
        void OnSettings(wxCommandEvent& event);
        void OnGetVssToolVersion(wxCommandEvent& event);
        void OnQuickTips(wxCommandEvent& event);      
        void OnAddWithVspecFile(wxCommandEvent& event);     
        void OnAdd(wxCommandEvent& event);
        void OnDelete(wxCommandEvent& event);
        void OnEdit(wxCommandEvent& event);
        void OnValidation(wxCommandEvent& event);
        void OnShow(wxCommandEvent& event);
        void OnCreateAPI(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);   
        void OnHelp(wxCommandEvent& event);
        void OnExit(wxCommandEvent& event);

        void LoadSpecification(const wxString& path);
        bool LoadMetaModel(const wxString& path, bool reportErrors);

        bool SyntaxCheckIsOK(bool doNotShowOnSuccess, const std::string& contextMessage = "");

        void RefreshAll();
        void FillAttributes();
        void FillCollection(wxListCtrl* list, const std::string& collectionKey,
                            const std::string& interfaceTypeName);
        wxListCtrl* GetSelectedCollectionList() const;
        const char* GetSelectedCollectionKey() const;
        std::string GetSelectedInterfaceType() const;
        /// Adds the meta model fields that are missing in @p entry so that every column can be edited.
        void CompleteEntryFromMetaModel(const std::string& interfaceType, const YamlNodePtr& entry) const;
        void UpdateCollectionButtonStates();
        void UpdateTitleAndStatus();

        FunctionSpecification m_specification;
        MetaModel   m_metaModel;
        bool m_contentChanged = false;

        wxNotebook* m_notebook = nullptr;
        wxListCtrl* m_attributeList = nullptr;
        wxListCtrl* m_signalList = nullptr;
        wxListCtrl* m_parameterList = nullptr;
        wxListCtrl* m_schedulingList = nullptr;
        wxListCtrl* m_errorList = nullptr;
        wxTextCtrl* m_nameText = nullptr;
        wxTextCtrl* m_versionText = nullptr;
        wxTextCtrl* m_descriptionText = nullptr;
        wxStaticText* m_metaModelLabel = nullptr;
        wxButton* m_newButton = nullptr;
        wxButton* m_readButton = nullptr;
        wxButton* m_metaButton = nullptr;
        wxButton* m_showMetaModelButton = nullptr;
        wxButton* m_settingsButton = nullptr;
        wxButton* m_saveButton = nullptr;
        wxButton* m_saveAsButton = nullptr;
        wxButton* m_addWithVspecFileButton = nullptr;
        wxButton* m_addButton = nullptr;
        wxButton* m_deleteButton = nullptr;
        wxButton* m_editButton = nullptr;
        wxButton* m_validationButton = nullptr;
        wxButton* m_showButton = nullptr;
        wxButton* m_createAPIButton = nullptr;

        std::string GetVssToolsVersion();
        void ShowVssToolsVersion();
        static bool IsVspecAvailable();
        static wxString GetVspecExecutable();
        static wxString GetVspecUnavailableMessage();
        wxString RunVspec2Json(const wxString& vspecFile, const wxString& outputFile);
        wxString GetGitVersion(const wxString& repoDir);
        wxString GetGitTag(const wxString& repoDir);
        std::string GetGitTagAndVersion(const std::string& prefix, const wxString& repoDir);

        wxDECLARE_EVENT_TABLE();
    };

} // namespace afd

#endif // AFD_MAIN_FRAME_H
