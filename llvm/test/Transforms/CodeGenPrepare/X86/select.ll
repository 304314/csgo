; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt -passes='require<profile-summary>,function(codegenprepare)' -S < %s | FileCheck %s
; RUN: opt -enable-debugify -passes='require<profile-summary>,function(codegenprepare)' -S < %s | FileCheck %s -check-prefix=DEBUG

target triple = "x86_64-unknown-unknown"

; Nothing to sink and convert here.

define i32 @no_sink(double %a, ptr %b, i32 %x, i32 %y)  {
; CHECK-LABEL: @no_sink(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[LOAD:%.*]] = load double, ptr [[B:%.*]], align 8
; CHECK-NEXT:    [[CMP:%.*]] = fcmp olt double [[LOAD]], [[A:%.*]]
; CHECK-NEXT:    [[SEL:%.*]] = select i1 [[CMP]], i32 [[X:%.*]], i32 [[Y:%.*]]
; CHECK-NEXT:    ret i32 [[SEL]]
;
entry:
  %load = load double, ptr %b, align 8
  %cmp = fcmp olt double %load, %a
  %sel = select i1 %cmp, i32 %x, i32 %y
  ret i32 %sel
}


; An 'fdiv' is expensive, so sink it rather than speculatively execute it.

define float @fdiv_true_sink(float %a, float %b) {
; CHECK-LABEL: @fdiv_true_sink(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[SEL_FR:%.*]] = freeze float [[A:%.*]]
; CHECK-NEXT:    [[CMP:%.*]] = fcmp ogt float [[SEL_FR]], 1.000000e+00
; CHECK-NEXT:    br i1 [[CMP]], label [[SELECT_TRUE_SINK:%.*]], label [[SELECT_END:%.*]]
; CHECK:       select.true.sink:
; CHECK-NEXT:    [[DIV:%.*]] = fdiv float [[A]], [[B:%.*]]
; CHECK-NEXT:    br label [[SELECT_END]]
; CHECK:       select.end:
; CHECK-NEXT:    [[SEL:%.*]] = phi float [ [[DIV]], [[SELECT_TRUE_SINK]] ], [ 2.000000e+00, [[ENTRY:%.*]] ]
; CHECK-NEXT:    ret float [[SEL]]
;
; DEBUG-LABEL: @fdiv_true_sink(
; DEBUG-NEXT:  entry:
; DEBUG-NEXT:    [[SEL_FR:%.*]] = freeze float [[A:%.*]]
; DEBUG-NEXT:    [[CMP:%.*]] = fcmp ogt float [[SEL_FR]], 1.000000e+00, !dbg !24
; DEBUG-NEXT:    call void @llvm.dbg.value(metadata i1 [[CMP]]
; DEBUG-NEXT:    br i1 [[CMP]], label [[SELECT_TRUE_SINK:%.*]], label [[SELECT_END:%.*]], !dbg
; DEBUG:       select.true.sink:
; DEBUG-NEXT:    [[DIV:%.*]] = fdiv float [[A]], [[B:%.*]]
; DEBUG-NEXT:    call void @llvm.dbg.value(metadata float [[DIV]]
; DEBUG-NEXT:    br label [[SELECT_END]], !dbg
; DEBUG:       select.end:
; DEBUG-NEXT:    [[SEL:%.*]] = phi float [ [[DIV]], [[SELECT_TRUE_SINK]] ], [ 2.000000e+00, [[ENTRY:%.*]] ], !dbg
; DEBUG-NEXT:    call void @llvm.dbg.value(metadata float [[SEL]]
; DEBUG-NEXT:    ret float [[SEL]]
;
entry:
  %div = fdiv float %a, %b
  %cmp = fcmp ogt float %a, 1.0
  %sel = select i1 %cmp, float %div, float 2.0
  ret float %sel
}

define float @fdiv_false_sink(float %a, float %b) {
; CHECK-LABEL: @fdiv_false_sink(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[SEL_FR:%.*]] = freeze float [[A:%.*]]
; CHECK-NEXT:    [[CMP:%.*]] = fcmp ogt float [[SEL_FR]], 3.000000e+00
; CHECK-NEXT:    br i1 [[CMP]], label [[SELECT_END:%.*]], label [[SELECT_FALSE_SINK:%.*]]
; CHECK:       select.false.sink:
; CHECK-NEXT:    [[DIV:%.*]] = fdiv float [[A]], [[B:%.*]]
; CHECK-NEXT:    br label [[SELECT_END]]
; CHECK:       select.end:
; CHECK-NEXT:    [[SEL:%.*]] = phi float [ 4.000000e+00, [[ENTRY:%.*]] ], [ [[DIV]], [[SELECT_FALSE_SINK]] ]
; CHECK-NEXT:    ret float [[SEL]]
;
; DEBUG-LABEL: @fdiv_false_sink(
; DEBUG-NEXT:  entry:
; DEBUG-NEXT:    [[SEL_FR:%.*]] = freeze float [[A:%.*]]
; DEBUG-NEXT:    [[CMP:%.*]] = fcmp ogt float [[SEL_FR]], 3.000000e+00, !dbg !33
; DEBUG-NEXT:    call void @llvm.dbg.value(metadata i1 [[CMP]]
; DEBUG-NEXT:    br i1 [[CMP]], label [[SELECT_END:%.*]], label [[SELECT_FALSE_SINK:%.*]], !dbg
; DEBUG:       select.false.sink:
; DEBUG-NEXT:    [[DIV:%.*]] = fdiv float [[A]], [[B:%.*]]
; DEBUG-NEXT:    call void @llvm.dbg.value(metadata float [[DIV]]
; DEBUG-NEXT:    br label [[SELECT_END]], !dbg
; DEBUG:       select.end:
; DEBUG-NEXT:    [[SEL:%.*]] = phi float [ 4.000000e+00, [[ENTRY:%.*]] ], [ [[DIV]], [[SELECT_FALSE_SINK]] ], !dbg
; DEBUG-NEXT:    call void @llvm.dbg.value(metadata float [[SEL]]
; DEBUG-NEXT:    ret float [[SEL]], !dbg
;
entry:
  %div = fdiv float %a, %b
  %cmp = fcmp ogt float %a, 3.0
  %sel = select i1 %cmp, float 4.0, float %div
  ret float %sel
}

define float @fdiv_both_sink(float %a, float %b) {
; CHECK-LABEL: @fdiv_both_sink(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[SEL_FR:%.*]] = freeze float [[A:%.*]]
; CHECK-NEXT:    [[CMP:%.*]] = fcmp ogt float [[SEL_FR]], 5.000000e+00
; CHECK-NEXT:    br i1 [[CMP]], label [[SELECT_TRUE_SINK:%.*]], label [[SELECT_FALSE_SINK:%.*]]
; CHECK:       select.true.sink:
; CHECK-NEXT:    [[DIV1:%.*]] = fdiv float [[A]], [[B:%.*]]
; CHECK-NEXT:    br label [[SELECT_END:%.*]]
; CHECK:       select.false.sink:
; CHECK-NEXT:    [[DIV2:%.*]] = fdiv float [[B]], [[A]]
; CHECK-NEXT:    br label [[SELECT_END]]
; CHECK:       select.end:
; CHECK-NEXT:    [[SEL:%.*]] = phi float [ [[DIV1]], [[SELECT_TRUE_SINK]] ], [ [[DIV2]], [[SELECT_FALSE_SINK]] ]
; CHECK-NEXT:    ret float [[SEL]]
;
entry:
  %div1 = fdiv float %a, %b
  %div2 = fdiv float %b, %a
  %cmp = fcmp ogt float %a, 5.0
  %sel = select i1 %cmp, float %div1, float %div2
  ret float %sel
}

; But if the select is marked unpredictable, then don't turn it into a branch.

define float @unpredictable_select(float %a, float %b) {
; CHECK-LABEL: @unpredictable_select(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[DIV:%.*]] = fdiv float [[A:%.*]], [[B:%.*]]
; CHECK-NEXT:    [[CMP:%.*]] = fcmp ogt float [[A]], 1.000000e+00
; CHECK-NEXT:    [[SEL:%.*]] = select i1 [[CMP]], float [[DIV]], float 2.000000e+00, !unpredictable !0
; CHECK-NEXT:    ret float [[SEL]]
;
entry:
  %div = fdiv float %a, %b
  %cmp = fcmp ogt float %a, 1.0
  %sel = select i1 %cmp, float %div, float 2.0, !unpredictable !0
  ret float %sel
}

!0 = !{}

; An 'fadd' is not too expensive, so it's ok to speculate.

define float @fadd_no_sink(float %a, float %b) {
; CHECK-LABEL: @fadd_no_sink(
; CHECK-NEXT:    [[ADD:%.*]] = fadd float [[A:%.*]], [[B:%.*]]
; CHECK-NEXT:    [[CMP:%.*]] = fcmp ogt float 6.000000e+00, [[A]]
; CHECK-NEXT:    [[SEL:%.*]] = select i1 [[CMP]], float [[ADD]], float 7.000000e+00
; CHECK-NEXT:    ret float [[SEL]]
;
  %add = fadd float %a, %b
  %cmp = fcmp ogt float 6.0, %a
  %sel = select i1 %cmp, float %add, float 7.0
  ret float %sel
}

; Possible enhancement: sinkability is only calculated with the direct
; operand of the select, so we don't try to sink this. The fdiv cost is not
; taken into account.

define float @fdiv_no_sink(float %a, float %b) {
; CHECK-LABEL: @fdiv_no_sink(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[DIV:%.*]] = fdiv float [[A:%.*]], [[B:%.*]]
; CHECK-NEXT:    [[ADD:%.*]] = fadd float [[DIV]], [[B]]
; CHECK-NEXT:    [[CMP:%.*]] = fcmp ogt float [[A]], 1.000000e+00
; CHECK-NEXT:    [[SEL:%.*]] = select i1 [[CMP]], float [[ADD]], float 8.000000e+00
; CHECK-NEXT:    ret float [[SEL]]
;
entry:
  %div = fdiv float %a, %b
  %add = fadd float %div, %b
  %cmp = fcmp ogt float %a, 1.0
  %sel = select i1 %cmp, float %add, float 8.0
  ret float %sel
}

; Do not transform the CFG if the select operands may have side effects.

declare ptr @bar(i32, i32, i32)
declare ptr @baz(i32, i32, i32)

define ptr @calls_no_sink(i32 %in) {
; CHECK-LABEL: @calls_no_sink(
; CHECK-NEXT:    [[CALL1:%.*]] = call ptr @bar(i32 1, i32 2, i32 3)
; CHECK-NEXT:    [[CALL2:%.*]] = call ptr @baz(i32 1, i32 2, i32 3)
; CHECK-NEXT:    [[TOBOOL:%.*]] = icmp ne i32 [[IN:%.*]], 0
; CHECK-NEXT:    [[SEL:%.*]] = select i1 [[TOBOOL]], ptr [[CALL1]], ptr [[CALL2]]
; CHECK-NEXT:    ret ptr [[SEL]]
;
  %call1 = call ptr @bar(i32 1, i32 2, i32 3)
  %call2 = call ptr @baz(i32 1, i32 2, i32 3)
  %tobool = icmp ne i32 %in, 0
  %sel = select i1 %tobool, ptr %call1, ptr %call2
  ret ptr %sel
}

define i32 @sdiv_no_sink(i32 %a, i32 %b) {
; CHECK-LABEL: @sdiv_no_sink(
; CHECK-NEXT:    [[DIV1:%.*]] = sdiv i32 [[A:%.*]], [[B:%.*]]
; CHECK-NEXT:    [[DIV2:%.*]] = sdiv i32 [[B]], [[A]]
; CHECK-NEXT:    [[CMP:%.*]] = icmp sgt i32 [[A]], 5
; CHECK-NEXT:    [[SEL:%.*]] = select i1 [[CMP]], i32 [[DIV1]], i32 [[DIV2]]
; CHECK-NEXT:    ret i32 [[SEL]]
;
  %div1 = sdiv i32 %a, %b
  %div2 = sdiv i32 %b, %a
  %cmp = icmp sgt i32 %a, 5
  %sel = select i1 %cmp, i32 %div1, i32 %div2
  ret i32 %sel
}

