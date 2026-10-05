
..
   # *******************************************************************************
   # Copyright (c) 2026 ZF Friedrichshafen AG
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
   #   Thomas Pfleiderer - Meta model added
   #   Saran Gundlapalli - function_specification_examples.rst updated according to metamodel v0.4.0
   # *******************************************************************************

As no final API specification is available yet, **these examples should be considered preliminary drafts.** They demonstrate the metamodel concepts and shall reuse canonical VSS semantics where possible. Any non-standard catalogue path must be identified as an approved extension.

Examples
========

Use the single-rate example when one periodic runnable is sufficient for a function, for example simple signal conditioning or threshold checks with one execution rhythm.

Use the multi-rate example when preprocessing and decision logic should run at different cycle times, for example 10 ms input conditioning and 20 ms output publication.

.. toctree::
   :caption: SpeedHazardDetection (single cyclic runnable)
   :maxdepth: 1

   examples/speed_hazard_detection_example

.. toctree::
   :caption: VehicleSpeedFusion (multi-rate cyclic runnables at 10 ms and 20 ms) 
   :maxdepth: 1

   examples/vehicle_speed_fusion_multirate_example

.. toctree::
   :caption: wheel Speed Plausibility Check including error and supervision example
   :maxdepth: 1

   examples/wheel_speed_plausibility_check_example

API
---

.. dropdown:: speed_hazard_detection.cpp
   :icon: code
   
   .. literalinclude:: examples/speed_hazard_detection.cpp
      :language: cpp
      :linenos:

.. dropdown:: speed_hazard_detection.hpp
   :icon: code
   
   .. literalinclude:: examples/speed_hazard_detection.hpp
      :language: cpp
      :linenos:
