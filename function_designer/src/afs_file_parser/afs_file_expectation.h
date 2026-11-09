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
 *   Thomas Pfleiderer - afs file parser added
 ********************************************************************************/

#ifndef AFP_AFS_FILE_EXPECTATION_H
#define AFP_AFS_FILE_EXPECTATION_H

#include <string>

#include "afs_types.h"

namespace afp 
{
    class AfsFileExpectation
    {
    public:
        /// Loads the expectations file and compares it with the parsed specification.
        /// Returns true if all expectations are met; otherwise `error` describes the problem(s).
        bool CheckExpectation(const std::string& expectationsFile, const FunctionSpecification& spec,
                              std::string& error) const;
    };
} // namespace afp

#endif // AFP_AFS_FILE_EXPECTATION_H
