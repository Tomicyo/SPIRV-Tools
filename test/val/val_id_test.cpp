// Copyright (c) 2015-2016 The Khronos Group Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <sstream>
#include <string>

#include "gmock/gmock.h"
#include "test_fixture.h"
#include "unit_spirv.h"
#include "val_fixtures.h"

// NOTE: The tests in this file are ONLY testing ID usage, there for the input
// SPIR-V does not follow the logical layout rules from the spec in all cases in
// order to makes the tests smaller. Validation of the whole module is handled
// in stages, ID validation is only one of these stages. All validation stages
// are stand alone.

namespace {

using ::testing::ValuesIn;
using ::testing::HasSubstr;
using spvtest::ScopedContext;
using std::ostringstream;
using std::string;
using std::vector;

using ValidateIdWithMessage = spvtest::ValidateBase<bool>;

string kGLSL450MemoryModel = R"(
     OpCapability Shader
     OpCapability Addresses
     OpCapability Pipes
     OpCapability LiteralSampler
     OpCapability DeviceEnqueue
     OpCapability Vector16
     OpCapability Int8
     OpCapability Int16
     OpCapability Int64
     OpCapability Float64
     OpMemoryModel Logical GLSL450
)";

string kOpenCLMemoryModel32 = R"(
     OpCapability Addresses
     OpCapability Linkage
     OpCapability Kernel
%1 = OpExtInstImport "OpenCL.std"
     OpMemoryModel Physical32 OpenCL
)";

string kOpenCLMemoryModel64 = R"(
     OpCapability Addresses
     OpCapability Linkage
     OpCapability Kernel
     OpCapability Int64
%1 = OpExtInstImport "OpenCL.std"
     OpMemoryModel Physical64 OpenCL
)";

// TODO: OpUndef

TEST_F(ValidateIdWithMessage, OpName) {
  string spirv = kGLSL450MemoryModel + R"(
     OpName %2 "name"
%1 = OpTypeInt 32 0
%2 = OpTypePointer UniformConstant %1
%3 = OpVariable %2 UniformConstant)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpMemberNameGood) {
  string spirv = kGLSL450MemoryModel + R"(
     OpMemberName %2 0 "foo"
%1 = OpTypeInt 32 0
%2 = OpTypeStruct %1)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpMemberNameTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
     OpMemberName %1 0 "foo"
%1 = OpTypeInt 32 0)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpMemberNameMemberBad) {
  string spirv = kGLSL450MemoryModel + R"(
     OpMemberName %2 1 "foo"
%1 = OpTypeInt 32 0
%2 = OpTypeStruct %1)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpLineGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpString "/path/to/source.file"
     OpLine %1 0 0
%2 = OpTypeInt 32 0
%3 = OpTypePointer Input %2
%4 = OpVariable %3 Input)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpLineFileBad) {
  string spirv = kGLSL450MemoryModel + R"(
     OpLine %2 0 0
%2 = OpTypeInt 32 0
%3 = OpTypePointer Input %2
%4 = OpVariable %3 Input)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpDecorateGood) {
  string spirv = kGLSL450MemoryModel + R"(
     OpDecorate %2 GLSLShared
%1 = OpTypeInt 64 0
%2 = OpTypeStruct %1 %1)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpDecorateBad) {
  string spirv = kGLSL450MemoryModel + R"(
OpDecorate %1 GLSLShared)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpMemberDecorateGood) {
  string spirv = kGLSL450MemoryModel + R"(
     OpMemberDecorate %2 0 Uniform
%1 = OpTypeInt 32 0
%2 = OpTypeStruct %1 %1)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpMemberDecorateBad) {
  string spirv = kGLSL450MemoryModel + R"(
     OpMemberDecorate %1 0 Uniform
%1 = OpTypeInt 32 0)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpMemberDecorateMemberBad) {
  string spirv = kGLSL450MemoryModel + R"(
     OpMemberDecorate %2 3 Uniform
%1 = OpTypeInt 32 0
%2 = OpTypeStruct %1 %1)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpGroupDecorateGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpDecorationGroup
     OpDecorate %1 Uniform
     OpDecorate %1 GLSLShared
     OpGroupDecorate %1 %3 %4
%2 = OpTypeInt 32 0
%3 = OpConstant %2 42
%4 = OpConstant %2 23)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpGroupDecorateDecorationGroupBad) {
  string spirv = kGLSL450MemoryModel + R"(
     OpGroupDecorate %2 %3 %4
%2 = OpTypeInt 32 0
%3 = OpConstant %2 42
%4 = OpConstant %2 23)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpGroupDecorateTargetBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpDecorationGroup
     OpDecorate %1 Uniform
     OpDecorate %1 GLSLShared
     OpGroupDecorate %1 %3
%2 = OpTypeInt 32 0)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

// TODO: OpGroupMemberDecorate
// TODO: OpExtInst

TEST_F(ValidateIdWithMessage, OpEntryPointGood) {
  string spirv = kGLSL450MemoryModel + R"(
     OpEntryPoint GLCompute %3 ""
%1 = OpTypeVoid
%2 = OpTypeFunction %1
%3 = OpFunction %1 None %2
%4 = OpLabel
     OpReturn
     OpFunctionEnd
)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpEntryPointFunctionBad) {
  string spirv = kGLSL450MemoryModel + R"(
     OpEntryPoint GLCompute %1 ""
%1 = OpTypeVoid)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpEntryPointParameterCountBad) {
  string spirv = kGLSL450MemoryModel + R"(
     OpEntryPoint GLCompute %3 ""
%1 = OpTypeVoid
%2 = OpTypeFunction %1 %1
%3 = OpFunction %1 None %2
%4 = OpLabel
     OpReturn
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpEntryPointReturnTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
     OpEntryPoint GLCompute %3 ""
%1 = OpTypeInt 32 0
%2 = OpTypeFunction %1
%3 = OpFunction %1 None %2
%4 = OpLabel
     OpReturn
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpExecutionModeGood) {
  string spirv = kGLSL450MemoryModel + R"(
     OpEntryPoint GLCompute %3 ""
     OpExecutionMode %3 LocalSize 1 1 1
%1 = OpTypeVoid
%2 = OpTypeFunction %1
%3 = OpFunction %1 None %2
%4 = OpLabel
     OpReturn
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpExecutionModeEntryPointMissing) {
  string spirv = kGLSL450MemoryModel + R"(
     OpExecutionMode %3 LocalSize 1 1 1
%1 = OpTypeVoid
%2 = OpTypeFunction %1
%3 = OpFunction %1 None %2
%4 = OpLabel
     OpReturn
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpExecutionModeEntryPointBad) {
  string spirv = kGLSL450MemoryModel + R"(
     OpEntryPoint GLCompute %3 "" %a
     OpExecutionMode %a LocalSize 1 1 1
%void = OpTypeVoid
%ptr = OpTypePointer Input %void
%a = OpVariable %ptr Input
%2 = OpTypeFunction %void
%3 = OpFunction %void None %2
%4 = OpLabel
     OpReturn
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpTypeVectorFloat) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 4)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpTypeVectorInt) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 1
%2 = OpTypeVector %1 4)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpTypeVectorUInt) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 64 0
%2 = OpTypeVector %1 4)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpTypeVectorBool) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeBool
%2 = OpTypeVector %1 4)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpTypeVectorComponentTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeFloat 32
%2 = OpTypePointer UniformConstant %1
%3 = OpTypeVector %2 4)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpTypeMatrixGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 2
%3 = OpTypeMatrix %2 3)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpTypeMatrixColumnTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpTypeMatrix %1 3)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpTypeSamplerGood) {
  // In Rev31, OpTypeSampler takes no arguments.
  string spirv = kGLSL450MemoryModel + R"(
%s = OpTypeSampler)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpTypeArrayGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpConstant %1 1
%3 = OpTypeArray %1 %2)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpTypeArrayElementTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpConstant %1 1
%3 = OpTypeArray %2 %2)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

// Signed or unsigned.
enum Signed { kSigned, kUnsigned };

// Creates an assembly snippet declaring OpTypeArray with the given length.
string MakeArrayLength(const string& len, Signed isSigned, int width) {
  ostringstream ss;
  ss << kGLSL450MemoryModel;
  ss << " %t = OpTypeInt " << width << (isSigned == kSigned ? " 1" : " 0")
     << " %l = OpConstant %t " << len << " %a = OpTypeArray %t %l";
  return ss.str();
}

// Tests OpTypeArray.  Parameter is the width (in bits) of the array-length's
// type.
class OpTypeArrayLengthTest
    : public spvtest::TextToBinaryTestBase<::testing::TestWithParam<int>> {
 protected:
  OpTypeArrayLengthTest()
      : position_(spv_position_t{0, 0, 0}),
        diagnostic_(spvDiagnosticCreate(&position_, "")) {}

  ~OpTypeArrayLengthTest() { spvDiagnosticDestroy(diagnostic_); }

  // Runs spvValidate() on v, printing any errors via spvDiagnosticPrint().
  spv_result_t Val(const SpirvVector& v) {
    spv_const_binary_t cbinary{v.data(), v.size()};
    const auto status =
        spvValidate(ScopedContext().context, &cbinary, &diagnostic_);
    if (status != SPV_SUCCESS) {
      spvDiagnosticPrint(diagnostic_);
    }
    return status;
  }

 private:
  spv_position_t position_;  // For creating diagnostic_.
  spv_diagnostic diagnostic_;
};

TEST_P(OpTypeArrayLengthTest, LengthPositive) {
  const int width = GetParam();
  EXPECT_EQ(SPV_SUCCESS,
            Val(CompileSuccessfully(MakeArrayLength("1", kSigned, width))));
  EXPECT_EQ(SPV_SUCCESS,
            Val(CompileSuccessfully(MakeArrayLength("1", kUnsigned, width))));
  EXPECT_EQ(SPV_SUCCESS,
            Val(CompileSuccessfully(MakeArrayLength("2", kSigned, width))));
  EXPECT_EQ(SPV_SUCCESS,
            Val(CompileSuccessfully(MakeArrayLength("2", kUnsigned, width))));
  EXPECT_EQ(SPV_SUCCESS,
            Val(CompileSuccessfully(MakeArrayLength("55", kSigned, width))));
  EXPECT_EQ(SPV_SUCCESS,
            Val(CompileSuccessfully(MakeArrayLength("55", kUnsigned, width))));
  const string fpad(width / 4 - 1, 'F');
  EXPECT_EQ(
      SPV_SUCCESS,
      Val(CompileSuccessfully(MakeArrayLength("0x7" + fpad, kSigned, width))));
  EXPECT_EQ(SPV_SUCCESS, Val(CompileSuccessfully(
                             MakeArrayLength("0xF" + fpad, kUnsigned, width))));
}

TEST_P(OpTypeArrayLengthTest, LengthZero) {
  const int width = GetParam();
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            Val(CompileSuccessfully(MakeArrayLength("0", kSigned, width))));
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            Val(CompileSuccessfully(MakeArrayLength("0", kUnsigned, width))));
}

TEST_P(OpTypeArrayLengthTest, LengthNegative) {
  const int width = GetParam();
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            Val(CompileSuccessfully(MakeArrayLength("-1", kSigned, width))));
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            Val(CompileSuccessfully(MakeArrayLength("-2", kSigned, width))));
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            Val(CompileSuccessfully(MakeArrayLength("-123", kSigned, width))));
  const string neg_max = "0x8" + string(width / 4 - 1, '0');
  EXPECT_EQ(SPV_ERROR_INVALID_ID,
            Val(CompileSuccessfully(MakeArrayLength(neg_max, kSigned, width))));
}

// The only valid widths for integers are 8, 16, 32, and 64.
INSTANTIATE_TEST_CASE_P(Widths, OpTypeArrayLengthTest,
                        ValuesIn(vector<int>{8, 16, 32, 64}));

TEST_F(ValidateIdWithMessage, OpTypeArrayLengthNull) {
  string spirv = kGLSL450MemoryModel + R"(
%i32 = OpTypeInt 32 1
%len = OpConstantNull %i32
%ary = OpTypeArray %i32 %len)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpTypeArrayLengthSpecConst) {
  string spirv = kGLSL450MemoryModel + R"(
%i32 = OpTypeInt 32 1
%len = OpSpecConstant %i32 2
%ary = OpTypeArray %i32 %len)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpTypeArrayLengthSpecConstOp) {
  string spirv = kGLSL450MemoryModel + R"(
%i32 = OpTypeInt 32 1
%c1 = OpConstant %i32 1
%c2 = OpConstant %i32 2
%len = OpSpecConstantOp %i32 IAdd %c1 %c2
%ary = OpTypeArray %i32 %len)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpTypeRuntimeArrayGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpTypeRuntimeArray %1)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpTypeRuntimeArrayBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpConstant %1 0
%3 = OpTypeRuntimeArray %2)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
// TODO: Object of this type can only be created with OpVariable using the
// Unifrom Storage Class

TEST_F(ValidateIdWithMessage, OpTypeStructGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpTypeFloat 64
%3 = OpTypePointer Input %1
%4 = OpTypeStruct %1 %2 %3)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpTypeStructMemberTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpTypeFloat 64
%3 = OpConstant %2 0.0
%4 = OpTypeStruct %1 %2 %3)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpTypePointerGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpTypePointer Input %1)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpTypePointerBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpConstant %1 0
%3 = OpTypePointer Input %2)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpTypeFunctionGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeFunction %1)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpTypeFunctionReturnTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpConstant %1 0
%3 = OpTypeFunction %2)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpTypeFunctionParameterBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpConstant %2 0
%4 = OpTypeFunction %1 %2 %3)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpTypePipeGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 16
%3 = OpTypePipe ReadOnly)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpConstantTrueGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeBool
%2 = OpConstantTrue %1)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpConstantTrueBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpConstantTrue %1)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpConstantFalseGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeBool
%2 = OpConstantTrue %1)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpConstantFalseBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpConstantFalse %1)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpConstantGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpConstant %1 1)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpConstantBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpConstant !1 !0)";
  // The expected failure code is implementation dependent (currently
  // INVALID_BINARY because the binary parser catches these cases) and may
  // change over time, but this must always fail.
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_BINARY, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpConstantCompositeVectorGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 4
%3 = OpConstant %1 3.14
%4 = OpConstantComposite %2 %3 %3 %3 %3)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpConstantCompositeVectorWithUndefGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 4
%3 = OpConstant %1 3.14
%9 = OpUndef %1
%4 = OpConstantComposite %2 %3 %3 %3 %9)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpConstantCompositeVectorResultTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 4
%3 = OpConstant %1 3.14
%4 = OpConstantComposite %1 %3 %3 %3 %3)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpConstantCompositeVectorConstituentTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 4
%4 = OpTypeInt 32 0
%3 = OpConstant %1 3.14
%5 = OpConstant %4 42 ; bad type for constant value
%6 = OpConstantComposite %2 %3 %5 %3 %3)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage,
       OpConstantCompositeVectorConstituentUndefTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 4
%4 = OpTypeInt 32 0
%3 = OpConstant %1 3.14
%5 = OpUndef %4 ; bad type for undef value
%6 = OpConstantComposite %2 %3 %5 %3 %3)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpConstantCompositeMatrixGood) {
  string spirv = kGLSL450MemoryModel + R"(
 %1 = OpTypeFloat 32
 %2 = OpTypeVector %1 4
 %3 = OpTypeMatrix %2 4
 %4 = OpConstant %1 1.0
 %5 = OpConstant %1 0.0
 %6 = OpConstantComposite %2 %4 %5 %5 %5
 %7 = OpConstantComposite %2 %5 %4 %5 %5
 %8 = OpConstantComposite %2 %5 %5 %4 %5
 %9 = OpConstantComposite %2 %5 %5 %5 %4
%10 = OpConstantComposite %3 %6 %7 %8 %9)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpConstantCompositeMatrixUndefGood) {
  string spirv = kGLSL450MemoryModel + R"(
 %1 = OpTypeFloat 32
 %2 = OpTypeVector %1 4
 %3 = OpTypeMatrix %2 4
 %4 = OpConstant %1 1.0
 %5 = OpConstant %1 0.0
 %6 = OpConstantComposite %2 %4 %5 %5 %5
 %7 = OpConstantComposite %2 %5 %4 %5 %5
 %8 = OpConstantComposite %2 %5 %5 %4 %5
 %9 = OpUndef %2
%10 = OpConstantComposite %3 %6 %7 %8 %9)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpConstantCompositeMatrixConstituentTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
 %1 = OpTypeFloat 32
 %2 = OpTypeVector %1 4
%11 = OpTypeVector %1 3
 %3 = OpTypeMatrix %2 4
 %4 = OpConstant %1 1.0
 %5 = OpConstant %1 0.0
 %6 = OpConstantComposite %2 %4 %5 %5 %5
 %7 = OpConstantComposite %2 %5 %4 %5 %5
 %8 = OpConstantComposite %2 %5 %5 %4 %5
 %9 = OpConstantComposite %11 %5 %5 %5
%10 = OpConstantComposite %3 %6 %7 %8 %9)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage,
       OpConstantCompositeMatrixConstituentUndefTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
 %1 = OpTypeFloat 32
 %2 = OpTypeVector %1 4
%11 = OpTypeVector %1 3
 %3 = OpTypeMatrix %2 4
 %4 = OpConstant %1 1.0
 %5 = OpConstant %1 0.0
 %6 = OpConstantComposite %2 %4 %5 %5 %5
 %7 = OpConstantComposite %2 %5 %4 %5 %5
 %8 = OpConstantComposite %2 %5 %5 %4 %5
 %9 = OpUndef %11
%10 = OpConstantComposite %3 %6 %7 %8 %9)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpConstantCompositeMatrixColumnTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
 %1 = OpTypeInt 32 0
 %2 = OpTypeFloat 32
 %3 = OpTypeVector %1 2
 %4 = OpTypeVector %3 2
 %5 = OpTypeMatrix %2 2
 %6 = OpConstant %1 42
 %7 = OpConstant %2 3.14
 %8 = OpConstantComposite %3 %6 %6
 %9 = OpConstantComposite %4 %7 %7
%10 = OpConstantComposite %5 %8 %9)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpConstantCompositeArrayGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpConstant %1 4
%3 = OpTypeArray %1 %2
%4 = OpConstantComposite %3 %2 %2 %2 %2)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpConstantCompositeArrayWithUndefGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpConstant %1 4
%9 = OpUndef %1
%3 = OpTypeArray %1 %2
%4 = OpConstantComposite %3 %2 %2 %2 %9)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpConstantCompositeArrayConstConstituentBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpConstant %1 4
%3 = OpTypeArray %1 %2
%4 = OpConstantComposite %3 %2 %2 %2 %1)";  // Uses a type as operand
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpConstantCompositeArrayConstituentTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpConstant %1 4
%3 = OpTypeArray %1 %2
%5 = OpTypeFloat 32
%6 = OpConstant %5 3.14 ; bad type for const value
%4 = OpConstantComposite %3 %2 %2 %2 %6)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpConstantCompositeArrayConstituentUndefTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpConstant %1 4
%3 = OpTypeArray %1 %2
%5 = OpTypeFloat 32
%6 = OpUndef %5 ; bad type for undef
%4 = OpConstantComposite %3 %2 %2 %2 %6)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpConstantCompositeStructGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpTypeInt 64 1
%3 = OpTypeStruct %1 %1 %2
%4 = OpConstant %1 42
%5 = OpConstant %2 4300000000
%6 = OpConstantComposite %3 %4 %4 %5)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpConstantCompositeStructUndefGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpTypeInt 64 1
%3 = OpTypeStruct %1 %1 %2
%4 = OpConstant %1 42
%5 = OpUndef %2
%6 = OpConstantComposite %3 %4 %4 %5)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpConstantCompositeStructMemberTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpTypeInt 64 1
%3 = OpTypeStruct %1 %1 %2
%4 = OpConstant %1 42
%5 = OpConstant %2 4300000000
%6 = OpConstantComposite %3 %4 %5 %4)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpConstantCompositeStructMemberUndefTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpTypeInt 64 1
%3 = OpTypeStruct %1 %1 %2
%4 = OpConstant %1 42
%5 = OpUndef %2
%6 = OpConstantComposite %3 %4 %5 %4)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpConstantSamplerGood) {
  string spirv = kGLSL450MemoryModel + R"(
%float = OpTypeFloat 32
%samplerType = OpTypeSampler
%3 = OpConstantSampler %samplerType ClampToEdge 0 Nearest)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpConstantSamplerResultTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeFloat 32
%2 = OpConstantSampler %1 Clamp 0 Nearest)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpConstantNullGood) {
  string spirv = kGLSL450MemoryModel + R"(
 %1 = OpTypeBool
 %2 = OpConstantNull %1
 %3 = OpTypeInt 32 0
 %4 = OpConstantNull %3
 %5 = OpTypeFloat 32
 %6 = OpConstantNull %5
 %7 = OpTypePointer UniformConstant %3
 %8 = OpConstantNull %7
 %9 = OpTypeEvent
%10 = OpConstantNull %9
%11 = OpTypeDeviceEvent
%12 = OpConstantNull %11
%13 = OpTypeReserveId
%14 = OpConstantNull %13
%15 = OpTypeQueue
%16 = OpConstantNull %15
%17 = OpTypeVector %5 2
%18 = OpConstantNull %17
%19 = OpTypeMatrix %17 2
%20 = OpConstantNull %19
%25 = OpConstant %3 8
%21 = OpTypeArray %3 %25
%22 = OpConstantNull %21
%23 = OpTypeStruct %3 %5 %1
%24 = OpConstantNull %23
%26 = OpTypeArray %17 %25
%27 = OpConstantNull %26
%28 = OpTypeStruct %7 %26 %26 %1
%29 = OpConstantNull %28
)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpConstantNullBasicBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpConstantNull %1)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpConstantNullArrayBad) {
  string spirv = kGLSL450MemoryModel + R"(
%2 = OpTypeInt 32 0
%3 = OpTypeSampler
%4 = OpConstant %2 4
%5 = OpTypeArray %3 %4
%6 = OpConstantNull %5)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpConstantNullStructBad) {
  string spirv = kGLSL450MemoryModel + R"(
%2 = OpTypeSampler
%3 = OpTypeStruct %2 %2
%4 = OpConstantNull %3)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpConstantNullRuntimeArrayBad) {
  string spirv = kGLSL450MemoryModel + R"(
%bool = OpTypeBool
%array = OpTypeRuntimeArray %bool
%null = OpConstantNull %array)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpSpecConstantTrueGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeBool
%2 = OpSpecConstantTrue %1)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpSpecConstantTrueBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpSpecConstantTrue %1)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpSpecConstantFalseGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeBool
%2 = OpSpecConstantFalse %1)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpSpecConstantFalseBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpSpecConstantFalse %1)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpSpecConstantGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeFloat 32
%2 = OpSpecConstant %1 42)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpSpecConstantBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpSpecConstant !1 !4)";
  // The expected failure code is implementation dependent (currently
  // INVALID_BINARY because the binary parser catches these cases) and may
  // change over time, but this must always fail.
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_BINARY, ValidateInstructions());
}

// Valid: SpecConstantComposite specializes to a vector.
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeVectorGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 4
%3 = OpSpecConstant %1 3.14
%4 = OpConstant %1 3.14
%5 = OpSpecConstantComposite %2 %3 %3 %4 %4)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

// Valid: Vector of floats and Undefs.
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeVectorWithUndefGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 4
%3 = OpSpecConstant %1 3.14
%5 = OpConstant %1 3.14
%9 = OpUndef %1
%4 = OpSpecConstantComposite %2 %3 %5 %3 %9)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

// Invalid: result type is float.
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeVectorResultTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 4
%3 = OpSpecConstant %1 3.14
%4 = OpSpecConstantComposite %1 %3 %3 %3 %3)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("is not a composite type"));
}

// Invalid: Vector contains a mix of Int and Float.
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeVectorConstituentTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 4
%4 = OpTypeInt 32 0
%3 = OpSpecConstant %1 3.14
%5 = OpConstant %4 42 ; bad type for constant value
%6 = OpSpecConstantComposite %2 %3 %5 %3 %3)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpSpecConstantComposite Constituent <id> '5's type "
                        "does not match Result Type <id> '2's vector element "
                        "type."));
}

// Invalid: Constituent is not a constant
TEST_F(ValidateIdWithMessage,
       OpSpecConstantCompositeVectorConstituentNotConstantBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 4
%3 = OpTypeInt 32 0
%4 = OpSpecConstant %1 3.14
%6 = OpSpecConstantComposite %2 %3 %4 %4 %4)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpSpecConstantComposite Constituent <id> '3' is not a "
                        "constant or undef."));
}

// Invalid: Vector contains a mix of Undef-int and Float.
TEST_F(ValidateIdWithMessage,
       OpSpecConstantCompositeVectorConstituentUndefTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 4
%4 = OpTypeInt 32 0
%3 = OpSpecConstant %1 3.14
%5 = OpUndef %4 ; bad type for undef value
%6 = OpSpecConstantComposite %2 %3 %5 %3 %3)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpSpecConstantComposite Constituent <id> '5's type "
                        "does not match Result Type <id> '2's vector element "
                        "type."));
}

// Invalid: Vector expects 3 components, but 4 specified.
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeVectorNumComponentsBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 3
%3 = OpConstant %1 3.14
%5 = OpSpecConstant %1 4.0
%6 = OpSpecConstantComposite %2 %3 %5 %3 %3)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpSpecConstantComposite Constituent <id> count does "
                        "not match Result Type <id> '2's vector component "
                        "count."));
}

// Valid: 4x4 matrix of floats
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeMatrixGood) {
  string spirv = kGLSL450MemoryModel + R"(
 %1 = OpTypeFloat 32
 %2 = OpTypeVector %1 4
 %3 = OpTypeMatrix %2 4
 %4 = OpConstant %1 1.0
 %5 = OpSpecConstant %1 0.0
 %6 = OpSpecConstantComposite %2 %4 %5 %5 %5
 %7 = OpSpecConstantComposite %2 %5 %4 %5 %5
 %8 = OpSpecConstantComposite %2 %5 %5 %4 %5
 %9 = OpSpecConstantComposite %2 %5 %5 %5 %4
%10 = OpSpecConstantComposite %3 %6 %7 %8 %9)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

// Valid: Matrix in which one column is Undef
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeMatrixUndefGood) {
  string spirv = kGLSL450MemoryModel + R"(
 %1 = OpTypeFloat 32
 %2 = OpTypeVector %1 4
 %3 = OpTypeMatrix %2 4
 %4 = OpConstant %1 1.0
 %5 = OpSpecConstant %1 0.0
 %6 = OpSpecConstantComposite %2 %4 %5 %5 %5
 %7 = OpSpecConstantComposite %2 %5 %4 %5 %5
 %8 = OpSpecConstantComposite %2 %5 %5 %4 %5
 %9 = OpUndef %2
%10 = OpSpecConstantComposite %3 %6 %7 %8 %9)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

// Invalid: Matrix in which the sizes of column vectors are not equal.
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeMatrixConstituentTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
 %1 = OpTypeFloat 32
 %2 = OpTypeVector %1 4
 %3 = OpTypeVector %1 3
 %4 = OpTypeMatrix %2 4
 %5 = OpSpecConstant %1 1.0
 %6 = OpConstant %1 0.0
 %7 = OpSpecConstantComposite %2 %5 %6 %6 %6
 %8 = OpSpecConstantComposite %2 %6 %5 %6 %6
 %9 = OpSpecConstantComposite %2 %6 %6 %5 %6
 %10 = OpSpecConstantComposite %3 %6 %6 %6
%11 = OpSpecConstantComposite %4 %7 %8 %9 %10)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpSpecConstantComposite Constituent <id> '10' vector "
                        "component count does not match Result Type <id> '4's "
                        "vector component count."));
}

// Invalid: Matrix type expects 4 columns but only 3 specified.
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeMatrixNumColsBad) {
  string spirv = kGLSL450MemoryModel + R"(
 %1 = OpTypeFloat 32
 %2 = OpTypeVector %1 4
 %3 = OpTypeMatrix %2 4
 %4 = OpSpecConstant %1 1.0
 %5 = OpConstant %1 0.0
 %6 = OpSpecConstantComposite %2 %4 %5 %5 %5
 %7 = OpSpecConstantComposite %2 %5 %4 %5 %5
 %8 = OpSpecConstantComposite %2 %5 %5 %4 %5
%10 = OpSpecConstantComposite %3 %6 %7 %8)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpSpecConstantComposite Constituent <id> count does "
                "not match Result Type <id> '3's matrix column count."));
}

// Invalid: Composite contains a non-const/undef component
TEST_F(ValidateIdWithMessage,
       OpSpecConstantCompositeMatrixConstituentNotConstBad) {
  string spirv = kGLSL450MemoryModel + R"(
 %1 = OpTypeFloat 32
 %2 = OpConstant %1 0.0
 %3 = OpTypeVector %1 4
 %4 = OpTypeMatrix %3 4
 %5 = OpSpecConstantComposite %3 %2 %2 %2 %2
 %6 = OpSpecConstantComposite %4 %5 %5 %5 %1)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpSpecConstantComposite Constituent <id> '1' is not a "
                        "constant composite or undef."));
}

// Invalid: Composite contains a column that is *not* a vector (it's an array)
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeMatrixColTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
 %1 = OpTypeFloat 32
 %2 = OpTypeInt 32 0
 %3 = OpSpecConstant %2 4
 %4 = OpConstant %1 0.0
 %5 = OpTypeVector %1 4
 %6 = OpTypeArray %2 %3
 %7 = OpTypeMatrix %5 4
 %8  = OpSpecConstantComposite %6 %3 %3 %3 %3
 %9  = OpSpecConstantComposite %5 %4 %4 %4 %4
 %10 = OpSpecConstantComposite %7 %9 %9 %9 %8)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpSpecConstantComposite Constituent <id> '8' type "
                        "does not match Result Type <id> '7's matrix column "
                        "type."));
}

// Invalid: Matrix with an Undef column of the wrong size.
TEST_F(ValidateIdWithMessage,
       OpSpecConstantCompositeMatrixConstituentUndefTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
 %1 = OpTypeFloat 32
 %2 = OpTypeVector %1 4
 %3 = OpTypeVector %1 3
 %4 = OpTypeMatrix %2 4
 %5 = OpSpecConstant %1 1.0
 %6 = OpSpecConstant %1 0.0
 %7 = OpSpecConstantComposite %2 %5 %6 %6 %6
 %8 = OpSpecConstantComposite %2 %6 %5 %6 %6
 %9 = OpSpecConstantComposite %2 %6 %6 %5 %6
 %10 = OpUndef %3
 %11 = OpSpecConstantComposite %4 %7 %8 %9 %10)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpSpecConstantComposite Constituent <id> '10' vector "
                        "component count does not match Result Type <id> '4's "
                        "vector component count."));
}

// Invalid: Matrix in which some columns are Int and some are Float.
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeMatrixColumnTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
 %1 = OpTypeInt 32 0
 %2 = OpTypeFloat 32
 %3 = OpTypeVector %1 2
 %4 = OpTypeVector %2 2
 %5 = OpTypeMatrix %4 2
 %6 = OpSpecConstant %1 42
 %7 = OpConstant %2 3.14
 %8 = OpSpecConstantComposite %3 %6 %6
 %9 = OpSpecConstantComposite %4 %7 %7
%10 = OpSpecConstantComposite %5 %8 %9)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpSpecConstantComposite Constituent <id> '8' "
                        "component type does not match Result Type <id> '5's "
                        "matrix column component type."));
}

// Valid: Array of integers
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeArrayGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpSpecConstant %1 4
%5 = OpConstant %1 5
%3 = OpTypeArray %1 %2
%6 = OpTypeArray %1 %5
%4 = OpSpecConstantComposite %3 %2 %2 %2 %2
%7 = OpSpecConstantComposite %3 %5 %5 %5 %5)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

// Invalid: Expecting an array of 4 components, but 3 specified.
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeArrayNumComponentsBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpSpecConstant %1 4
%3 = OpTypeArray %1 %2
%4 = OpSpecConstantComposite %3 %2 %2 %2)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpSpecConstantComposite Constituent count does not "
                        "match Result Type <id> '3's array length."));
}

// Valid: Array of Integers and Undef-int
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeArrayWithUndefGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpSpecConstant %1 4
%9 = OpUndef %1
%3 = OpTypeArray %1 %2
%4 = OpSpecConstantComposite %3 %2 %2 %2 %9)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

// Invalid: Array uses a type as operand.
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeArrayConstConstituentBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpConstant %1 4
%3 = OpTypeArray %1 %2
%4 = OpSpecConstantComposite %3 %2 %2 %2 %1)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpSpecConstantComposite Constituent <id> '1' is not a "
                        "constant or undef."));
}

// Invalid: Array has a mix of Int and Float components.
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeArrayConstituentTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpConstant %1 4
%3 = OpTypeArray %1 %2
%4 = OpTypeFloat 32
%5 = OpSpecConstant %4 3.14 ; bad type for const value
%6 = OpSpecConstantComposite %3 %2 %2 %2 %5)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpSpecConstantComposite Constituent <id> '5's type "
                        "does not match Result Type <id> '3's array element "
                        "type."));
}

// Invalid: Array has a mix of Int and Undef-float.
TEST_F(ValidateIdWithMessage,
       OpSpecConstantCompositeArrayConstituentUndefTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpSpecConstant %1 4
%3 = OpTypeArray %1 %2
%5 = OpTypeFloat 32
%6 = OpUndef %5 ; bad type for undef
%4 = OpSpecConstantComposite %3 %2 %2 %2 %6)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpSpecConstantComposite Constituent <id> '5's type "
                        "does not match Result Type <id> '3's array element "
                        "type."));
}

// Valid: Struct of {Int32,Int32,Int64}.
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeStructGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpTypeInt 64 1
%3 = OpTypeStruct %1 %1 %2
%4 = OpConstant %1 42
%5 = OpSpecConstant %2 4300000000
%6 = OpSpecConstantComposite %3 %4 %4 %5)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

// Invalid: missing one int32 struct member.
TEST_F(ValidateIdWithMessage,
       OpSpecConstantCompositeStructMissingComponentBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%3 = OpTypeStruct %1 %1 %1
%4 = OpConstant %1 42
%5 = OpSpecConstant %1 430
%6 = OpSpecConstantComposite %3 %4 %5)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpSpecConstantComposite Constituent <id> '2' count "
                        "does not match Result Type <id> '2's struct member "
                        "count."));
}

// Valid: Struct uses Undef-int64.
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeStructUndefGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpTypeInt 64 1
%3 = OpTypeStruct %1 %1 %2
%4 = OpSpecConstant %1 42
%5 = OpUndef %2
%6 = OpSpecConstantComposite %3 %4 %4 %5)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

// Invalid: Composite contains non-const/undef component.
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeStructNonConstBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpTypeInt 64 1
%3 = OpTypeStruct %1 %1 %2
%4 = OpSpecConstant %1 42
%5 = OpUndef %2
%6 = OpSpecConstantComposite %3 %4 %1 %5)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpSpecConstantComposite Constituent <id> '1' is not a "
                        "constant or undef."));
}

// Invalid: Struct component type does not match expected specialization type.
// Second component was expected to be Int32, but got Int64.
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeStructMemberTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpTypeInt 64 1
%3 = OpTypeStruct %1 %1 %2
%4 = OpConstant %1 42
%5 = OpSpecConstant %2 4300000000
%6 = OpSpecConstantComposite %3 %4 %5 %4)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpSpecConstantComposite Constituent <id> '5' type "
                        "does not match the Result Type <id> '3's member "
                        "type."));
}

// Invalid: Undef-int64 used when Int32 was expected.
TEST_F(ValidateIdWithMessage, OpSpecConstantCompositeStructMemberUndefTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 0
%2 = OpTypeInt 64 1
%3 = OpTypeStruct %1 %1 %2
%4 = OpSpecConstant %1 42
%5 = OpUndef %2
%6 = OpSpecConstantComposite %3 %4 %5 %4)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpSpecConstantComposite Constituent <id> '5' type "
                        "does not match the Result Type <id> '3's member "
                        "type."));
}

// TODO: OpSpecConstantOp

TEST_F(ValidateIdWithMessage, OpVariableGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 1
%2 = OpTypePointer Input %1
%3 = OpVariable %2 Input)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpVariableInitializerGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 1
%2 = OpTypePointer Input %1
%3 = OpConstant %1 42
%4 = OpVariable %2 Input %3)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
// TODO: Positive test OpVariable with OpConstantNull of OpTypePointer
TEST_F(ValidateIdWithMessage, OpVariableResultTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 1
%2 = OpVariable %1 Input)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpVariableInitializerBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeInt 32 1
%2 = OpTypePointer Input %1
%3 = OpVariable %2 Input %2)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpLoadGood) {
  string spirv = kGLSL450MemoryModel + R"(
 %1 = OpTypeVoid
 %2 = OpTypeInt 32 1
 %3 = OpTypePointer UniformConstant %2
 %4 = OpTypeFunction %1
 %5 = OpVariable %3 UniformConstant
 %6 = OpFunction %1 None %4
 %7 = OpLabel
 %8 = OpLoad %2 %5
 %9 = OpReturn
%10 = OpFunctionEnd
)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpLoadResultTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 1
%3 = OpTypePointer UniformConstant %2
%4 = OpTypeFunction %1
%5 = OpVariable %3 UniformConstant
%6 = OpFunction %1 None %4
%7 = OpLabel
%8 = OpLoad %3 %5
     OpReturn
     OpFunctionEnd
)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpLoadPointerBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 1
%9 = OpTypeFloat 32
%3 = OpTypePointer UniformConstant %2
%4 = OpTypeFunction %1
%6 = OpFunction %1 None %4
%7 = OpLabel
%8 = OpLoad %9 %3
     OpReturn
     OpFunctionEnd
)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpStoreGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 1
%3 = OpTypePointer UniformConstant %2
%4 = OpTypeFunction %1
%5 = OpConstant %2 42
%6 = OpVariable %3 UniformConstant
%7 = OpFunction %1 None %4
%8 = OpLabel
     OpStore %6 %5
     OpReturn
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpStorePointerBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 1
%3 = OpTypePointer UniformConstant %2
%4 = OpTypeFunction %1
%5 = OpConstant %2 42
%6 = OpVariable %3 UniformConstant
%7 = OpFunction %1 None %4
%8 = OpLabel
     OpStore %3 %5
     OpReturn
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpStoreObjectGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 1
%3 = OpTypePointer UniformConstant %2
%4 = OpTypeFunction %1
%5 = OpConstant %2 42
%6 = OpVariable %3 UniformConstant
%7 = OpFunction %1 None %4
%8 = OpLabel
     OpStore %6 %7
     OpReturn
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpStoreTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 1
%9 = OpTypeFloat 32
%3 = OpTypePointer UniformConstant %2
%4 = OpTypeFunction %1
%5 = OpConstant %9 3.14
%6 = OpVariable %3 UniformConstant
%7 = OpFunction %1 None %4
%8 = OpLabel
     OpStore %6 %5
     OpReturn
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpStoreVoid) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 1
%3 = OpTypePointer UniformConstant %2
%4 = OpTypeFunction %1
%6 = OpVariable %3 UniformConstant
%7 = OpFunction %1 None %4
%8 = OpLabel
%9 = OpFunctionCall %1 %7
     OpStore %6 %9
     OpReturn
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpStoreLabel) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 1
%3 = OpTypePointer UniformConstant %2
%4 = OpTypeFunction %1
%6 = OpVariable %3 UniformConstant
%7 = OpFunction %1 None %4
%8 = OpLabel
     OpStore %6 %8
     OpReturn
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

// TODO: enable when this bug is fixed:
// https://cvs.khronos.org/bugzilla/show_bug.cgi?id=15404
TEST_F(ValidateIdWithMessage, DISABLED_OpStoreFunction) {
  string spirv = kGLSL450MemoryModel + R"(
%2 = OpTypeInt 32 1
%3 = OpTypePointer UniformConstant %2
%4 = OpTypeFunction %2
%5 = OpConstant %2 123
%6 = OpVariable %3 UniformConstant
%7 = OpFunction %2 None %4
%8 = OpLabel
     OpStore %6 %7
     OpReturnValue %5
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpCopyMemoryGood) {
  string spirv = kGLSL450MemoryModel + R"(
 %1 = OpTypeVoid
 %2 = OpTypeInt 32 0
 %3 = OpTypePointer UniformConstant %2
 %4 = OpConstant %2 42
 %5 = OpVariable %3 UniformConstant %4
 %6 = OpTypePointer Function %2
 %7 = OpTypeFunction %1
 %8 = OpFunction %1 None %7
 %9 = OpLabel
%10 = OpVariable %6 Function
      OpCopyMemory %10 %5 None
      OpReturn
      OpFunctionEnd
)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpCopyMemoryBad) {
  string spirv = kGLSL450MemoryModel + R"(
 %1 = OpTypeVoid
 %2 = OpTypeInt 32 0
 %3 = OpTypePointer UniformConstant %2
 %4 = OpConstant %2 42
 %5 = OpVariable %3 UniformConstant %4
%11 = OpTypeFloat 32
 %6 = OpTypePointer Function %11
 %7 = OpTypeFunction %1
 %8 = OpFunction %1 None %7
 %9 = OpLabel
%10 = OpVariable %6 Function
      OpCopyMemory %10 %5 None
      OpReturn
      OpFunctionEnd
)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

// TODO: OpCopyMemorySized
TEST_F(ValidateIdWithMessage, OpCopyMemorySizedGood) {
  string spirv = kGLSL450MemoryModel + R"(
 %1 = OpTypeVoid
 %2 = OpTypeInt 32 0
 %3 = OpTypePointer UniformConstant %2
 %4 = OpTypePointer Function %2
 %5 = OpConstant %2 4
 %6 = OpVariable %3 UniformConstant %5
 %7 = OpTypeFunction %1
 %8 = OpFunction %1 None %7
 %9 = OpLabel
%10 = OpVariable %4 Function
      OpCopyMemorySized %10 %6 %5 None
      OpReturn
      OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpCopyMemorySizedTargetBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypePointer UniformConstant %2
%4 = OpTypePointer Function %2
%5 = OpConstant %2 4
%6 = OpVariable %3 UniformConstant %5
%7 = OpTypeFunction %1
%8 = OpFunction %1 None %7
%9 = OpLabel
     OpCopyMemorySized %9 %6 %5 None
     OpReturn
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpCopyMemorySizedSourceBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypePointer UniformConstant %2
%4 = OpTypePointer Function %2
%5 = OpConstant %2 4
%6 = OpTypeFunction %1
%7 = OpFunction %1 None %6
%8 = OpLabel
%9 = OpVariable %4 Function
     OpCopyMemorySized %9 %6 %5 None
     OpReturn
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpCopyMemorySizedSizeBad) {
  string spirv = kGLSL450MemoryModel + R"(
 %1 = OpTypeVoid
 %2 = OpTypeInt 32 0
 %3 = OpTypePointer UniformConstant %2
 %4 = OpTypePointer Function %2
 %5 = OpConstant %2 4
 %6 = OpVariable %3 UniformConstant %5
 %7 = OpTypeFunction %1
 %8 = OpFunction %1 None %7
 %9 = OpLabel
%10 = OpVariable %4 Function
      OpCopyMemorySized %10 %6 %6 None
      OpReturn
      OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpCopyMemorySizedSizeTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
 %1 = OpTypeVoid
 %2 = OpTypeInt 32 0
 %3 = OpTypePointer UniformConstant %2
 %4 = OpTypePointer Function %2
 %5 = OpConstant %2 4
 %6 = OpVariable %3 UniformConstant %5
 %7 = OpTypeFunction %1
%11 = OpTypeFloat 32
%12 = OpConstant %11 1.0
 %8 = OpFunction %1 None %7
 %9 = OpLabel
%10 = OpVariable %4 Function
      OpCopyMemorySized %10 %6 %12 None
      OpReturn
      OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

// TODO: OpAccessChain
// TODO: OpInBoundsAccessChain
// TODO: OpArrayLength
// TODO: OpImagePointer
// TODO: OpGenericPtrMemSemantics

TEST_F(ValidateIdWithMessage, OpFunctionGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 1
%3 = OpTypeFunction %1 %2 %2
%4 = OpFunction %1 None %3
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpFunctionResultTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 1
%5 = OpConstant %2 42
%3 = OpTypeFunction %1 %2 %2
%4 = OpFunction %2 None %3
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpFunctionFunctionTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 1
%4 = OpFunction %1 None %2
OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpFunctionParameterGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypeFunction %1 %2
%4 = OpFunction %1 None %3
%5 = OpFunctionParameter %2
%6 = OpLabel
     OpReturn
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpFunctionParameterMultipleGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypeFunction %1 %2 %2
%4 = OpFunction %1 None %3
%5 = OpFunctionParameter %2
%6 = OpFunctionParameter %2
%7 = OpLabel
     OpReturn
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpFunctionParameterResultTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypeFunction %1 %2
%4 = OpFunction %1 None %3
%5 = OpFunctionParameter %1
%6 = OpLabel
     OpReturn
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpFunctionCallGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypeFunction %2 %2
%4 = OpTypeFunction %1
%5 = OpConstant %2 42 ;21

%6 = OpFunction %2 None %3
%7 = OpFunctionParameter %2
%8 = OpLabel
     OpReturnValue %7
     OpFunctionEnd

%10 = OpFunction %1 None %4
%11 = OpLabel
%12 = OpFunctionCall %2 %6 %5
      OpReturn
      OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpFunctionCallResultTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypeFunction %2 %2
%4 = OpTypeFunction %1
%5 = OpConstant %2 42 ;21

%6 = OpFunction %2 None %3
%7 = OpFunctionParameter %2
%8 = OpLabel
%9 = OpLoad %2 %7
     OpReturnValue %9
     OpFunctionEnd

%10 = OpFunction %1 None %4
%11 = OpLabel
%12 = OpFunctionCall %1 %6 %5
      OpReturn
      OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpFunctionCallFunctionBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypeFunction %2 %2
%4 = OpTypeFunction %1
%5 = OpConstant %2 42 ;21

%10 = OpFunction %1 None %4
%11 = OpLabel
%12 = OpFunctionCall %2 %5 %5
      OpReturn
      OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpFunctionCallArgumentTypeBad) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypeFunction %2 %2
%4 = OpTypeFunction %1
%5 = OpConstant %2 42

%13 = OpTypeFloat 32
%14 = OpConstant %13 3.14

%6 = OpFunction %2 None %3
%7 = OpFunctionParameter %2
%8 = OpLabel
%9 = OpLoad %2 %7
     OpReturnValue %9
     OpFunctionEnd

%10 = OpFunction %1 None %4
%11 = OpLabel
%12 = OpFunctionCall %2 %6 %14
      OpReturn
      OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
#if 0
TEST_F(ValidateIdWithMessage, OpFunctionCallArgumentCountBar) {
  const char *spirv = R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypeFunction %2 %2
%4 = OpTypeFunction %1
%5 = OpConstant %2 42 ;21

%6 = OpFunction %2 None %3
%7 = OpFunctionParameter %2
%8 = OpLabel
%9 = OpLoad %2 %7
     OpReturnValue %9
     OpFunctionEnd

%10 = OpFunction %1 None %4
%11 = OpLabel
      OpReturn
%12 = OpFunctionCall %2 %6 %5
      OpFunctionEnd)";
  CHECK(spirv, SPV_ERROR_INVALID_ID);
}
#endif

// TODO: OpSampledImage
// TODO: The many things that changed with how images are used.
// TODO: OpTextureSample
// TODO: OpTextureSampleDref
// TODO: OpTextureSampleLod
// TODO: OpTextureSampleProj
// TODO: OpTextureSampleGrad
// TODO: OpTextureSampleOffset
// TODO: OpTextureSampleProjLod
// TODO: OpTextureSampleProjGrad
// TODO: OpTextureSampleLodOffset
// TODO: OpTextureSampleProjOffset
// TODO: OpTextureSampleGradOffset
// TODO: OpTextureSampleProjLodOffset
// TODO: OpTextureSampleProjGradOffset
// TODO: OpTextureFetchTexelLod
// TODO: OpTextureFetchTexelOffset
// TODO: OpTextureFetchSample
// TODO: OpTextureFetchTexel
// TODO: OpTextureGather
// TODO: OpTextureGatherOffset
// TODO: OpTextureGatherOffsets
// TODO: OpTextureQuerySizeLod
// TODO: OpTextureQuerySize
// TODO: OpTextureQueryLevels
// TODO: OpTextureQuerySamples
// TODO: OpConvertUToF
// TODO: OpConvertFToS
// TODO: OpConvertSToF
// TODO: OpConvertUToF
// TODO: OpUConvert
// TODO: OpSConvert
// TODO: OpFConvert
// TODO: OpConvertPtrToU
// TODO: OpConvertUToPtr
// TODO: OpPtrCastToGeneric
// TODO: OpGenericCastToPtr
// TODO: OpBitcast
// TODO: OpGenericCastToPtrExplicit
// TODO: OpSatConvertSToU
// TODO: OpSatConvertUToS
// TODO: OpVectorExtractDynamic
// TODO: OpVectorInsertDynamic
// TODO: OpVectorShuffle
// TODO: OpCompositeConstruct
// TODO: OpCompositeExtract
// TODO: OpCompositeInsert
// TODO: OpCopyObject
// TODO: OpTranspose
// TODO: OpSNegate
// TODO: OpFNegate
// TODO: OpNot
// TODO: OpIAdd
// TODO: OpFAdd
// TODO: OpISub
// TODO: OpFSub
// TODO: OpIMul
// TODO: OpFMul
// TODO: OpUDiv
// TODO: OpSDiv
// TODO: OpFDiv
// TODO: OpUMod
// TODO: OpSRem
// TODO: OpSMod
// TODO: OpFRem
// TODO: OpFMod
// TODO: OpVectorTimesScalar
// TODO: OpMatrixTimesScalar
// TODO: OpVectorTimesMatrix
// TODO: OpMatrixTimesVector
// TODO: OpMatrixTimesMatrix
// TODO: OpOuterProduct
// TODO: OpDot
// TODO: OpShiftRightLogical
// TODO: OpShiftRightArithmetic
// TODO: OpShiftLeftLogical
// TODO: OpBitwiseOr
// TODO: OpBitwiseXor
// TODO: OpBitwiseAnd
// TODO: OpAny
// TODO: OpAll
// TODO: OpIsNan
// TODO: OpIsInf
// TODO: OpIsFinite
// TODO: OpIsNormal
// TODO: OpSignBitSet
// TODO: OpLessOrGreater
// TODO: OpOrdered
// TODO: OpUnordered
// TODO: OpLogicalOr
// TODO: OpLogicalXor
// TODO: OpLogicalAnd
// TODO: OpSelect
// TODO: OpIEqual
// TODO: OpFOrdEqual
// TODO: OpFUnordEqual
// TODO: OpINotEqual
// TODO: OpFOrdNotEqual
// TODO: OpFUnordNotEqual
// TODO: OpULessThan
// TODO: OpSLessThan
// TODO: OpFOrdLessThan
// TODO: OpFUnordLessThan
// TODO: OpUGreaterThan
// TODO: OpSGreaterThan
// TODO: OpFOrdGreaterThan
// TODO: OpFUnordGreaterThan
// TODO: OpULessThanEqual
// TODO: OpSLessThanEqual
// TODO: OpFOrdLessThanEqual
// TODO: OpFUnordLessThanEqual
// TODO: OpUGreaterThanEqual
// TODO: OpSGreaterThanEqual
// TODO: OpFOrdGreaterThanEqual
// TODO: OpFUnordGreaterThanEqual
// TODO: OpDPdx
// TODO: OpDPdy
// TODO: OpFWidth
// TODO: OpDPdxFine
// TODO: OpDPdyFine
// TODO: OpFwidthFine
// TODO: OpDPdxCoarse
// TODO: OpDPdyCoarse
// TODO: OpFwidthCoarse
// TODO: OpPhi
// TODO: OpLoopMerge
// TODO: OpSelectionMerge
// TODO: OpBranch
// TODO: OpBranchConditional
// TODO: OpSwitch

TEST_F(ValidateIdWithMessage, OpReturnValueConstantGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypeFunction %2
%4 = OpConstant %2 42
%5 = OpFunction %2 None %3
%6 = OpLabel
     OpReturnValue %4
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpReturnValueVariableGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 0 ;10
%3 = OpTypeFunction %2
%8 = OpTypePointer Function %2 ;18
%4 = OpConstant %2 42 ;22
%5 = OpFunction %2 None %3 ;27
%6 = OpLabel ;29
%7 = OpVariable %8 Function %4 ;34
%9 = OpLoad %2 %7
     OpReturnValue %9 ;36
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpReturnValueExpressionGood) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypeFunction %2
%4 = OpConstant %2 42
%5 = OpFunction %2 None %3
%6 = OpLabel
%7 = OpIAdd %2 %4 %4
     OpReturnValue %7
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpReturnValueIsType) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypeFunction %2
%5 = OpFunction %2 None %3
%6 = OpLabel
     OpReturnValue %1
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpReturnValueIsLabel) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypeFunction %2
%5 = OpFunction %2 None %3
%6 = OpLabel
     OpReturnValue %6
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpReturnValueIsVoid) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypeFunction %1
%5 = OpFunction %1 None %3
%6 = OpLabel
%7 = OpFunctionCall %1 %5
     OpReturnValue %7
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpReturnValueIsVariableInPhysical) {
  // It's valid to return a pointer in a physical addressing model.
  string spirv = kGLSL450MemoryModel + R"(
     OpMemoryModel Physical32 OpenCL
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypePointer Private %2
%4 = OpTypeFunction %3
%5 = OpFunction %3 None %4
%6 = OpLabel
%7 = OpVariable %3 Function
     OpReturnValue %7
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpReturnValueIsVariableInLogical) {
  // It's invalid to return a pointer in a physical addressing model.
  string spirv = kGLSL450MemoryModel + R"(
     OpMemoryModel Logical GLSL450
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypePointer Private %2
%4 = OpTypeFunction %3
%5 = OpFunction %3 None %4
%6 = OpLabel
%7 = OpVariable %3 Function
     OpReturnValue %7
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

// TODO: enable when this bug is fixed:
// https://cvs.khronos.org/bugzilla/show_bug.cgi?id=15404
TEST_F(ValidateIdWithMessage, DISABLED_OpReturnValueIsFunction) {
  string spirv = kGLSL450MemoryModel + R"(
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypeFunction %2
%5 = OpFunction %2 None %3
%6 = OpLabel
     OpReturnValue %5
     OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, UndefinedTypeId) {
  string spirv = kGLSL450MemoryModel + R"(
%s = OpTypeStruct %i32
)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, UndefinedIdScope) {
  string spirv = kGLSL450MemoryModel + R"(
%u32    = OpTypeInt 32 0
%memsem = OpConstant %u32 0
%void   = OpTypeVoid
%void_f = OpTypeFunction %void
%f      = OpFunction %void None %void_f
%l      = OpLabel
          OpMemoryBarrier %undef %memsem
          OpReturn
          OpFunctionEnd
)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, UndefinedIdMemSem) {
  string spirv = kGLSL450MemoryModel + R"(
%u32    = OpTypeInt 32 0
%scope  = OpConstant %u32 0
%void   = OpTypeVoid
%void_f = OpTypeFunction %void
%f      = OpFunction %void None %void_f
%l      = OpLabel
          OpMemoryBarrier %scope %undef
          OpReturn
          OpFunctionEnd
)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage,
       KernelOpEntryPointAndOpInBoundsPtrAccessChainGood) {
  string spirv = kOpenCLMemoryModel32 + R"(
      OpEntryPoint Kernel %2 "simple_kernel"
      OpSource OpenCL_C 200000
      OpDecorate %3 BuiltIn GlobalInvocationId
      OpDecorate %3 Constant
      OpDecorate %4 FuncParamAttr NoCapture
      OpDecorate %3 LinkageAttributes "__spirv_GlobalInvocationId" Import
 %5 = OpTypeInt 32 0
 %6 = OpTypeVector %5 3
 %7 = OpTypePointer UniformConstant %6
 %3 = OpVariable %7 UniformConstant
 %8 = OpTypeVoid
 %9 = OpTypeStruct %5
%10 = OpTypePointer CrossWorkgroup %9
%11 = OpTypeFunction %8 %10
%12 = OpConstant %5 0
%13 = OpTypePointer CrossWorkgroup %5
%14 = OpConstant %5 42
 %2 = OpFunction %8 None %11
 %4 = OpFunctionParameter %10
%15 = OpLabel
%16 = OpLoad %6 %3 Aligned 0
%17 = OpCompositeExtract %5 %16 0
%18 = OpInBoundsPtrAccessChain %13 %4 %17 %12
      OpStore %18 %14 Aligned 4
      OpReturn
      OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpPtrAccessChainGood) {
  string spirv = kOpenCLMemoryModel64 + R"(
      OpEntryPoint Kernel %2 "another_kernel"
      OpSource OpenCL_C 200000
      OpDecorate %3 BuiltIn GlobalInvocationId
      OpDecorate %3 Constant
      OpDecorate %4 FuncParamAttr NoCapture
      OpDecorate %3 LinkageAttributes "__spirv_GlobalInvocationId" Import
 %5 = OpTypeInt 64 0
 %6 = OpTypeVector %5 3
 %7 = OpTypePointer UniformConstant %6
 %3 = OpVariable %7 UniformConstant
 %8 = OpTypeVoid
 %9 = OpTypeInt 32 0
%10 = OpTypeStruct %9
%11 = OpTypePointer CrossWorkgroup %10
%12 = OpTypeFunction %8 %11
%13 = OpConstant %5 4294967295
%14 = OpConstant %9 0
%15 = OpTypePointer CrossWorkgroup %9
%16 = OpConstant %9 42
 %2 = OpFunction %8 None %12
 %4 = OpFunctionParameter %11
%17 = OpLabel
%18 = OpLoad %6 %3 Aligned 0
%19 = OpCompositeExtract %5 %18 0
%20 = OpBitwiseAnd %5 %19 %13
%21 = OpPtrAccessChain %15 %4 %20 %14
      OpStore %21 %16 Aligned 4
      OpReturn
      OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateIdWithMessage, OpLoadBitcastPointerGood) {
  string spirv = kOpenCLMemoryModel64 + R"(
%2  = OpTypeVoid
%3  = OpTypeInt 32 1
%4  = OpTypeFloat 32
%5  = OpTypePointer UniformConstant %3
%6  = OpTypePointer UniformConstant %4
%7  = OpVariable %5 UniformConstant
%8  = OpTypeFunction %2
%9  = OpFunction %2 None %8
%10 = OpLabel
%11 = OpBitcast %6 %7
%12 = OpLoad %4 %11
      OpReturn
      OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpLoadBitcastNonPointerBad) {
  string spirv = kOpenCLMemoryModel64 + R"(
%2  = OpTypeVoid
%3  = OpTypeInt 32 1
%4  = OpTypeFloat 32
%5  = OpTypePointer UniformConstant %3
%6  = OpTypeFunction %2
%7  = OpVariable %5 UniformConstant
%8  = OpFunction %2 None %6
%9  = OpLabel
%10 = OpLoad %3 %7
%11 = OpBitcast %4 %10
%12 = OpLoad %3 %11
      OpReturn
      OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpStoreBitcastPointerGood) {
  string spirv = kOpenCLMemoryModel64 + R"(
%2  = OpTypeVoid
%3  = OpTypeInt 32 1
%4  = OpTypeFloat 32
%5  = OpTypePointer Function %3
%6  = OpTypePointer Function %4
%7  = OpTypeFunction %2
%8  = OpConstant %3 42
%9  = OpFunction %2 None %7
%10 = OpLabel
%11 = OpVariable %6 Function
%12 = OpBitcast %5 %11
      OpStore %12 %8
      OpReturn
      OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}
TEST_F(ValidateIdWithMessage, OpStoreBitcastNonPointerBad) {
  string spirv = kOpenCLMemoryModel64 + R"(
%2  = OpTypeVoid
%3  = OpTypeInt 32 1
%4  = OpTypeFloat 32
%5  = OpTypePointer Function %4
%6  = OpTypeFunction %2
%7  = OpConstant %4 42
%8  = OpFunction %2 None %6
%9  = OpLabel
%10 = OpVariable %5 Function
%11 = OpBitcast %3 %7
      OpStore %11 %7
      OpReturn
      OpFunctionEnd)";
  CompileSuccessfully(spirv.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
}

// TODO: OpLifetimeStart
// TODO: OpLifetimeStop
// TODO: OpAtomicInit
// TODO: OpAtomicLoad
// TODO: OpAtomicStore
// TODO: OpAtomicExchange
// TODO: OpAtomicCompareExchange
// TODO: OpAtomicCompareExchangeWeak
// TODO: OpAtomicIIncrement
// TODO: OpAtomicIDecrement
// TODO: OpAtomicIAdd
// TODO: OpAtomicISub
// TODO: OpAtomicUMin
// TODO: OpAtomicUMax
// TODO: OpAtomicAnd
// TODO: OpAtomicOr
// TODO: OpAtomicXor
// TODO: OpAtomicIMin
// TODO: OpAtomicIMax
// TODO: OpEmitStreamVertex
// TODO: OpEndStreamPrimitive
// TODO: OpAsyncGroupCopy
// TODO: OpWaitGroupEvents
// TODO: OpGroupAll
// TODO: OpGroupAny
// TODO: OpGroupBroadcast
// TODO: OpGroupIAdd
// TODO: OpGroupFAdd
// TODO: OpGroupFMin
// TODO: OpGroupUMin
// TODO: OpGroupSMin
// TODO: OpGroupFMax
// TODO: OpGroupUMax
// TODO: OpGroupSMax
// TODO: OpEnqueueMarker
// TODO: OpEnqueueKernel
// TODO: OpGetKernelNDrangeSubGroupCount
// TODO: OpGetKernelNDrangeMaxSubGroupSize
// TODO: OpGetKernelWorkGroupSize
// TODO: OpGetKernelPreferredWorkGroupSizeMultiple
// TODO: OpRetainEvent
// TODO: OpReleaseEvent
// TODO: OpCreateUserEvent
// TODO: OpIsValidEvent
// TODO: OpSetUserEventStatus
// TODO: OpCaptureEventProfilingInfo
// TODO: OpGetDefaultQueue
// TODO: OpBuildNDRange
// TODO: OpReadPipe
// TODO: OpWritePipe
// TODO: OpReservedReadPipe
// TODO: OpReservedWritePipe
// TODO: OpReserveReadPipePackets
// TODO: OpReserveWritePipePackets
// TODO: OpCommitReadPipe
// TODO: OpCommitWritePipe
// TODO: OpIsValidReserveId
// TODO: OpGetNumPipePackets
// TODO: OpGetMaxPipePackets
// TODO: OpGroupReserveReadPipePackets
// TODO: OpGroupReserveWritePipePackets
// TODO: OpGroupCommitReadPipe
// TODO: OpGroupCommitWritePipe

}  // anonymous namespace