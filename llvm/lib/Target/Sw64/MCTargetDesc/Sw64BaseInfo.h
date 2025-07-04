//===-- Sw64BaseInfo.h - Top level definitions for SW64 MC ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains small standalone helper functions and enum definitions for
// the Sw64 target useful for the compiler back-end and the MC libraries.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_SW64_MCTARGETDESC_SW64BASEINFO_H
#define LLVM_LIB_TARGET_SW64_MCTARGETDESC_SW64BASEINFO_H

#include "Sw64FixupKinds.h"
#include "Sw64MCTargetDesc.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/ErrorHandling.h"

namespace llvm {

/// Sw64II - This namespace holds all of the target specific flags that
/// instruction info tracks.
///
namespace Sw64II {
/// Target Operand Flag enum.
enum TOF {
  //===------------------------------------------------------------------===//
  // Sw64 Specific MachineOperand flags.
  MO_NO_FLAG,

  /// MO_GOT - Represents the offset into the global offset table at which
  /// the address the relocation entry symbol resides during execution.
  MO_LITERAL, // LITERAL
  MO_GPDISP,
  MO_GPDISP_HI,
  MO_GPDISP_LO,

  /// MO_GOT_CALL - Represents the offset into the global offset table at
  /// which the address of a call site relocation entry symbol resides
  /// during execution. This is different from the above since this flag
  /// can only be present in call instructions.
  MO_GOT_CALL,

  /// MO_ABS_HI/LO - Represents the hi or low part of an absolute symbol
  /// address.
  MO_ABS_HI,
  MO_ABS_LO,
  MO_ABS_HILO,
  MO_ABS_LI,

  /// MO_GPREL - Represents the offset from the current gp value to be used
  /// for the relocatable object file being produced.
  MO_GPREL_HI,
  MO_GPREL_LO,

  /// MO_TLSGD - Represents the offset into the global offset table at which
  // the module ID and TSL block offset reside during execution (General
  // Dynamic TLS).
  MO_TLSGD,

  /// MO_TLSLDM - Represents the offset into the global offset table at which
  // the module ID and TSL block offset reside during execution (Local
  // Dynamic TLS).
  MO_TLSLDM,
  MO_DTPREL_HI,
  MO_DTPREL_LO,

  /// MO_GOTTPREL - Represents the offset from the thread pointer (Initial
  // Exec TLS).
  MO_GOTTPREL,

  /// MO_TPREL_HI/LO - Represents the hi and low part of the offset from
  // the thread pointer (Local Exec TLS).
  MO_TPREL_HI,
  MO_TPREL_LO,

  // S32/64 Flags.
  MO_GPOFF_HI,
  MO_GPOFF_LO,
  MO_GOT_DISP,
  MO_GOT_PAGE,
  MO_GOT_OFST,

  /// MO_HIGHER/HIGHEST - Represents the highest or higher half word of a
  /// 64-bit symbol address.
  MO_HIGHER,
  MO_HIGHEST,

  /// MO_GOT_HI16/LO16, MO_CALL_HI16/LO16 - Relocations used for large GOTs.
  MO_GOT_HI16,
  MO_GOT_LO16,
  MO_CALL_HI16,
  MO_CALL_LO16,

  /// Helper operand used to generate R_SW64_JALR
  MO_JALR,

  // LITERAL_GOT
  MO_LITERAL_GOT,

  MO_HINT,
  MO_LITERAL_BASE = 0x40,
  MO_LITUSE = 0x80 // LITERAL
};

enum {
  //===------------------------------------------------------------------===//
  // Instruction encodings.  These are the standard/most common forms for
  // Sw64 instructions.
  //

  // Pseudo - This represents an instruction that is a pseudo instruction
  // or one that has not been implemented yet.  It is illegal to code generate
  // it, but tolerated for intermediate implementation stages.
  Pseudo = 0,

  /// FrmR - This form is for instructions of the format R.
  FrmR = 1,
  /// FrmI - This form is for instructions of the format I.
  FrmI = 2,
  /// FrmJ - This form is for instructions of the format J.
  FrmJ = 3,
  /// FrmFR - This form is for instructions of the format FR.
  FrmFR = 4,
  /// FrmFI - This form is for instructions of the format FI.
  FrmFI = 5,
  /// FrmOther - This form is for instructions that have no specific format.
  FrmOther = 6,
  FormMask = 15,
  /// IsCTI - Instruction is a Control Transfer Instruction.
  IsCTI = 1 << 4,
  /// HasForbiddenSlot - Instruction has a forbidden slot.
  HasForbiddenSlot = 1 << 5,
  /// IsPCRelativeLoad - A Load instruction with implicit source register
  ///                    ($pc) with explicit offset and destination register
  IsPCRelativeLoad = 1 << 6,
  /// HasFCCRegOperand - Instruction uses an $fcc<x> register.
  HasFCCRegOperand = 1 << 7
};
} // namespace Sw64II
} // namespace llvm
#endif
