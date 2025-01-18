
declare i32 @scanf(i8*, ...)
declare i32 @printf(i8*, ...)
declare void @exit(i32)
@.int_specifier_scan = constant [3 x i8] c"%d\00"
@.int_specifier = constant [4 x i8] c"%d\0A\00"
@.str_specifier = constant [4 x i8] c"%s\0A\00"
define i32 @readi(i32) {
    %ret_val = alloca i32
    %spec_ptr = getelementptr [3 x i8], [3 x i8]* @.int_specifier_scan, i32 0, i32 0
    call i32 (i8*, ...) @scanf(i8* %spec_ptr, i32* %ret_val)
    %val = load i32, i32* %ret_val
    ret i32 %val
}
define void @printi(i32) {
    %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0
    call i32 (i8*, ...) @printf(i8* %spec_ptr, i32 %0)
    ret void
}
define void @print(i8*) {
    %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0
    call i32 (i8*, ...) @printf(i8* %spec_ptr, i8* %0)
    ret void
}
define void @main() {
%y = alloca i32
%t0 = add i32 0, 5
store i32 %t0, i32* %y
%z = alloca i32
%t1 = add i32 0, 5
store i32 %t1, i32* %z
%b = alloca i32
%t2 = add i32 0, 2
%t3 = load i32, i32* %y
%t4 = load i32, i32* %z
%t5 = sub i32 %t3, %t4
br i1 %t5, label %div_ok, label %div_err
iv_ok:
%t6 = sdiv i32 %t2, %t5
iv_err:
call void @print(i8* getelementptr ([21 x i8], [21 x i8]* @.str, i32 0, i32 0))
call void @exit(i32 1)
store i32 %t6, i32* %b
}
