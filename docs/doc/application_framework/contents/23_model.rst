..
   # *******************************************************************************
   # Copyright (c) 2024 Contributors to the Eclipse Foundation
   #
   # See the NOTICE file(s) distributed with this work for additional
   # information regarding copyright ownership.
   #
   # This program and the accompanying materials are made available under the
   # terms of the Apache License Version 2.0 which is available at
   # https://www.apache.org/licenses/LICENSE-2.0
   #
   # SPDX-License-Identifier: Apache-2.0
   # *******************************************************************************

Model (vafmodel)
================

The underlying data model that is used as sole input for all generation steps is available in form of a `Pydantic <https://docs.pydantic.dev/latest/>`__ model named *vafmodel*.

A JSON schema can also be generated from this representation to enable the import and export of JSON files to and from a given vafmodel instance. In the following, the basic structure of the vafmodel is presented.

MainModel
---------

The root of each instance of a vafmodel is given by the **class MainModel**. It consists of the following members:

- **schema_reference**: Is an optional string with field alias "$schema", where the string should contain the location of the JSON schema file if set, to help other tools that allow schema validation to do this with the exported JSON.
- **BaseTypes**: Is a list of BaseTypes. The class BaseType is presented below.
- **DataTypeDefinitions**: Is a list of DataTypeDefinitions. The class DataTypeDefinition is presented below.
- **ModuleInterfaces**: Is a list of ModuleInterfaces. The class ModuleInterface is presented below.
- **ApplicationModules**: Is a list of ApplicationModules. The class ApplicationModule is presented below.
- **PlatformConsumerModules**: Is a list of PlatformModules. Here, in the role of a module interface consumer from the perspective of an application module that uses this module. The class PlatformModule is presented below.
- **PlatformProviderModules**: Is a list of PlatformModules. Here, in the role of a module interface provider from the perspective of an application module that uses this module. The class PlatformModule is presented below.
- **Executables**: Is a list of Executables. The class Executable is presented below.
- **SILKITAdditionalConfiguration**: Is an optional member that contains the necessary information for the provision of the SIL Kit consumer and provider modules. The class SILKITAdditionalConfigurationType is presented below.

BaseType
--------

The **class BaseType** is an enum type with which can take the following values: 

- "int8_t"
- "int16_t"
- "int32_t"
- "int64_t"
- "uint8_t"
- "uint16_t"
- "uint32_t"
- "uint64_t"
- "float"
- "double"
- "bool"

Each of these literals stands for an intrinsic datatype of the target language C++.

DataTypeDefinition
------------------

The **class DataTypeDefinition** is a container for complex datatypes with the same namespace. It consists of the following members:

- **Namespace**: A string value containing the namespace as value.
- **Strings**: Is a list of Strings. The class String is presented below.
- **Enums**: Is a list of VafEnums. The class VafEnum is presented below.
- **Arrays**: Is a list of Arrays. The class Array is presented below.
- **Maps**: Is a list of Maps. The class Map is presented below.
- **TypeRefs**: Is a list of TypeRefs. The class TypeRef is presented below.
- **Structs**: Is a list of Structs. The class Struct is presented below.
- **Vectors**: Is a list of Vectors. The class Vector is presented below.

Strings
-------

The **class String** defines a string type. It has only one member:

- **Name**: A string value containing the name of the string type as value.

VafEnum
-------

The **class VafEnum** defines an enum type. It consists of the following members:

- **Name**: A string value containing the name of the enum type as value.
- **BaseType**: Is an optional DataTypeRef. The class DataTypeRef is presented below.
- **Literals**: Is a list of EnumLiterals. The class EnumLiteral is presented below.

EnumLiteral
-----------

The **class EnumLiteral** defines a literal of an enum datatype. It consists of the following members:

- **Label**: A string value containing the name of the label as value.
- **Value**: An integer value containing the value of the enumeration for given label.

Arrays
------

The **class Array** defines an array type. It consists of the following members:

- **Name**: A string value containing the name of the array type as value.
- **TypeRef**: A DataTypeRef referencing to the base type of the array. The class DataTypeRef is presented below.
- **Size**: An integer value containing the size of the array.

Maps
----

The **class Map** defines a map type. It consists of the following members:

- **Name**: A string value containing the name of the map type as value.
- **MapKeyTypeRef**: A DataTypeRef referencing to the key type of the map. The class DataTypeRef is presented below.
- **MapValueTypeRef**: A DataTypeRef referencing to the value type of the map. The class DataTypeRef is presented below.

TypeRefs
--------

The **class TypeRef** defines a typeref type. It consists of the following members:

- **Name**: A string value containing the name of the typeref type as value.
- **TypeRef**: A DataTypeRef referencing to the referenced type of the typeref. The class DataTypeRef is presented below.

Structs
-------

The **class Struct** defines a struct type. It consists of the following members:

- **Name**: A string value containing the name of the struct type as value.
- **SubElements**: Is a list of SubElements. The class SubElement is presented below.

SubElement
----------

The **class SubElement** defines a sub element of a struct. It consists of the following members:

- **Name**: A string value containing the name of the struct sub element as value.
- **TypeRef**: A DataTypeRef referencing to the base type of the sub element. The class DataTypeRef is presented below.
- **Min**: An optional floating point value representing the minimum value that the sub element can have as a value.
- **Max**: An optional floating point value representing the maximum value that the sub element can have as a value.

Vectors
-------

The **class Vector** defines a vector type. It consists of the following members:

- **Name**: A string value containing the name of the vector type as value.
- **TypeRef**: A DataTypeRef referencing to the base type of the vector. The class DataTypeRef is presented below.
- **Size**: An optional integer value containing the initial size of the vector.

Executable
----------

The **class Executable** defines an executable. It consists of the following members:

- **Name**: A string value containing the name of the executable as value.
- **ExecutorPeriod**: A string value containing main executor period of the executable as value.
- **InternalCommunicationModules**: Is a list of PlatformModules defining internal communication. The class PlatformModule is presented below. 
- **ApplicationModules**: Is a list of ExecutableApplicationModuleMappings. The class ExecutableApplicationModuleMapping is presented below.

ExecutableApplicationModuleMapping
----------------------------------

The **class ExecutableApplicationModuleMapping** defines a mapping of an application module to the executable. The mapping includes in particular the concrete instantiation of the abstract instantiations of the provided and consumed module interfaces of the application module.

- **ApplicationModuleRef**: A ApplicationModuleRefType referencing to the application module. The class ApplicationModuleRefType is presented below.
- **InterfaceInstanceToModuleMappings**: Is a list of InterfaceInstanceToModuleMappings. The class InterfaceInstanceToModuleMapping is presented below.
- **TaskMapping**: Is a list of ExecutableTaskMappings. The class ExecutableTaskMapping is presented below.

InterfaceInstanceToModuleMapping
--------------------------------

The **class InterfaceInstanceToModuleMapping** defines the concrete instantiation of the abstract instantiation of an application module with an platform module. It consists of the following members:

- **InstanceName**: A string value containing the instance name of the provided or consumed interface of the application module as value.
- **ModuleRef**: A PlatformModuleRefType referencing to the platform module. The class PlatformModuleRefType is presented below.

ExecutableTaskMapping
---------------------

The **class ExecutableTaskMapping** defines the mapping of an task of an application module to an executable. It consists of the following members:

- **TaskName**: A string value containing the task name as value.
- **Offset**: An optional integer value containing the execution offset as value.
- **Budget**: An optional string value containing the budget of the task value.

ApplicationModuleRefType
------------------------

The **class ApplicationModuleRefType** is an annotated ApplicationModule for resolving the referenced ApplicationModule, which is referenced in the JSON representation via a string value that contains the namespace path to the referenced ApplicationModule . The class ApplicationModule is presented below.

ApplicationModule
-----------------

The **ApplicationModule** class defines an application module that consists of tasks to be executed in an executable, as well as provided and consumed module interfaces that are used by the tasks for data exchange and remote operation calls. It consists of the following members:

- **Name**: A string value containing the name of the application module as value.
- **Namespace**: A string value containing the namespace of the application module as value.
- **ConsumedInterfaces**: Is a list of ApplicationModuleConsumedInterfaces. The class ApplicationModuleConsumedInterface is presented below.
- **ProvidedInterfaces**: Is a list of ApplicationModuleProvidedInterfaces. The class ApplicationModuleProvidedInterface is presented below.
- **ImplementationProperties**: An optional class of implementation properties for the application module. The class ImplementationProperty is presented below.
- **Tasks**: Is a list of ApplicationModuleTasks. The class ApplicationModuleTask is presented below.

ApplicationModuleProvidedInterface
----------------------------------

The **class ApplicationModuleProvidedInterface** defines an abstract instantiation of a provided module interface within an application module. It consists of the following members:

- **InstanceName**: A string value containing the instance name of the provided interface of the application module as value.
- **ModuleInterfaceRef**: A ModuleInterfaceRef referencing to the module interface of the provided interface of the application module. The class ModuleInterfaceRefType is presented below.

ApplicationModuleConsumedInterface
----------------------------------

The **class ApplicationModuleConsumedInterface** defines an abstract instantiation of a consumed module interface within an application module. It consists of the following members:

- **InstanceName**: A string value containing the instance name of the application module consumed interface as value.
- **ModuleInterfaceRef**: A ModuleInterfaceRef referencing to the module interface of the application module consumed interface. The class ModuleInterfaceRefType is presented below. 
- **IsOptional**: A boolean value with value "TRUE" if the application module consumed interface is optional otherwise "FALSE".

ImplementationProperty
----------------------

The **class ImplementationProperty** defines the implementation properties of an application module. It consists of the following members:

- **GenerateUnitTestStubs**: A optional boolean value with value "TRUE" if unit test stubs shall be generated for the application module otherwise "FALSE". If it is not set, no unit test stubs are generated.
- **InstallationPath**: An optional string value containing the installation path within an integration project of the application module as value.

ApplicationModuleTasks
----------------------

The **class ApplicationModuleTasks** defines a task within an application module. It consists of the following members:

- **Name**: A string value containing the name of the application module task as value.
- **Period**: A string value containing the period of the application module task as value.
- **PreferredOffset**: An optional integer value containing the preferred offset of the application module task as value.
- **RunAfter**: A list of string values, each containing the name of an application module to run after as value.

PlatformModule
--------------

The **class PlatformModule** defines a realization of a module interface via a specific platform. It consists of the following members:

- **Name**: A string value containing the name of the platform module as value.
- **Namespace**: A string value containing the namespace of the platform module as value.
- **ModuleInterfaceRef**: A ModuleInterfaceRef referencing to the module interface of the platform module. The class ModuleInterfaceRefType is presented below.
- **OriginalEcoSystem**: An optional OriginalEcoSystemEnum indicating the platform type. The class OriginalEcoSystemEnum is presented below. If not platform type is given it is defined to be internal communication.
- **ConnectionPointRef**: An optional ConnectionPointRefType referencing to the connection point of the platform module. The class ConnectionPointRefType is presented below.

OriginalEcoSystemEnum
---------------------

The **class OriginalEcoSystemEnum** is an enum type which indicates the platform type of the platform module. Supported is:

- "SILKIT"

ConnectionPointRefType
----------------------

The **class ConnectionPointRefType** is an annotated SILKITConnectionPoint for resolving the referenced *ConnectionPoint, which is referenced in the JSON representation via a string value that contains the namespace path to the referenced* ConnectionPoint. The class SILKITConnectionPoint is presented below.

ModuleInterfaceRefType
----------------------

The **class ModuleInterfaceRefType** is an annotated ModuleInterface for resolving the referenced ModuleInterface, which is referenced in the JSON representation via a string value that contains the namespace path to the referenced ModuleInterface . The class ModuleInterface is presented below.

ModuleInterface
---------------

The **class ModuleInterface** defines an interface consisting of data elements and operations. It consists of the following members:

- **Name**: A string value containing the name of the module interface as value.
- **Namespace**: A string value containing the namespace of the module interface as value.
- **OperationOutputNamespace**: An optional string value containing the operation output datatype namespace as value.
- **DataElements**: Is a list of DataElements. The class DataElement is presented below.
- **Operations** : Is a list of Operations. The class Operation is presented below.

DataElement
-----------

The **class DataElement** defines an element with message semantics in a module interface. It consists of the following members:

- **Name**: A string value containing the name of the data element as value.
- **TypeRef**: A DataTypeRef referencing to the base type of the vector. The class DataTypeRef is presented below.
- **InitialValue**: An optional string value containing the initial value definition in C++ style as value.
- **Min**: An optional floating point value representing the minimum value that the data element can have as a value.
- **Max**: An optional floating point value representing the maximum value that the data element can have as a value.

Operation
---------

The **class Operation** defines an element with remote procedure call semantics in a module interface. It consists of the following members:

- **Name**: A string value containing the name of the operation as value.
- **Parameters**: Is a list of Parameters. The class Parameter is presented below.

Parameter
---------

The **class Parameter** defines an in or out parameter of an operation. It consists of the following members:

- **Name**: A string value containing the name of the parameter as value.
- **TypeRef**: A DataTypeRef referencing to the base type of the parameter. The class DataTypeRef is presented below.
- **Direction**: A ParameterDirection defining the direction of the parameter. The class ParameterDirection is presented below. 

ParameterDirection
------------------

The **class ParameterDirection** is an enum type which indicates the direction of the parameter. It can take following values:

- "IN"
- "OUT"
- "INOUT"


DataTypeRef
-----------

The **class DataTypeRef** is an annotated Datatype for resolving the referenced Datatype, which is referenced in the JSON representation via a string value that contains the namespace path to the referenced Datatype . The class Datatype is presented below.

DataType
--------

The **class DataType** represents a datatype by its name and namespace of a datatype. It consists of the following members:

- **Name**: A string value containing the name of the represented datatype.
- **Namespace**: A string value containing the namespace of the represented datatype.

OperationRefType
----------------

The **class OperationRefType** is an annotated Operation for resolving the referenced Operation, which is referenced in the JSON representation via a string value that contains the path to the referenced Operation. The class Operation is presented above.

DataElementRefType
------------------

The **class DataElementRefType** is an annotated DataElement for resolving the referenced DataElement, which is referenced in the JSON representation via a string value that contains the path to the referenced DataElement. The class Operation is presented above.

SILKITAdditionalConfigurationType
---------------------------------

The **class SILKITAdditionalConfigurationType** contains all additional configuration information needed for the SILKIT platform. It consists of the following members:

- **ConnectionPoints**: Is a list of SILKITConnectionPoints. The class SILKITConnectionPoint is presented below.

SILKITConnectionPoint
---------------------

The **class SILKITConnectionPoint** contains the information of a SILKIT connection point. It consists of the following members:

- **Name**: A string value containing the name of the connection point as value.
- **ServiceInterfaceName**: A string value containing the instance name of the connection point as value.
