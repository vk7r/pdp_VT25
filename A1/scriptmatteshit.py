import numpy as np
import matplotlib.pyplot as plt

# Define the efficiency functions
def efficiency(n, p):
    return n / (n + p * np.log2(p))

def new_efficiency(n, p, k, x):
    return (x * n) / (x * n + k * p * np.log2(k * p))

# Parameters for plotting
p_values = np.linspace(1, 20, 100)  # Range of p values
n = 100  # Initial problem size

# Scaling factor k for increasing the number of processors
k = 4  # Increase processors by a factor of 2

# Generate efficiency values for both scenarios
efficiency_values = efficiency(n, p_values)
scaled_efficiency_values = new_efficiency(n, p_values, k, 1)  # Assume initial x = 1 for now

# Plot the efficiency curves
plt.figure(figsize=(10, 6))
plt.plot(p_values, efficiency_values, label=f'Original Efficiency (n={n})', color='blue')
plt.plot(p_values, scaled_efficiency_values, label=f'New Efficiency (p={k}*p)', color='red', linestyle='dashed')

# Add labels and title
plt.xlabel('Number of Processors (p)')
plt.ylabel('Efficiency (E)')
plt.title('Efficiency before and after increasing p by a factor of k')
plt.legend()
plt.grid(True)

# Show the plot
plt.show()
