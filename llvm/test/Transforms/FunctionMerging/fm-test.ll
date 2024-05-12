; RUN: opt -passes=func-merging -S < %s | FileCheck %s
; REQUIRES: target=x86_64{{.*}}

target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx13.0.0"

; Function Attrs: noinline nounwind optnone ssp uwtable
define void @insertionsort(ptr noundef %0, i32 noundef %1) #0 {
; CHECK-LABEL: @insertionsort(ptr noundef %0, i32 noundef %1) #0 {
; CHECK: %3 = tail call i32 @_m_f_0(i1 false, ptr %0, i32 %1) #1
; CHECK: ret void
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  store i32 %1, ptr %4, align 4
  store i32 1, ptr %5, align 4
  br label %8

8:                                                ; preds = %53, %2
  %9 = load i32, ptr %5, align 4
  %10 = load i32, ptr %4, align 4
  %11 = icmp slt i32 %9, %10
  br i1 %11, label %12, label %56

12:                                               ; preds = %8
  %13 = load ptr, ptr %3, align 8
  %14 = load i32, ptr %5, align 4
  %15 = sext i32 %14 to i64
  %16 = getelementptr inbounds i32, ptr %13, i64 %15
  %17 = load i32, ptr %16, align 4
  store i32 %17, ptr %6, align 4
  %18 = load i32, ptr %5, align 4
  %19 = sub nsw i32 %18, 1
  store i32 %19, ptr %7, align 4
  br label %20

20:                                               ; preds = %33, %12
  %21 = load i32, ptr %7, align 4
  %22 = icmp sge i32 %21, 0
  br i1 %22, label %23, label %31

23:                                               ; preds = %20
  %24 = load ptr, ptr %3, align 8
  %25 = load i32, ptr %7, align 4
  %26 = sext i32 %25 to i64
  %27 = getelementptr inbounds i32, ptr %24, i64 %26
  %28 = load i32, ptr %27, align 4
  %29 = load i32, ptr %6, align 4
  %30 = icmp sgt i32 %28, %29
  br label %31

31:                                               ; preds = %23, %20
  %32 = phi i1 [ false, %20 ], [ %30, %23 ]
  br i1 %32, label %33, label %46

33:                                               ; preds = %31
  %34 = load ptr, ptr %3, align 8
  %35 = load i32, ptr %7, align 4
  %36 = sext i32 %35 to i64
  %37 = getelementptr inbounds i32, ptr %34, i64 %36
  %38 = load i32, ptr %37, align 4
  %39 = load ptr, ptr %3, align 8
  %40 = load i32, ptr %7, align 4
  %41 = add nsw i32 %40, 1
  %42 = sext i32 %41 to i64
  %43 = getelementptr inbounds i32, ptr %39, i64 %42
  store i32 %38, ptr %43, align 4
  %44 = load i32, ptr %7, align 4
  %45 = sub nsw i32 %44, 1
  store i32 %45, ptr %7, align 4
  br label %20, !llvm.loop !6

46:                                               ; preds = %31
  %47 = load i32, ptr %6, align 4
  %48 = load ptr, ptr %3, align 8
  %49 = load i32, ptr %7, align 4
  %50 = add nsw i32 %49, 1
  %51 = sext i32 %50 to i64
  %52 = getelementptr inbounds i32, ptr %48, i64 %51
  store i32 %47, ptr %52, align 4
  br label %53

53:                                               ; preds = %46
  %54 = load i32, ptr %5, align 4
  %55 = add nsw i32 %54, 1
  store i32 %55, ptr %5, align 4
  br label %8, !llvm.loop !8

56:                                               ; preds = %8
  ret void
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define i32 @insertionsort2(ptr noundef %0, i32 noundef %1) #0 {
; CHECK-LABEL: @insertionsort2(ptr noundef %0, i32 noundef %1) #0 {
; CHECK: %3 = tail call i32 @_m_f_0(i1 true, ptr %0, i32 %1) #1
; CHECK: ret i32 %3
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  store i32 %1, ptr %4, align 4
  store i32 1, ptr %5, align 4
  br label %8

8:                                                ; preds = %53, %2
  %9 = load i32, ptr %5, align 4
  %10 = load i32, ptr %4, align 4
  %11 = icmp slt i32 %9, %10
  br i1 %11, label %12, label %56

12:                                               ; preds = %8
  %13 = load ptr, ptr %3, align 8
  %14 = load i32, ptr %5, align 4
  %15 = sext i32 %14 to i64
  %16 = getelementptr inbounds i32, ptr %13, i64 %15
  %17 = load i32, ptr %16, align 4
  store i32 %17, ptr %6, align 4
  %18 = load i32, ptr %5, align 4
  %19 = sub nsw i32 %18, 1
  store i32 %19, ptr %7, align 4
  br label %20

20:                                               ; preds = %33, %12
  %21 = load i32, ptr %7, align 4
  %22 = icmp sge i32 %21, 0
  br i1 %22, label %23, label %31

23:                                               ; preds = %20
  %24 = load ptr, ptr %3, align 8
  %25 = load i32, ptr %7, align 4
  %26 = sext i32 %25 to i64
  %27 = getelementptr inbounds i32, ptr %24, i64 %26
  %28 = load i32, ptr %27, align 4
  %29 = load i32, ptr %6, align 4
  %30 = icmp sgt i32 %28, %29
  br label %31

31:                                               ; preds = %23, %20
  %32 = phi i1 [ false, %20 ], [ %30, %23 ]
  br i1 %32, label %33, label %46

33:                                               ; preds = %31
  %34 = load ptr, ptr %3, align 8
  %35 = load i32, ptr %7, align 4
  %36 = sext i32 %35 to i64
  %37 = getelementptr inbounds i32, ptr %34, i64 %36
  %38 = load i32, ptr %37, align 4
  %39 = load ptr, ptr %3, align 8
  %40 = load i32, ptr %7, align 4
  %41 = add nsw i32 %40, 1
  %42 = sext i32 %41 to i64
  %43 = getelementptr inbounds i32, ptr %39, i64 %42
  store i32 %38, ptr %43, align 4
  %44 = load i32, ptr %7, align 4
  %45 = sub nsw i32 %44, 1
  store i32 %45, ptr %7, align 4
  br label %20, !llvm.loop !9

46:                                               ; preds = %31
  %47 = load i32, ptr %6, align 4
  %48 = load ptr, ptr %3, align 8
  %49 = load i32, ptr %7, align 4
  %50 = add nsw i32 %49, 1
  %51 = sext i32 %50 to i64
  %52 = getelementptr inbounds i32, ptr %48, i64 %51
  store i32 %47, ptr %52, align 4
  br label %53

53:                                               ; preds = %46
  %54 = load i32, ptr %5, align 4
  %55 = add nsw i32 %54, 1
  store i32 %55, ptr %5, align 4
  br label %8, !llvm.loop !10

56:                                               ; preds = %8
  ret i32 1
}

; CHECK-LABEL: @_m_f_0(i1 %0, ptr %1, i32 %2) {
; CHECK: entry:
; CHECK:   %3 = alloca ptr, align 8
; CHECK:   %4 = alloca i32, align 4
; CHECK:   %5 = alloca i32, align 4
; CHECK:   %6 = alloca i32, align 4
; CHECK:   %7 = alloca i32, align 4
; CHECK:   store ptr %1, ptr %3, align 8
; CHECK:   store i32 %2, ptr %4, align 4
; CHECK:   store i32 1, ptr %5, align 4
; CHECK:   br label %m.label.bb8

; CHECK: m.label.bb8:                                      ; preds = %m.inst.bb42, %entry
; CHECK:   %8 = load i32, ptr %5, align 4
; CHECK:   %9 = load i32, ptr %4, align 4
; CHECK:   %10 = icmp slt i32 %8, %9
; CHECK:   br i1 %10, label %m.inst.bb16, label %m.term.bb14

; CHECK: m.term.bb14:                                      ; preds = %m.label.bb8
; CHECK:   %11 = select i1 %0, i32 1, i32 undef
; CHECK:   ret i32 %11

; CHECK: m.inst.bb16:                                      ; preds = %m.label.bb8
; CHECK:   %12 = load ptr, ptr %3, align 8
; CHECK:   %13 = load i32, ptr %5, align 4
; CHECK:   %14 = sext i32 %13 to i64
; CHECK:   %15 = getelementptr inbounds i32, ptr %12, i64 %14
; CHECK:   %16 = load i32, ptr %15, align 4
; CHECK:   store i32 %16, ptr %6, align 4
; CHECK:   %17 = load i32, ptr %5, align 4
; CHECK:   %18 = sub nsw i32 %17, 1
; CHECK:   store i32 %18, ptr %7, align 4
; CHECK:   br label %m.label.bb26

; CHECK: m.label.bb26:                                     ; preds = %m.inst.bb56, %m.inst.bb16
; CHECK:   %19 = load i32, ptr %7, align 4
; CHECK:   %20 = icmp sge i32 %19, 0
; CHECK:   br i1 %20, label %m.inst.bb31, label %m.inst.bb42

; CHECK: m.inst.bb31:                                      ; preds = %m.label.bb26
; CHECK:   %21 = load ptr, ptr %3, align 8
; CHECK:   %22 = load i32, ptr %7, align 4
; CHECK:   %23 = sext i32 %22 to i64
; CHECK:   %24 = getelementptr inbounds i32, ptr %21, i64 %23
; CHECK:   %25 = load i32, ptr %24, align 4
; CHECK:   %26 = load i32, ptr %6, align 4
; CHECK:   %27 = icmp sgt i32 %25, %26
; CHECK:   br i1 %27, label %m.inst.bb56, label %m.inst.bb42

; CHECK: m.inst.bb42:                                      ; preds = %m.label.bb26, %m.inst.bb31
; CHECK:   %28 = load i32, ptr %6, align 4
; CHECK:   %29 = load ptr, ptr %3, align 8
; CHECK:   %30 = load i32, ptr %7, align 4
; CHECK:   %31 = add nsw i32 %30, 1
; CHECK:   %32 = sext i32 %31 to i64
; CHECK:   %33 = getelementptr inbounds i32, ptr %29, i64 %32
; CHECK:   store i32 %28, ptr %33, align 4
; CHECK:   %34 = load i32, ptr %5, align 4
; CHECK:   %35 = add nsw i32 %34, 1
; CHECK:   store i32 %35, ptr %5, align 4
; CHECK:   br label %m.label.bb8

; CHECK: m.inst.bb56:                                      ; preds = %m.inst.bb31
; CHECK:   %36 = load ptr, ptr %3, align 8
; CHECK:   %37 = load i32, ptr %7, align 4
; CHECK:   %38 = sext i32 %37 to i64
; CHECK:   %39 = getelementptr inbounds i32, ptr %36, i64 %38
; CHECK:   %40 = load i32, ptr %39, align 4
; CHECK:   %41 = load ptr, ptr %3, align 8
; CHECK:   %42 = load i32, ptr %7, align 4
; CHECK:   %43 = add nsw i32 %42, 1
; CHECK:   %44 = sext i32 %43 to i64
; CHECK:   %45 = getelementptr inbounds i32, ptr %41, i64 %44
; CHECK:   store i32 %40, ptr %45, align 4
; CHECK:   %46 = load i32, ptr %7, align 4
; CHECK:   %47 = sub nsw i32 %46, 1
; CHECK:   store i32 %47, ptr %7, align 4
; CHECK:   br label %m.label.bb26

attributes #0 = { noinline nounwind optnone ssp uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 13, i32 3]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 15.0.7 (git@gitee.com:h836419908_2062810111/llvm-project.git 6c8d0437527ffd58e84041594d6cfad743ebdab9)"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = distinct !{!8, !7}
!9 = distinct !{!9, !7}
!10 = distinct !{!10, !7}
