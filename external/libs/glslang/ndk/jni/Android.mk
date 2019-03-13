SAVED_LOCAL_PATH := $(LOCAL_PATH)
LOCAL_PATH := $(abspath $(call my-dir))

# base of the Vulkan-Samples repository
BASE_DIR := ../../../../..

# base dir of the glslang repository
GLSLANG_DIR := $(BASE_DIR)/../glslang
GLSLANG_OS_FLAGS := -DGLSLANG_OSINCLUDE_UNIX

# SPIRV
include $(CLEAR_VARS)
LOCAL_MODULE			:= SPIRV
LOCAL_CXXFLAGS			:= -std=c++11 -fno-exceptions -fno-rtti $(GLSLANG_OS_FLAGS)
LOCAL_SRC_FILES			:= \
							$(GLSLANG_DIR)/SPIRV/GlslangToSpv.cpp \
							$(GLSLANG_DIR)/SPIRV/InReadableOrder.cpp \
							$(GLSLANG_DIR)/SPIRV/Logger.cpp \
							$(GLSLANG_DIR)/SPIRV/SPVRemapper.cpp \
							$(GLSLANG_DIR)/SPIRV/SpvBuilder.cpp \
							$(GLSLANG_DIR)/SPIRV/disassemble.cpp \
							$(GLSLANG_DIR)/SPIRV/doc.cpp
LOCAL_C_INCLUDES		:= $(GLSLANG_DIR) $(GLSLANG_DIR)/glslang/SPIRV
LOCAL_EXPORT_C_INCLUDES	:= \
							$(GLSLANG_DIR) \
							$(GLSLANG_DIR)/SPIRV
include $(BUILD_STATIC_LIBRARY)

# OSDependent
include $(CLEAR_VARS)
LOCAL_MODULE			:= OSDependent
LOCAL_CXXFLAGS			:= -std=c++11 -fno-exceptions -fno-rtti $(GLSLANG_OS_FLAGS)
LOCAL_SRC_FILES			:= $(GLSLANG_DIR)/glslang/OSDependent/Unix/ossource.cpp
LOCAL_C_INCLUDES		:= \
							$(GLSLANG_DIR) \
							$(GLSLANG_DIR)/glslang/OSDependent/Unix/
LOCAL_EXPORT_C_INCLUDES	:= \
							$(GLSLANG_DIR) \
							$(GLSLANG_DIR)/glslang/OSDependent/Unix/
include $(BUILD_STATIC_LIBRARY)

# OGLCompiler
include $(CLEAR_VARS)
LOCAL_MODULE			:= OGLCompiler
LOCAL_CXXFLAGS			:= -std=c++11 -fno-exceptions -fno-rtti $(GLSLANG_OS_FLAGS)
LOCAL_SRC_FILES			:= $(GLSLANG_DIR)/OGLCompilersDLL/InitializeDll.cpp
LOCAL_C_INCLUDES		:= $(GLSLANG_DIR)/OGLCompilersDLL
LOCAL_EXPORT_C_INCLUDES	:= $(GLSLANG_DIR)
LOCAL_STATIC_LIBRARIES	:= OSDependent
include $(BUILD_STATIC_LIBRARY)

# HLSL
include $(CLEAR_VARS)
LOCAL_MODULE			:= HLSL
LOCAL_CXXFLAGS			:= -std=c++11 -fno-exceptions -fno-rtti
LOCAL_SRC_FILES			:= \
							${GLSLANG_DIR}/hlsl/hlslAttributes.cpp \
							$(GLSLANG_DIR)/hlsl/hlslGrammar.cpp \
							$(GLSLANG_DIR)/hlsl/hlslOpMap.cpp \
							$(GLSLANG_DIR)/hlsl/hlslParseables.cpp \
							$(GLSLANG_DIR)/hlsl/hlslParseHelper.cpp \
							$(GLSLANG_DIR)/hlsl/hlslScanContext.cpp \
							$(GLSLANG_DIR)/hlsl/hlslTokenStream.cpp
LOCAL_C_INCLUDES		:= \
							$(GLSLANG_DIR) \
							$(GLSLANG_DIR)/hlsl
include $(BUILD_STATIC_LIBRARY)

# glslang
include $(CLEAR_VARS)
LOCAL_MODULE			:= glslang
LOCAL_CXXFLAGS			:= -std=c++11 -fno-exceptions -fno-rtti $(GLSLANG_OS_FLAGS)
LOCAL_SRC_FILES			:= \
							$(GLSLANG_DIR)/glslang/GenericCodeGen/CodeGen.cpp \
							$(GLSLANG_DIR)/glslang/GenericCodeGen/Link.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/Constant.cpp \
							${GLSLANG_DIR}/glslang/MachineIndependent/ParseContextBase.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/glslang_tab.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/InfoSink.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/Initialize.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/Intermediate.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/intermOut.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/IntermTraverse.cpp \
							${GLSLANG_DIR}/glslang/MachineIndependent/iomapper.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/limits.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/linkValidate.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/parseConst.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/ParseHelper.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/PoolAlloc.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/propagateNoContraction.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/reflection.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/RemoveTree.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/Scan.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/ShaderLang.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/SymbolTable.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/Versions.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/preprocessor/PpAtom.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/preprocessor/PpContext.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/preprocessor/Pp.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/preprocessor/PpMemory.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/preprocessor/PpScanner.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/preprocessor/PpSymbols.cpp \
							$(GLSLANG_DIR)/glslang/MachineIndependent/preprocessor/PpTokens.cpp
LOCAL_C_INCLUDES		:= $(GLSLANG_DIR) \
							$(GLSLANG_DIR)/glslang/MachineIndependent
LOCAL_EXPORT_C_INCLUDES	:= $(GLSLANG_DIR)
LOCAL_STATIC_LIBRARIES	:= OSDependent OGLCompiler SPIRV HLSL
include $(BUILD_STATIC_LIBRARY)

LOCAL_PATH := $(SAVED_LOCAL_PATH)
