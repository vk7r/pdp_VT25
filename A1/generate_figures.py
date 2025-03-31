import matplotlib.pyplot as plt
import numpy as np

data = {
    100: [0.000043, 0.005805, 0.000238, 0.000169],
    1000: [0.000044, 0.015918, 0.000208, 0.000245],
    10000: [0.000047, 0.000147, 0.000208, 0.000344],
    100000: [0.000102, 0.006059, 0.000408, 0.000533],
    4194304: [0.002427, 0.007939, 0.010665, 0.012254],
}

array_sizes = list(data.keys())
num_processes = [2, 4, 8]
speedups = {p: [] for p in num_processes}

for size, times in data.items():
    sequential_time = times[0]
    for i, p in enumerate(num_processes):
        speedups[p].append(sequential_time / times[i + 1])

plt.figure(figsize=(10, 6))
for p in num_processes:
    plt.plot(array_sizes, speedups[p], marker='o', label=f'{p} Processes')

plt.xscale('log')
plt.xlabel('Array Size')
plt.ylabel('Speedup')
plt.title('Speedup vs Array Size')
plt.legend()
plt.grid(True, which='both', linestyle='--', linewidth=0.5)
plt.show()
