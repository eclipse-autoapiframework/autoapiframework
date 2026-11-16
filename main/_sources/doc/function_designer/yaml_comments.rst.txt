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
   #   Thomas Pfleiderer - afs file parser added
   #   Thomas Pfleiderer - support 4.1, Support YAML comments, Drag & Drop, Enums in afs files added
   # *******************************************************************************

YAML comments in AFS files
==========================

The Function Designer preserves supported YAML comments when it parses and
writes an AFS file. Comments are attached to the following mapping entry or
sequence item, then written with that node. The supported syntax is:

Full-line comments
------------------

Put ``#`` at the start of a line (indentation is allowed). Full-line comments
can appear before a mapping entry or sequence item. Consecutive comment lines
are preserved together; blank lines between comments and the next node are
skipped.

.. code-block:: yaml

   # Vehicle signal definition
   # Speed reported by the sensor
   name: VehicleSpeed

   values:
     # The vehicle is stationary
     - PARK
     - DRIVE

Inline comments
---------------

Put an inline comment after a non-empty scalar value, separated from the value
by at least one space or tab. The comment continues to the end of the line.

.. code-block:: yaml

   dataType: enum  # Signal uses an enumeration
   values:
     - PARK  # Vehicle is stationary

Multiple comment lines
----------------------

YAML has no block-comment syntax. Write each comment line with its own ``#``.
This also applies when temporarily commenting out configuration entries.

.. code-block:: yaml

   # Define the signal type
   # and its associated enumeration.
   dataType: enum
   enumRef: GearPosition

   # dataType: int32
   # default: 0

Hash characters in values
-------------------------

A ``#`` inside a quoted string is part of the value, not a comment. A ``#``
inside a block scalar (``|``, ``|-``, ``>``, or ``>-``) is also literal text.

.. code-block:: yaml

   message: "# is part of this string"
   errorCode: 'Error #42'
   description: >-
     Vehicle status:
     # parked

Implementation notes and limits
--------------------------------

The parser recognizes inline comments when ``#`` is at the start of the
remainder or is preceded by a space or tab; quoting is respected. The writer
emits leading comments before mapping entries and sequence items, and inline
comments after scalar values and scalar sequence items.

Comment preservation is intended for parse/write round trips through the
Function Designer; it does not preserve the original spacing or exact comment
placement. In particular, do not put an inline comment on a block-scalar
header (for example, ``description: >-  # comment``), because that comment is
not emitted when the file is written. Comments after the final node are not
guaranteed to be retained.