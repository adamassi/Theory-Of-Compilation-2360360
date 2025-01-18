@.str0 = constant [12 x i8] c"val is true\00"@.str1 = constant [13 x i8] c"val is false\00"@.str2 = constant [5 x i8] c"true\00"@.str3 = constant [5 x i8] c"true\00"
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
define void @printByValue(i1) {
%val = alloca i32
store i32* %val, i32* %val
%t0 = load i32, i32* %val
br i1 %t0, label %%label_0, label %%label_1
label_0:
%t1 = call void @print(@.str0)
br label %%label_2
label_1:
%t2 = call void @print(@.str1)
br label %%label_2
label_2:
}
define void @main() {
%t3 = add i1 0, 1
%t4 = call void @printByValue(%t3)
%t5 = add i1 0, 0
%t6 = call void @printByValue(%t5)
%t7 = add i1 0, 1
br i1 %t7, label %%label_3, label %%label_4
label_3:
%t8 = call void @print(@.str2)
br label %%label_5
label_4:
br label %%label_5
label_5:
%t9 = add i1 0, 0
br i1 %t9, label %%label_6, label %%label_7
label_6:
br label %%label_8
label_7:
%t11 = add i1 0, 0
br i1 %t11, label %%label_9, label %%label_10
label_9:
%t13 = add i1 0, 1
br label %%label_11
label_10:
br label %%label_11
label_11:
%t12 = phi i1 [ %t13, %%label_9 ], [ 0, %%label_10 ]
br label %%label_8
label_8:
%t10 = phi i1 [ 1, %%label_6 ], [ %t12, %%label_7 ]
br i1 %t10, label %%label_12, label %%label_13
label_12:
%t14 = call void @print(@.str3)
br label %%label_14
label_13:
br label %%label_14
label_14:
}
