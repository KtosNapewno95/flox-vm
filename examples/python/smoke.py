import time

start_time = time.perf_counter_ns()

# --- PODSTAWOWE STAŁE SYSTEMOWE ---
MATH_ZERO        = 0
MATH_JEDEN       = 1
MATH_MINUS_JEDEN = -1

MATH_RAND_SEED     = 123456789
MATH_RAND_MNOZNIK  = 1664525
MATH_RAND_PRZYROST = 1013904223
MATH_RAND_MODULUS  = 1073741824
math_rand_zakres   = 1000001

print("--- [Python] TYTANICZNY TEST 2M CZASTECZEK ---")

liczba_czasteczek = 2000000

Czasteczki_X = [0] * liczba_czasteczek
Czasteczki_Y = [0] * liczba_czasteczek
Czasteczki_Z = [0] * liczba_czasteczek

cel_x = 500
cel_y = 1000
cel_z = 500
procent_czasu = 35

print("-> Rozpoczynam tytaniczne obliczenia dla 2 000 000 obiektów...")

i = 0
while i < liczba_czasteczek:
    math_rand_min = -200
    math_rand_max = 200
    
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
    pozycja_start_x = math_rand_wynik + math_rand_min

    math_lerp_start = pozycja_start_x
    math_lerp_end = cel_x
    math_lerp_t = procent_czasu
    
    math_tmp_diff = math_lerp_end - math_lerp_start
    math_tmp_mul = math_tmp_diff * math_lerp_t
    math_tmp_div = math_tmp_mul // 100
    nowe_x = math_lerp_start + math_tmp_div

    math_abs_a = nowe_x
    math_abs_wynik = math_abs_a
    if math_abs_a < MATH_ZERO:
        math_abs_wynik = math_abs_a * MATH_MINUS_JEDEN
    finalne_x = math_abs_wynik

    Czasteczki_X[i] = finalne_x
    Czasteczki_Y[i] = cel_y
    Czasteczki_Z[i] = cel_z

    i = i + MATH_JEDEN

end_time = time.perf_counter_ns()
elapsed_micros = (end_time - start_time) / 1000

print(f"\n[Python VM Performance]: Kod wykonany w {elapsed_micros:.2f} mikrosekund!")
