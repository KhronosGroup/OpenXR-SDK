if( WIN32 )
    add_definitions( -DGLSLANG_OSINCLUDE_WIN32 )
elseif( APPLE )
    add_definitions( -DGLSLANG_OSINCLUDE_UNIX )
elseif( UNIX )
    add_definitions( -fPIC )
    add_definitions( -DGLSLANG_OSINCLUDE_UNIX )
else(WIN32)
    message("unknown platform")
endif(WIN32)

if( CMAKE_COMPILER_IS_GNUCXX )
    add_definitions( -std=c++11 )
elseif( ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" )
    add_definitions( -std=c++11 )
endif()

set( GLSLANG_DIR "${CMAKE_SOURCE_DIR}/../glslang" )

# SPIRV
set( SOURCES
    ${GLSLANG_DIR}/SPIRV/GlslangToSpv.cpp
    ${GLSLANG_DIR}/SPIRV/InReadableOrder.cpp
    ${GLSLANG_DIR}/SPIRV/Logger.cpp
    ${GLSLANG_DIR}/SPIRV/SpvBuilder.cpp
    ${GLSLANG_DIR}/SPIRV/SPVRemapper.cpp
    ${GLSLANG_DIR}/SPIRV/doc.cpp
    ${GLSLANG_DIR}/SPIRV/disassemble.cpp )
set( HEADERS
    ${GLSLANG_DIR}/SPIRV/spirv.hpp
    ${GLSLANG_DIR}/SPIRV/GLSL.std.450.h
    ${GLSLANG_DIR}/SPIRV/GlslangToSpv.h
    ${GLSLANG_DIR}/SPIRV/Logger.h
    ${GLSLANG_DIR}/SPIRV/SpvBuilder.h
    ${GLSLANG_DIR}/SPIRV/SPVRemapper.h
    ${GLSLANG_DIR}/SPIRV/spvIR.h
    ${GLSLANG_DIR}/SPIRV/doc.h
    ${GLSLANG_DIR}/SPIRV/disassemble.h )
add_library( SPIRV STATIC ${SOURCES} ${HEADERS} )
set_property( TARGET SPIRV PROPERTY FOLDER external/libs/glslang )

# OGLCompiler
set( SOURCES
    ${GLSLANG_DIR}/OGLCompilersDLL/InitializeDll.cpp
    ${GLSLANG_DIR}/OGLCompilersDLL/InitializeDll.h )
add_library( OGLCompiler STATIC ${SOURCES} )
set_property( TARGET OGLCompiler PROPERTY FOLDER external/libs/glslang )

# HLSL
set( SOURCES
    ${GLSLANG_DIR}/hlsl/hlslAttributes.cpp
    ${GLSLANG_DIR}/hlsl/hlslParseHelper.cpp
    ${GLSLANG_DIR}/hlsl/hlslScanContext.cpp
    ${GLSLANG_DIR}/hlsl/hlslOpMap.cpp
    ${GLSLANG_DIR}/hlsl/hlslTokenStream.cpp
    ${GLSLANG_DIR}/hlsl/hlslGrammar.cpp
    ${GLSLANG_DIR}/hlsl/hlslParseables.cpp )
set( HEADERS
    ${GLSLANG_DIR}/hlsl/hlslAttributes.h
    ${GLSLANG_DIR}/hlsl/hlslParseHelper.h
    ${GLSLANG_DIR}/hlsl/hlslTokens.h
    ${GLSLANG_DIR}/hlsl/hlslScanContext.h
    ${GLSLANG_DIR}/hlsl/hlslOpMap.h
    ${GLSLANG_DIR}/hlsl/hlslTokenStream.h
    ${GLSLANG_DIR}/hlsl/hlslGrammar.h
    ${GLSLANG_DIR}/hlsl/hlslParseables.h )
add_library( HLSL STATIC ${SOURCES} ${HEADERS} )
set_property( TARGET HLSL PROPERTY FOLDER external/libs/glslang )

# glslang
set( SOURCES
    ${GLSLANG_DIR}/glslang/MachineIndependent/glslang.y
    ${GLSLANG_DIR}/glslang/MachineIndependent/glslang_tab.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/Constant.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/InfoSink.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/Initialize.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/IntermTraverse.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/Intermediate.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/ParseContextBase.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/ParseHelper.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/PoolAlloc.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/RemoveTree.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/Scan.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/ShaderLang.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/SymbolTable.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/Versions.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/intermOut.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/iomapper.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/limits.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/linkValidate.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/parseConst.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/reflection.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/preprocessor/Pp.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/preprocessor/PpAtom.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/preprocessor/PpContext.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/preprocessor/PpMemory.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/preprocessor/PpScanner.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/preprocessor/PpSymbols.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/preprocessor/PpTokens.cpp
    ${GLSLANG_DIR}/glslang/MachineIndependent/propagateNoContraction.cpp
    ${GLSLANG_DIR}/glslang/GenericCodeGen/CodeGen.cpp
    ${GLSLANG_DIR}/glslang/GenericCodeGen/Link.cpp )
set( HEADERS
    ${GLSLANG_DIR}/glslang/Public/ShaderLang.h
    ${GLSLANG_DIR}/glslang/Include/arrays.h
    ${GLSLANG_DIR}/glslang/Include/BaseTypes.h
    ${GLSLANG_DIR}/glslang/Include/Common.h
    ${GLSLANG_DIR}/glslang/Include/ConstantUnion.h
    ${GLSLANG_DIR}/glslang/Include/InfoSink.h
    ${GLSLANG_DIR}/glslang/Include/InitializeGlobals.h
    ${GLSLANG_DIR}/glslang/Include/intermediate.h
    ${GLSLANG_DIR}/glslang/Include/PoolAlloc.h
    ${GLSLANG_DIR}/glslang/Include/ResourceLimits.h
    ${GLSLANG_DIR}/glslang/Include/revision.h
    ${GLSLANG_DIR}/glslang/Include/ShHandle.h
    ${GLSLANG_DIR}/glslang/Include/Types.h
    ${GLSLANG_DIR}/glslang/MachineIndependent/glslang_tab.cpp.h
    ${GLSLANG_DIR}/glslang/MachineIndependent/gl_types.h
    ${GLSLANG_DIR}/glslang/MachineIndependent/Initialize.h
    ${GLSLANG_DIR}/glslang/MachineIndependent/iomapper.h
    ${GLSLANG_DIR}/glslang/MachineIndependent/localintermediate.h
    ${GLSLANG_DIR}/glslang/MachineIndependent/ParseHelper.h
    ${GLSLANG_DIR}/glslang/MachineIndependent/reflection.h
    ${GLSLANG_DIR}/glslang/MachineIndependent/RemoveTree.h
    ${GLSLANG_DIR}/glslang/MachineIndependent/Scan.h
    ${GLSLANG_DIR}/glslang/MachineIndependent/ScanContext.h
    ${GLSLANG_DIR}/glslang/MachineIndependent/SymbolTable.h
    ${GLSLANG_DIR}/glslang/MachineIndependent/Versions.h
    ${GLSLANG_DIR}/glslang/MachineIndependent/parseVersions.h
    ${GLSLANG_DIR}/glslang/MachineIndependent/propagateNoContraction.h
    ${GLSLANG_DIR}/glslang/MachineIndependent/preprocessor/PpContext.h
    ${GLSLANG_DIR}/glslang/MachineIndependent/preprocessor/PpTokens.h )
add_library( glslang STATIC ${SOURCES} ${HEADERS} )
set_property( TARGET glslang PROPERTY FOLDER external/libs/glslang )

# OSDependent
if ( WIN32 )
    set( SOURCES
        ${GLSLANG_DIR}/glslang/OSDependent/Windows/ossource.cpp
        ${GLSLANG_DIR}/glslang/OSDependent/osinclude.h )
    add_library( OSDependent STATIC ${SOURCES} )
    set_property( TARGET OSDependent PROPERTY FOLDER external/libs/glslang )
elseif( APPLE )
    set( SOURCES
        ${GLSLANG_DIR}/glslang/OSDependent/Unix/ossource.cpp
        ${GLSLANG_DIR}/glslang/OSDependent/osinclude.h )
    add_library( OSDependent STATIC ${SOURCES} )
    set_property( TARGET OSDependent PROPERTY FOLDER external/libs/glslang )
elseif( UNIX )
    set( SOURCES
        ${GLSLANG_DIR}/glslang/OSDependent/Unix/ossource.cpp
        ${GLSLANG_DIR}/glslang/OSDependent/osinclude.h )
    add_library( OSDependent STATIC ${SOURCES} )
    set_property( TARGET OSDependent PROPERTY FOLDER external/libs/glslang )
else()
    message("unknown platform")
endif()
