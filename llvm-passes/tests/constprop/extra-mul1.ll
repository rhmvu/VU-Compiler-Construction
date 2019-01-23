; ModuleID = "edge"
target triple = "x86_64-unknown-linux-gnu"
target datalayout = ""

declare i32 @"printf"(i8* %".1", ...) 

declare i32 @"atoi"(i8* %".1") 

define i32 @"main"(i32 %"param.argc", i8** %"argv") 
{
entry:
  %"argc" = alloca i32
  store i32 %"param.argc", i32* %"argc"
  %"i" = alloca i32
  store i32 0, i32* %"i"
  br label %"entry.wcond"
entry.wcond:
  %"i.1" = load i32, i32* %"i"
  %".7" = icmp slt i32 %"i.1", 5
  br i1 %".7", label %"entry.wbody", label %"entry.wendbody"
entry.wbody:
  %"input" = alloca i32
  %"argv.ptr" = getelementptr i8*, i8** %"argv", i32 2
  %"argv.idx" = load i8*, i8** %"argv.ptr"
  %".9" = call i32 @"atoi"(i8* %"argv.idx")
  store i32 %".9", i32* %"input"
  %"i.2" = load i32, i32* %"i"
  ; CHECK: %.11 = add i32 %i.2, 1
  %".11" = add i32 %"i.2", 1
  %q = mul i32 1, %".11"
  %z = add i32 0, %q
  ; CHECK: %z = add i32 0, %.11
  %p = sub i32 %q, 0
  %n = add i32 2, %p
  ; CHECK: %n = add i32 2, %.11
  %m = sub i32 0, %n
  %o = mul i32 1, %m
  ; CHECK: %o = mul i32 1, %m
  
  store i32 %".11", i32* %"i"
  br label %"entry.wcond"
entry.wendbody:
  ret i32 0
}