; ModuleID = './../../llvm/lib/Transforms/llvm-dead-virtual-pass/TestFiles/testExample.ll'
source_filename = "testExample.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

$_ZN9testClassC2Ev = comdat any

$_ZN4Base12testFunctionEv = comdat any

$_ZN4BaseC2Ev = comdat any

$_ZN4Base11virtual_fooEv = comdat any

$_ZN4Base11virtual_barEv = comdat any

$_ZTV9testClass = comdat any

$_ZTS9testClass = comdat any

$_ZTS4Base = comdat any

$_ZTI4Base = comdat any

$_ZTI9testClass = comdat any

$_ZTV4Base = comdat any

@_ZTV9testClass = linkonce_odr dso_local unnamed_addr constant { [4 x ptr] } { [4 x ptr] [ptr null, ptr @_ZTI9testClass, ptr @_ZN4Base11virtual_fooEv, ptr @_ZN4Base11virtual_barEv] }, comdat, align 8
@_ZTVN10__cxxabiv120__si_class_type_infoE = external global ptr
@_ZTS9testClass = linkonce_odr dso_local constant [11 x i8] c"9testClass\00", comdat, align 1
@_ZTVN10__cxxabiv117__class_type_infoE = external global ptr
@_ZTS4Base = linkonce_odr dso_local constant [6 x i8] c"4Base\00", comdat, align 1
@_ZTI4Base = linkonce_odr dso_local constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr @_ZTS4Base }, comdat, align 8
@_ZTI9testClass = linkonce_odr dso_local constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTS9testClass, ptr @_ZTI4Base }, comdat, align 8
@_ZTV4Base = linkonce_odr dso_local unnamed_addr constant { [4 x ptr] } { [4 x ptr] [ptr null, ptr @_ZTI4Base, ptr @_ZN4Base11virtual_fooEv, ptr @_ZN4Base11virtual_barEv] }, comdat, align 8

; Function Attrs: mustprogress noinline norecurse optnone uwtable
declare dso_local noundef i32 @main() #0

; Function Attrs: nobuiltin allocsize(0)
declare noundef nonnull ptr @_Znwm(i64 noundef) #1

; Function Attrs: noinline nounwind optnone uwtable
declare dso_local void @_ZN9testClassC2Ev(ptr noundef nonnull align 8 dereferenceable(8)) unnamed_addr #2 comdat align 2

; Function Attrs: mustprogress noinline nounwind optnone uwtable
declare dso_local void @_ZN4Base12testFunctionEv(ptr noundef nonnull align 8 dereferenceable(8)) #3 comdat align 2

; Function Attrs: noinline nounwind optnone uwtable
declare dso_local void @_ZN4BaseC2Ev(ptr noundef nonnull align 8 dereferenceable(8)) unnamed_addr #2 comdat align 2

; Function Attrs: mustprogress noinline nounwind optnone uwtable
declare dso_local void @_ZN4Base11virtual_fooEv(ptr noundef nonnull align 8 dereferenceable(8)) unnamed_addr #3 comdat align 2

; Function Attrs: mustprogress noinline nounwind optnone uwtable
declare dso_local void @_ZN4Base11virtual_barEv(ptr noundef nonnull align 8 dereferenceable(8)) unnamed_addr #3 comdat align 2

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #4

attributes #0 = { mustprogress noinline norecurse optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nobuiltin allocsize(0) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nocallback nofree nounwind willreturn memory(argmem: write) }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 14.0.0-1ubuntu1"}
