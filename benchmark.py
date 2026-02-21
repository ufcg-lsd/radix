import os
import subprocess
import time
import struct
import random
import re
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import scipy

EXECUTABLE = "./sorter"     

DATA_DIR = "build" 
RESULTS_DIR = "results" 

os.makedirs(DATA_DIR, exist_ok=True)
os.makedirs(RESULTS_DIR, exist_ok=True)

INPUT_FILENAME = os.path.join(DATA_DIR, "input_bench.bin")
OUTPUT_FILENAME = os.path.join(DATA_DIR, "output_bench.bin")
RESULTS_CSV = os.path.join(RESULTS_DIR, "benchmark_data.csv")

# Scenario 1: Scalability (Fixed Memory, Increasing File Size)
# Testing file sizes from 100MB up to 1GB
SCENARIO_SIZES = [100, 200, 500, 1000] 
FIXED_MEMORY = 100                     

# Scenario 2: Memory Impact (Fixed File Size, Increasing RAM)
# 500MB file with RAM varying from 50MB to 500MB 
FIXED_FILE_SIZE = 500                  
SCENARIO_MEMORY = [50, 100, 200, 500] 

# Sorting algorithms
ALGOS = ["std", "radix"]

# Number of times we are running the experiment
NUM_RUNS = 10 


def compile_cpp():
    """Calls the root Makefile to ensure the binary exists and is up to date."""
    print("Checking compilation...")
    try:
        subprocess.check_call(["make", "sorter"], stdout=subprocess.DEVNULL)
        print("Binary up to date!")
    except subprocess.CalledProcessError:
        print("Compilation error via Makefile. :(")
        exit(1)


def generate_data(size_mb):
    """This function generates random data for the tests."""
    expected_size = size_mb * 1024 ** 2

    if os.path.exists(INPUT_FILENAME):
        if os.path.getsize(INPUT_FILENAME) == expected_size:
            return
        
        os.remove(INPUT_FILENAME)
    
    print(f"Generating {size_mb} MB dataset in {DATA_DIR}...")

    chunk_size = 1_000_000
    num_elements = expected_size // 8
    remaining = num_elements
    with open(INPUT_FILENAME, "wb") as f:
        while remaining > 0:
            batch_size = min(chunk_size, remaining)
            data = [random.uniform(-1e9, 1e9) for _ in range(batch_size)]
            f.write(struct.pack(f"{batch_size}d", *data))
            remaining -= batch_size


def parse_time(output):
    """Extract execution time from the C++ main.cpp output."""
    match = re.search(r"Execution time:\s*([0-9.]+)", output)
    return float(match.group(1)) if match else None 


def run_test(size_mb, mem_mb, algo):
    """Runs the ./sorter executable.""" 
    cmd = [EXECUTABLE, INPUT_FILENAME, OUTPUT_FILENAME, str(mem_mb), algo]
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        time_taken = parse_time(result.stdout)
        
        if time_taken is None:
            print(f"Failed to parse time. Output: {result.stdout}")
            return None, None
            
        throughput = size_mb / time_taken # MB/s
        return time_taken, throughput
        
    except subprocess.CalledProcessError as e:
        print(f"Error executing {algo}: {e.stderr}")
        return None, None


def run_scalability():
    """Varies file size with fixed memory."""
    print("\n--- TEST 1: SCALABILITY (Increasing File Size) ---")

    results = []
    for size in SCENARIO_SIZES:
        generate_data(size)
        for algo in ALGOS:
            stats_res = run_batch_execution("scalability", size, FIXED_MEMORY, algo)
            if stats_res:
                results.append(stats_res)

    return results


def run_memory_impact():
    """Varies memory limit with fixed file size."""
    print("\n--- TEST 2: MEMORY IMPACT (Fixed 500MB File) ---")

    generate_data(FIXED_FILE_SIZE)
    results = []
    for mem in SCENARIO_MEMORY:
        for algo in ALGOS:
            stats_res = run_batch_execution("memory_impact", FIXED_FILE_SIZE, mem, algo)
            if stats_res:
                results.append(stats_res)

    return results


def run_batch_execution(scenario, size_mb, mem_mb, algo):
    """Runs N times and returns mean and standard deviation."""
    print(f"  -> {algo.upper()}: [Size={size_mb}MB, Mem={mem_mb}MB]:")
    
    times = []
    for run_idx in range(NUM_RUNS):
        drop_cache()
        time_taken, throughput = run_test(size_mb, mem_mb, algo)
        if time_taken is not None:
            times.append(time_taken)
            print(f"    Run {run_idx + 1}/{NUM_RUNS}: {time_taken:.3f}s (Throughput: {throughput:.2f} MB/s)")
    
    mean = np.mean(times)
    std = np.std(times, ddof=1)
    
    t_crit = scipy.stats.t.ppf(0.975, len(times) - 1)
    ci = t_crit * (std / np.sqrt(len(times)))
    
    print(" Done.")
    return {
        "scenario": scenario,
        "file_size_mb": size_mb,
        "memory_mb": mem_mb,
        "algorithm": algo,
        "time_mean": mean,
        "time_std": std,
        "time_ci": ci,
        "throughput_mean": size_mb / mean
    }


def plot_efficiency(df, x_col, filename, title):
    """Plots percentage gain of Radix vs Std based on mean times."""
    plt.figure(figsize=(10, 6))
    
    pivot = df.pivot(index=x_col, columns='algorithm', values='time_mean')
    pivot['gain'] = ((pivot['std'] - pivot['radix']) / pivot['std']) * 100
    
    plt.plot(pivot.index, pivot['gain'], marker='D', color='green', linewidth=2)
    
    for x, y in zip(pivot.index, pivot['gain']):
        plt.annotate(f'{y:.1f}%', (x, y), textcoords="offset points", 
                     xytext=(0,10), ha='center', weight='bold')

    plt.title(title)
    plt.xlabel(x_col.replace('_', ' ').title())
    plt.ylabel("Performance Gain (%)")
    plt.grid(True, linestyle='--', alpha=0.7)
    
    path = os.path.join(RESULTS_DIR, filename)
    plt.savefig(path)
    plt.close()


def plot_execution_times(df, x_col, filename, title, xlabel):
    """
    Plots absolute time with a 95% Confidence Interval shaded area.
    Matches the visual style of the provided reference.
    """
    plt.figure(figsize=(8, 6))
    
    colors = {'std': '#1f77b4', 'radix': '#ff7f0e'}
    
    for algo in ALGOS:
        subset = df[df['algorithm'] == algo].sort_values(by=x_col)
        x = subset[x_col]
        y = subset['time_mean']
        ci = subset['time_ci']

        plt.plot(x, y, marker='o', label=algo, color=colors[algo], linewidth=2)
        
        plt.fill_between(x, (y - ci), (y + ci), color=colors[algo], alpha=0.15)

    plt.title(title, fontsize=13, fontweight='bold')
    plt.xlabel(xlabel, fontsize=11)
    plt.ylabel("Seconds (s)", fontsize=11)
    
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.legend()
    
    plt.tight_layout()
    plt.savefig(os.path.join(RESULTS_DIR, filename), dpi=300)
    plt.close()


def drop_cache():
    subprocess.run(["sudo", "sh", "-c", "echo 3 > /proc/sys/vm/drop_caches"], check=True)
    time.sleep(1)


def run_benchmark():
    res_scale = run_scalability()
    res_mem = run_memory_impact()
    
    df = pd.DataFrame(res_scale + res_mem)
    df.to_csv(RESULTS_CSV, index=False)
    print(f"\n[DATA] Results saved to {RESULTS_CSV}")

    if df.empty:
        raise RuntimeError("Error while executing the benchmark.")
    
    df_scale = df[df['scenario'] == 'scalability']
    if not df_scale.empty:
        plot_execution_times(df_scale, 'file_size_mb', 'scalability_time.png', 
                             'Scalability - Total Time (95% CI)', 'File Size (MB)')
        plot_efficiency(df_scale, 'file_size_mb', 'efficiency_scalability.png', 
                        'Radix Sort Speedup vs Input Size')

    df_mem = df[df['scenario'] == 'memory_impact']
    if not df_mem.empty:
        plot_execution_times(df_mem, 'memory_mb', 'memory_time.png', 
                            'Memory Impact - Total Time (95% CI)', 'Memory Limit (MB)')
        plot_efficiency(df_mem, 'memory_mb', 'efficiency_memory.png', 
                        'Radix Sort Speedup vs Memory Limit')
        
    if os.path.exists(INPUT_FILENAME): 
        os.remove(INPUT_FILENAME)
    if os.path.exists(OUTPUT_FILENAME): 
        os.remove(OUTPUT_FILENAME)


def main():
    compile_cpp()
    run_benchmark()


if __name__ == "__main__":
    main()
