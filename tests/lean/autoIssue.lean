import Lean
structure A :=
  x : Nat
  a' : x = 1 := by trivial

#check @A.a'

example (z : A) : z.x = 1 := by
  have := z.a'
  trace_state
  exact this

example (z : A) : z.x = 1 := by
  have := z.2
  trace_state
  exact this

#check @A.rec

example (z : A) : z.x = 1 := by
  cases z; rename_i x a'
  trace_state
  subst a'
  rfl

example (z : A) : z.x = 1 := by
  induction z with
  | mk x a' =>
    trace_state
    subst a'
    rfl

structure B :=
  x : Nat
  y : Nat := 2

example (b : B) : b = { x := b.x, y := b.y } := by
  cases b with
  | mk x y => trace_state; rfl

open Lean
open Lean.Meta

def tst : MetaM Unit :=
  withLocalDeclD `a (mkConst ``A) fun a => do
    let e := mkProj ``A 1 a
    IO.println (← Meta.ppExpr (← inferType e))

#eval tst
