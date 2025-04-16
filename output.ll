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
  %arr.idx0 = getelementptr inbounds [3 x [3 x i32]], ptr %arr, i32 0, i32 0
  %arr.idx19 = getelementptr inbounds [3 x i32], ptr %arr.idx0, i32 0
  store i32 1, ptr %arr.idx19, align 4
  %arr.idx010 = getelementptr inbounds [3 x [3 x i32]], ptr %arr, i32 0, i32 0
  %arr.idx111 = getelementptr inbounds [3 x i32], ptr %arr.idx010, i32 1
  store i32 2, ptr %arr.idx111, align 4
  %arr.idx012 = getelementptr inbounds [3 x [3 x i32]], ptr %arr, i32 0, i32 0
  %arr.idx113 = getelementptr inbounds [3 x i32], ptr %arr.idx012, i32 2
  store i32 3, ptr %arr.idx113, align 4
  %arr.idx014 = getelementptr inbounds [3 x [3 x i32]], ptr %arr, i32 0, i32 1
  %arr.idx115 = getelementptr inbounds [3 x i32], ptr %arr.idx014, i32 0
  store i32 4, ptr %arr.idx115, align 4
  %arr.idx016 = getelementptr inbounds [3 x [3 x i32]], ptr %arr, i32 0, i32 1
  %arr.idx117 = getelementptr inbounds [3 x i32], ptr %arr.idx016, i32 1
  store i32 5, ptr %arr.idx117, align 4
  %arr.idx018 = getelementptr inbounds [3 x [3 x i32]], ptr %arr, i32 0, i32 1
  %arr.idx119 = getelementptr inbounds [3 x i32], ptr %arr.idx018, i32 2
  store i32 6, ptr %arr.idx119, align 4
  %arr.idx020 = getelementptr inbounds [3 x [3 x i32]], ptr %arr, i32 0, i32 2
  %arr.idx121 = getelementptr inbounds [3 x i32], ptr %arr.idx020, i32 0
  store i32 7, ptr %arr.idx121, align 4
  %arr.idx022 = getelementptr inbounds [3 x [3 x i32]], ptr %arr, i32 0, i32 2
  %arr.idx123 = getelementptr inbounds [3 x i32], ptr %arr.idx022, i32 1
  store i32 8, ptr %arr.idx123, align 4
  %arr.idx024 = getelementptr inbounds [3 x [3 x i32]], ptr %arr, i32 0, i32 2
  %arr.idx125 = getelementptr inbounds [3 x i32], ptr %arr.idx024, i32 2
  store i32 9, ptr %arr.idx125, align 4
  %arr.idx026 = getelementptr inbounds [3 x [3 x i32]], ptr %arr, i32 0, i32 0
  %arr.idx127 = getelementptr inbounds [3 x i32], ptr %arr.idx026, i32 0
  %0 = load i32, ptr %arr.idx127, align 4
  %arr.idx028 = getelementptr inbounds [3 x [3 x i32]], ptr %arr, i32 0, i32 0
  %arr.idx129 = getelementptr inbounds [3 x i32], ptr %arr.idx028, i32 1
  %1 = load i32, ptr %arr.idx129, align 4
  %arr.idx030 = getelementptr inbounds [3 x [3 x i32]], ptr %arr, i32 0, i32 0
  %arr.idx131 = getelementptr inbounds [3 x i32], ptr %arr.idx030, i32 2
  %2 = load i32, ptr %arr.idx131, align 4
  %printfCall = call i32 (ptr, ...) @printf(ptr @fmt.1, i32 %0, i32 %1, i32 %2)
  %arr.idx032 = getelementptr inbounds [3 x [3 x i32]], ptr %arr, i32 0, i32 1
  %arr.idx133 = getelementptr inbounds [3 x i32], ptr %arr.idx032, i32 0
  %3 = load i32, ptr %arr.idx133, align 4
  %arr.idx034 = getelementptr inbounds [3 x [3 x i32]], ptr %arr, i32 0, i32 1
  %arr.idx135 = getelementptr inbounds [3 x i32], ptr %arr.idx034, i32 1
  %4 = load i32, ptr %arr.idx135, align 4
  %arr.idx036 = getelementptr inbounds [3 x [3 x i32]], ptr %arr, i32 0, i32 1
  %arr.idx137 = getelementptr inbounds [3 x i32], ptr %arr.idx036, i32 2
  %5 = load i32, ptr %arr.idx137, align 4
  %printfCall38 = call i32 (ptr, ...) @printf(ptr @fmt.2, i32 %3, i32 %4, i32 %5)
  %arr.idx039 = getelementptr inbounds [3 x [3 x i32]], ptr %arr, i32 0, i32 2
  %arr.idx140 = getelementptr inbounds [3 x i32], ptr %arr.idx039, i32 0
  %6 = load i32, ptr %arr.idx140, align 4
  %arr.idx041 = getelementptr inbounds [3 x [3 x i32]], ptr %arr, i32 0, i32 2
  %arr.idx142 = getelementptr inbounds [3 x i32], ptr %arr.idx041, i32 1
  %7 = load i32, ptr %arr.idx142, align 4
  %arr.idx043 = getelementptr inbounds [3 x [3 x i32]], ptr %arr, i32 0, i32 2
  %arr.idx144 = getelementptr inbounds [3 x i32], ptr %arr.idx043, i32 2
  %8 = load i32, ptr %arr.idx144, align 4
  %printfCall45 = call i32 (ptr, ...) @printf(ptr @fmt.3, i32 %6, i32 %7, i32 %8)
  ret i32 0
}

declare i32 @printf(ptr, ...)
