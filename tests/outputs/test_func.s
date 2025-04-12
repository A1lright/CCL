; ModuleID = 'SysY_module'
source_filename = "SysY_module"

@fmt = private unnamed_addr constant [3 x i8] c"%d\00", align 1

define i32 @getint() {
entry:
  %n = alloca i32, align 4
  %scanfCall = call i32 (ptr, ...) @scanf(ptr @fmt, ptr %n)
  %n.load = load i32, ptr %n, align 4
  ret i32 %n.load
}

declare i32 @scanf(ptr, ...)

define i32 @add(i32 %0, i32 %1) {
entry:
  %x = alloca i32, align 4
  store i32 %0, ptr %x, align 4
  %y = alloca i32, align 4
  store i32 %1, ptr %y, align 4
  %z = alloca i32, align 4
  store i32 0, ptr %z, align 4
  %loadtmp = load i32, ptr %x, align 4
  %loadtmp1 = load i32, ptr %x, align 4
  %loadtmp2 = load i32, ptr %y, align 4
  %loadtmp3 = load i32, ptr %y, align 4
  %addtmp = add i32 %loadtmp1, %loadtmp3
  store i32 %addtmp, ptr %z, align 4
  %loadtmp4 = load i32, ptr %z, align 4
  %loadtmp5 = load i32, ptr %z, align 4
  ret i32 %loadtmp5
}

define i32 @main() {
entry:
  %a = alloca i32, align 4
  store i32 0, ptr %a, align 4
  %b = alloca i32, align 4
  store i32 0, ptr %b, align 4
  store i32 1, ptr %a, align 4
  store i32 2, ptr %b, align 4
  %c = alloca i32, align 4
  store i32 0, ptr %c, align 4
  %loadtmp = load i32, ptr %a, align 4
  %loadtmp1 = load i32, ptr %a, align 4
  %loadtmp2 = load i32, ptr %b, align 4
  %loadtmp3 = load i32, ptr %b, align 4
  %calltmp = call i32 @add(i32 %loadtmp1, i32 %loadtmp3)
  store i32 %calltmp, ptr %c, align 4
  %loadtmp4 = load i32, ptr %c, align 4
  %loadtmp5 = load i32, ptr %c, align 4
  ret i32 %loadtmp5
}
