/- "Proving in the Large" chapter of CPDT -/

inductive exp : Type
| Const (n : nat) : exp
| Plus (e1 e2 : exp) : exp
| Mult (e1 e2 : exp) : exp

open exp

def exp_eval : exp → nat
| (Const n)    := n
| (Plus e1 e2) := exp_eval e1 + exp_eval e2
| (Mult e1 e2) := exp_eval e1 * exp_eval e2

def times (k : nat) : exp → exp
| (Const n)    := Const (k * n)
| (Plus e1 e2) := Plus (times e1) (times e2)
| (Mult e1 e2) := Mult (times e1) e2

attribute [simp] exp_eval times mul_add

theorem eval_times (k e) : exp_eval (times k e) = k * exp_eval e :=
by induction e; simp_using_hs

/- CPDT: induction e; crush. -/

def reassoc : exp → exp
| (Const n)    := (Const n)
| (Plus e1 e2) :=
  let e1' := reassoc e1 in
  let e2' := reassoc e2 in
  match e2' with
  | (Plus e21 e22) := Plus (Plus e1' e21) e22
  | _              := Plus e1' e2'
  end
| (Mult e1 e2) :=
  let e1' := reassoc e1 in
  let e2' := reassoc e2 in
  match e2' with
  | (Mult e21 e22) := Mult (Mult e1' e21) e22
  | _              := Mult e1' e2'
  end

attribute [simp] reassoc

lemma rewr : ∀ a b c d : nat, b * c = d → a * b * c = a * d :=
by intros; simp_using_hs

theorem reassoc_correct (e) : exp_eval (reassoc e) = exp_eval e :=
by induction e; simp; try {cases (reassoc e2); dsimp at ih_2; dsimp; simp_using_hs}
