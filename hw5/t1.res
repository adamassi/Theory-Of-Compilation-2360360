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
%t0 = load i32, i32* %val
br i1 %t0, label %label_0, label %label_1
label_0:
%t1 = getelementptr [12 x i8], [12 x i8]* @.str0, i32 0, i32 0
call void @print(i8* %t1)
br label %label_2
label_1:
%t3 = getelementptr [13 x i8], [13 x i8]* @.str1, i32 0, i32 0
call void @print(i8* %t3)
br label %label_2
label_2:
ret void
}
define void @main() {
%t5 = add i1 0, 1
call void @printByValue(i1 %t5)
%t7 = add i1 0, 0
call void @printByValue(i1 %t7)
%t9 = add i1 0, 1
br i1 %t9, label %label_3, label %label_4
label_3:
%t10 = getelementptr [5 x i8], [5 x i8]* @.str2, i32 0, i32 0
call void @print(i8* %t10)
br label %label_5
label_4:
label_5:
%t12 = add i1 0, 0
br i1 %t12, label %%label_6, label %%label_7
label_6:
br label %%label_8
label_7:
%t14 = add i1 0, 0
br i1 %t14, label %%label_9, label %%label_10
label_9:
%t16 = add i1 0, 1
br label %%label_11
label_10:
br label %%label_11
label_11:
%t15 = phi i1 [ %t16, %%label_9 ], [ 0, %%label_10 ]
br label %%label_8
label_8:
%t13 = phi i1 [ 1, %%label_6 ], [ %t15, %%label_7 ]
br i1 %t13, label %label_12, label %label_13
label_12:
%t17 = getelementptr [5 x i8], [5 x i8]* @.str3, i32 0, i32 0
call void @print(i8* %t17)
br label %label_14
label_13:
label_14:
ret void
}
