; ModuleID = '<string>'
source_filename = "<string>"
target triple = "x86_64-unknown-linux-gnu"

@.str.0 = unnamed_addr constant [4 x i8] c"%d\0A\00"

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %i = alloca i32
  store i32 1, i32* %i
  br label %entry.cond

entry.cond:                                       ; preds = %entry.body.endif, %entry
  %i.1 = load i32, i32* %i
  %.4 = icmp slt i32 %i.1, 5
  br i1 %.4, label %entry.body, label %entry.endbody

entry.body:                                       ; preds = %entry.cond
  %i.2 = load i32, i32* %i
  %.6 = icmp eq i32 %i.2, 3
  br i1 %.6, label %entry.body.if, label %entry.body.endif

entry.body.if:                                    ; preds = %entry.body
  br label %entry.body.endif

entry.body.endif:                                 ; preds = %entry.body.if, %entry.body
  %.str.0 = getelementptr inbounds [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %i.3 = load i32, i32* %i
  %.9 = call i32 (i8*, ...) @printf(i8* %.str.0, i32 %i.3)
  %i.4 = load i32, i32* %i
  %.10 = add i32 %i.4, 1
  store i32 %.10, i32* %i
  br label %entry.cond

entry.endbody:                                    ; preds = %entry.cond
  ret i32 0
}
