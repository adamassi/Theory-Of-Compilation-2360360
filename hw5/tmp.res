@.str0 = constant [12 x i8] c"val is true\00"@.str1 = constant [13 x i8] c"val is false\00"@.str2 = constant [10 x i8] c"y is here\00"@.str3 = constant [5 x i8] c"yyyy\00"@.str4 = constant [5 x i8] c"true\00"@.str5 = constant [6 x i8] c"false\00"@.str6 = constant [6 x i8] c"2true\00"@.str7 = constant [7 x i8] c"2false\00"
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
define void @printByValue(i1 %val) {
%val_ptr = alloca i1
store i1 %val, i1* %val_ptr
%y = alloca i32
%t0 = add i32 0, 0
store i32 %t0, i32* %y
%t1 = load i1, i1* %val_ptr
br i1 %t1, label %label_0, label %label_1
label_0:
%t2 = getelementptr [12 x i8], [12 x i8]* @.str0, i32 0, i32 0
call void @print(i8* %t2)
br label %label_2
label_1:
%t4 = getelementptr [13 x i8], [13 x i8]* @.str1, i32 0, i32 0
call void @print(i8* %t4)
br label %label_2
label_2:
%t6 = load i32, i32* %y
%t7 = add i32 0, 0
%t8 = icmp eq i32 %t6, %t7
br i1 %t8, label %label_3, label %label_4
label_3:
%t9 = getelementptr [10 x i8], [10 x i8]* @.str2, i32 0, i32 0
call void @print(i8* %t9)
br label %label_5
label_4:
%t11 = getelementptr [5 x i8], [5 x i8]* @.str3, i32 0, i32 0
call void @print(i8* %t11)
br label %label_5
label_5:
ret void
}
define void @main() {
%t13 = add i1 0, 1
call void @printByValue(i1 %t13)
%t15 = add i1 0, 0
call void @printByValue(i1 %t15)
%x = alloca i32
%t17 = add i32 0, 5
store i32 %t17, i32* %x
%t18 = load i32, i32* %x
%t19 = add i32 0, 5
%t20 = icmp eq i32 %t18, %t19
br i1 %t20, label %label_6, label %label_7
label_6:
%t21 = getelementptr [5 x i8], [5 x i8]* @.str4, i32 0, i32 0
call void @print(i8* %t21)
br label %label_8
label_7:
%t23 = getelementptr [6 x i8], [6 x i8]* @.str5, i32 0, i32 0
call void @print(i8* %t23)
br label %label_8
label_8:
%t25 = add i1 0, 0
br i1 %t25, label %label_9, label %label_10
label_9:
br label %label_11
label_10:
%t27 = add i1 0, 0
br i1 %t27, label %label_12, label %label_13
label_12:
%t29 = add i1 0, 1
br label %label_14
label_13:
br label %label_14
label_14:
%t28 = phi i1 [ %t29, %label_12 ], [ 0, %label_13 ]
br label %label_11
label_11:
%t26 = phi i1 [ 1, %label_9 ], [ %t28, %label_10 ]
br i1 %t26, label %label_15, label %label_16
label_15:
%t30 = getelementptr [6 x i8], [6 x i8]* @.str6, i32 0, i32 0
call void @print(i8* %t30)
br label %label_17
label_16:
%t32 = getelementptr [7 x i8], [7 x i8]* @.str7, i32 0, i32 0
call void @print(i8* %t32)
br label %label_17
label_17:
ret void
}
