
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
   # *******************************************************************************

Current status of the project:

.. figure:: ../meta_model/figures/time_line.png
   :alt: time line of the project

Software Architecture
=====================

Runtime Specification File
--------------------------

A :doc:`Function Specification File </doc/function_specification/function_specification_file>` fully describes exactly **one** function in
isolation: its signals, its parameters, and the scheduling of its own runnables. It intentionally does not know anything about the other
functions running on the same system.

A real system, however, is composed of several such functions that need to be instantiated and executed together, in a defined order, and
connected to the same middleware. The **Runtime Specification File** captures this system-level composition on top of one or more Function
Specification Files:

- **Function reference** - which Function Specification Files (i.e. which functions) are part of this runtime/system.
- **Sequencing** - the execution order across those functions, i.e. how the ``previousRunnableRef`` chains of the individual functions are
  stitched together into one overall schedule.

.. figure:: figures/runtime_specification_architecture.png
   :alt: One Specification - many artefacts

As shown above, each ``Software Function`` (the function's business logic) is executed through its own ``Function Adapter``, which wraps it
behind the stable, middleware-independent contract defined by the meta model. The ``Runtime Scheduler`` is generated from the Runtime
Specification File and is responsible for triggering the OS-level ``Init``/``Compute``/``Terminate`` calls of every function, in the order
described by the Runtime Specification File, through the ``Middleware Abstraction Layer``.

.. note::
   The Runtime Specification File is not yet formally defined in the meta model (unlike the Function Specification File, which is defined by
   ``Eclipse-autoapiframework-Metamodel`` version 0.4.0). The concepts above describe the intended scope; a versioned schema, file extension,
   and worked examples are still to be added.

Open topics still to be worked out for the Runtime Specification File:

- Mapping of an output signal of one function to the input signal of another (data flow / connections).
- Explicit dependency declarations between functions, beyond the ordering implied by ``previousRunnableRef``.
- Initialization / system setup information.
- Deployment information. 
