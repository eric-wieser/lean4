mutual
  def isEven : Nat → Bool
    | 0 => true
    | n+1 => isOdd n
  decreasing_by apply Nat.lt_succ_self
  def isOdd : Nat → Bool
    | 0 => false
    | n+1 => isEven n
  decreasing_by apply Nat.lt_succ_self
end

theorem isEven_double (x : Nat) : isEven (2 * x) = true := by
  induction x with
  | zero => simp [isEven]
  | succ x ih =>
    unfold isEven
    trace_state
    rw [Nat.mul_succ, Nat.add_succ]
    simp
    unfold isOdd
    trace_state
    simp
    exact ih

theorem isEven_succ_succ (x : Nat) : isEven (x + 2) = isEven x := by
  conv => lhs; unfold isEven; simp; unfold isOdd; simp
