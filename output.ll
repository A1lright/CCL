; ModuleID = 'SysY_module'
source_filename = "SysY_module"

@fmt = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@fmt.1 = private unnamed_addr constant [7 x i8] c"\22true\22\00", align 1
@fmt.2 = private unnamed_addr constant [8 x i8] c"\22false\22\00", align 1

define i32 @getint() {
entry:
  %n = alloca i32, align 4
  %scanfCall = call i32 (ptr, ...) @scanf(ptr @fmt, ptr %n)
  %n.load = load i32, ptr %n, align 4
  ret i32 %n.load
}

declare i32 @scanf(ptr, ...)

define i32 @main() {
entry:
  %a = alloca i32, align 4
  store i32 0, ptr %a, align 4
  %getintCall = call i32 @getint()
  store i32 %getintCall, ptr %a, align 4
  %b = alloca i32, align 4
  store i32 4, ptr %b, align 4
  %c = alloca i32, align 4
  store i32 0, ptr %c, align 4
  %loadtmp = load i32, ptr %a, align 4
  %loadtmp1 = load i32, ptr %b, align 4
  %addtmp = add i32 %loadtmp, %loadtmp1
  store i32 %addtmp, ptr %c, align 4
  %loadtmp2 = load i32, ptr %c, align 4
  %gttmp = icmp sgt i32 %loadtmp2, 2
  %rel_ext = zext i1 %gttmp to i32
  %land_single = icmp ne i32 %rel_ext, 0
  %land_single_ext = zext i1 %land_single to i32
  %lor_single = icmp ne i32 %land_single_ext, 0
  %lor_single_ext = zext i1 %lor_single to i32
  %ifcond = icmp ne i32 %lor_single_ext, 0
  br i1 %ifcond, label %then, label %else

then:                                             ; preds = %entry
  %printfCall = call i32 (ptr, ...) @printf(ptr @fmt.1)
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %loadtmp4 = load i32, ptr %a, align 4
  ret i32 %loadtmp4

else:                                             ; preds = %entry
  %printfCall3 = call i32 (ptr, ...) @printf(ptr @fmt.2)
  br label %ifcont
}

declare i32 @printf(ptr, ...)
