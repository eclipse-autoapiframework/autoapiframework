..
   # *******************************************************************************
   # Copyright (c) 2026 Contributors to the Eclipse Foundation
   #
   # See the NOTICE file(s) distributed with this work for additional
   # information regarding copyright ownership.
   #
   # This program and the accompanying materials are made available under the
   # terms of the Apache License Version 2.0 which is available at
   # https://www.apache.org/licenses/LICENSE-2.0
   #
   # SPDX-License-Identifier: Apache-2.0
   #
   # Contributors:
   #   Thomas Pfleiderer - first documentation
   # *******************************************************************************

Blueprint project (IFEX + VAF)
==============================

This blueprint extends the one with VSS as provided in `bp-vss-vaf <../bp-vss-vaf/>`__. Instead of VSS, Interface Exchange (IFEX) is used as description and input format.

IFEX
----

IFEX does not follow the signal-tree approach as realized by VSS. Instead, datatypes and complete interfaces can be defined in a layered way. The representation and editing format is YAML. Please find the corresponding IFEX file for the speed hazard detection sample application in the `ifex-sample-zf.yaml <../../../../examples/bp-ifex-vaf/model/ifex/ifex-sample-zf.yaml>`__.

VAF interface project
---------------------

This file is consumed as input artifact by a VAF interface project. Based on this input, the following interfaces are defined in the Config as Code file `interfaces.py <../../../../examples/bp-ifex-vaf/Interfaces/interfaces.py>`__: 

- SpeedIf
- SpeedCalculationIf 
- HazardIf 
- HazardDetectionIf

Finally, the complete interface definition gets exported to the VAF model format in JSON.

Next steps
----------

From here, the same steps apply as described for the VSS blueprint. See the corresponding `README <../../../../examples/bp-vss-vaf/README.md>`__ and `project folder <../../../../examples/bp-ifex-vaf/>`__ for details. Application module and integration can be reused from there.
