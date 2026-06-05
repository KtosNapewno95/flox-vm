import time

# ====================================================================
# [Flox Standard Library Benchmark - math.py]
# Pythonowa emulacja rejestrów i logiki Floxa do testu wydajności
# ====================================================================

# Najlepszy możliwy timer (sprzętowy licznik o najwyższej rozdzielczości)
start_time = time.perf_counter_ns()

# --- PODSTAWOWE STAŁE SYSTEMOWE ---
MATH_ZERO        = 0
MATH_JEDEN       = 1
MATH_MINUS_JEDEN = -1

# --- REJESTRY WEJŚCIOWE DLA UŻYTKOWNIKA ---
math_a           = 0
math_b           = 0

# --- FLAGI STERUJĄCE (SYMULACJA STANU JAK WE FLOXIE) ---
# Wszystkie flagi ustawione na 0 na starcie, aby sprawdzić narzut samych if-ów
math_do_base     = 0
math_do_mod      = 0
math_do_abs      = 0
math_do_pow      = 0
math_do_sgn      = 0
math_do_minmax   = 0
math_do_rand     = 0
math_do_lerp     = 0
math_do_pow2     = 0

# --- REJESTRY WYNIKOWE DLA UŻYTKOWNIKA ---
math_wynik_add     = 0
math_wynik_sub     = 0
math_wynik_mul     = 0
math_wynik_div     = 0
math_mod_wynik     = 0
math_abs_wynik     = 0
math_pow_wynik     = 0
math_sgn_wynik     = 0
math_min_wynik     = 0
math_max_wynik     = 0
math_rand_wynik    = 0
math_lerp_wynik    = 0
math_pow2_wynik    = 0

# --- SZYBKI GENERATOR LOSOWY (LCG) ---
MATH_RAND_SEED     = 123456789
MATH_RAND_MNOZNIK  = 1664525
MATH_RAND_PRZYROST = 1013904223
MATH_RAND_MODULUS  = 1073741824
math_rand_zakres   = 1000001

# --- REJESTRY WEJŚCIOWE DLA ZAAWANSOWANYCH FUNKCJI ---
math_rand_min      = 0
math_rand_max      = 0
math_lerp_start    = 0
math_lerp_end      = 0
math_lerp_t        = 0
math_pow2_n        = 0

# --- REJESTRY POMOCNICZE ---
math_tmp_div       = 0
math_tmp_mul       = 0
math_pow_i         = 0
math_tmp_diff      = 0

# ====================================================================
# SEKCJE WYKONAWCZE (SPRAWDZANIE WARUNKÓW)
# ====================================================================

if math_do_base == MATH_JEDEN:
    math_wynik_add = math_a + math_b
    math_wynik_sub = math_a - math_b
    math_wynik_mul = math_a * math_b
    math_wynik_div = math_a // math_b if math_b != 0 else 0
    math_do_base = MATH_ZERO

if math_do_mod == MATH_JEDEN:
    math_tmp_div = math_a // math_b
    math_tmp_mul = math_tmp_div * math_b
    math_mod_wynik = math_a - math_tmp_mul
    math_do_mod = MATH_ZERO

if math_do_abs == MATH_JEDEN:
    math_abs_wynik = math_a
    if math_a < MATH_ZERO:
        math_abs_wynik = math_a * MATH_MINUS_JEDEN
    math_do_abs = MATH_ZERO

if math_do_pow == MATH_JEDEN:
    math_pow_wynik = MATH_JEDEN
    math_pow_i = MATH_ZERO
    while math_pow_i < math_b:
        math_pow_wynik = math_pow_wynik * math_a
        math_pow_i = math_pow_i + MATH_JEDEN
    math_do_pow = MATH_ZERO

if math_do_sgn == MATH_JEDEN:
    math_sgn_wynik = MATH_ZERO
    if math_a < MATH_ZERO: math_sgn_wynik = MATH_MINUS_JEDEN
    if MATH_ZERO < math_a: math_sgn_wynik = MATH_JEDEN
    math_do_sgn = MATH_ZERO

if math_do_minmax == MATH_JEDEN:
    math_min_wynik = math_a
    if math_b < math_a: math_min_wynik = math_b
    math_max_wynik = math_a
    if math_a < math_b: math_max_wynik = math_b
    math_do_minmax = MATH_ZERO

if math_do_rand == MATH_JEDEN:
    math_tmp_diff = math_rand_max - math_rand_min
    math_tmp_diff = math_tmp_diff + MATH_JEDEN
    math_tmp_div = MATH_RAND_SEED * MATH_RAND_MNOZNIK
    math_tmp_mul = math_tmp_div + MATH_RAND_PRZYROST
    math_pow_i = math_tmp_mul // MATH_RAND_MODULUS
    math_pow_wynik = math_pow_i * MATH_RAND_MODULUS
    MATH_RAND_SEED = math_tmp_mul - math_pow_wynik
    math_tmp_div = MATH_RAND_SEED // math_tmp_diff
    math_tmp_mul = math_tmp_div * math_tmp_diff
    math_rand_wynik = MATH_RAND_SEED - math_tmp_mul
    math_rand_wynik = math_rand_wynik + math_rand_min
    math_do_rand = MATH_ZERO

if math_do_lerp == MATH_JEDEN:
    math_tmp_diff = math_lerp_end - math_lerp_start
    math_tmp_mul = math_tmp_diff * math_lerp_t
    math_tmp_div = math_tmp_mul // 100
    math_lerp_wynik = math_lerp_start + math_tmp_div
    math_do_lerp = MATH_ZERO

if math_do_pow2 == MATH_JEDEN:
    math_pow2_wynik = MATH_JEDEN
    math_pow_i = MATH_ZERO
    while math_pow_i < math_pow2_n:
        math_pow2_wynik = math_pow2_wynik * 2
        math_pow_i = math_pow_i + MATH_JEDEN
    math_do_pow2 = MATH_ZERO

# Koniec pomiaru czasu
end_time = time.perf_counter_ns()
elapsed_micros = (end_time - start_time) / 1000

print(f"[Python VM Performance]: Kod wykonany w {elapsed_micros:.2f} mikrosekund!")
