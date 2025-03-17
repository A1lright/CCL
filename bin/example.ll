; ModuleID = 'SysY_module'
source_filename = "SysY_module"

@fmt = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@fmt.1 = private unnamed_addr constant [5 x i8] c"\22%d\22\00", align 1

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
  %c = alloca i32, align 4
  %getintCall = call i32 @getint()
  store i32 %getintCall, ptr %c, align 4
  %0 = load i32, ptr %c, align 4
  %printfCall = call i32 (ptr, ...) @printf(ptr @fmt.1, i32 %0)
  %1 = load i32, ptr %c, align 4
  ret i32 %1
}

declare i32 @printf(ptr, ...)