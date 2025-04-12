; ModuleID = 'SysY_module'
source_filename = "SysY_module"

@fmt = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@fmt.1 = private unnamed_addr constant [13 x i8] c"\22%d %d %d\\n\22\00", align 1
@fmt.2 = private unnamed_addr constant [13 x i8] c"\22%d %d %d\\n\22\00", align 1
@fmt.3 = private unnamed_addr constant [13 x i8] c"\22%d %d %d\\n\22\00", align 1

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
  %arr = alloca [3 x [3 x i32]], align 4
  %arr.idx = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 0, i32 0
  store i32 0, ptr %arr.idx, align 4
  %arr.idx1 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 0, i32 1
  store i32 0, ptr %arr.idx1, align 4
  %arr.idx2 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 0, i32 2
  store i32 0, ptr %arr.idx2, align 4
  %arr.idx3 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 1, i32 0
  store i32 0, ptr %arr.idx3, align 4
  %arr.idx4 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 1, i32 1
  store i32 0, ptr %arr.idx4, align 4
  %arr.idx5 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 1, i32 2
  store i32 0, ptr %arr.idx5, align 4
  %arr.idx6 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 2, i32 0
  store i32 0, ptr %arr.idx6, align 4
  %arr.idx7 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 2, i32 1
  store i32 0, ptr %arr.idx7, align 4
  %arr.idx8 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 2, i32 2
  store i32 0, ptr %arr.idx8, align 4
  %arr.elem = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 0, i32 0
  store i32 1, ptr %arr.elem, align 4
  %arr.elem9 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 0, i32 1
  store i32 2, ptr %arr.elem9, align 4
  %arr.elem10 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 0, i32 2
  store i32 3, ptr %arr.elem10, align 4
  %arr.elem11 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 1, i32 0
  store i32 4, ptr %arr.elem11, align 4
  %arr.elem12 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 1, i32 1
  store i32 5, ptr %arr.elem12, align 4
  %arr.elem13 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 1, i32 2
  store i32 6, ptr %arr.elem13, align 4
  %arr.elem14 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 2, i32 0
  store i32 7, ptr %arr.elem14, align 4
  %arr.elem15 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 2, i32 1
  store i32 8, ptr %arr.elem15, align 4
  %arr.elem16 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 2, i32 2
  store i32 9, ptr %arr.elem16, align 4
  %arr.elem17 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 0, i32 0
  %loadtmp = load i32, ptr %arr.elem17, align 4
  %loadtmp18 = load i32, ptr %arr.elem17, align 4
  %arr.elem19 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 0, i32 1
  %loadtmp20 = load i32, ptr %arr.elem19, align 4
  %loadtmp21 = load i32, ptr %arr.elem19, align 4
  %arr.elem22 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 0, i32 2
  %loadtmp23 = load i32, ptr %arr.elem22, align 4
  %loadtmp24 = load i32, ptr %arr.elem22, align 4
  %printfCall = call i32 (ptr, ...) @printf(ptr @fmt.1, i32 %loadtmp18, i32 %loadtmp21, i32 %loadtmp24)
  %arr.elem25 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 1, i32 0
  %loadtmp26 = load i32, ptr %arr.elem25, align 4
  %loadtmp27 = load i32, ptr %arr.elem25, align 4
  %arr.elem28 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 1, i32 1
  %loadtmp29 = load i32, ptr %arr.elem28, align 4
  %loadtmp30 = load i32, ptr %arr.elem28, align 4
  %arr.elem31 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 1, i32 2
  %loadtmp32 = load i32, ptr %arr.elem31, align 4
  %loadtmp33 = load i32, ptr %arr.elem31, align 4
  %printfCall34 = call i32 (ptr, ...) @printf(ptr @fmt.2, i32 %loadtmp27, i32 %loadtmp30, i32 %loadtmp33)
  %arr.elem35 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 2, i32 0
  %loadtmp36 = load i32, ptr %arr.elem35, align 4
  %loadtmp37 = load i32, ptr %arr.elem35, align 4
  %arr.elem38 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 2, i32 1
  %loadtmp39 = load i32, ptr %arr.elem38, align 4
  %loadtmp40 = load i32, ptr %arr.elem38, align 4
  %arr.elem41 = getelementptr [3 x [3 x i32]], ptr %arr, i32 0, i32 2, i32 2
  %loadtmp42 = load i32, ptr %arr.elem41, align 4
  %loadtmp43 = load i32, ptr %arr.elem41, align 4
  %printfCall44 = call i32 (ptr, ...) @printf(ptr @fmt.3, i32 %loadtmp37, i32 %loadtmp40, i32 %loadtmp43)
  ret i32 0
}

declare i32 @printf(ptr, ...)
