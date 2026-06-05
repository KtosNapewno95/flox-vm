# *tested on python 3.12
import time
start_ns = time.perf_counter_ns()

for i in range(10001):
    print("Siema!")

end_ns = time.perf_counter_ns()
print(f"\n[Python]: {(end_ns - start_ns) // 1000} mikrosekund")
