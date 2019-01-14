; ModuleID = '<string>'
source_filename = "<string>"
target triple = "x86_64-unknown-linux-gnu"

@.str.0 = unnamed_addr constant [7 x i8] c"%d %d\0A\00"

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %i = alloca i32
  store i32 0, i32* %i
  br label %entry.cond

entry.cond:                                       ; preds = %entry.body.endbody, %entry
  %i.1 = load i32, i32* %i
  %.4 = icmp slt i32 %i.1, 5
  br i1 %.4, label %entry.body, label %entry.endbody

entry.body:                                       ; preds = %entry.cond
  %j = alloca i32
  store i32 1, i32* %j
  br label %entry.body.cond

entry.body.cond:                                  ; preds = %entry.body.body, %entry.body
  %j.1 = load i32, i32* %j
  %.8 = icmp slt i32 %j.1, 2
  br i1 %.8, label %entry.body.body, label %entry.body.endbody

entry.body.body:                                  ; preds = %entry.body.cond
  %.str.0 = getelementptr inbounds [7 x i8], [7 x i8]* @.str.0, i32 0, i32 0
  %i.2 = load i32, i32* %i
  %j.2 = load i32, i32* %j
  %.10 = call i32 (i8*, ...) @printf(i8* %.str.0, i32 %i.2, i32 %j.2)
  %j.3 = load i32, i32* %j
  %.11 = add i32 %j.3, 1
  store i32 %.11, i32* %j
  br label %entry.body.cond

entry.body.endbody:                               ; preds = %entry.body.cond
  %i.3 = load i32, i32* %i
  %.14 = add i32 %i.3, 1
  store i32 %.14, i32* %i
  br label %entry.cond

entry.endbody:                                    ; preds = %entry.cond
  ret i32 0
}
