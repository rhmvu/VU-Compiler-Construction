define i32 @main() {
entry:
  br label %entry.while.cond

entry.while.cond:                                 ; preds = %entry.while.body, %entry
  %i.0 = phi i32 [ 1, %entry ], [ %.7, %entry.while.body ]
  %.6 = icmp slt i32 %i.0, 10
  br i1 %.6, label %entry.while.body, label %entry.endwhile

entry.while.body:                                 ; preds = %entry.while.cond
  %.7 = add i32 %i.0, 1
  %x = add i32 2, 2
  %y = add i32 0, %x
  %z = mul i32 1, %y
  ; CHECK: %.7 = add i32 %i.0, 1
  %q = sub i32 %z, 0
  ; CHECK: %q = sub i32 4, 0
  br label %entry.while.cond

entry.endwhile:                                   ; preds = %entry.while.cond
  ret i32 0
}
