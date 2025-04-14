; ModuleID = 'SysY_module'
source_filename = "SysY_module"

@fmt = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@fmt.1 = private unnamed_addr constant [7 x i8] c"\22%d\\n\22\00", align 1

define i32 @getint() {
entry:
  %n = alloca i32, align 4
  %scanfCall = call i32 (ptr, ...) @scanf(ptr @fmt, ptr %n)
  %n.load = load i32, ptr %n, align 4
  ret i32 %n.load
}

declare i32 @scanf(ptr, ...)

define void @func(ptr %0) {
entry:
  %a.addr = alloca ptr, align 8
  store ptr %0, ptr %a.addr, align 8
  %a.idx0 = getelementptr inbounds ptr, ptr %a.addr, i32 1
  %a.idx1 = getelementptr inbounds ptr, ptr %a.idx0, i32 0
  store i32 100, ptr %a.idx1, align 4
  ret void
}

define i32 @main() {
entry:
  %arr = alloca [2 x [2 x i32]], align 4
  %arr.idx = getelementptr [2 x [2 x i32]], ptr %arr, i32 0, i32 0, i32 0
  store i32 1, ptr %arr.idx, align 4
  %arr.idx1 = getelementptr [2 x [2 x i32]], ptr %arr, i32 0, i32 0, i32 1
  store i32 2, ptr %arr.idx1, align 4
  %arr.idx2 = getelementptr [2 x [2 x i32]], ptr %arr, i32 0, i32 1, i32 0
  store i32 3, ptr %arr.idx2, align 4
  %arr.idx3 = getelementptr [2 x [2 x i32]], ptr %arr, i32 0, i32 1, i32 1
  store i32 4, ptr %arr.idx3, align 4
  %loadtmp = load i32, ptr %arr, align 4
  call void @func(ptr %arr)
  %arr.idx0 = getelementptr inbounds [2 x [2 x i32]], ptr %arr, i32 1
  %arr.idx14 = getelementptr inbounds [2 x i32], ptr %arr.idx0, i32 0
  %loadtmp5 = load i32, ptr %arr.idx14, align 4
  %0 = load i32, ptr %arr.idx14, align 4
  %printfCall = call i32 (ptr, ...) @printf(ptr @fmt.1, i32 %0)
  ret i32 0
}

declare i32 @printf(ptr, ...)
