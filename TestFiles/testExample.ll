; ModuleID = 'testExample.cpp'
source_filename = "testExample.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%class.Base = type { i32 (...)** }
%class.Derived = type { %class.Base }

$_ZN7DerivedC2Ev = comdat any

$_ZN4BaseC2Ev = comdat any

$_ZN7Derived3fooEv = comdat any

$_ZN4Base3barEv = comdat any

$_ZN4Base3fooEv = comdat any

$_ZTV7Derived = comdat any

$_ZTS7Derived = comdat any

$_ZTS4Base = comdat any

$_ZTI4Base = comdat any

$_ZTI7Derived = comdat any

$_ZTV4Base = comdat any

@_ZTV7Derived = linkonce_odr dso_local unnamed_addr constant { [4 x i8*] } { [4 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI7Derived to i8*), i8* bitcast (void (%class.Derived*)* @_ZN7Derived3fooEv to i8*), i8* bitcast (void (%class.Base*)* @_ZN4Base3barEv to i8*)] }, comdat, align 8
@_ZTVN10__cxxabiv120__si_class_type_infoE = external global i8*
@_ZTS7Derived = linkonce_odr dso_local constant [9 x i8] c"7Derived\00", comdat, align 1
@_ZTVN10__cxxabiv117__class_type_infoE = external global i8*
@_ZTS4Base = linkonce_odr dso_local constant [6 x i8] c"4Base\00", comdat, align 1
@_ZTI4Base = linkonce_odr dso_local constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @_ZTS4Base, i32 0, i32 0) }, comdat, align 8
@_ZTI7Derived = linkonce_odr dso_local constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @_ZTS7Derived, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI4Base to i8*) }, comdat, align 8
@_ZTV4Base = linkonce_odr dso_local unnamed_addr constant { [4 x i8*] } { [4 x i8*] [i8* null, i8* bitcast ({ i8*, i8* }* @_ZTI4Base to i8*), i8* bitcast (void (%class.Base*)* @_ZN4Base3fooEv to i8*), i8* bitcast (void (%class.Base*)* @_ZN4Base3barEv to i8*)] }, comdat, align 8

; Function Attrs: mustprogress noinline norecurse optnone uwtable
define dso_local noundef i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca %class.Base*, align 8
  store i32 0, i32* %1, align 4
  %3 = call noalias noundef nonnull i8* @_Znwm(i64 noundef 8) #4
  %4 = bitcast i8* %3 to %class.Derived*
  call void @_ZN7DerivedC2Ev(%class.Derived* noundef nonnull align 8 dereferenceable(8) %4) #5
  %5 = bitcast %class.Derived* %4 to %class.Base*
  store %class.Base* %5, %class.Base** %2, align 8
  %6 = load %class.Base*, %class.Base** %2, align 8
  %7 = bitcast %class.Base* %6 to void (%class.Base*)***
  %8 = load void (%class.Base*)**, void (%class.Base*)*** %7, align 8
  %9 = getelementptr inbounds void (%class.Base*)*, void (%class.Base*)** %8, i64 1
  %10 = load void (%class.Base*)*, void (%class.Base*)** %9, align 8
  call void %10(%class.Base* noundef nonnull align 8 dereferenceable(8) %6)
  ret i32 0
}

; Function Attrs: nobuiltin allocsize(0)
declare noundef nonnull i8* @_Znwm(i64 noundef) #1

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN7DerivedC2Ev(%class.Derived* noundef nonnull align 8 dereferenceable(8) %0) unnamed_addr #2 comdat align 2 {
  %2 = alloca %class.Derived*, align 8
  store %class.Derived* %0, %class.Derived** %2, align 8
  %3 = load %class.Derived*, %class.Derived** %2, align 8
  %4 = bitcast %class.Derived* %3 to %class.Base*
  call void @_ZN4BaseC2Ev(%class.Base* noundef nonnull align 8 dereferenceable(8) %4) #5
  %5 = bitcast %class.Derived* %3 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [4 x i8*] }, { [4 x i8*] }* @_ZTV7Derived, i32 0, inrange i32 0, i32 2) to i32 (...)**), i32 (...)*** %5, align 8
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN4BaseC2Ev(%class.Base* noundef nonnull align 8 dereferenceable(8) %0) unnamed_addr #2 comdat align 2 {
  %2 = alloca %class.Base*, align 8
  store %class.Base* %0, %class.Base** %2, align 8
  %3 = load %class.Base*, %class.Base** %2, align 8
  %4 = bitcast %class.Base* %3 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [4 x i8*] }, { [4 x i8*] }* @_ZTV4Base, i32 0, inrange i32 0, i32 2) to i32 (...)**), i32 (...)*** %4, align 8
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN7Derived3fooEv(%class.Derived* noundef nonnull align 8 dereferenceable(8) %0) unnamed_addr #3 comdat align 2 {
  %2 = alloca %class.Derived*, align 8
  store %class.Derived* %0, %class.Derived** %2, align 8
  %3 = load %class.Derived*, %class.Derived** %2, align 8
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN4Base3barEv(%class.Base* noundef nonnull align 8 dereferenceable(8) %0) unnamed_addr #3 comdat align 2 {
  %2 = alloca %class.Base*, align 8
  store %class.Base* %0, %class.Base** %2, align 8
  %3 = load %class.Base*, %class.Base** %2, align 8
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN4Base3fooEv(%class.Base* noundef nonnull align 8 dereferenceable(8) %0) unnamed_addr #3 comdat align 2 {
  %2 = alloca %class.Base*, align 8
  store %class.Base* %0, %class.Base** %2, align 8
  %3 = load %class.Base*, %class.Base** %2, align 8
  ret void
}

attributes #0 = { mustprogress noinline norecurse optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nobuiltin allocsize(0) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { builtin allocsize(0) }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 14.0.0-1ubuntu1"}
