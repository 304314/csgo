#include "Utils/AArch64SMEAttributes.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"

#include "gtest/gtest.h"

using namespace llvm;
using SA = SMEAttrs;

std::unique_ptr<Module> parseIR(const char *IR) {
  static LLVMContext C;
  SMDiagnostic Err;
  return parseAssemblyString(IR, Err, C);
}

TEST(SMEAttributes, Constructors) {
  LLVMContext Context;

  ASSERT_TRUE(SA(*parseIR("declare void @foo()")->getFunction("foo"))
                  .hasNonStreamingInterfaceAndBody());

  ASSERT_TRUE(SA(*parseIR("declare void @foo() \"aarch64_pstate_sm_body\"")
                      ->getFunction("foo"))
                  .hasNonStreamingInterface());

  ASSERT_TRUE(SA(*parseIR("declare void @foo() \"aarch64_pstate_sm_enabled\"")
                      ->getFunction("foo"))
                  .hasStreamingInterface());

  ASSERT_TRUE(SA(*parseIR("declare void @foo() \"aarch64_pstate_sm_body\"")
                      ->getFunction("foo"))
                  .hasStreamingBody());

  ASSERT_TRUE(
      SA(*parseIR("declare void @foo() \"aarch64_pstate_sm_compatible\"")
              ->getFunction("foo"))
          .hasStreamingCompatibleInterface());

  ASSERT_TRUE(SA(*parseIR("declare void @foo() \"aarch64_pstate_za_shared\"")
                      ->getFunction("foo"))
                  .hasSharedZAInterface());

  ASSERT_TRUE(SA(*parseIR("declare void @foo() \"aarch64_pstate_za_new\"")
                      ->getFunction("foo"))
                  .hasNewZABody());

  ASSERT_TRUE(SA(*parseIR("declare void @foo() \"aarch64_pstate_za_preserved\"")
                      ->getFunction("foo"))
                  .preservesZA());

  // Invalid combinations.
  EXPECT_DEBUG_DEATH(SA(SA::SM_Enabled | SA::SM_Compatible),
                     "SM_Enabled and SM_Compatible are mutually exclusive");
  EXPECT_DEBUG_DEATH(SA(SA::ZA_New | SA::ZA_Shared),
                     "ZA_New and ZA_Shared are mutually exclusive");
  EXPECT_DEBUG_DEATH(SA(SA::ZA_New | SA::ZA_Preserved),
                     "ZA_New and ZA_Preserved are mutually exclusive");

  // Test that the set() methods equally check validity.
  EXPECT_DEBUG_DEATH(SA(SA::SM_Enabled).set(SA::SM_Compatible),
                     "SM_Enabled and SM_Compatible are mutually exclusive");
  EXPECT_DEBUG_DEATH(SA(SA::SM_Compatible).set(SA::SM_Enabled),
                     "SM_Enabled and SM_Compatible are mutually exclusive");
}

TEST(SMEAttributes, Basics) {
  // Test PSTATE.SM interfaces.
  ASSERT_TRUE(SA(SA::Normal).hasNonStreamingInterfaceAndBody());
  ASSERT_TRUE(SA(SA::SM_Enabled).hasStreamingInterface());
  ASSERT_TRUE(SA(SA::SM_Body).hasStreamingBody());
  ASSERT_TRUE(SA(SA::SM_Body).hasNonStreamingInterface());
  ASSERT_FALSE(SA(SA::SM_Body).hasNonStreamingInterfaceAndBody());
  ASSERT_FALSE(SA(SA::SM_Body).hasStreamingInterface());
  ASSERT_TRUE(SA(SA::SM_Compatible).hasStreamingCompatibleInterface());
  ASSERT_TRUE(
      SA(SA::SM_Compatible | SA::SM_Body).hasStreamingCompatibleInterface());
  ASSERT_TRUE(SA(SA::SM_Compatible | SA::SM_Body).hasStreamingBody());
  ASSERT_FALSE(SA(SA::SM_Compatible | SA::SM_Body).hasNonStreamingInterface());

  // Test PSTATE.ZA interfaces.
  ASSERT_FALSE(SA(SA::ZA_Shared).hasPrivateZAInterface());
  ASSERT_TRUE(SA(SA::ZA_Shared).hasSharedZAInterface());
  ASSERT_TRUE(SA(SA::ZA_Shared).hasZAState());
  ASSERT_FALSE(SA(SA::ZA_Shared).preservesZA());
  ASSERT_TRUE(SA(SA::ZA_Shared | SA::ZA_Preserved).preservesZA());

  ASSERT_TRUE(SA(SA::ZA_New).hasPrivateZAInterface());
  ASSERT_TRUE(SA(SA::ZA_New).hasNewZABody());
  ASSERT_TRUE(SA(SA::ZA_New).hasZAState());
  ASSERT_FALSE(SA(SA::ZA_New).preservesZA());

  ASSERT_TRUE(SA(SA::Normal).hasPrivateZAInterface());
  ASSERT_FALSE(SA(SA::Normal).hasNewZABody());
  ASSERT_FALSE(SA(SA::Normal).hasZAState());
  ASSERT_FALSE(SA(SA::Normal).preservesZA());
}

TEST(SMEAttributes, Transitions) {
  // Normal -> Normal
  ASSERT_FALSE(SA(SA::Normal).requiresSMChange(SA(SA::Normal)));
  // Normal -> Normal + LocallyStreaming
  ASSERT_FALSE(SA(SA::Normal).requiresSMChange(SA(SA::Normal | SA::SM_Body)));

  // Normal -> Streaming
  ASSERT_TRUE(SA(SA::Normal).requiresSMChange(SA(SA::SM_Enabled)));
  // Normal -> Streaming + LocallyStreaming
  ASSERT_TRUE(
      SA(SA::Normal).requiresSMChange(SA(SA::SM_Enabled | SA::SM_Body)));

  // Normal -> Streaming-compatible
  ASSERT_FALSE(SA(SA::Normal).requiresSMChange(SA(SA::SM_Compatible)));
  // Normal -> Streaming-compatible + LocallyStreaming
  ASSERT_FALSE(
      SA(SA::Normal).requiresSMChange(SA(SA::SM_Compatible | SA::SM_Body)));

  // Streaming -> Normal
  ASSERT_TRUE(SA(SA::SM_Enabled).requiresSMChange(SA(SA::Normal)));
  // Streaming -> Normal + LocallyStreaming
  ASSERT_TRUE(
      SA(SA::SM_Enabled).requiresSMChange(SA(SA::Normal | SA::SM_Body)));

  // Streaming -> Streaming
  ASSERT_FALSE(SA(SA::SM_Enabled).requiresSMChange(SA(SA::SM_Enabled)));
  // Streaming -> Streaming + LocallyStreaming
  ASSERT_FALSE(
      SA(SA::SM_Enabled).requiresSMChange(SA(SA::SM_Enabled | SA::SM_Body)));

  // Streaming -> Streaming-compatible
  ASSERT_FALSE(SA(SA::SM_Enabled).requiresSMChange(SA(SA::SM_Compatible)));
  // Streaming -> Streaming-compatible + LocallyStreaming
  ASSERT_FALSE(
      SA(SA::SM_Enabled).requiresSMChange(SA(SA::SM_Compatible | SA::SM_Body)));

  // Streaming-compatible -> Normal
  ASSERT_TRUE(SA(SA::SM_Compatible).requiresSMChange(SA(SA::Normal)));
  ASSERT_TRUE(
      SA(SA::SM_Compatible).requiresSMChange(SA(SA::Normal | SA::SM_Body)));

  // Streaming-compatible -> Streaming
  ASSERT_TRUE(SA(SA::SM_Compatible).requiresSMChange(SA(SA::SM_Enabled)));
  // Streaming-compatible -> Streaming + LocallyStreaming
  ASSERT_TRUE(
      SA(SA::SM_Compatible).requiresSMChange(SA(SA::SM_Enabled | SA::SM_Body)));

  // Streaming-compatible -> Streaming-compatible
  ASSERT_FALSE(SA(SA::SM_Compatible).requiresSMChange(SA(SA::SM_Compatible)));
  // Streaming-compatible -> Streaming-compatible + LocallyStreaming
  ASSERT_FALSE(SA(SA::SM_Compatible)
                   .requiresSMChange(SA(SA::SM_Compatible | SA::SM_Body)));
}
