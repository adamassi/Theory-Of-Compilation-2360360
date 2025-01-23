@.str0 = constant [5 x i8] c"true\00"@.str1 = constant [6 x i8] c"false\00"
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
@divide_by_zero_message = constant [23 x i8] c"Error division by zero\00"
define void @divide_by_zero_error() {
    %divide_by_zero_ptr = getelementptr [23 x i8], [23 x i8]* @divide_by_zero_message, i32 0, i32 0
    call void @print(i8* %divide_by_zero_ptr)
    call void @exit(i32 0)
    ret void
}
define void @main() {
%t0 = add i1 0, 1
br i1 %t0, label %label_0, label %label_1
label_0:
%t2 = add i1 0, 0
br i1 %t2, label %label_3, label %label_4
label_3:
br label %label_5
label_4:
%t4 = add i1 0, 1
br label %label_5
label_5:
%t3 = phi i1 [ 1, %label_3 ], [ %t4, %label_4 ]
br label %label_2
label_1:
br label %label_2
label_2:
%t1 = phi i1 [ %t3, %label_5], [ 0, %label_1 ]
br i1 %t1, label %label_6, label %label_7
label_6:
%t6 = add i1 0, 1
br i1 %t6, label %label_9, label %label_10
label_9:
br label %label_11
label_10:
%t8 = add i1 0, 0
br label %label_11
label_11:
%t7 = phi i1 [ 1, %label_9 ], [ %t8, %label_10 ]
br label %label_8
label_7:
br label %label_8
label_8:
%t5 = phi i1 [ %t7, %label_11], [ 0, %label_7 ]
br i1 %t5, label %label_12, label %label_13
label_12:
%t9 = getelementptr [5 x i8], [5 x i8]* @.str0, i32 0, i32 0
call void @print(i8* %t9)
br label %label_14
label_13:
%t11 = getelementptr [6 x i8], [6 x i8]* @.str1, i32 0, i32 0
call void @print(i8* %t11)
br label %label_14
label_14:
ret void
}
