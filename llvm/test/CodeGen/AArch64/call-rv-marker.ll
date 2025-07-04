; RUN: llc -o - %s | FileCheck --check-prefix=SELDAG --check-prefix=CHECK %s
; RUN: llc -global-isel -o - %s | FileCheck --check-prefix=GISEL --check-prefix=CHECK %s

target datalayout = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"
target triple = "arm64-apple-ios"

declare ptr @foo0(i32)
declare ptr @foo1()

declare void @llvm.objc.release(ptr)
declare void @objc_object(ptr)

declare void @foo2(ptr)

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)

declare ptr @_ZN1SD1Ev(ptr nonnull dereferenceable(1))

declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)


%struct.S = type { i8 }

@g = dso_local global ptr null, align 8
@fptr = dso_local global ptr null, align 8

define dso_local ptr @rv_marker_1_retain() {
; CHECK-LABEL: _rv_marker_1_retain:
; CHECK:         bl _foo1
; CHECK-NEXT:    mov x29, x29
; CHECK-NEXT:    bl _objc_retainAutoreleasedReturnValue
;
entry:
  %call = call ptr @foo1() [ "clang.arc.attachedcall"(ptr @objc_retainAutoreleasedReturnValue) ]
  ret ptr %call
}

define dso_local ptr @rv_marker_1_unsafeClaim() {
; CHECK-LABEL: _rv_marker_1_unsafeClaim:
; CHECK:         bl _foo1
; CHECK-NEXT:    mov x29, x29
; CHECK-NEXT:    bl _objc_unsafeClaimAutoreleasedReturnValue
;
entry:
  %call = call ptr @foo1() [ "clang.arc.attachedcall"(ptr @objc_unsafeClaimAutoreleasedReturnValue) ]
  ret ptr %call
}

define dso_local void @rv_marker_2_select(i32 %c) {
; CHECK-LABEL: _rv_marker_2_select:
; SELDAG:        cinc  w0, w8, eq
; GISEL:         csinc w0, w8, wzr, eq
; CHECK-NEXT:    bl _foo0
; CHECK-NEXT:    mov x29, x29
; CHECK-NEXT:    bl _objc_retainAutoreleasedReturnValue
; CHECK-NEXT:    ldp x29, x30, [sp], #16
; CHECK-NEXT:    b _foo2
;
entry:
  %tobool.not = icmp eq i32 %c, 0
  %.sink = select i1 %tobool.not, i32 2, i32 1
  %call1 = call ptr @foo0(i32 %.sink) [ "clang.arc.attachedcall"(ptr @objc_retainAutoreleasedReturnValue) ]
  tail call void @foo2(ptr %call1)
  ret void
}

define dso_local void @rv_marker_3() personality ptr @__gxx_personality_v0 {
; SELDAG-LABEL: rv_marker_3:
; SELDAG:       Lfunc_begin0:
; SELDAG-NEXT:    .cfi_startproc
; SELDAG-NEXT:    .cfi_personality 155, ___gxx_personality_v0
; SELDAG-NEXT:    .cfi_lsda 16, Lexception0
; SELDAG-NEXT:  ; %bb.0: ; %entry
; SELDAG-NEXT:    stp x20, x19, [sp, #-32]! ; 16-byte Folded Spill
; SELDAG-NEXT:    stp x29, x30, [sp, #16] ; 16-byte Folded Spill
; SELDAG-NEXT:    .cfi_def_cfa_offset 32
; SELDAG-NEXT:    .cfi_offset w30, -8
; SELDAG-NEXT:    .cfi_offset w29, -16
; SELDAG-NEXT:    .cfi_offset w19, -24
; SELDAG-NEXT:    .cfi_offset w20, -32
; SELDAG-NEXT:    bl _foo1
; SELDAG-NEXT:    mov x29, x29
; SELDAG-NEXT:    bl _objc_retainAutoreleasedReturnValue
; SELDAG-NEXT:    mov x19, x0
; SELDAG-NEXT:  Ltmp0:
; SELDAG-NEXT:    bl _objc_object
; SELDAG-NEXT:  Ltmp1:
; SELDAG-NEXT:  ; %bb.1: ; %invoke.cont
; SELDAG-NEXT:    ldp x29, x30, [sp, #16] ; 16-byte Folded Reload
; SELDAG-NEXT:    mov x0, x19
; SELDAG-NEXT:    ldp x20, x19, [sp], #32 ; 16-byte Folded Reload
; SELDAG-NEXT:    b _objc_release
; SELDAG-NEXT:  LBB3_2: ; %lpad
; SELDAG-NEXT:  Ltmp2:
; SELDAG-NEXT:    mov x20, x0
; SELDAG-NEXT:    mov x0, x19
; SELDAG-NEXT:    bl _objc_release
; SELDAG-NEXT:    mov x0, x20
; SELDAG-NEXT:    bl __Unwind_Resume
; SELDAG-NEXT:  Lfunc_end0:
; SELDAG-NEXT:    .cfi_endproc
; SELDAG-NEXT:    .section __TEXT,__gcc_except_tab
; SELDAG-NEXT:    .p2align 2, 0x0
; SELDAG-NEXT:  GCC_except_table3:
; SELDAG-NEXT:  Lexception0:
; SELDAG-NEXT:    .byte 255 ; @LPStart Encoding = omit
; SELDAG-NEXT:    .byte 255 ; @TType Encoding = omit
; SELDAG-NEXT:    .byte 1 ; Call site Encoding = uleb128
; SELDAG-NEXT:    .uleb128 Lcst_end0-Lcst_begin0
; SELDAG-NEXT:  Lcst_begin0:
; SELDAG-NEXT:    .uleb128 Lfunc_begin0-Lfunc_begin0 ; >> Call Site 1 <<
; SELDAG-NEXT:    .uleb128 Ltmp0-Lfunc_begin0 ; Call between Lfunc_begin0 and Ltmp0
; SELDAG-NEXT:    .byte 0 ; has no landing pad
; SELDAG-NEXT:    .byte 0 ; On action: cleanup
; SELDAG-NEXT:    .uleb128 Ltmp0-Lfunc_begin0 ; >> Call Site 2 <<
; SELDAG-NEXT:    .uleb128 Ltmp1-Ltmp0 ; Call between Ltmp0 and Ltmp1
; SELDAG-NEXT:    .uleb128 Ltmp2-Lfunc_begin0 ; jumps to Ltmp2
; SELDAG-NEXT:    .byte 0 ; On action: cleanup
; SELDAG-NEXT:    .uleb128 Ltmp1-Lfunc_begin0 ; >> Call Site 3 <<
; SELDAG-NEXT:    .uleb128 Lfunc_end0-Ltmp1 ; Call between Ltmp1 and Lfunc_end0
; SELDAG-NEXT:    .byte 0 ; has no landing pad
; SELDAG-NEXT:    .byte 0 ; On action: cleanup
; SELDAG-NEXT:  Lcst_end0:
; SELDAG-NEXT:    .p2align 2, 0x0
;
; GISEL-LABEL: rv_marker_3:
; GISEL:       Lfunc_begin0:
; GISEL-NEXT:    .cfi_startproc
; GISEL-NEXT:    .cfi_personality 155, ___gxx_personality_v0
; GISEL-NEXT:    .cfi_lsda 16, Lexception0
; GISEL-NEXT:  ; %bb.0: ; %entry
; GISEL-NEXT:    stp x20, x19, [sp, #-32]! ; 16-byte Folded Spill
; GISEL-NEXT:    stp x29, x30, [sp, #16] ; 16-byte Folded Spill
; GISEL-NEXT:    .cfi_def_cfa_offset 32
; GISEL-NEXT:    .cfi_offset w30, -8
; GISEL-NEXT:    .cfi_offset w29, -16
; GISEL-NEXT:    .cfi_offset w19, -24
; GISEL-NEXT:    .cfi_offset w20, -32
; GISEL-NEXT:    bl _foo1
; GISEL-NEXT:    mov x29, x29
; GISEL-NEXT:    bl _objc_retainAutoreleasedReturnValue
; GISEL-NEXT:    mov x19, x0
; GISEL-NEXT:  Ltmp0:
; GISEL-NEXT:    bl _objc_object
; GISEL-NEXT:  Ltmp1:
; GISEL-NEXT:  ; %bb.1: ; %invoke.cont
; GISEL-NEXT:  Lloh0:
; GISEL-NEXT:    adrp x1, _objc_release@GOTPAGE
; GISEL-NEXT:    mov x0, x19
; GISEL-NEXT:  Lloh1:
; GISEL-NEXT:    ldr x1, [x1, _objc_release@GOTPAGEOFF]
; GISEL-NEXT:    ldp x29, x30, [sp, #16] ; 16-byte Folded Reload
; GISEL-NEXT:    ldp x20, x19, [sp], #32 ; 16-byte Folded Reload
; GISEL-NEXT:    br x1
; GISEL-NEXT:  LBB3_2: ; %lpad
; GISEL-NEXT:  Ltmp2:
; GISEL-NEXT:  Lloh2:
; GISEL-NEXT:    adrp x8, _objc_release@GOTPAGE
; GISEL-NEXT:    mov x20, x0
; GISEL-NEXT:    mov x0, x19
; GISEL-NEXT:  Lloh3:
; GISEL-NEXT:    ldr x8, [x8, _objc_release@GOTPAGEOFF]
; GISEL-NEXT:    blr x8
; GISEL-NEXT:    mov x0, x20
; GISEL-NEXT:    bl __Unwind_Resume
; GISEL-NEXT:    .loh AdrpLdrGot Lloh0, Lloh1
; GISEL-NEXT:    .loh AdrpLdrGot Lloh2, Lloh3
; GISEL-NEXT:  Lfunc_end0:
; GISEL-NEXT:    .cfi_endproc
; GISEL-NEXT:    .section __TEXT,__gcc_except_tab
; GISEL-NEXT:    .p2align 2, 0x0
; GISEL-NEXT:  GCC_except_table3:
; GISEL-NEXT:  Lexception0:
; GISEL-NEXT:    .byte 255 ; @LPStart Encoding = omit
; GISEL-NEXT:    .byte 255 ; @TType Encoding = omit
; GISEL-NEXT:    .byte 1 ; Call site Encoding = uleb128
; GISEL-NEXT:    .uleb128 Lcst_end0-Lcst_begin0
; GISEL-NEXT:  Lcst_begin0:
; GISEL-NEXT:    .uleb128 Lfunc_begin0-Lfunc_begin0 ; >> Call Site 1 <<
; GISEL-NEXT:    .uleb128 Ltmp0-Lfunc_begin0 ; Call between Lfunc_begin0 and Ltmp0
; GISEL-NEXT:    .byte 0 ; has no landing pad
; GISEL-NEXT:    .byte 0 ; On action: cleanup
; GISEL-NEXT:    .uleb128 Ltmp0-Lfunc_begin0 ; >> Call Site 2 <<
; GISEL-NEXT:    .uleb128 Ltmp1-Ltmp0 ; Call between Ltmp0 and Ltmp1
; GISEL-NEXT:    .uleb128 Ltmp2-Lfunc_begin0 ; jumps to Ltmp2
; GISEL-NEXT:    .byte 0 ; On action: cleanup
; GISEL-NEXT:    .uleb128 Ltmp1-Lfunc_begin0 ; >> Call Site 3 <<
; GISEL-NEXT:    .uleb128 Lfunc_end0-Ltmp1 ; Call between Ltmp1 and Lfunc_end0
; GISEL-NEXT:    .byte 0 ; has no landing pad
; GISEL-NEXT:    .byte 0 ; On action: cleanup
; GISEL-NEXT:  Lcst_end0:
; GISEL-NEXT:    .p2align 2, 0x0
entry:
  %call = call ptr @foo1() [ "clang.arc.attachedcall"(ptr @objc_retainAutoreleasedReturnValue) ]
  invoke void @objc_object(ptr %call) #5
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  tail call void @llvm.objc.release(ptr %call)
  ret void

lpad:                                             ; preds = %entry
  %0 = landingpad { ptr, i32 }
          cleanup
  tail call void @llvm.objc.release(ptr %call)
  resume { ptr, i32 } %0
}

define dso_local void @rv_marker_4() personality ptr @__gxx_personality_v0 {
; SELDAG-LABEL: rv_marker_4:
; SELDAG:       Lfunc_begin1:
; SELDAG-NEXT:    .cfi_startproc
; SELDAG-NEXT:    .cfi_personality 155, ___gxx_personality_v0
; SELDAG-NEXT:    .cfi_lsda 16, Lexception1
; SELDAG-NEXT:  ; %bb.0: ; %entry
; SELDAG-NEXT:    sub sp, sp, #48
; SELDAG-NEXT:    stp x20, x19, [sp, #16] ; 16-byte Folded Spill
; SELDAG-NEXT:    stp x29, x30, [sp, #32] ; 16-byte Folded Spill
; SELDAG-NEXT:    .cfi_def_cfa_offset 48
; SELDAG-NEXT:    .cfi_offset w30, -8
; SELDAG-NEXT:    .cfi_offset w29, -16
; SELDAG-NEXT:    .cfi_offset w19, -24
; SELDAG-NEXT:    .cfi_offset w20, -32
; SELDAG-NEXT:  Ltmp3:
; SELDAG-NEXT:    bl _foo1
; SELDAG-NEXT:    mov x29, x29
; SELDAG-NEXT:    bl _objc_retainAutoreleasedReturnValue
; SELDAG-NEXT:  Ltmp4:
; SELDAG-NEXT:  ; %bb.1: ; %invoke.cont
; SELDAG-NEXT:  Ltmp6:
; SELDAG-NEXT:    mov x19, x0
; SELDAG-NEXT:    bl _objc_object
; SELDAG-NEXT:  Ltmp7:
; SELDAG-NEXT:  ; %bb.2: ; %invoke.cont2
; SELDAG-NEXT:    mov x0, x19
; SELDAG-NEXT:    bl _objc_release
; SELDAG-NEXT:    add x0, sp, #15
; SELDAG-NEXT:    bl __ZN1SD1Ev
; SELDAG-NEXT:    ldp x29, x30, [sp, #32] ; 16-byte Folded Reload
; SELDAG-NEXT:    ldp x20, x19, [sp, #16] ; 16-byte Folded Reload
; SELDAG-NEXT:    add sp, sp, #48
; SELDAG-NEXT:    ret
; SELDAG-NEXT:  LBB4_3: ; %lpad1
; SELDAG-NEXT:  Ltmp8:
; SELDAG-NEXT:    mov x20, x0
; SELDAG-NEXT:    mov x0, x19
; SELDAG-NEXT:    bl _objc_release
; SELDAG-NEXT:    b LBB4_5
; SELDAG-NEXT:  LBB4_4: ; %lpad
; SELDAG-NEXT:  Ltmp5:
; SELDAG-NEXT:    mov x20, x0
; SELDAG-NEXT:  LBB4_5: ; %ehcleanup
; SELDAG-NEXT:    add x0, sp, #15
; SELDAG-NEXT:    bl __ZN1SD1Ev
; SELDAG-NEXT:    mov x0, x20
; SELDAG-NEXT:    bl __Unwind_Resume
; SELDAG-NEXT:  Lfunc_end1:
; SELDAG-NEXT:    .cfi_endproc
; SELDAG-NEXT:    .section __TEXT,__gcc_except_tab
; SELDAG-NEXT:    .p2align 2, 0x0
; SELDAG-NEXT:  GCC_except_table4:
; SELDAG-NEXT:  Lexception1:
; SELDAG-NEXT:    .byte 255 ; @LPStart Encoding = omit
; SELDAG-NEXT:    .byte 255 ; @TType Encoding = omit
; SELDAG-NEXT:    .byte 1 ; Call site Encoding = uleb128
; SELDAG-NEXT:    .uleb128 Lcst_end1-Lcst_begin1
; SELDAG-NEXT:  Lcst_begin1:
; SELDAG-NEXT:    .uleb128 Ltmp3-Lfunc_begin1 ; >> Call Site 1 <<
; SELDAG-NEXT:    .uleb128 Ltmp4-Ltmp3 ; Call between Ltmp3 and Ltmp4
; SELDAG-NEXT:    .uleb128 Ltmp5-Lfunc_begin1 ; jumps to Ltmp5
; SELDAG-NEXT:    .byte 0 ; On action: cleanup
; SELDAG-NEXT:    .uleb128 Ltmp6-Lfunc_begin1 ; >> Call Site 2 <<
; SELDAG-NEXT:    .uleb128 Ltmp7-Ltmp6 ; Call between Ltmp6 and Ltmp7
; SELDAG-NEXT:    .uleb128 Ltmp8-Lfunc_begin1 ; jumps to Ltmp8
; SELDAG-NEXT:    .byte 0 ; On action: cleanup
; SELDAG-NEXT:    .uleb128 Ltmp7-Lfunc_begin1 ; >> Call Site 3 <<
; SELDAG-NEXT:    .uleb128 Lfunc_end1-Ltmp7 ; Call between Ltmp7 and Lfunc_end1
; SELDAG-NEXT:    .byte 0 ; has no landing pad
; SELDAG-NEXT:    .byte 0 ; On action: cleanup
; SELDAG-NEXT:  Lcst_end1:
; SELDAG-NEXT:    .p2align 2, 0x0
;
; GISEL-LABEL: rv_marker_4:
; GISEL:       Lfunc_begin1:
; GISEL-NEXT:    .cfi_startproc
; GISEL-NEXT:    .cfi_personality 155, ___gxx_personality_v0
; GISEL-NEXT:    .cfi_lsda 16, Lexception1
; GISEL-NEXT:  ; %bb.0: ; %entry
; GISEL-NEXT:    sub sp, sp, #48
; GISEL-NEXT:    stp x20, x19, [sp, #16] ; 16-byte Folded Spill
; GISEL-NEXT:    stp x29, x30, [sp, #32] ; 16-byte Folded Spill
; GISEL-NEXT:    .cfi_def_cfa_offset 48
; GISEL-NEXT:    .cfi_offset w30, -8
; GISEL-NEXT:    .cfi_offset w29, -16
; GISEL-NEXT:    .cfi_offset w19, -24
; GISEL-NEXT:    .cfi_offset w20, -32
; GISEL-NEXT:  Ltmp3:
; GISEL-NEXT:    bl _foo1
; GISEL-NEXT:    mov x29, x29
; GISEL-NEXT:    bl _objc_retainAutoreleasedReturnValue
; GISEL-NEXT:  Ltmp4:
; GISEL-NEXT:  ; %bb.1: ; %invoke.cont
; GISEL-NEXT:  Ltmp6:
; GISEL-NEXT:    mov x19, x0
; GISEL-NEXT:    bl _objc_object
; GISEL-NEXT:  Ltmp7:
; GISEL-NEXT:  ; %bb.2: ; %invoke.cont2
; GISEL-NEXT:  Lloh4:
; GISEL-NEXT:    adrp x8, _objc_release@GOTPAGE
; GISEL-NEXT:    mov x0, x19
; GISEL-NEXT:  Lloh5:
; GISEL-NEXT:    ldr x8, [x8, _objc_release@GOTPAGEOFF]
; GISEL-NEXT:    blr x8
; GISEL-NEXT:    add x0, sp, #15
; GISEL-NEXT:    bl __ZN1SD1Ev
; GISEL-NEXT:    ldp x29, x30, [sp, #32] ; 16-byte Folded Reload
; GISEL-NEXT:    ldp x20, x19, [sp, #16] ; 16-byte Folded Reload
; GISEL-NEXT:    add sp, sp, #48
; GISEL-NEXT:    ret
; GISEL-NEXT:  LBB4_3: ; %lpad1
; GISEL-NEXT:  Ltmp8:
; GISEL-NEXT:  Lloh6:
; GISEL-NEXT:    adrp x8, _objc_release@GOTPAGE
; GISEL-NEXT:    mov x20, x0
; GISEL-NEXT:    mov x0, x19
; GISEL-NEXT:  Lloh7:
; GISEL-NEXT:    ldr x8, [x8, _objc_release@GOTPAGEOFF]
; GISEL-NEXT:    blr x8
; GISEL-NEXT:    b LBB4_5
; GISEL-NEXT:  LBB4_4: ; %lpad
; GISEL-NEXT:  Ltmp5:
; GISEL-NEXT:    mov x20, x0
; GISEL-NEXT:  LBB4_5: ; %ehcleanup
; GISEL-NEXT:    add x0, sp, #15
; GISEL-NEXT:    bl __ZN1SD1Ev
; GISEL-NEXT:    mov x0, x20
; GISEL-NEXT:    bl __Unwind_Resume
; GISEL-NEXT:    .loh AdrpLdrGot Lloh4, Lloh5
; GISEL-NEXT:    .loh AdrpLdrGot Lloh6, Lloh7
; GISEL-NEXT:  Lfunc_end1:
; GISEL-NEXT:    .cfi_endproc
; GISEL-NEXT:    .section __TEXT,__gcc_except_tab
; GISEL-NEXT:    .p2align 2, 0x0
; GISEL-NEXT:  GCC_except_table4:
; GISEL-NEXT:  Lexception1:
; GISEL-NEXT:    .byte 255 ; @LPStart Encoding = omit
; GISEL-NEXT:    .byte 255 ; @TType Encoding = omit
; GISEL-NEXT:    .byte 1 ; Call site Encoding = uleb128
; GISEL-NEXT:    .uleb128 Lcst_end1-Lcst_begin1
; GISEL-NEXT:  Lcst_begin1:
; GISEL-NEXT:    .uleb128 Ltmp3-Lfunc_begin1 ; >> Call Site 1 <<
; GISEL-NEXT:    .uleb128 Ltmp4-Ltmp3 ; Call between Ltmp3 and Ltmp4
; GISEL-NEXT:    .uleb128 Ltmp5-Lfunc_begin1 ; jumps to Ltmp5
; GISEL-NEXT:    .byte 0 ; On action: cleanup
; GISEL-NEXT:    .uleb128 Ltmp6-Lfunc_begin1 ; >> Call Site 2 <<
; GISEL-NEXT:    .uleb128 Ltmp7-Ltmp6 ; Call between Ltmp6 and Ltmp7
; GISEL-NEXT:    .uleb128 Ltmp8-Lfunc_begin1 ; jumps to Ltmp8
; GISEL-NEXT:    .byte 0 ; On action: cleanup
; GISEL-NEXT:    .uleb128 Ltmp7-Lfunc_begin1 ; >> Call Site 3 <<
; GISEL-NEXT:    .uleb128 Lfunc_end1-Ltmp7 ; Call between Ltmp7 and Lfunc_end1
; GISEL-NEXT:    .byte 0 ; has no landing pad
; GISEL-NEXT:    .byte 0 ; On action: cleanup
; GISEL-NEXT:  Lcst_end1:
; GISEL-NEXT:    .p2align 2, 0x0
entry:
  %s = alloca %struct.S, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %s) #2
  %call = invoke ptr @foo1() [ "clang.arc.attachedcall"(ptr @objc_retainAutoreleasedReturnValue) ]
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  invoke void @objc_object(ptr %call) #5
          to label %invoke.cont2 unwind label %lpad1

invoke.cont2:                                     ; preds = %invoke.cont
  tail call void @llvm.objc.release(ptr %call)
  %call3 = call ptr @_ZN1SD1Ev(ptr nonnull dereferenceable(1) %s)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %s)
  ret void

lpad:                                             ; preds = %entry
  %0 = landingpad { ptr, i32 }
          cleanup
  br label %ehcleanup

lpad1:                                            ; preds = %invoke.cont
  %1 = landingpad { ptr, i32 }
          cleanup
  tail call void @llvm.objc.release(ptr %call)
  br label %ehcleanup

ehcleanup:                                        ; preds = %lpad1, %lpad
  %.pn = phi { ptr, i32 } [ %1, %lpad1 ], [ %0, %lpad ]
  %call4 = call ptr @_ZN1SD1Ev(ptr nonnull dereferenceable(1) %s)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %s)
  resume { ptr, i32 } %.pn
}

define dso_local ptr @rv_marker_5_indirect_call() {
; SELDAG-LABEL: rv_marker_5_indirect_call:
; SELDAG:       ; %bb.0: ; %entry
; SELDAG-NEXT:    stp x20, x19, [sp, #-32]! ; 16-byte Folded Spill
; SELDAG-NEXT:    stp x29, x30, [sp, #16] ; 16-byte Folded Spill
; SELDAG-NEXT:    .cfi_def_cfa_offset 32
; SELDAG-NEXT:    .cfi_offset w30, -8
; SELDAG-NEXT:    .cfi_offset w29, -16
; SELDAG-NEXT:    .cfi_offset w19, -24
; SELDAG-NEXT:    .cfi_offset w20, -32
; SELDAG-NEXT:  Lloh0:
; SELDAG-NEXT:    adrp x8, _fptr@PAGE
; SELDAG-NEXT:  Lloh1:
; SELDAG-NEXT:    ldr x8, [x8, _fptr@PAGEOFF]
; SELDAG-NEXT:    blr x8
; SELDAG-NEXT:    mov x29, x29
; SELDAG-NEXT:    bl _objc_retainAutoreleasedReturnValue
; SELDAG-NEXT:    mov x19, x0
; SELDAG-NEXT:    bl _foo2
; SELDAG-NEXT:    ldp x29, x30, [sp, #16] ; 16-byte Folded Reload
; SELDAG-NEXT:    mov x0, x19
; SELDAG-NEXT:    ldp x20, x19, [sp], #32 ; 16-byte Folded Reload
; SELDAG-NEXT:    ret
; SELDAG-NEXT:    .loh AdrpLdr Lloh0, Lloh1
;
; GISEL-LABEL: rv_marker_5_indirect_call:
; GISEL:       ; %bb.0: ; %entry
; GISEL-NEXT:    stp x20, x19, [sp, #-32]! ; 16-byte Folded Spill
; GISEL-NEXT:    stp x29, x30, [sp, #16] ; 16-byte Folded Spill
; GISEL-NEXT:    .cfi_def_cfa_offset 32
; GISEL-NEXT:    .cfi_offset w30, -8
; GISEL-NEXT:    .cfi_offset w29, -16
; GISEL-NEXT:    .cfi_offset w19, -24
; GISEL-NEXT:    .cfi_offset w20, -32
; GISEL-NEXT:  Lloh8:
; GISEL-NEXT:    adrp x8, _fptr@PAGE
; GISEL-NEXT:  Lloh9:
; GISEL-NEXT:    ldr x8, [x8, _fptr@PAGEOFF]
; GISEL-NEXT:    blr x8
; GISEL-NEXT:    mov x29, x29
; GISEL-NEXT:    bl _objc_retainAutoreleasedReturnValue
; GISEL-NEXT:    mov x19, x0
; GISEL-NEXT:    bl _foo2
; GISEL-NEXT:    ldp x29, x30, [sp, #16] ; 16-byte Folded Reload
; GISEL-NEXT:    mov x0, x19
; GISEL-NEXT:    ldp x20, x19, [sp], #32 ; 16-byte Folded Reload
; GISEL-NEXT:    ret
; GISEL-NEXT:    .loh AdrpLdr Lloh8, Lloh9
entry:
  %0 = load ptr, ptr @fptr, align 8
  %call = call ptr %0() [ "clang.arc.attachedcall"(ptr @objc_retainAutoreleasedReturnValue) ]
  tail call void @foo2(ptr %call)
  ret ptr %call
}

declare ptr @foo(i64, i64, i64)

define dso_local void @rv_marker_multiarg(i64 %a, i64 %b, i64 %c) {
; CHECK-LABEL: _rv_marker_multiarg:
; CHECK:         mov [[TMP:x[0-9]+]], x0
; CHECK-NEXT:    mov x0, x2
; CHECK-NEXT:    mov x2, [[TMP]]
; CHECK-NEXT:    bl  _foo
; CHECK-NEXT:    mov x29, x29
; CHECK-NEXT:    bl _objc_retainAutoreleasedReturnValue
  call ptr @foo(i64 %c, i64 %b, i64 %a) [ "clang.arc.attachedcall"(ptr @objc_retainAutoreleasedReturnValue) ]
  ret void
}

declare ptr @objc_retainAutoreleasedReturnValue(ptr)
declare ptr @objc_unsafeClaimAutoreleasedReturnValue(ptr)
declare i32 @__gxx_personality_v0(...)
