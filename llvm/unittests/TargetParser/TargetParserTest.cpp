//===----------- TargetParser.cpp - Target Parser -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/TargetParser/TargetParser.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/ARMBuildAttributes.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/TargetParser/AArch64TargetParser.h"
#include "llvm/TargetParser/ARMTargetParser.h"
#include "llvm/TargetParser/ARMTargetParserCommon.h"
#include "llvm/TargetParser/Triple.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <optional>
#include <string>

using namespace llvm;

namespace {
const char *ARMArch[] = {
    "armv4",        "armv4t",      "armv5",          "armv5t",      "armv5e",
    "armv5te",      "armv5tej",    "armv6",          "armv6j",      "armv6k",
    "armv6hl",      "armv6t2",     "armv6kz",        "armv6z",      "armv6zk",
    "armv6-m",      "armv6m",      "armv6sm",        "armv6s-m",    "armv7-a",
    "armv7",        "armv7a",      "armv7ve",        "armv7hl",     "armv7l",
    "armv7-r",      "armv7r",      "armv7-m",        "armv7m",      "armv7k",
    "armv7s",       "armv7e-m",    "armv7em",        "armv8-a",     "armv8",
    "armv8a",       "armv8l",      "armv8.1-a",      "armv8.1a",    "armv8.2-a",
    "armv8.2a",     "armv8.3-a",   "armv8.3a",       "armv8.4-a",   "armv8.4a",
    "armv8.5-a",    "armv8.5a",    "armv8.6-a",      "armv8.6a",    "armv8.7-a",
    "armv8.7a",     "armv8.8-a",   "armv8.8a",       "armv8.9-a",   "armv8.9a",
    "armv8-r",      "armv8r",      "armv8-m.base",   "armv8m.base", "armv8-m.main",
    "armv8m.main",  "iwmmxt",      "iwmmxt2",        "xscale",      "armv8.1-m.main",
    "armv9-a",      "armv9",       "armv9a",         "armv9.1-a",   "armv9.1a",
    "armv9.2-a",    "armv9.2a",    "armv9.3-a",      "armv9.3a",    "armv9.4-a",
    "armv9.4a",
};

template <ARM::ISAKind ISAKind>
std::string FormatExtensionFlags(uint64_t Flags) {
  std::vector<StringRef> Features;

  if (ISAKind == ARM::ISAKind::AARCH64) {
    // AEK_NONE is not meant to be shown to the user so the target parser
    // does not recognise it. It is relevant here though.
    if (Flags & AArch64::AEK_NONE)
      Features.push_back("none");
    AArch64::getExtensionFeatures(Flags, Features);
  } else {
    if (Flags & ARM::AEK_NONE)
      Features.push_back("none");
    ARM::getExtensionFeatures(Flags, Features);
  }

  // The target parser also includes every extension you don't have.
  // E.g. if AEK_CRC is not set then it adds "-crc". Not useful here.
  Features.erase(std::remove_if(Features.begin(), Features.end(),
                                [](StringRef extension) {
                                  return extension.startswith("-");
                                }),
                 Features.end());

  return llvm::join(Features, ", ");
}

template <ARM::ISAKind ISAKind> struct AssertSameExtensionFlags {
  AssertSameExtensionFlags(StringRef CPUName) : CPUName(CPUName) {}

  testing::AssertionResult operator()(const char *m_expr, const char *n_expr,
                                      uint64_t ExpectedFlags,
                                      uint64_t GotFlags) {
    if (ExpectedFlags == GotFlags)
      return testing::AssertionSuccess();

    return testing::AssertionFailure() << llvm::formatv(
               "CPU: {4}\n"
               "Expected extension flags: {0} ({1:x})\n"
               "     Got extension flags: {2} ({3:x})\n",
               FormatExtensionFlags<ISAKind>(ExpectedFlags), ExpectedFlags,
               FormatExtensionFlags<ISAKind>(GotFlags), GotFlags, CPUName);
  }

private:
  StringRef CPUName;
};

struct ARMCPUTestParams {
  ARMCPUTestParams(StringRef CPUName, StringRef ExpectedArch,
                   StringRef ExpectedFPU, uint64_t ExpectedFlags,
                   StringRef CPUAttr)
      : CPUName(CPUName), ExpectedArch(ExpectedArch), ExpectedFPU(ExpectedFPU),
        ExpectedFlags(ExpectedFlags), CPUAttr(CPUAttr) {}

  friend std::ostream &operator<<(std::ostream &os,
                                  const ARMCPUTestParams &params) {
    return os << "\"" << params.CPUName.str() << "\", \""
              << params.ExpectedArch.str() << "\", \""
              << params.ExpectedFPU.str() << "\", 0x" << std::hex
              << params.ExpectedFlags << ", \"" << params.CPUAttr.str() << "\"";
  }

  StringRef CPUName;
  StringRef ExpectedArch;
  StringRef ExpectedFPU;
  uint64_t ExpectedFlags;
  StringRef CPUAttr;
};

class ARMCPUTestFixture : public ::testing::TestWithParam<ARMCPUTestParams> {};

TEST_P(ARMCPUTestFixture, ARMCPUTests) {
  auto params = GetParam();

  ARM::ArchKind AK = ARM::parseCPUArch(params.CPUName);
  EXPECT_EQ(params.ExpectedArch, ARM::getArchName(AK));

  ARM::FPUKind FPUKind = ARM::getDefaultFPU(params.CPUName, AK);
  EXPECT_EQ(params.ExpectedFPU, ARM::getFPUName(FPUKind));

  uint64_t default_extensions = ARM::getDefaultExtensions(params.CPUName, AK);
  EXPECT_PRED_FORMAT2(
      AssertSameExtensionFlags<ARM::ISAKind::ARM>(params.CPUName),
      params.ExpectedFlags, default_extensions);

  EXPECT_EQ(params.CPUAttr, ARM::getCPUAttr(AK));
}

// Note that we include ARM::AEK_NONE even when there are other extensions
// we expect. This is because the default extensions for a CPU are the sum
// of the default extensions for its architecture and for the CPU.
// So if a CPU has no extra extensions, it adds AEK_NONE.
INSTANTIATE_TEST_SUITE_P(
    ARMCPUTestsPart1, ARMCPUTestFixture,
    ::testing::Values(
        ARMCPUTestParams("invalid", "invalid", "invalid", ARM::AEK_NONE, ""),
        ARMCPUTestParams("generic", "invalid", "none", ARM::AEK_NONE, ""),

        ARMCPUTestParams("arm8", "armv4", "none", ARM::AEK_NONE, "4"),
        ARMCPUTestParams("arm810", "armv4", "none", ARM::AEK_NONE, "4"),
        ARMCPUTestParams("strongarm", "armv4", "none", ARM::AEK_NONE, "4"),
        ARMCPUTestParams("strongarm110", "armv4", "none", ARM::AEK_NONE, "4"),
        ARMCPUTestParams("strongarm1100", "armv4", "none", ARM::AEK_NONE, "4"),
        ARMCPUTestParams("strongarm1110", "armv4", "none", ARM::AEK_NONE, "4"),
        ARMCPUTestParams("arm7tdmi", "armv4t", "none", ARM::AEK_NONE, "4T"),
        ARMCPUTestParams("arm7tdmi-s", "armv4t", "none", ARM::AEK_NONE, "4T"),
        ARMCPUTestParams("arm710t", "armv4t", "none", ARM::AEK_NONE, "4T"),
        ARMCPUTestParams("arm720t", "armv4t", "none", ARM::AEK_NONE, "4T"),
        ARMCPUTestParams("arm9", "armv4t", "none", ARM::AEK_NONE, "4T"),
        ARMCPUTestParams("arm9tdmi", "armv4t", "none", ARM::AEK_NONE, "4T"),
        ARMCPUTestParams("arm920", "armv4t", "none", ARM::AEK_NONE, "4T"),
        ARMCPUTestParams("arm920t", "armv4t", "none", ARM::AEK_NONE, "4T"),
        ARMCPUTestParams("arm922t", "armv4t", "none", ARM::AEK_NONE, "4T"),
        ARMCPUTestParams("arm940t", "armv4t", "none", ARM::AEK_NONE, "4T"),
        ARMCPUTestParams("ep9312", "armv4t", "none", ARM::AEK_NONE, "4T"),
        ARMCPUTestParams("arm10tdmi", "armv5t", "none", ARM::AEK_NONE, "5T"),
        ARMCPUTestParams("arm1020t", "armv5t", "none", ARM::AEK_NONE, "5T"),
        ARMCPUTestParams("arm9e", "armv5te", "none",
                         ARM::AEK_NONE | ARM::AEK_DSP, "5TE"),
        ARMCPUTestParams("arm946e-s", "armv5te", "none",
                         ARM::AEK_NONE | ARM::AEK_DSP, "5TE"),
        ARMCPUTestParams("arm966e-s", "armv5te", "none",
                         ARM::AEK_NONE | ARM::AEK_DSP, "5TE"),
        ARMCPUTestParams("arm968e-s", "armv5te", "none",
                         ARM::AEK_NONE | ARM::AEK_DSP, "5TE"),
        ARMCPUTestParams("arm10e", "armv5te", "none",
                         ARM::AEK_NONE | ARM::AEK_DSP, "5TE"),
        ARMCPUTestParams("arm1020e", "armv5te", "none",
                         ARM::AEK_NONE | ARM::AEK_DSP, "5TE"),
        ARMCPUTestParams("arm1022e", "armv5te", "none",
                         ARM::AEK_NONE | ARM::AEK_DSP, "5TE"),
        ARMCPUTestParams("arm926ej-s", "armv5tej", "none",
                         ARM::AEK_NONE | ARM::AEK_DSP, "5TEJ"),
        ARMCPUTestParams("arm1136j-s", "armv6", "none",
                         ARM::AEK_NONE | ARM::AEK_DSP, "6"),
        ARMCPUTestParams("arm1136jf-s", "armv6", "vfpv2",
                         ARM::AEK_NONE | ARM::AEK_DSP, "6"),
        ARMCPUTestParams("arm1176jz-s", "armv6kz", "none",
                         ARM::AEK_NONE | ARM::AEK_SEC | ARM::AEK_DSP, "6KZ"),
        ARMCPUTestParams("mpcore", "armv6k", "vfpv2",
                         ARM::AEK_NONE | ARM::AEK_DSP, "6K"),
        ARMCPUTestParams("mpcorenovfp", "armv6k", "none",
                         ARM::AEK_NONE | ARM::AEK_DSP, "6K"),
        ARMCPUTestParams("arm1176jzf-s", "armv6kz", "vfpv2",
                         ARM::AEK_NONE | ARM::AEK_SEC | ARM::AEK_DSP, "6KZ"),
        ARMCPUTestParams("arm1156t2-s", "armv6t2", "none",
                         ARM::AEK_NONE | ARM::AEK_DSP, "6T2"),
        ARMCPUTestParams("arm1156t2f-s", "armv6t2", "vfpv2",
                         ARM::AEK_NONE | ARM::AEK_DSP, "6T2"),
        ARMCPUTestParams("cortex-m0", "armv6-m", "none", ARM::AEK_NONE, "6-M"),
        ARMCPUTestParams("cortex-m0plus", "armv6-m", "none", ARM::AEK_NONE,
                         "6-M"),
        ARMCPUTestParams("cortex-m1", "armv6-m", "none", ARM::AEK_NONE, "6-M"),
        ARMCPUTestParams("sc000", "armv6-m", "none", ARM::AEK_NONE, "6-M"),
        ARMCPUTestParams("cortex-a5", "armv7-a", "neon-vfpv4",
                         ARM::AEK_MP | ARM::AEK_SEC | ARM::AEK_DSP, "7-A"),
        ARMCPUTestParams("cortex-a7", "armv7-a", "neon-vfpv4",
                         ARM::AEK_HWDIVTHUMB | ARM::AEK_HWDIVARM | ARM::AEK_MP |
                             ARM::AEK_SEC | ARM::AEK_VIRT | ARM::AEK_DSP,
                         "7-A"),
        ARMCPUTestParams("cortex-a8", "armv7-a", "neon",
                         ARM::AEK_SEC | ARM::AEK_DSP, "7-A")));

// gtest in llvm has a limit of 50 test cases when using ::Values so we split
// them into 2 blocks
INSTANTIATE_TEST_SUITE_P(
    ARMCPUTestsPart2, ARMCPUTestFixture,
    ::testing::Values(
        ARMCPUTestParams("cortex-a9", "armv7-a", "neon-fp16",
                         ARM::AEK_MP | ARM::AEK_SEC | ARM::AEK_DSP, "7-A"),
        ARMCPUTestParams("cortex-a12", "armv7-a", "neon-vfpv4",
                         ARM::AEK_SEC | ARM::AEK_MP | ARM::AEK_VIRT |
                             ARM::AEK_HWDIVARM | ARM::AEK_HWDIVTHUMB |
                             ARM::AEK_DSP,
                         "7-A"),
        ARMCPUTestParams("cortex-a15", "armv7-a", "neon-vfpv4",
                         ARM::AEK_SEC | ARM::AEK_MP | ARM::AEK_VIRT |
                             ARM::AEK_HWDIVARM | ARM::AEK_HWDIVTHUMB |
                             ARM::AEK_DSP,
                         "7-A"),
        ARMCPUTestParams("cortex-a17", "armv7-a", "neon-vfpv4",
                         ARM::AEK_SEC | ARM::AEK_MP | ARM::AEK_VIRT |
                             ARM::AEK_HWDIVARM | ARM::AEK_HWDIVTHUMB |
                             ARM::AEK_DSP,
                         "7-A"),
        ARMCPUTestParams("krait", "armv7-a", "neon-vfpv4",
                         ARM::AEK_HWDIVARM | ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP,
                         "7-A"),
        ARMCPUTestParams("cortex-r4", "armv7-r", "none",
                         ARM::AEK_NONE | ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP,
                         "7-R"),
        ARMCPUTestParams("cortex-r4f", "armv7-r", "vfpv3-d16",
                         ARM::AEK_NONE | ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP,
                         "7-R"),
        ARMCPUTestParams("cortex-r5", "armv7-r", "vfpv3-d16",
                         ARM::AEK_MP | ARM::AEK_HWDIVARM | ARM::AEK_HWDIVTHUMB |
                             ARM::AEK_DSP,
                         "7-R"),
        ARMCPUTestParams("cortex-r7", "armv7-r", "vfpv3-d16-fp16",
                         ARM::AEK_MP | ARM::AEK_HWDIVARM | ARM::AEK_HWDIVTHUMB |
                             ARM::AEK_DSP,
                         "7-R"),
        ARMCPUTestParams("cortex-r8", "armv7-r", "vfpv3-d16-fp16",
                         ARM::AEK_MP | ARM::AEK_HWDIVARM | ARM::AEK_HWDIVTHUMB |
                             ARM::AEK_DSP,
                         "7-R"),
        ARMCPUTestParams("cortex-r52", "armv8-r", "neon-fp-armv8",
                         ARM::AEK_NONE | ARM::AEK_CRC | ARM::AEK_MP |
                             ARM::AEK_VIRT | ARM::AEK_HWDIVARM |
                             ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP,
                         "8-R"),
        ARMCPUTestParams("sc300", "armv7-m", "none",
                         ARM::AEK_NONE | ARM::AEK_HWDIVTHUMB, "7-M"),
        ARMCPUTestParams("cortex-m3", "armv7-m", "none",
                         ARM::AEK_NONE | ARM::AEK_HWDIVTHUMB, "7-M"),
        ARMCPUTestParams("cortex-m4", "armv7e-m", "fpv4-sp-d16",
                         ARM::AEK_NONE | ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP,
                         "7E-M"),
        ARMCPUTestParams("cortex-m7", "armv7e-m", "fpv5-d16",
                         ARM::AEK_NONE | ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP,
                         "7E-M"),
        ARMCPUTestParams("cortex-a32", "armv8-a", "crypto-neon-fp-armv8",
                         ARM::AEK_CRC | ARM::AEK_SEC | ARM::AEK_MP |
                             ARM::AEK_VIRT | ARM::AEK_HWDIVARM |
                             ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP,
                         "8-A"),
        ARMCPUTestParams("cortex-a35", "armv8-a", "crypto-neon-fp-armv8",
                         ARM::AEK_CRC | ARM::AEK_SEC | ARM::AEK_MP |
                             ARM::AEK_VIRT | ARM::AEK_HWDIVARM |
                             ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP,
                         "8-A"),
        ARMCPUTestParams("cortex-a53", "armv8-a", "crypto-neon-fp-armv8",
                         ARM::AEK_CRC | ARM::AEK_SEC | ARM::AEK_MP |
                             ARM::AEK_VIRT | ARM::AEK_HWDIVARM |
                             ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP,
                         "8-A"),
        ARMCPUTestParams("cortex-a55", "armv8.2-a", "crypto-neon-fp-armv8",
                         ARM::AEK_CRC | ARM::AEK_SEC | ARM::AEK_MP |
                             ARM::AEK_VIRT | ARM::AEK_HWDIVARM |
                             ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP |
                             ARM::AEK_FP16 | ARM::AEK_RAS | ARM::AEK_DOTPROD,
                         "8.2-A"),
        ARMCPUTestParams("cortex-a57", "armv8-a", "crypto-neon-fp-armv8",
                         ARM::AEK_CRC | ARM::AEK_SEC | ARM::AEK_MP |
                             ARM::AEK_VIRT | ARM::AEK_HWDIVARM |
                             ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP,
                         "8-A"),
        ARMCPUTestParams("cortex-a72", "armv8-a", "crypto-neon-fp-armv8",
                         ARM::AEK_CRC | ARM::AEK_SEC | ARM::AEK_MP |
                             ARM::AEK_VIRT | ARM::AEK_HWDIVARM |
                             ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP,
                         "8-A"),
        ARMCPUTestParams("cortex-a73", "armv8-a", "crypto-neon-fp-armv8",
                         ARM::AEK_CRC | ARM::AEK_SEC | ARM::AEK_MP |
                             ARM::AEK_VIRT | ARM::AEK_HWDIVARM |
                             ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP,
                         "8-A"),
        ARMCPUTestParams("cortex-a75", "armv8.2-a", "crypto-neon-fp-armv8",
                         ARM::AEK_CRC | ARM::AEK_SEC | ARM::AEK_MP |
                             ARM::AEK_VIRT | ARM::AEK_HWDIVARM |
                             ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP |
                             ARM::AEK_FP16 | ARM::AEK_RAS | ARM::AEK_DOTPROD,
                         "8.2-A"),
        ARMCPUTestParams("cortex-a76", "armv8.2-a", "crypto-neon-fp-armv8",
                         ARM::AEK_CRC | ARM::AEK_SEC | ARM::AEK_MP |
                             ARM::AEK_VIRT | ARM::AEK_HWDIVARM |
                             ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP |
                             ARM::AEK_FP16 | ARM::AEK_RAS | ARM::AEK_DOTPROD,
                         "8.2-A"),
        ARMCPUTestParams("cortex-a76ae", "armv8.2-a", "crypto-neon-fp-armv8",
                         ARM::AEK_CRC | ARM::AEK_SEC | ARM::AEK_MP |
                             ARM::AEK_VIRT | ARM::AEK_HWDIVARM |
                             ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP |
                             ARM::AEK_FP16 | ARM::AEK_RAS | ARM::AEK_DOTPROD,
                         "8.2-A"),
        ARMCPUTestParams("cortex-a78c", "armv8.2-a", "crypto-neon-fp-armv8",
                         ARM::AEK_SEC | ARM::AEK_MP | ARM::AEK_VIRT |
                             ARM::AEK_HWDIVARM | ARM::AEK_HWDIVTHUMB |
                             ARM::AEK_DSP | ARM::AEK_CRC | ARM::AEK_RAS |
                             ARM::AEK_FP16 | ARM::AEK_DOTPROD,
                         "8.2-A"),
        ARMCPUTestParams("cortex-a710", "armv9-a", "neon-fp-armv8",
                         ARM::AEK_SEC | ARM::AEK_MP | ARM::AEK_VIRT |
                             ARM::AEK_HWDIVARM | ARM::AEK_HWDIVTHUMB |
                             ARM::AEK_DSP | ARM::AEK_CRC | ARM::AEK_RAS |
                             ARM::AEK_DOTPROD | ARM::AEK_FP16FML |
                             ARM::AEK_BF16 | ARM::AEK_I8MM | ARM::AEK_SB,
                         "9-A"),
        ARMCPUTestParams("cortex-a77", "armv8.2-a", "crypto-neon-fp-armv8",
                         ARM::AEK_CRC | ARM::AEK_SEC | ARM::AEK_MP |
                             ARM::AEK_VIRT | ARM::AEK_HWDIVARM |
                             ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP |
                             ARM::AEK_FP16 | ARM::AEK_RAS | ARM::AEK_DOTPROD,
                         "8.2-A"),
        ARMCPUTestParams("cortex-a78", "armv8.2-a", "crypto-neon-fp-armv8",
                         ARM::AEK_DOTPROD | ARM::AEK_FP16 | ARM::AEK_SEC |
                             ARM::AEK_MP | ARM::AEK_VIRT | ARM::AEK_HWDIVARM |
                             ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP | ARM::AEK_CRC |
                             ARM::AEK_RAS,
                         "8.2-A"),
        ARMCPUTestParams("cortex-x1", "armv8.2-a", "crypto-neon-fp-armv8",
                         ARM::AEK_RAS | ARM::AEK_FP16 | ARM::AEK_DOTPROD |
                             ARM::AEK_SEC | ARM::AEK_MP | ARM::AEK_VIRT |
                             ARM::AEK_HWDIVARM | ARM::AEK_HWDIVTHUMB |
                             ARM::AEK_DSP | ARM::AEK_CRC,
                         "8.2-A"),
        ARMCPUTestParams("cortex-x1c", "armv8.2-a", "crypto-neon-fp-armv8",
                         ARM::AEK_RAS | ARM::AEK_FP16 | ARM::AEK_DOTPROD |
                             ARM::AEK_SEC | ARM::AEK_MP | ARM::AEK_VIRT |
                             ARM::AEK_HWDIVARM | ARM::AEK_HWDIVTHUMB |
                             ARM::AEK_DSP | ARM::AEK_CRC,
                         "8.2-A"),
        ARMCPUTestParams("neoverse-n1", "armv8.2-a", "crypto-neon-fp-armv8",
                         ARM::AEK_CRC | ARM::AEK_SEC | ARM::AEK_MP |
                             ARM::AEK_VIRT | ARM::AEK_HWDIVARM |
                             ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP |
                             ARM::AEK_FP16 | ARM::AEK_RAS | ARM::AEK_DOTPROD,
                         "8.2-A"),
        ARMCPUTestParams("neoverse-n2", "armv8.5-a", "crypto-neon-fp-armv8",
                         ARM::AEK_CRC | ARM::AEK_HWDIVTHUMB |
                             ARM::AEK_HWDIVARM | ARM::AEK_MP | ARM::AEK_SEC |
                             ARM::AEK_VIRT | ARM::AEK_DSP | ARM::AEK_BF16 |
                             ARM::AEK_DOTPROD | ARM::AEK_RAS | ARM::AEK_I8MM |
                             ARM::AEK_SB,
                         "8.5-A"),
        ARMCPUTestParams("neoverse-v1", "armv8.4-a", "crypto-neon-fp-armv8",
                         ARM::AEK_SEC | ARM::AEK_MP | ARM::AEK_VIRT |
                             ARM::AEK_HWDIVARM | ARM::AEK_HWDIVTHUMB |
                             ARM::AEK_DSP | ARM::AEK_CRC | ARM::AEK_RAS |
                             ARM::AEK_FP16 | ARM::AEK_BF16 | ARM::AEK_DOTPROD,
                         "8.4-A"),
        ARMCPUTestParams("cyclone", "armv8-a", "crypto-neon-fp-armv8",
                         ARM::AEK_CRC | ARM::AEK_SEC | ARM::AEK_MP |
                             ARM::AEK_VIRT | ARM::AEK_HWDIVARM |
                             ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP,
                         "8-A"),
        ARMCPUTestParams("exynos-m3", "armv8-a", "crypto-neon-fp-armv8",
                         ARM::AEK_CRC | ARM::AEK_SEC | ARM::AEK_MP |
                             ARM::AEK_VIRT | ARM::AEK_HWDIVARM |
                             ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP,
                         "8-A"),
        ARMCPUTestParams("exynos-m4", "armv8.2-a", "crypto-neon-fp-armv8",
                         ARM::AEK_CRC | ARM::AEK_SEC | ARM::AEK_MP |
                             ARM::AEK_VIRT | ARM::AEK_HWDIVARM |
                             ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP |
                             ARM::AEK_DOTPROD | ARM::AEK_FP16 | ARM::AEK_RAS,
                         "8.2-A"),
        ARMCPUTestParams("exynos-m5", "armv8.2-a", "crypto-neon-fp-armv8",
                         ARM::AEK_CRC | ARM::AEK_SEC | ARM::AEK_MP |
                             ARM::AEK_VIRT | ARM::AEK_HWDIVARM |
                             ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP |
                             ARM::AEK_DOTPROD | ARM::AEK_FP16 | ARM::AEK_RAS,
                         "8.2-A"),
        ARMCPUTestParams("cortex-m23", "armv8-m.base", "none",
                         ARM::AEK_NONE | ARM::AEK_HWDIVTHUMB, "8-M.Baseline"),
        ARMCPUTestParams("cortex-m33", "armv8-m.main", "fpv5-sp-d16",
                         ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP, "8-M.Mainline"),
        ARMCPUTestParams("cortex-m35p", "armv8-m.main", "fpv5-sp-d16",
                         ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP, "8-M.Mainline"),
        ARMCPUTestParams("cortex-m55", "armv8.1-m.main",
                         "fp-armv8-fullfp16-d16",
                         ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP | ARM::AEK_SIMD |
                             ARM::AEK_FP | ARM::AEK_RAS | ARM::AEK_LOB |
                             ARM::AEK_FP16,
                         "8.1-M.Mainline"),
        ARMCPUTestParams("cortex-m85", "armv8.1-m.main",
                         "fp-armv8-fullfp16-d16",
                         ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP | ARM::AEK_SIMD |
                             ARM::AEK_FP | ARM::AEK_RAS | ARM::AEK_LOB |
                             ARM::AEK_FP16 | ARM::AEK_PACBTI,
                         "8.1-M.Mainline"),
        ARMCPUTestParams("iwmmxt", "iwmmxt", "none", ARM::AEK_NONE, "iwmmxt"),
        ARMCPUTestParams("xscale", "xscale", "none", ARM::AEK_NONE, "xscale"),
        ARMCPUTestParams("swift", "armv7s", "neon-vfpv4",
                         ARM::AEK_HWDIVARM | ARM::AEK_HWDIVTHUMB | ARM::AEK_DSP,
                         "7-S")));

static constexpr unsigned NumARMCPUArchs = 89;

TEST(TargetParserTest, testARMCPUArchList) {
  SmallVector<StringRef, NumARMCPUArchs> List;
  ARM::fillValidCPUArchList(List);

  // No list exists for these in this test suite, so ensure all are
  // valid, and match the expected 'magic' count.
  EXPECT_EQ(List.size(), NumARMCPUArchs);
  for(StringRef CPU : List) {
    EXPECT_NE(ARM::parseCPUArch(CPU), ARM::ArchKind::INVALID);
  }
}

TEST(TargetParserTest, testInvalidARMArch) {
  auto InvalidArchStrings = {"armv", "armv99", "noarm"};
  for (const char* InvalidArch : InvalidArchStrings)
    EXPECT_EQ(ARM::parseArch(InvalidArch), ARM::ArchKind::INVALID);
}

bool testARMArch(StringRef Arch, StringRef DefaultCPU, StringRef SubArch,
                 unsigned ArchAttr) {
  ARM::ArchKind AK = ARM::parseArch(Arch);
  bool Result = (AK != ARM::ArchKind::INVALID);
  Result &= ARM::getDefaultCPU(Arch).equals(DefaultCPU);
  Result &= ARM::getSubArch(AK).equals(SubArch);
  Result &= (ARM::getArchAttr(AK) == ArchAttr);
  return Result;
}

TEST(TargetParserTest, testARMArch) {
  EXPECT_TRUE(
      testARMArch("armv4", "strongarm", "v4",
                          ARMBuildAttrs::CPUArch::v4));
  EXPECT_TRUE(
      testARMArch("armv4t", "arm7tdmi", "v4t",
                          ARMBuildAttrs::CPUArch::v4T));
  EXPECT_TRUE(
      testARMArch("armv5t", "arm10tdmi", "v5",
                          ARMBuildAttrs::CPUArch::v5T));
  EXPECT_TRUE(
      testARMArch("armv5te", "arm1022e", "v5e",
                          ARMBuildAttrs::CPUArch::v5TE));
  EXPECT_TRUE(
      testARMArch("armv5tej", "arm926ej-s", "v5e",
                          ARMBuildAttrs::CPUArch::v5TEJ));
  EXPECT_TRUE(
      testARMArch("armv6", "arm1136jf-s", "v6",
                          ARMBuildAttrs::CPUArch::v6));
  EXPECT_TRUE(
      testARMArch("armv6k", "mpcore", "v6k",
                          ARMBuildAttrs::CPUArch::v6K));
  EXPECT_TRUE(
      testARMArch("armv6t2", "arm1156t2-s", "v6t2",
                          ARMBuildAttrs::CPUArch::v6T2));
  EXPECT_TRUE(
      testARMArch("armv6kz", "arm1176jzf-s", "v6kz",
                          ARMBuildAttrs::CPUArch::v6KZ));
  EXPECT_TRUE(
      testARMArch("armv6-m", "cortex-m0", "v6m",
                          ARMBuildAttrs::CPUArch::v6_M));
  EXPECT_TRUE(
      testARMArch("armv7-a", "generic", "v7",
                          ARMBuildAttrs::CPUArch::v7));
  EXPECT_TRUE(
      testARMArch("armv7ve", "generic", "v7ve",
                          ARMBuildAttrs::CPUArch::v7));
  EXPECT_TRUE(
      testARMArch("armv7-r", "cortex-r4", "v7r",
                          ARMBuildAttrs::CPUArch::v7));
  EXPECT_TRUE(
      testARMArch("armv7-m", "cortex-m3", "v7m",
                          ARMBuildAttrs::CPUArch::v7));
  EXPECT_TRUE(
      testARMArch("armv7e-m", "cortex-m4", "v7em",
                          ARMBuildAttrs::CPUArch::v7E_M));
  EXPECT_TRUE(
      testARMArch("armv8-a", "generic", "v8a",
                          ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(
      testARMArch("armv8.1-a", "generic", "v8.1a",
                          ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(
      testARMArch("armv8.2-a", "generic", "v8.2a",
                          ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(
      testARMArch("armv8.3-a", "generic", "v8.3a",
                          ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(
      testARMArch("armv8.4-a", "generic", "v8.4a",
                          ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(
      testARMArch("armv8.5-a", "generic", "v8.5a",
                          ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(
      testARMArch("armv8.6-a", "generic", "v8.6a",
                          ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(
      testARMArch("armv8.7-a", "generic", "v8.7a",
                          ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(testARMArch("armv8.8-a", "generic", "v8.8a",
                          ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(
      testARMArch("armv8.9-a", "generic", "v8.9a",
                          ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(
      testARMArch("armv9-a", "generic", "v9a",
                          ARMBuildAttrs::CPUArch::v9_A));
  EXPECT_TRUE(
      testARMArch("armv9.1-a", "generic", "v9.1a",
                          ARMBuildAttrs::CPUArch::v9_A));
  EXPECT_TRUE(
      testARMArch("armv9.2-a", "generic", "v9.2a",
                          ARMBuildAttrs::CPUArch::v9_A));
  EXPECT_TRUE(
      testARMArch("armv9.3-a", "generic", "v9.3a",
                          ARMBuildAttrs::CPUArch::v9_A));
  EXPECT_TRUE(
      testARMArch("armv9.4-a", "generic", "v9.4a",
                          ARMBuildAttrs::CPUArch::v9_A));
  EXPECT_TRUE(
      testARMArch("armv8-r", "cortex-r52", "v8r",
                          ARMBuildAttrs::CPUArch::v8_R));
  EXPECT_TRUE(
      testARMArch("armv8-m.base", "generic", "v8m.base",
                          ARMBuildAttrs::CPUArch::v8_M_Base));
  EXPECT_TRUE(
      testARMArch("armv8-m.main", "generic", "v8m.main",
                          ARMBuildAttrs::CPUArch::v8_M_Main));
  EXPECT_TRUE(
      testARMArch("armv8.1-m.main", "generic", "v8.1m.main",
                          ARMBuildAttrs::CPUArch::v8_1_M_Main));
  EXPECT_TRUE(
      testARMArch("iwmmxt", "iwmmxt", "",
                          ARMBuildAttrs::CPUArch::v5TE));
  EXPECT_TRUE(
      testARMArch("iwmmxt2", "generic", "",
                          ARMBuildAttrs::CPUArch::v5TE));
  EXPECT_TRUE(
      testARMArch("xscale", "xscale", "v5e",
                          ARMBuildAttrs::CPUArch::v5TE));
  EXPECT_TRUE(
      testARMArch("armv7s", "swift", "v7s",
                          ARMBuildAttrs::CPUArch::v7));
  EXPECT_TRUE(
      testARMArch("armv7k", "generic", "v7k",
                          ARMBuildAttrs::CPUArch::v7));
}

bool testARMExtension(StringRef CPUName,ARM::ArchKind ArchKind, StringRef ArchExt) {
  return ARM::getDefaultExtensions(CPUName, ArchKind) &
         ARM::parseArchExt(ArchExt);
}

TEST(TargetParserTest, testARMExtension) {
  EXPECT_FALSE(testARMExtension("strongarm", ARM::ArchKind::INVALID, "dsp"));
  EXPECT_FALSE(testARMExtension("arm7tdmi", ARM::ArchKind::INVALID, "dsp"));
  EXPECT_FALSE(testARMExtension("arm10tdmi",
                                ARM::ArchKind::INVALID, "simd"));
  EXPECT_FALSE(testARMExtension("arm1022e", ARM::ArchKind::INVALID, "simd"));
  EXPECT_FALSE(testARMExtension("arm926ej-s",
                                ARM::ArchKind::INVALID, "simd"));
  EXPECT_FALSE(testARMExtension("arm1136jf-s",
                                ARM::ArchKind::INVALID, "crypto"));
  EXPECT_FALSE(testARMExtension("arm1156t2-s",
                                ARM::ArchKind::INVALID, "crypto"));
  EXPECT_FALSE(testARMExtension("arm1176jzf-s",
                                ARM::ArchKind::INVALID, "crypto"));
  EXPECT_FALSE(testARMExtension("cortex-m0",
                                ARM::ArchKind::INVALID, "crypto"));
  EXPECT_FALSE(testARMExtension("cortex-a8",
                                ARM::ArchKind::INVALID, "crypto"));
  EXPECT_FALSE(testARMExtension("cortex-r4",
                                ARM::ArchKind::INVALID, "crypto"));
  EXPECT_FALSE(testARMExtension("cortex-m3",
                                ARM::ArchKind::INVALID, "crypto"));
  EXPECT_FALSE(testARMExtension("cortex-a53",
                                ARM::ArchKind::INVALID, "ras"));
  EXPECT_FALSE(testARMExtension("cortex-a53",
                                ARM::ArchKind::INVALID, "fp16"));
  EXPECT_TRUE(testARMExtension("cortex-a55",
                                ARM::ArchKind::INVALID, "fp16"));
  EXPECT_FALSE(testARMExtension("cortex-a55",
                                ARM::ArchKind::INVALID, "fp16fml"));
  EXPECT_TRUE(testARMExtension("cortex-a75",
                                ARM::ArchKind::INVALID, "fp16"));
  EXPECT_FALSE(testARMExtension("cortex-a75",
                                ARM::ArchKind::INVALID, "fp16fml"));
  EXPECT_FALSE(testARMExtension("cortex-r52",
                                ARM::ArchKind::INVALID, "ras"));
  EXPECT_FALSE(testARMExtension("iwmmxt", ARM::ArchKind::INVALID, "crc"));
  EXPECT_FALSE(testARMExtension("xscale", ARM::ArchKind::INVALID, "crc"));
  EXPECT_FALSE(testARMExtension("swift", ARM::ArchKind::INVALID, "crc"));

  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV4, "dsp"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV4T, "dsp"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV5T, "simd"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV5TE, "simd"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV5TEJ, "simd"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV6, "crypto"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV6K, "crypto"));
  EXPECT_FALSE(testARMExtension("generic",
                                ARM::ArchKind::ARMV6T2, "crypto"));
  EXPECT_FALSE(testARMExtension("generic",
                                ARM::ArchKind::ARMV6KZ, "crypto"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV6M, "crypto"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV7A, "crypto"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV7R, "crypto"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV7M, "crypto"));
  EXPECT_FALSE(testARMExtension("generic",
                                ARM::ArchKind::ARMV7EM, "crypto"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV8A, "ras"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV8_1A, "ras"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV8_2A, "profile"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV8_2A, "fp16"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV8_2A, "fp16fml"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV8_3A, "fp16"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV8_3A, "fp16fml"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV8_4A, "fp16"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV8_4A, "fp16fml"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV8R, "ras"));
  EXPECT_FALSE(testARMExtension("generic",
                                ARM::ArchKind::ARMV8MBaseline, "crc"));
  EXPECT_FALSE(testARMExtension("generic",
                                ARM::ArchKind::ARMV8MMainline, "crc"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::IWMMXT, "crc"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::IWMMXT2, "crc"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::XSCALE, "crc"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV7S, "crypto"));
  EXPECT_FALSE(testARMExtension("generic", ARM::ArchKind::ARMV7K, "crypto"));
}

TEST(TargetParserTest, ARMFPUVersion) {
  for (ARM::FPUKind FK = static_cast<ARM::FPUKind>(0);
       FK <= ARM::FPUKind::FK_LAST;
       FK = static_cast<ARM::FPUKind>(static_cast<unsigned>(FK) + 1))
    if (FK == ARM::FK_LAST || ARM::getFPUName(FK) == "invalid" ||
        ARM::getFPUName(FK) == "none" || ARM::getFPUName(FK) == "softvfp")
      EXPECT_EQ(ARM::FPUVersion::NONE, ARM::getFPUVersion(FK));
    else
      EXPECT_NE(ARM::FPUVersion::NONE, ARM::getFPUVersion(FK));
}

TEST(TargetParserTest, ARMFPUNeonSupportLevel) {
  for (ARM::FPUKind FK = static_cast<ARM::FPUKind>(0);
       FK <= ARM::FPUKind::FK_LAST;
       FK = static_cast<ARM::FPUKind>(static_cast<unsigned>(FK) + 1))
    if (FK == ARM::FK_LAST ||
        ARM::getFPUName(FK).find("neon") == std::string::npos)
      EXPECT_EQ(ARM::NeonSupportLevel::None,
                 ARM::getFPUNeonSupportLevel(FK));
    else
      EXPECT_NE(ARM::NeonSupportLevel::None,
                ARM::getFPUNeonSupportLevel(FK));
}

TEST(TargetParserTest, ARMFPURestriction) {
  for (ARM::FPUKind FK = static_cast<ARM::FPUKind>(0);
       FK <= ARM::FPUKind::FK_LAST;
       FK = static_cast<ARM::FPUKind>(static_cast<unsigned>(FK) + 1)) {
    if (FK == ARM::FK_LAST ||
        (ARM::getFPUName(FK).find("d16") == std::string::npos &&
         ARM::getFPUName(FK).find("vfpv3xd") == std::string::npos))
      EXPECT_EQ(ARM::FPURestriction::None, ARM::getFPURestriction(FK));
    else
      EXPECT_NE(ARM::FPURestriction::None, ARM::getFPURestriction(FK));
  }
}

TEST(TargetParserTest, ARMExtensionFeatures) {
  std::map<uint64_t, std::vector<StringRef>> Extensions;

  for (auto &Ext : ARM::ARCHExtNames) {
    if (!Ext.Feature.empty() && !Ext.NegFeature.empty())
      Extensions[Ext.ID] = {Ext.Feature, Ext.NegFeature};
  }

  Extensions[ARM::AEK_HWDIVARM]   = { "+hwdiv-arm", "-hwdiv-arm" };
  Extensions[ARM::AEK_HWDIVTHUMB] = { "+hwdiv",     "-hwdiv" };

  std::vector<StringRef> Features;

  EXPECT_FALSE(ARM::getExtensionFeatures(ARM::AEK_INVALID, Features));

  for (auto &E : Extensions) {
    // test +extension
    Features.clear();
    ARM::getExtensionFeatures(E.first, Features);
    EXPECT_TRUE(llvm::is_contained(Features, E.second.at(0)));
    EXPECT_EQ(Extensions.size(), Features.size());

    // test -extension
    Features.clear();
    ARM::getExtensionFeatures(~E.first, Features);
    EXPECT_TRUE(llvm::is_contained(Features, E.second.at(1)));
    EXPECT_EQ(Extensions.size(), Features.size());
  }
}

TEST(TargetParserTest, ARMFPUFeatures) {
  std::vector<StringRef> Features;
  for (ARM::FPUKind FK = static_cast<ARM::FPUKind>(0);
       FK <= ARM::FPUKind::FK_LAST;
       FK = static_cast<ARM::FPUKind>(static_cast<unsigned>(FK) + 1)) {
    if (FK == ARM::FK_INVALID || FK >= ARM::FK_LAST)
      EXPECT_FALSE(ARM::getFPUFeatures(FK, Features));
    else
      EXPECT_TRUE(ARM::getFPUFeatures(FK, Features));
  }
}

TEST(TargetParserTest, ARMArchExtFeature) {
  const char *ArchExt[][4] = {{"crc", "nocrc", "+crc", "-crc"},
                              {"crypto", "nocrypto", "+crypto", "-crypto"},
                              {"dsp", "nodsp", "+dsp", "-dsp"},
                              {"fp", "nofp", nullptr, nullptr},
                              {"idiv", "noidiv", nullptr, nullptr},
                              {"mp", "nomp", nullptr, nullptr},
                              {"simd", "nosimd", nullptr, nullptr},
                              {"sec", "nosec", nullptr, nullptr},
                              {"virt", "novirt", nullptr, nullptr},
                              {"fp16", "nofp16", "+fullfp16", "-fullfp16"},
                              {"fp16fml", "nofp16fml", "+fp16fml", "-fp16fml"},
                              {"ras", "noras", "+ras", "-ras"},
                              {"dotprod", "nodotprod", "+dotprod", "-dotprod"},
                              {"os", "noos", nullptr, nullptr},
                              {"iwmmxt", "noiwmmxt", nullptr, nullptr},
                              {"iwmmxt2", "noiwmmxt2", nullptr, nullptr},
                              {"maverick", "maverick", nullptr, nullptr},
                              {"xscale", "noxscale", nullptr, nullptr},
                              {"sb", "nosb", "+sb", "-sb"},
                              {"i8mm", "noi8mm", "+i8mm", "-i8mm"},
                              {"mve", "nomve", "+mve", "-mve"},
                              {"mve.fp", "nomve.fp", "+mve.fp", "-mve.fp"}};

  for (unsigned i = 0; i < std::size(ArchExt); i++) {
    EXPECT_EQ(StringRef(ArchExt[i][2]), ARM::getArchExtFeature(ArchExt[i][0]));
    EXPECT_EQ(StringRef(ArchExt[i][3]), ARM::getArchExtFeature(ArchExt[i][1]));
  }
}

static bool
testArchExtDependency(const char *ArchExt,
                      const std::initializer_list<const char *> &Expected) {
  std::vector<StringRef> Features;
  ARM::FPUKind FPUKind;

  if (!ARM::appendArchExtFeatures("", ARM::ArchKind::ARMV8_1MMainline, ArchExt,
                                  Features, FPUKind))
    return false;

  return llvm::all_of(Expected, [&](StringRef Ext) {
    return llvm::is_contained(Features, Ext);
  });
}

TEST(TargetParserTest, ARMArchExtDependencies) {
  EXPECT_TRUE(testArchExtDependency("mve", {"+mve", "+dsp"}));
  EXPECT_TRUE(testArchExtDependency("mve.fp", {"+mve.fp", "+mve", "+dsp"}));
  EXPECT_TRUE(testArchExtDependency("nodsp", {"-dsp", "-mve", "-mve.fp"}));
  EXPECT_TRUE(testArchExtDependency("nomve", {"-mve", "-mve.fp"}));
}

TEST(TargetParserTest, ARMparseHWDiv) {
  const char *hwdiv[] = {"thumb", "arm", "arm,thumb", "thumb,arm"};

  for (unsigned i = 0; i < std::size(hwdiv); i++)
    EXPECT_NE(ARM::AEK_INVALID, ARM::parseHWDiv((StringRef)hwdiv[i]));
}

TEST(TargetParserTest, ARMparseArchEndianAndISA) {
  const char *Arch[] = {
      "v2",        "v2a",    "v3",    "v3m",    "v4",       "v4t",
      "v5",        "v5t",    "v5e",   "v5te",   "v5tej",    "v6",
      "v6j",       "v6k",    "v6hl",  "v6t2",   "v6kz",     "v6z",
      "v6zk",      "v6-m",   "v6m",   "v6sm",   "v6s-m",    "v7-a",
      "v7",        "v7a",    "v7ve",  "v7hl",   "v7l",      "v7-r",
      "v7r",       "v7-m",   "v7m",   "v7k",    "v7s",      "v7e-m",
      "v7em",      "v8-a",   "v8",    "v8a",    "v8l",      "v8.1-a",
      "v8.1a",     "v8.2-a", "v8.2a", "v8.3-a", "v8.3a",    "v8.4-a",
      "v8.4a",     "v8.5-a", "v8.5a", "v8.6-a", "v8.6a",    "v8.7-a",
      "v8.7a",     "v8.8-a", "v8.8a", "v8-r",   "v8m.base", "v8m.main",
      "v8.1m.main"};

  for (unsigned i = 0; i < std::size(Arch); i++) {
    std::string arm_1 = "armeb" + (std::string)(Arch[i]);
    std::string arm_2 = "arm" + (std::string)(Arch[i]) + "eb";
    std::string arm_3 = "arm" + (std::string)(Arch[i]);
    std::string thumb_1 = "thumbeb" + (std::string)(Arch[i]);
    std::string thumb_2 = "thumb" + (std::string)(Arch[i]) + "eb";
    std::string thumb_3 = "thumb" + (std::string)(Arch[i]);

    EXPECT_EQ(ARM::EndianKind::BIG, ARM::parseArchEndian(arm_1));
    EXPECT_EQ(ARM::EndianKind::BIG, ARM::parseArchEndian(arm_2));
    EXPECT_EQ(ARM::EndianKind::LITTLE, ARM::parseArchEndian(arm_3));

    EXPECT_EQ(ARM::ISAKind::ARM, ARM::parseArchISA(arm_1));
    EXPECT_EQ(ARM::ISAKind::ARM, ARM::parseArchISA(arm_2));
    EXPECT_EQ(ARM::ISAKind::ARM, ARM::parseArchISA(arm_3));
    if (i >= 4) {
      EXPECT_EQ(ARM::EndianKind::BIG, ARM::parseArchEndian(thumb_1));
      EXPECT_EQ(ARM::EndianKind::BIG, ARM::parseArchEndian(thumb_2));
      EXPECT_EQ(ARM::EndianKind::LITTLE, ARM::parseArchEndian(thumb_3));

      EXPECT_EQ(ARM::ISAKind::THUMB, ARM::parseArchISA(thumb_1));
      EXPECT_EQ(ARM::ISAKind::THUMB, ARM::parseArchISA(thumb_2));
      EXPECT_EQ(ARM::ISAKind::THUMB, ARM::parseArchISA(thumb_3));
    }
  }

  EXPECT_EQ(ARM::EndianKind::LITTLE, ARM::parseArchEndian("aarch64"));
  EXPECT_EQ(ARM::EndianKind::LITTLE, ARM::parseArchEndian("arm64_32"));
  EXPECT_EQ(ARM::EndianKind::BIG, ARM::parseArchEndian("aarch64_be"));

  EXPECT_EQ(ARM::ISAKind::AARCH64, ARM::parseArchISA("aarch64"));
  EXPECT_EQ(ARM::ISAKind::AARCH64, ARM::parseArchISA("aarch64_be"));
  EXPECT_EQ(ARM::ISAKind::AARCH64, ARM::parseArchISA("arm64"));
  EXPECT_EQ(ARM::ISAKind::AARCH64, ARM::parseArchISA("arm64_be"));
  EXPECT_EQ(ARM::ISAKind::AARCH64, ARM::parseArchISA("arm64_32"));
  EXPECT_EQ(ARM::ISAKind::AARCH64, ARM::parseArchISA("aarch64_32"));
}

TEST(TargetParserTest, ARMparseArchProfile) {
  for (unsigned i = 0; i < std::size(ARMArch); i++) {
    switch (ARM::parseArch(ARMArch[i])) {
    case ARM::ArchKind::ARMV6M:
    case ARM::ArchKind::ARMV7M:
    case ARM::ArchKind::ARMV7EM:
    case ARM::ArchKind::ARMV8MMainline:
    case ARM::ArchKind::ARMV8MBaseline:
    case ARM::ArchKind::ARMV8_1MMainline:
      EXPECT_EQ(ARM::ProfileKind::M, ARM::parseArchProfile(ARMArch[i]));
      break;
    case ARM::ArchKind::ARMV7R:
    case ARM::ArchKind::ARMV8R:
      EXPECT_EQ(ARM::ProfileKind::R, ARM::parseArchProfile(ARMArch[i]));
      break;
    case ARM::ArchKind::ARMV7A:
    case ARM::ArchKind::ARMV7VE:
    case ARM::ArchKind::ARMV7K:
    case ARM::ArchKind::ARMV8A:
    case ARM::ArchKind::ARMV8_1A:
    case ARM::ArchKind::ARMV8_2A:
    case ARM::ArchKind::ARMV8_3A:
    case ARM::ArchKind::ARMV8_4A:
    case ARM::ArchKind::ARMV8_5A:
    case ARM::ArchKind::ARMV8_6A:
    case ARM::ArchKind::ARMV8_7A:
    case ARM::ArchKind::ARMV8_8A:
    case ARM::ArchKind::ARMV8_9A:
    case ARM::ArchKind::ARMV9A:
    case ARM::ArchKind::ARMV9_1A:
    case ARM::ArchKind::ARMV9_2A:
    case ARM::ArchKind::ARMV9_3A:
    case ARM::ArchKind::ARMV9_4A:
      EXPECT_EQ(ARM::ProfileKind::A, ARM::parseArchProfile(ARMArch[i]));
      break;
    default:
      EXPECT_EQ(ARM::ProfileKind::INVALID, ARM::parseArchProfile(ARMArch[i]));
      break;
    }
  }
}

TEST(TargetParserTest, ARMparseArchVersion) {
  for (unsigned i = 0; i < std::size(ARMArch); i++)
    if (((std::string)ARMArch[i]).substr(0, 4) == "armv")
      EXPECT_EQ((ARMArch[i][4] - 48u), ARM::parseArchVersion(ARMArch[i]));
    else
      EXPECT_EQ(5u, ARM::parseArchVersion(ARMArch[i]));
}

TEST(TargetParserTest, getARMCPUForArch) {
  // Platform specific defaults.
  {
    llvm::Triple Triple("arm--nacl");
    EXPECT_EQ("cortex-a8", ARM::getARMCPUForArch(Triple));
  }
  {
    llvm::Triple Triple("arm--openbsd");
    EXPECT_EQ("cortex-a8", ARM::getARMCPUForArch(Triple));
  }
  {
    llvm::Triple Triple("armv6-unknown-freebsd");
    EXPECT_EQ("arm1176jzf-s", ARM::getARMCPUForArch(Triple));
  }
  {
    llvm::Triple Triple("thumbv6-unknown-freebsd");
    EXPECT_EQ("arm1176jzf-s", ARM::getARMCPUForArch(Triple));
  }
  {
    llvm::Triple Triple("armebv6-unknown-freebsd");
    EXPECT_EQ("arm1176jzf-s", ARM::getARMCPUForArch(Triple));
  }
  {
    llvm::Triple Triple("arm--win32");
    EXPECT_EQ("cortex-a9", ARM::getARMCPUForArch(Triple));
    EXPECT_EQ("generic", ARM::getARMCPUForArch(Triple, "armv8-a"));
  }
  // Some alternative architectures
  {
    llvm::Triple Triple("armv7k-apple-ios9");
    EXPECT_EQ("cortex-a7", ARM::getARMCPUForArch(Triple));
  }
  {
    llvm::Triple Triple("armv7k-apple-watchos3");
    EXPECT_EQ("cortex-a7", ARM::getARMCPUForArch(Triple));
  }
  {
    llvm::Triple Triple("armv7k-apple-tvos9");
    EXPECT_EQ("cortex-a7", ARM::getARMCPUForArch(Triple));
  }
  // armeb is permitted, but armebeb is not
  {
    llvm::Triple Triple("armeb-none-eabi");
    EXPECT_EQ("arm7tdmi", ARM::getARMCPUForArch(Triple));
  }
  {
    llvm::Triple Triple("armebeb-none-eabi");
    EXPECT_EQ("", ARM::getARMCPUForArch(Triple));
  }
  {
    llvm::Triple Triple("armebv6eb-none-eabi");
    EXPECT_EQ("", ARM::getARMCPUForArch(Triple));
  }
  // xscaleeb is permitted, but armebxscale is not
  {
    llvm::Triple Triple("xscaleeb-none-eabi");
    EXPECT_EQ("xscale", ARM::getARMCPUForArch(Triple));
  }
  {
    llvm::Triple Triple("armebxscale-none-eabi");
    EXPECT_EQ("", ARM::getARMCPUForArch(Triple));
  }
}

class AArch64CPUTestFixture
    : public ::testing::TestWithParam<ARMCPUTestParams> {};

TEST_P(AArch64CPUTestFixture, testAArch64CPU) {
  ARMCPUTestParams params = GetParam();

  const std::optional<AArch64::CpuInfo> Cpu = AArch64::parseCpu(params.CPUName);
  EXPECT_TRUE(Cpu);
  EXPECT_EQ(params.ExpectedArch, Cpu->Arch.Name);

  EXPECT_PRED_FORMAT2(
      AssertSameExtensionFlags<ARM::ISAKind::AARCH64>(params.CPUName),
      params.ExpectedFlags, Cpu->getImpliedExtensions());
}

INSTANTIATE_TEST_SUITE_P(
    AArch64CPUTests, AArch64CPUTestFixture,
    ::testing::Values(
        ARMCPUTestParams("cortex-a34", "armv8-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD,
                         "8-A"),
        ARMCPUTestParams("cortex-a35", "armv8-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD,
                         "8-A"),
        ARMCPUTestParams("cortex-a53", "armv8-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD,
                         "8-A"),
        ARMCPUTestParams("cortex-a55", "armv8.2-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD | AArch64::AEK_RAS |
                             AArch64::AEK_LSE | AArch64::AEK_RDM |
                             AArch64::AEK_FP16 | AArch64::AEK_DOTPROD |
                             AArch64::AEK_RCPC,
                         "8.2-A"),
        ARMCPUTestParams(
            "cortex-a510", "armv9-a", "neon-fp-armv8",
            AArch64::AEK_CRC | AArch64::AEK_FP | AArch64::AEK_SIMD |
                AArch64::AEK_RAS | AArch64::AEK_LSE | AArch64::AEK_RDM |
                AArch64::AEK_RCPC | AArch64::AEK_DOTPROD | AArch64::AEK_BF16 |
                AArch64::AEK_I8MM | AArch64::AEK_SVE | AArch64::AEK_SVE2 |
                AArch64::AEK_SVE2BITPERM | AArch64::AEK_PAUTH |
                AArch64::AEK_MTE | AArch64::AEK_SSBS | AArch64::AEK_FP16 |
                AArch64::AEK_FP16FML | AArch64::AEK_SB,
            "9-A"),
        ARMCPUTestParams("cortex-a57", "armv8-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD,
                         "8-A"),
        ARMCPUTestParams("cortex-a65", "armv8.2-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_DOTPROD |
                             AArch64::AEK_FP | AArch64::AEK_FP16 |
                             AArch64::AEK_LSE | AArch64::AEK_RAS |
                             AArch64::AEK_RCPC | AArch64::AEK_RDM |
                             AArch64::AEK_SIMD | AArch64::AEK_SSBS,
                         "8.2-A"),
        ARMCPUTestParams("cortex-a65ae", "armv8.2-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_DOTPROD |
                             AArch64::AEK_FP | AArch64::AEK_FP16 |
                             AArch64::AEK_LSE | AArch64::AEK_RAS |
                             AArch64::AEK_RCPC | AArch64::AEK_RDM |
                             AArch64::AEK_SIMD | AArch64::AEK_SSBS,
                         "8.2-A"),
        ARMCPUTestParams("cortex-a72", "armv8-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD,
                         "8-A"),
        ARMCPUTestParams("cortex-a73", "armv8-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD,
                         "8-A"),
        ARMCPUTestParams("cortex-a75", "armv8.2-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD | AArch64::AEK_RAS |
                             AArch64::AEK_LSE | AArch64::AEK_RDM |
                             AArch64::AEK_FP16 | AArch64::AEK_DOTPROD |
                             AArch64::AEK_RCPC,
                         "8.2-A"),
        ARMCPUTestParams("cortex-a76", "armv8.2-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_RDM | AArch64::AEK_SIMD |
                             AArch64::AEK_RAS | AArch64::AEK_LSE |
                             AArch64::AEK_FP16 | AArch64::AEK_DOTPROD |
                             AArch64::AEK_RCPC | AArch64::AEK_SSBS,
                         "8.2-A"),
        ARMCPUTestParams("cortex-a76ae", "armv8.2-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_RDM | AArch64::AEK_SIMD |
                             AArch64::AEK_RAS | AArch64::AEK_LSE |
                             AArch64::AEK_FP16 | AArch64::AEK_DOTPROD |
                             AArch64::AEK_RCPC | AArch64::AEK_SSBS,
                         "8.2-A"),
        ARMCPUTestParams("cortex-a77", "armv8.2-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_RDM | AArch64::AEK_SIMD |
                             AArch64::AEK_RAS | AArch64::AEK_LSE |
                             AArch64::AEK_FP16 | AArch64::AEK_DOTPROD |
                             AArch64::AEK_RCPC | AArch64::AEK_SSBS,
                         "8.2-A"),
        ARMCPUTestParams("cortex-a78", "armv8.2-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_RDM | AArch64::AEK_SIMD |
                             AArch64::AEK_RAS | AArch64::AEK_LSE |
                             AArch64::AEK_FP16 | AArch64::AEK_DOTPROD |
                             AArch64::AEK_RCPC | AArch64::AEK_SSBS |
                             AArch64::AEK_PROFILE,
                         "8.2-A"),
        ARMCPUTestParams(
            "cortex-a78c", "armv8.2-a", "crypto-neon-fp-armv8",
            AArch64::AEK_RAS | AArch64::AEK_CRC | AArch64::AEK_AES |
                AArch64::AEK_SHA2 | AArch64::AEK_FP | AArch64::AEK_SIMD |
                AArch64::AEK_RAS | AArch64::AEK_LSE | AArch64::AEK_RDM |
                AArch64::AEK_FP16 | AArch64::AEK_DOTPROD | AArch64::AEK_RCPC |
                AArch64::AEK_SSBS | AArch64::AEK_PROFILE | AArch64::AEK_FLAGM |
                AArch64::AEK_PAUTH | AArch64::AEK_FP16FML,
            "8.2-A"),
        ARMCPUTestParams(
            "cortex-a710", "armv9-a", "neon-fp-armv8",
            AArch64::AEK_CRC | AArch64::AEK_FP | AArch64::AEK_SIMD |
                AArch64::AEK_RAS | AArch64::AEK_LSE | AArch64::AEK_RDM |
                AArch64::AEK_RCPC | AArch64::AEK_DOTPROD | AArch64::AEK_MTE |
                AArch64::AEK_FP16 | AArch64::AEK_FP16FML | AArch64::AEK_SVE |
                AArch64::AEK_SVE2 | AArch64::AEK_SVE2BITPERM |
                AArch64::AEK_PAUTH | AArch64::AEK_FLAGM | AArch64::AEK_SB |
                AArch64::AEK_I8MM | AArch64::AEK_BF16,
            "9-A"),
        ARMCPUTestParams(
            "cortex-a715", "armv9-a", "neon-fp-armv8",
            AArch64::AEK_CRC | AArch64::AEK_FP | AArch64::AEK_BF16 |
                AArch64::AEK_SIMD | AArch64::AEK_RAS | AArch64::AEK_LSE |
                AArch64::AEK_RDM | AArch64::AEK_RCPC | AArch64::AEK_DOTPROD |
                AArch64::AEK_MTE | AArch64::AEK_PAUTH | AArch64::AEK_SVE |
                AArch64::AEK_SVE2 | AArch64::AEK_SVE2BITPERM |
                AArch64::AEK_SSBS | AArch64::AEK_SB | AArch64::AEK_I8MM |
                AArch64::AEK_PERFMON | AArch64::AEK_PREDRES |
                AArch64::AEK_PROFILE | AArch64::AEK_FP16FML |
                AArch64::AEK_FP16 | AArch64::AEK_FLAGM,
            "9-A"),
        ARMCPUTestParams(
            "neoverse-v1", "armv8.4-a", "crypto-neon-fp-armv8",
            AArch64::AEK_RAS | AArch64::AEK_SVE | AArch64::AEK_SSBS |
                AArch64::AEK_RCPC | AArch64::AEK_CRC | AArch64::AEK_FP |
                AArch64::AEK_SIMD | AArch64::AEK_RAS | AArch64::AEK_LSE |
                AArch64::AEK_RDM | AArch64::AEK_RCPC | AArch64::AEK_DOTPROD |
                AArch64::AEK_AES | AArch64::AEK_SHA2 | AArch64::AEK_SHA3 |
                AArch64::AEK_SM4 | AArch64::AEK_FP16 | AArch64::AEK_BF16 |
                AArch64::AEK_PROFILE | AArch64::AEK_RAND |
                AArch64::AEK_FP16FML | AArch64::AEK_I8MM,
            "8.4-A"),
        ARMCPUTestParams("neoverse-v2", "armv9-a", "neon-fp-armv8",
                         AArch64::AEK_RAS | AArch64::AEK_SVE |
                             AArch64::AEK_SSBS | AArch64::AEK_RCPC |
                             AArch64::AEK_CRC | AArch64::AEK_FP |
                             AArch64::AEK_SIMD | AArch64::AEK_MTE |
                             AArch64::AEK_LSE | AArch64::AEK_RDM |
                             AArch64::AEK_RCPC | AArch64::AEK_DOTPROD |
                             AArch64::AEK_FP16 | AArch64::AEK_BF16 |
                             AArch64::AEK_SVE2 | AArch64::AEK_PROFILE |
                             AArch64::AEK_FP16FML | AArch64::AEK_I8MM |
                             AArch64::AEK_SVE2BITPERM | AArch64::AEK_RAND,
                         "9-A"),
        ARMCPUTestParams("cortex-r82", "armv8-r", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_RDM |
                             AArch64::AEK_SSBS | AArch64::AEK_DOTPROD |
                             AArch64::AEK_FP | AArch64::AEK_SIMD |
                             AArch64::AEK_FP16 | AArch64::AEK_FP16FML |
                             AArch64::AEK_RAS | AArch64::AEK_RCPC |
                             AArch64::AEK_LSE | AArch64::AEK_SB,
                         "8-R"),
        ARMCPUTestParams("cortex-x1", "armv8.2-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_RDM | AArch64::AEK_SIMD |
                             AArch64::AEK_RAS | AArch64::AEK_LSE |
                             AArch64::AEK_FP16 | AArch64::AEK_DOTPROD |
                             AArch64::AEK_RCPC | AArch64::AEK_SSBS |
                             AArch64::AEK_PROFILE,
                         "8.2-A"),
        ARMCPUTestParams("cortex-x1c", "armv8.2-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_RDM | AArch64::AEK_SIMD |
                             AArch64::AEK_RAS | AArch64::AEK_LSE |
                             AArch64::AEK_FP16 | AArch64::AEK_DOTPROD |
                             AArch64::AEK_RCPC | AArch64::AEK_SSBS |
                             AArch64::AEK_PAUTH | AArch64::AEK_PROFILE,
                         "8.2-A"),
        ARMCPUTestParams("cortex-x2", "armv9-a", "neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_FP |
                             AArch64::AEK_SIMD | AArch64::AEK_RAS |
                             AArch64::AEK_LSE | AArch64::AEK_RDM |
                             AArch64::AEK_RCPC | AArch64::AEK_DOTPROD |
                             AArch64::AEK_MTE | AArch64::AEK_PAUTH |
                             AArch64::AEK_I8MM | AArch64::AEK_BF16 |
                             AArch64::AEK_SVE | AArch64::AEK_SVE2 |
                             AArch64::AEK_SVE2BITPERM | AArch64::AEK_SSBS |
                             AArch64::AEK_SB | AArch64::AEK_FP16 |
                             AArch64::AEK_FP16FML,
                         "9-A"),
        ARMCPUTestParams(
            "cortex-x3",
            "armv9-a", "neon-fp-armv8",
            AArch64::AEK_CRC | AArch64::AEK_FP | AArch64::AEK_BF16 |
                AArch64::AEK_SIMD | AArch64::AEK_RAS | AArch64::AEK_LSE |
                AArch64::AEK_RDM | AArch64::AEK_RCPC | AArch64::AEK_DOTPROD |
                AArch64::AEK_MTE | AArch64::AEK_PAUTH | AArch64::AEK_SVE |
                AArch64::AEK_SVE2 | AArch64::AEK_SVE2BITPERM | AArch64::AEK_SB |
                AArch64::AEK_PROFILE | AArch64::AEK_PERFMON |
                AArch64::AEK_I8MM | AArch64::AEK_FP16 | AArch64::AEK_FP16FML |
                AArch64::AEK_PREDRES | AArch64::AEK_FLAGM | AArch64::AEK_SSBS,
            "9-A"),
        ARMCPUTestParams("cyclone", "armv8-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_NONE | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD,
                         "8-A"),
        ARMCPUTestParams("apple-a7", "armv8-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_NONE | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD,
                         "8-A"),
        ARMCPUTestParams("apple-a8", "armv8-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_NONE | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD,
                         "8-A"),
        ARMCPUTestParams("apple-a9", "armv8-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_NONE | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD,
                         "8-A"),
        ARMCPUTestParams("apple-a10", "armv8-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_RDM | AArch64::AEK_SIMD,
                         "8-A"),
        ARMCPUTestParams("apple-a11", "armv8.2-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_LSE | AArch64::AEK_RAS |
                             AArch64::AEK_RDM | AArch64::AEK_SIMD |
                             AArch64::AEK_FP16,
                         "8.2-A"),
        ARMCPUTestParams("apple-a12", "armv8.3-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD | AArch64::AEK_LSE |
                             AArch64::AEK_RAS | AArch64::AEK_RDM |
                             AArch64::AEK_RCPC | AArch64::AEK_FP16,
                         "8.3-A"),
        ARMCPUTestParams("apple-a13", "armv8.4-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_SHA3 |
                             AArch64::AEK_FP | AArch64::AEK_SIMD |
                             AArch64::AEK_LSE | AArch64::AEK_RAS |
                             AArch64::AEK_RDM | AArch64::AEK_RCPC |
                             AArch64::AEK_DOTPROD | AArch64::AEK_FP16 |
                             AArch64::AEK_FP16FML | AArch64::AEK_SHA3,
                         "8.4-A"),
        ARMCPUTestParams("apple-a14", "armv8.5-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_SHA3 |
                             AArch64::AEK_FP | AArch64::AEK_SIMD |
                             AArch64::AEK_LSE | AArch64::AEK_RAS |
                             AArch64::AEK_RDM | AArch64::AEK_RCPC |
                             AArch64::AEK_DOTPROD | AArch64::AEK_FP16 |
                             AArch64::AEK_FP16FML | AArch64::AEK_SHA3,
                         "8.5-A"),
        ARMCPUTestParams("apple-a15", "armv8.5-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_SHA3 |
                             AArch64::AEK_FP | AArch64::AEK_SIMD |
                             AArch64::AEK_LSE | AArch64::AEK_RAS |
                             AArch64::AEK_RDM | AArch64::AEK_RCPC |
                             AArch64::AEK_DOTPROD | AArch64::AEK_FP16 |
                             AArch64::AEK_FP16FML | AArch64::AEK_SHA3 |
                             AArch64::AEK_BF16 | AArch64::AEK_I8MM,
                         "8.5-A"),
        ARMCPUTestParams("apple-a16", "armv8.5-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_SHA3 |
                             AArch64::AEK_FP | AArch64::AEK_SIMD |
                             AArch64::AEK_LSE | AArch64::AEK_RAS |
                             AArch64::AEK_RDM | AArch64::AEK_RCPC |
                             AArch64::AEK_DOTPROD | AArch64::AEK_FP16 |
                             AArch64::AEK_FP16FML | AArch64::AEK_SHA3 |
                             AArch64::AEK_BF16 | AArch64::AEK_I8MM,
                         "8.5-A"),
        ARMCPUTestParams("apple-m1", "armv8.5-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_SHA3 |
                             AArch64::AEK_FP | AArch64::AEK_SIMD |
                             AArch64::AEK_LSE | AArch64::AEK_RAS |
                             AArch64::AEK_RDM | AArch64::AEK_RCPC |
                             AArch64::AEK_DOTPROD | AArch64::AEK_FP16 |
                             AArch64::AEK_FP16FML | AArch64::AEK_SHA3,
                         "8.5-A"),
        ARMCPUTestParams("apple-m2", "armv8.5-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_SHA3 |
                             AArch64::AEK_FP | AArch64::AEK_SIMD |
                             AArch64::AEK_LSE | AArch64::AEK_RAS |
                             AArch64::AEK_RDM | AArch64::AEK_RCPC |
                             AArch64::AEK_DOTPROD | AArch64::AEK_FP16 |
                             AArch64::AEK_FP16FML | AArch64::AEK_SHA3 |
                             AArch64::AEK_BF16 | AArch64::AEK_I8MM,
                         "8.5-A"),
        ARMCPUTestParams("apple-s4", "armv8.3-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD | AArch64::AEK_LSE |
                             AArch64::AEK_RAS | AArch64::AEK_RDM |
                             AArch64::AEK_RCPC | AArch64::AEK_FP16,
                         "8.3-A"),
        ARMCPUTestParams("apple-s5", "armv8.3-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD | AArch64::AEK_LSE |
                             AArch64::AEK_RAS | AArch64::AEK_RDM |
                             AArch64::AEK_RCPC | AArch64::AEK_FP16,
                         "8.3-A"),
        ARMCPUTestParams("exynos-m3", "armv8-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD,
                         "8-A"),
        ARMCPUTestParams("exynos-m4", "armv8.2-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_DOTPROD |
                             AArch64::AEK_FP | AArch64::AEK_FP16 |
                             AArch64::AEK_LSE | AArch64::AEK_RAS |
                             AArch64::AEK_RDM | AArch64::AEK_SIMD,
                         "8.2-A"),
        ARMCPUTestParams("exynos-m5", "armv8.2-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_DOTPROD |
                             AArch64::AEK_FP | AArch64::AEK_FP16 |
                             AArch64::AEK_LSE | AArch64::AEK_RAS |
                             AArch64::AEK_RDM | AArch64::AEK_SIMD,
                         "8.2-A"),
        ARMCPUTestParams("falkor", "armv8-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD | AArch64::AEK_RDM,
                         "8-A"),
        ARMCPUTestParams("kryo", "armv8-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD,
                         "8-A"),
        ARMCPUTestParams("neoverse-e1", "armv8.2-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_DOTPROD |
                             AArch64::AEK_FP | AArch64::AEK_FP16 |
                             AArch64::AEK_LSE | AArch64::AEK_RAS |
                             AArch64::AEK_RCPC | AArch64::AEK_RDM |
                             AArch64::AEK_SIMD | AArch64::AEK_SSBS,
                         "8.2-A"),
        ARMCPUTestParams("neoverse-n1", "armv8.2-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_DOTPROD |
                             AArch64::AEK_FP | AArch64::AEK_FP16 |
                             AArch64::AEK_LSE | AArch64::AEK_PROFILE |
                             AArch64::AEK_RAS | AArch64::AEK_RCPC |
                             AArch64::AEK_RDM | AArch64::AEK_SIMD |
                             AArch64::AEK_SSBS,
                         "8.2-A"),
        ARMCPUTestParams(
            "neoverse-n2", "armv8.5-a", "crypto-neon-fp-armv8",
            AArch64::AEK_CRC | AArch64::AEK_AES | AArch64::AEK_SHA2 |
                AArch64::AEK_SHA3 | AArch64::AEK_SM4 | AArch64::AEK_FP |
                AArch64::AEK_SIMD | AArch64::AEK_FP16 | AArch64::AEK_RAS |
                AArch64::AEK_LSE | AArch64::AEK_SVE | AArch64::AEK_DOTPROD |
                AArch64::AEK_RCPC | AArch64::AEK_RDM | AArch64::AEK_MTE |
                AArch64::AEK_SSBS | AArch64::AEK_SB | AArch64::AEK_SVE2 |
                AArch64::AEK_SVE2BITPERM | AArch64::AEK_BF16 |
                AArch64::AEK_I8MM,
            "8.5-A"),
        ARMCPUTestParams(
            "ampere1", "armv8.6-a", "crypto-neon-fp-armv8",
            AArch64::AEK_CRC | AArch64::AEK_FP | AArch64::AEK_FP16 |
                AArch64::AEK_SIMD | AArch64::AEK_RAS | AArch64::AEK_LSE |
                AArch64::AEK_RDM | AArch64::AEK_RCPC | AArch64::AEK_DOTPROD |
                AArch64::AEK_SHA3 | AArch64::AEK_BF16 | AArch64::AEK_SHA2 |
                AArch64::AEK_AES | AArch64::AEK_I8MM | AArch64::AEK_SSBS |
                AArch64::AEK_SB | AArch64::AEK_RAND,
            "8.6-A"),
        ARMCPUTestParams(
            "ampere1a", "armv8.6-a", "crypto-neon-fp-armv8",
            AArch64::AEK_CRC | AArch64::AEK_FP | AArch64::AEK_FP16 |
                AArch64::AEK_SIMD | AArch64::AEK_RAS | AArch64::AEK_LSE |
                AArch64::AEK_RDM | AArch64::AEK_RCPC | AArch64::AEK_DOTPROD |
                AArch64::AEK_SM4 | AArch64::AEK_SHA3 | AArch64::AEK_BF16 |
                AArch64::AEK_SHA2 | AArch64::AEK_AES | AArch64::AEK_I8MM |
                AArch64::AEK_SSBS | AArch64::AEK_SB | AArch64::AEK_RAND |
                AArch64::AEK_MTE,
            "8.6-A"),
        ARMCPUTestParams(
            "neoverse-512tvb", "armv8.4-a", "crypto-neon-fp-armv8",
            AArch64::AEK_RAS | AArch64::AEK_SVE | AArch64::AEK_SSBS |
                AArch64::AEK_RCPC | AArch64::AEK_CRC | AArch64::AEK_FP |
                AArch64::AEK_SIMD | AArch64::AEK_RAS | AArch64::AEK_LSE |
                AArch64::AEK_RDM | AArch64::AEK_RCPC | AArch64::AEK_DOTPROD |
                AArch64::AEK_AES | AArch64::AEK_SHA2 | AArch64::AEK_SHA3 |
                AArch64::AEK_SM4 | AArch64::AEK_FP16 | AArch64::AEK_BF16 |
                AArch64::AEK_PROFILE | AArch64::AEK_RAND |
                AArch64::AEK_FP16FML | AArch64::AEK_I8MM,
            "8.4-A"),
        ARMCPUTestParams("thunderx2t99", "armv8.1-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_LSE |
                             AArch64::AEK_RDM | AArch64::AEK_FP |
                             AArch64::AEK_SIMD,
                         "8.1-A"),
        ARMCPUTestParams("thunderx3t110", "armv8.3-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_LSE |
                             AArch64::AEK_RDM | AArch64::AEK_FP |
                             AArch64::AEK_SIMD | AArch64::AEK_RAS |
                             AArch64::AEK_RCPC,
                         "8.3-A"),
        ARMCPUTestParams("thunderx", "armv8-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_SIMD |
                             AArch64::AEK_FP,
                         "8-A"),
        ARMCPUTestParams("thunderxt81", "armv8-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_SIMD |
                             AArch64::AEK_FP,
                         "8-A"),
        ARMCPUTestParams("thunderxt83", "armv8-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_SIMD |
                             AArch64::AEK_FP,
                         "8-A"),
        ARMCPUTestParams("thunderxt88", "armv8-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_SIMD |
                             AArch64::AEK_FP,
                         "8-A"),
        ARMCPUTestParams("tsv110", "armv8.2-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD | AArch64::AEK_RAS |
                             AArch64::AEK_LSE | AArch64::AEK_RDM |
                             AArch64::AEK_PROFILE | AArch64::AEK_FP16 |
                             AArch64::AEK_FP16FML | AArch64::AEK_DOTPROD,
                         "8.2-A"),
        ARMCPUTestParams(
            "hip09", "armv8.5-a", "crypto-neon-fp-armv8",
            AArch64::AEK_CRC | AArch64::AEK_FP | AArch64::AEK_SIMD |
                AArch64::AEK_RAS | AArch64::AEK_LSE | AArch64::AEK_RDM |
                AArch64::AEK_RCPC | AArch64::AEK_DOTPROD | AArch64::AEK_AES |
                AArch64::AEK_SM4 | AArch64::AEK_SHA2 | AArch64::AEK_SHA3 |
                AArch64::AEK_FP16 | AArch64::AEK_PROFILE |
                AArch64::AEK_FP16FML | AArch64::AEK_SVE | AArch64::AEK_I8MM |
                AArch64::AEK_F32MM | AArch64::AEK_F64MM | AArch64::AEK_BF16,
            "8.5-A"),
        ARMCPUTestParams(
            "hip10c", "armv8.5-a", "crypto-neon-fp-armv8",
            AArch64::AEK_F32MM | AArch64::AEK_F64MM | AArch64::AEK_FLAGM |
                AArch64::AEK_FP16 | AArch64::AEK_FP16FML | AArch64::AEK_MTE |
                AArch64::AEK_PAUTH | AArch64::AEK_PERFMON | AArch64::AEK_PROFILE |
                AArch64::AEK_RAND | AArch64::AEK_SB | AArch64::AEK_SHA2 |
                AArch64::AEK_SHA3 | AArch64::AEK_SM4 | AArch64::AEK_SSBS |
                AArch64::AEK_SVE | AArch64::AEK_BF16 | AArch64::AEK_CRC |
                AArch64::AEK_DOTPROD | AArch64::AEK_FP | AArch64::AEK_I8MM |
                AArch64::AEK_LSE | AArch64::AEK_RAS | AArch64::AEK_RCPC |
                AArch64::AEK_RDM | AArch64::AEK_SIMD,
            "8.5-A"),
        ARMCPUTestParams(
            "hip11", "armv9-a", "neon-fp-armv8",
            AArch64::AEK_CRC | AArch64::AEK_FP | AArch64::AEK_SIMD |
                AArch64::AEK_RAS | AArch64::AEK_LSE | AArch64::AEK_RDM |
                AArch64::AEK_RCPC | AArch64::AEK_SVE | AArch64::AEK_SVE2 |
                AArch64::AEK_DOTPROD | AArch64::AEK_MTE | AArch64::AEK_FP16FML |
                AArch64::AEK_FP16 | AArch64::AEK_SVE2BITPERM |
                AArch64::AEK_PAUTH | AArch64::AEK_FLAGM | AArch64::AEK_SB |
                AArch64::AEK_I8MM | AArch64::AEK_BF16 | AArch64::AEK_SME |
                AArch64::AEK_SMEF64F64 | AArch64::AEK_AES | AArch64::AEK_SM4 |
                AArch64::AEK_SHA2 | AArch64::AEK_SHA3 | AArch64::AEK_SMEFA64,
            "9-A"),
        ARMCPUTestParams(
            "hip12", "armv9.2-a", "crypto-neon-fp-armv8",
            AArch64::AEK_SVE | AArch64::AEK_SVE2 | AArch64::AEK_SVE2BITPERM |
                AArch64::AEK_SVE2AES | AArch64::AEK_SVE2SM4 | AArch64::AEK_SVE2SHA3 |
                AArch64::AEK_FP16 | AArch64::AEK_PERFMON | AArch64::AEK_PROFILE |
                AArch64::AEK_HBC | AArch64::AEK_RCPC3 | AArch64::AEK_BF16 |
                AArch64::AEK_CRC | AArch64::AEK_DOTPROD | AArch64::AEK_FP |
                AArch64::AEK_I8MM | AArch64::AEK_LSE | AArch64::AEK_SIMD |
                AArch64::AEK_PAUTH | AArch64::AEK_RAS | AArch64::AEK_RCPC |
                AArch64::AEK_RDM | AArch64::AEK_LS64 | AArch64::AEK_BRBE |
                AArch64::AEK_FPAC,
            "9.2-A"),
        ARMCPUTestParams("a64fx", "armv8.2-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD | AArch64::AEK_FP16 |
                             AArch64::AEK_RAS | AArch64::AEK_LSE |
                             AArch64::AEK_SVE | AArch64::AEK_RDM,
                         "8.2-A"),
        ARMCPUTestParams("carmel", "armv8.2-a", "crypto-neon-fp-armv8",
                         AArch64::AEK_CRC | AArch64::AEK_AES |
                             AArch64::AEK_SHA2 | AArch64::AEK_FP |
                             AArch64::AEK_SIMD | AArch64::AEK_FP16 |
                             AArch64::AEK_RAS | AArch64::AEK_LSE |
                             AArch64::AEK_RDM,
                         "8.2-A")));

// Note: number of CPUs includes aliases.
static constexpr unsigned NumAArch64CPUArchs = 66;

TEST(TargetParserTest, testAArch64CPUArchList) {
  SmallVector<StringRef, NumAArch64CPUArchs> List;
  AArch64::fillValidCPUArchList(List);

  // No list exists for these in this test suite, so ensure all are
  // valid, and match the expected 'magic' count.
  EXPECT_EQ(List.size(), NumAArch64CPUArchs);
  for(StringRef CPU : List) {
    EXPECT_TRUE(AArch64::parseCpu(CPU));
  }
}

bool testAArch64Arch(StringRef Arch, StringRef DefaultCPU, StringRef SubArch,
                     unsigned ArchAttr) {
  const std::optional<AArch64::ArchInfo> AI = AArch64::parseArch(Arch);
  return AI.has_value();
}

TEST(TargetParserTest, testAArch64Arch) {
  EXPECT_TRUE(testAArch64Arch("armv8-a", "cortex-a53", "v8a",
                              ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(testAArch64Arch("armv8.1-a", "generic", "v8.1a",
                              ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(testAArch64Arch("armv8.2-a", "generic", "v8.2a",
                              ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(testAArch64Arch("armv8.3-a", "generic", "v8.3a",
                              ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(testAArch64Arch("armv8.4-a", "generic", "v8.4a",
                              ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(testAArch64Arch("armv8.5-a", "generic", "v8.5a",
                              ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(testAArch64Arch("armv8.6-a", "generic", "v8.6a",
                              ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(testAArch64Arch("armv8.7-a", "generic", "v8.7a",
                              ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(testAArch64Arch("armv8.8-a", "generic", "v8.8a",
                              ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(testAArch64Arch("armv8.9-a", "generic", "v8.9a",
                              ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(testAArch64Arch("armv9-a", "generic", "v9a",
                              ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(testAArch64Arch("armv9.1-a", "generic", "v9.1a",
                              ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(testAArch64Arch("armv9.2-a", "generic", "v9.2a",
                              ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(testAArch64Arch("armv9.3-a", "generic", "v9.3a",
                              ARMBuildAttrs::CPUArch::v8_A));
  EXPECT_TRUE(testAArch64Arch("armv9.4-a", "generic", "v9.4a",
                              ARMBuildAttrs::CPUArch::v8_A));
}

bool testAArch64Extension(StringRef CPUName, StringRef ArchExt) {
  std::optional<AArch64::ExtensionInfo> Extension =
      AArch64::parseArchExtension(ArchExt);
  if (!Extension)
    return false;
  std::optional<AArch64::CpuInfo> CpuInfo = AArch64::parseCpu(CPUName);
  return CpuInfo->getImpliedExtensions() & Extension->ID;
}

bool testAArch64Extension(const AArch64::ArchInfo &AI, StringRef ArchExt) {
  std::optional<AArch64::ExtensionInfo> Extension =
      AArch64::parseArchExtension(ArchExt);
  if (!Extension)
    return false;
  return AI.DefaultExts & Extension->ID;
}

TEST(TargetParserTest, testAArch64Extension) {
  EXPECT_FALSE(testAArch64Extension("cortex-a34", "ras"));
  EXPECT_FALSE(testAArch64Extension("cortex-a35", "ras"));
  EXPECT_FALSE(testAArch64Extension("cortex-a53", "ras"));
  EXPECT_TRUE(testAArch64Extension("cortex-a55", "ras"));
  EXPECT_TRUE(testAArch64Extension("cortex-a55", "fp16"));
  EXPECT_FALSE(testAArch64Extension("cortex-a55", "fp16fml"));
  EXPECT_TRUE(testAArch64Extension("cortex-a55", "dotprod"));
  EXPECT_FALSE(testAArch64Extension("cortex-a57", "ras"));
  EXPECT_FALSE(testAArch64Extension("cortex-a72", "ras"));
  EXPECT_FALSE(testAArch64Extension("cortex-a73", "ras"));
  EXPECT_TRUE(testAArch64Extension("cortex-a75", "ras"));
  EXPECT_TRUE(testAArch64Extension("cortex-a75", "fp16"));
  EXPECT_FALSE(testAArch64Extension("cortex-a75", "fp16fml"));
  EXPECT_TRUE(testAArch64Extension("cortex-a75", "dotprod"));
  EXPECT_TRUE(testAArch64Extension("cortex-r82", "ras"));
  EXPECT_TRUE(testAArch64Extension("cortex-r82", "fp16"));
  EXPECT_TRUE(testAArch64Extension("cortex-r82", "fp16fml"));
  EXPECT_TRUE(testAArch64Extension("cortex-r82", "dotprod"));
  EXPECT_TRUE(testAArch64Extension("cortex-r82", "lse"));
  EXPECT_FALSE(testAArch64Extension("cyclone", "ras"));
  EXPECT_FALSE(testAArch64Extension("exynos-m3", "ras"));
  EXPECT_TRUE(testAArch64Extension("exynos-m4", "dotprod"));
  EXPECT_TRUE(testAArch64Extension("exynos-m4", "fp16"));
  EXPECT_TRUE(testAArch64Extension("exynos-m4", "lse"));
  EXPECT_TRUE(testAArch64Extension("exynos-m4", "ras"));
  EXPECT_TRUE(testAArch64Extension("exynos-m4", "rdm"));
  EXPECT_TRUE(testAArch64Extension("exynos-m5", "dotprod"));
  EXPECT_TRUE(testAArch64Extension("exynos-m5", "fp16"));
  EXPECT_TRUE(testAArch64Extension("exynos-m5", "lse"));
  EXPECT_TRUE(testAArch64Extension("exynos-m5", "ras"));
  EXPECT_TRUE(testAArch64Extension("exynos-m5", "rdm"));
  EXPECT_TRUE(testAArch64Extension("falkor", "rdm"));
  EXPECT_FALSE(testAArch64Extension("kryo", "ras"));
  EXPECT_TRUE(testAArch64Extension("saphira", "crc"));
  EXPECT_TRUE(testAArch64Extension("saphira", "lse"));
  EXPECT_TRUE(testAArch64Extension("saphira", "rdm"));
  EXPECT_TRUE(testAArch64Extension("saphira", "ras"));
  EXPECT_TRUE(testAArch64Extension("saphira", "rcpc"));
  EXPECT_TRUE(testAArch64Extension("saphira", "profile"));
  EXPECT_FALSE(testAArch64Extension("saphira", "fp16"));
  EXPECT_FALSE(testAArch64Extension("thunderx2t99", "ras"));
  EXPECT_FALSE(testAArch64Extension("thunderx", "lse"));
  EXPECT_FALSE(testAArch64Extension("thunderxt81", "lse"));
  EXPECT_FALSE(testAArch64Extension("thunderxt83", "lse"));
  EXPECT_FALSE(testAArch64Extension("thunderxt88", "lse"));
  EXPECT_TRUE(testAArch64Extension("tsv110", "aes"));
  EXPECT_TRUE(testAArch64Extension("tsv110", "sha2"));
  EXPECT_FALSE(testAArch64Extension("tsv110", "sha3"));
  EXPECT_FALSE(testAArch64Extension("tsv110", "sm4"));
  EXPECT_TRUE(testAArch64Extension("tsv110", "ras"));
  EXPECT_TRUE(testAArch64Extension("tsv110", "profile"));
  EXPECT_TRUE(testAArch64Extension("tsv110", "fp16"));
  EXPECT_TRUE(testAArch64Extension("tsv110", "fp16fml"));
  EXPECT_TRUE(testAArch64Extension("tsv110", "dotprod"));
  EXPECT_TRUE(testAArch64Extension("a64fx", "fp16"));
  EXPECT_TRUE(testAArch64Extension("a64fx", "sve"));
  EXPECT_FALSE(testAArch64Extension("a64fx", "sve2"));
  EXPECT_TRUE(testAArch64Extension("carmel", "aes"));
  EXPECT_TRUE(testAArch64Extension("carmel", "sha2"));
  EXPECT_TRUE(testAArch64Extension("carmel", "fp16"));

  EXPECT_FALSE(testAArch64Extension(AArch64::ARMV8A, "ras"));
  EXPECT_FALSE(testAArch64Extension(AArch64::ARMV8_1A, "ras"));
  EXPECT_FALSE(testAArch64Extension(AArch64::ARMV8_2A, "profile"));
  EXPECT_FALSE(testAArch64Extension(AArch64::ARMV8_2A, "fp16"));
  EXPECT_FALSE(testAArch64Extension(AArch64::ARMV8_2A, "fp16fml"));
  EXPECT_FALSE(testAArch64Extension(AArch64::ARMV8_3A, "fp16"));
  EXPECT_FALSE(testAArch64Extension(AArch64::ARMV8_3A, "fp16fml"));
  EXPECT_FALSE(testAArch64Extension(AArch64::ARMV8_4A, "fp16"));
  EXPECT_FALSE(testAArch64Extension(AArch64::ARMV8_4A, "fp16fml"));
}

TEST(TargetParserTest, AArch64ExtensionFeatures) {
  std::vector<uint64_t> Extensions = {
      AArch64::AEK_CRC,     AArch64::AEK_LSE,       AArch64::AEK_RDM,
      AArch64::AEK_CRYPTO,  AArch64::AEK_SM4,       AArch64::AEK_SHA3,
      AArch64::AEK_SHA2,    AArch64::AEK_AES,       AArch64::AEK_DOTPROD,
      AArch64::AEK_FP,      AArch64::AEK_SIMD,      AArch64::AEK_FP16,
      AArch64::AEK_FP16FML, AArch64::AEK_PROFILE,   AArch64::AEK_RAS,
      AArch64::AEK_SVE,     AArch64::AEK_SVE2,      AArch64::AEK_SVE2AES,
      AArch64::AEK_SVE2SM4, AArch64::AEK_SVE2SHA3,  AArch64::AEK_SVE2BITPERM,
      AArch64::AEK_RCPC,    AArch64::AEK_RAND,      AArch64::AEK_MTE,
      AArch64::AEK_SSBS,    AArch64::AEK_SB,        AArch64::AEK_PREDRES,
      AArch64::AEK_BF16,    AArch64::AEK_I8MM,      AArch64::AEK_F32MM,
      AArch64::AEK_F64MM,   AArch64::AEK_TME,       AArch64::AEK_LS64,
      AArch64::AEK_BRBE,    AArch64::AEK_PAUTH,     AArch64::AEK_FLAGM,
      AArch64::AEK_SME,     AArch64::AEK_SMEF64F64, AArch64::AEK_SMEI16I64,
      AArch64::AEK_SME2,    AArch64::AEK_HBC,       AArch64::AEK_MOPS,
      AArch64::AEK_PERFMON, AArch64::AEK_SVE2p1,    AArch64::AEK_SME2p1,
      AArch64::AEK_B16B16,  AArch64::AEK_SMEF16F16, AArch64::AEK_CSSC,
      AArch64::AEK_RCPC3,   AArch64::AEK_THE,       AArch64::AEK_D128,
      AArch64::AEK_LSE128,  AArch64::AEK_SPECRES2,  AArch64::AEK_RASv2,
      AArch64::AEK_ITE,     AArch64::AEK_GCS,       AArch64::AEK_SMEFA64,
      AArch64::AEK_FPAC,
  };

  std::vector<StringRef> Features;

  uint64_t ExtVal = 0;
  for (auto Ext : Extensions)
    ExtVal |= Ext;

  // NONE has no feature names.
  // We return True here because NONE is a valid choice.
  EXPECT_TRUE(AArch64::getExtensionFeatures(AArch64::AEK_NONE, Features));
  EXPECT_TRUE(!Features.size());

  AArch64::getExtensionFeatures(ExtVal, Features);
  EXPECT_EQ(Extensions.size(), Features.size());

  EXPECT_TRUE(llvm::is_contained(Features, "+crc"));
  EXPECT_TRUE(llvm::is_contained(Features, "+lse"));
  EXPECT_TRUE(llvm::is_contained(Features, "+rdm"));
  EXPECT_TRUE(llvm::is_contained(Features, "+crypto"));
  EXPECT_TRUE(llvm::is_contained(Features, "+sm4"));
  EXPECT_TRUE(llvm::is_contained(Features, "+sha3"));
  EXPECT_TRUE(llvm::is_contained(Features, "+sha2"));
  EXPECT_TRUE(llvm::is_contained(Features, "+aes"));
  EXPECT_TRUE(llvm::is_contained(Features, "+dotprod"));
  EXPECT_TRUE(llvm::is_contained(Features, "+fp-armv8"));
  EXPECT_TRUE(llvm::is_contained(Features, "+neon"));
  EXPECT_TRUE(llvm::is_contained(Features, "+fullfp16"));
  EXPECT_TRUE(llvm::is_contained(Features, "+fp16fml"));
  EXPECT_TRUE(llvm::is_contained(Features, "+spe"));
  EXPECT_TRUE(llvm::is_contained(Features, "+ras"));
  EXPECT_TRUE(llvm::is_contained(Features, "+sve"));
  EXPECT_TRUE(llvm::is_contained(Features, "+sve2"));
  EXPECT_TRUE(llvm::is_contained(Features, "+sve2-aes"));
  EXPECT_TRUE(llvm::is_contained(Features, "+sve2-sm4"));
  EXPECT_TRUE(llvm::is_contained(Features, "+sve2-sha3"));
  EXPECT_TRUE(llvm::is_contained(Features, "+sve2-bitperm"));
  EXPECT_TRUE(llvm::is_contained(Features, "+sve2p1"));
  EXPECT_TRUE(llvm::is_contained(Features, "+b16b16"));
  EXPECT_TRUE(llvm::is_contained(Features, "+rcpc"));
  EXPECT_TRUE(llvm::is_contained(Features, "+rand"));
  EXPECT_TRUE(llvm::is_contained(Features, "+mte"));
  EXPECT_TRUE(llvm::is_contained(Features, "+ssbs"));
  EXPECT_TRUE(llvm::is_contained(Features, "+sb"));
  EXPECT_TRUE(llvm::is_contained(Features, "+predres"));
  EXPECT_TRUE(llvm::is_contained(Features, "+bf16"));
  EXPECT_TRUE(llvm::is_contained(Features, "+i8mm"));
  EXPECT_TRUE(llvm::is_contained(Features, "+f32mm"));
  EXPECT_TRUE(llvm::is_contained(Features, "+f64mm"));
  EXPECT_TRUE(llvm::is_contained(Features, "+tme"));
  EXPECT_TRUE(llvm::is_contained(Features, "+ls64"));
  EXPECT_TRUE(llvm::is_contained(Features, "+brbe"));
  EXPECT_TRUE(llvm::is_contained(Features, "+pauth"));
  EXPECT_TRUE(llvm::is_contained(Features, "+flagm"));
  EXPECT_TRUE(llvm::is_contained(Features, "+sme"));
  EXPECT_TRUE(llvm::is_contained(Features, "+sme-f64f64"));
  EXPECT_TRUE(llvm::is_contained(Features, "+sme-i16i64"));
  EXPECT_TRUE(llvm::is_contained(Features, "+sme-f16f16"));
  EXPECT_TRUE(llvm::is_contained(Features, "+sme2"));
  EXPECT_TRUE(llvm::is_contained(Features, "+sme2p1"));
  EXPECT_TRUE(llvm::is_contained(Features, "+hbc"));
  EXPECT_TRUE(llvm::is_contained(Features, "+mops"));
  EXPECT_TRUE(llvm::is_contained(Features, "+perfmon"));
  EXPECT_TRUE(llvm::is_contained(Features, "+cssc"));
  EXPECT_TRUE(llvm::is_contained(Features, "+rcpc3"));
  EXPECT_TRUE(llvm::is_contained(Features, "+the"));
  EXPECT_TRUE(llvm::is_contained(Features, "+d128"));
  EXPECT_TRUE(llvm::is_contained(Features, "+lse128"));
  EXPECT_TRUE(llvm::is_contained(Features, "+specres2"));
  EXPECT_TRUE(llvm::is_contained(Features, "+ite"));
  EXPECT_TRUE(llvm::is_contained(Features, "+gcs"));
  EXPECT_TRUE(llvm::is_contained(Features, "+sme-fa64"));
  EXPECT_TRUE(llvm::is_contained(Features, "+fpac"));
  // Assuming we listed every extension above, this should produce the same
  // result. (note that AEK_NONE doesn't have a name so it won't be in the
  // result despite its bit being set)
  std::vector<StringRef> AllFeatures;
  EXPECT_TRUE(AArch64::getExtensionFeatures(-1, AllFeatures));
  EXPECT_THAT(Features, ::testing::ContainerEq(AllFeatures));
}

TEST(TargetParserTest, AArch64ArchFeatures) {
  EXPECT_EQ(AArch64::ARMV8A.ArchFeature, "+v8a");
  EXPECT_EQ(AArch64::ARMV8_1A.ArchFeature, "+v8.1a");
  EXPECT_EQ(AArch64::ARMV8_2A.ArchFeature, "+v8.2a");
  EXPECT_EQ(AArch64::ARMV8_3A.ArchFeature, "+v8.3a");
  EXPECT_EQ(AArch64::ARMV8_4A.ArchFeature, "+v8.4a");
  EXPECT_EQ(AArch64::ARMV8_5A.ArchFeature, "+v8.5a");
  EXPECT_EQ(AArch64::ARMV8_6A.ArchFeature, "+v8.6a");
  EXPECT_EQ(AArch64::ARMV8_7A.ArchFeature, "+v8.7a");
  EXPECT_EQ(AArch64::ARMV8_8A.ArchFeature, "+v8.8a");
  EXPECT_EQ(AArch64::ARMV8_9A.ArchFeature, "+v8.9a");
  EXPECT_EQ(AArch64::ARMV9A.ArchFeature, "+v9a");
  EXPECT_EQ(AArch64::ARMV9_1A.ArchFeature, "+v9.1a");
  EXPECT_EQ(AArch64::ARMV9_2A.ArchFeature, "+v9.2a");
  EXPECT_EQ(AArch64::ARMV9_3A.ArchFeature, "+v9.3a");
  EXPECT_EQ(AArch64::ARMV9_4A.ArchFeature, "+v9.4a");
  EXPECT_EQ(AArch64::ARMV8R.ArchFeature, "+v8r");
}

TEST(TargetParserTest, AArch64ArchPartialOrder) {
  for (const auto *A : AArch64::ArchInfos) {
    EXPECT_EQ(*A, *A);

    // v8r has no relation to other valid architectures
    if (*A != AArch64::ARMV8R) {
      EXPECT_FALSE(A->implies(AArch64::ARMV8R));
      EXPECT_FALSE(AArch64::ARMV8R.implies(*A));
    }
  }

  for (const auto *A : {
           &AArch64::ARMV8_1A,
           &AArch64::ARMV8_2A,
           &AArch64::ARMV8_3A,
           &AArch64::ARMV8_4A,
           &AArch64::ARMV8_5A,
           &AArch64::ARMV8_6A,
           &AArch64::ARMV8_7A,
           &AArch64::ARMV8_8A,
           &AArch64::ARMV8_9A,
       })
    EXPECT_TRUE(A->implies(AArch64::ARMV8A));

  for (const auto *A : {&AArch64::ARMV9_1A, &AArch64::ARMV9_2A,
                        &AArch64::ARMV9_3A, &AArch64::ARMV9_4A})
    EXPECT_TRUE(A->implies(AArch64::ARMV9A));

  EXPECT_TRUE(AArch64::ARMV8_1A.implies(AArch64::ARMV8A));
  EXPECT_TRUE(AArch64::ARMV8_2A.implies(AArch64::ARMV8_1A));
  EXPECT_TRUE(AArch64::ARMV8_3A.implies(AArch64::ARMV8_2A));
  EXPECT_TRUE(AArch64::ARMV8_4A.implies(AArch64::ARMV8_3A));
  EXPECT_TRUE(AArch64::ARMV8_5A.implies(AArch64::ARMV8_4A));
  EXPECT_TRUE(AArch64::ARMV8_6A.implies(AArch64::ARMV8_5A));
  EXPECT_TRUE(AArch64::ARMV8_7A.implies(AArch64::ARMV8_6A));
  EXPECT_TRUE(AArch64::ARMV8_8A.implies(AArch64::ARMV8_7A));
  EXPECT_TRUE(AArch64::ARMV8_9A.implies(AArch64::ARMV8_8A));

  EXPECT_TRUE(AArch64::ARMV9_1A.implies(AArch64::ARMV9A));
  EXPECT_TRUE(AArch64::ARMV9_2A.implies(AArch64::ARMV9_1A));
  EXPECT_TRUE(AArch64::ARMV9_3A.implies(AArch64::ARMV9_2A));
  EXPECT_TRUE(AArch64::ARMV9_4A.implies(AArch64::ARMV9_3A));

  EXPECT_TRUE(AArch64::ARMV9A.implies(AArch64::ARMV8_5A));
  EXPECT_TRUE(AArch64::ARMV9_1A.implies(AArch64::ARMV8_6A));
  EXPECT_TRUE(AArch64::ARMV9_2A.implies(AArch64::ARMV8_7A));
  EXPECT_TRUE(AArch64::ARMV9_3A.implies(AArch64::ARMV8_8A));
  EXPECT_TRUE(AArch64::ARMV9_4A.implies(AArch64::ARMV8_9A));
}

TEST(TargetParserTest, AArch64ArchExtFeature) {
  const char *ArchExt[][4] = {
      {"crc", "nocrc", "+crc", "-crc"},
      {"crypto", "nocrypto", "+crypto", "-crypto"},
      {"flagm", "noflagm", "+flagm", "-flagm"},
      {"fp", "nofp", "+fp-armv8", "-fp-armv8"},
      {"simd", "nosimd", "+neon", "-neon"},
      {"fp16", "nofp16", "+fullfp16", "-fullfp16"},
      {"fp16fml", "nofp16fml", "+fp16fml", "-fp16fml"},
      {"profile", "noprofile", "+spe", "-spe"},
      {"ras", "noras", "+ras", "-ras"},
      {"lse", "nolse", "+lse", "-lse"},
      {"rdm", "nordm", "+rdm", "-rdm"},
      {"sve", "nosve", "+sve", "-sve"},
      {"sve2", "nosve2", "+sve2", "-sve2"},
      {"sve2-aes", "nosve2-aes", "+sve2-aes", "-sve2-aes"},
      {"sve2-sm4", "nosve2-sm4", "+sve2-sm4", "-sve2-sm4"},
      {"sve2-sha3", "nosve2-sha3", "+sve2-sha3", "-sve2-sha3"},
      {"sve2p1", "nosve2p1", "+sve2p1", "-sve2p1"},
      {"b16b16", "nob16b16", "+b16b16", "-b16b16"},
      {"sve2-bitperm", "nosve2-bitperm", "+sve2-bitperm", "-sve2-bitperm"},
      {"dotprod", "nodotprod", "+dotprod", "-dotprod"},
      {"rcpc", "norcpc", "+rcpc", "-rcpc"},
      {"rng", "norng", "+rand", "-rand"},
      {"memtag", "nomemtag", "+mte", "-mte"},
      {"tme", "notme", "+tme", "-tme"},
      {"pauth", "nopauth", "+pauth", "-pauth"},
      {"ssbs", "nossbs", "+ssbs", "-ssbs"},
      {"sb", "nosb", "+sb", "-sb"},
      {"predres", "nopredres", "+predres", "-predres"},
      {"i8mm", "noi8mm", "+i8mm", "-i8mm"},
      {"f32mm", "nof32mm", "+f32mm", "-f32mm"},
      {"f64mm", "nof64mm", "+f64mm", "-f64mm"},
      {"sme", "nosme", "+sme", "-sme"},
      {"sme-fa64", "nosme-fa64", "+sme-fa64", "-sme-fa64"},
      {"sme-f64f64", "nosme-f64f64", "+sme-f64f64", "-sme-f64f64"},
      {"sme-i16i64", "nosme-i16i64", "+sme-i16i64", "-sme-i16i64"},
      {"sme-f16f16", "nosme-f16f16", "+sme-f16f16", "-sme-f16f16"},
      {"sme2", "nosme2", "+sme2", "-sme2"},
      {"sme2p1", "nosme2p1", "+sme2p1", "-sme2p1"},
      {"hbc", "nohbc", "+hbc", "-hbc"},
      {"mops", "nomops", "+mops", "-mops"},
      {"pmuv3", "nopmuv3", "+perfmon", "-perfmon"},
      {"predres2", "nopredres2", "+specres2", "-specres2"},
      {"rasv2", "norasv2", "+rasv2", "-rasv2"},
      {"gcs", "nogcs", "+gcs", "-gcs"},
  };

  for (unsigned i = 0; i < std::size(ArchExt); i++) {
    EXPECT_EQ(StringRef(ArchExt[i][2]),
              AArch64::getArchExtFeature(ArchExt[i][0]));
    EXPECT_EQ(StringRef(ArchExt[i][3]),
              AArch64::getArchExtFeature(ArchExt[i][1]));
  }
}

} // namespace
