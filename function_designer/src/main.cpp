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
#include <wx/wx.h>

#include "main_frame.h"

namespace afd 
{
    class DesignerApp : public wxApp 
    {
    public:
        bool OnInit() override 
        {
            SetAppName(afd::kApplicationName);
            wxInitAllImageHandlers();
            MainFrame* frame = new MainFrame();
            frame->Show(true);
            return true;
        }
    };

} // namespace afd

wxIMPLEMENT_APP(afd::DesignerApp);
