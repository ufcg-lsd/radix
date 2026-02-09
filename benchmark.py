import os
import subprocess
import time
import struct
import random
import re
import pandas as pd
import matplotlib.pyplot as plt

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
# 500MB file with RAM varying from 50MB to 1GB
FIXED_FILE_SIZE = 500                  
SCENARIO_MEMORY = [50, 100, 200, 500, 1000] 

ALGOS = ["std", "radix"]

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
    """Generates a binary file if it doesn't exist or has the wrong size."""
    expected_size = size_mb * 1024 * 1024
    if os.path.exists(INPUT_FILENAME) and os.path.getsize(INPUT_FILENAME) == expected_size:
        return 
    
    print(f"Generating {size_mb} MB dataset in {DATA_DIR}...")
    if os.path.exists(INPUT_FILENAME): 
        os.remove(INPUT_FILENAME)

    num_integers = expected_size // 8
    chunk_size = 1_000_000

    with open(INPUT_FILENAME, "wb") as f:
        remaining = num_integers
        while remaining > 0:
            batch = min(chunk_size, remaining)
            data = [random.uniform(-1e9, 1e9) for _ in range(batch)]
            f.write(struct.pack(f'{batch}d', *data))
            remaining -= batch

def parse_time(output):
    """Parses execution time from the C++ main.cpp output."""
    # Search for execution time X s in main.cpp output
    match = re.search(r"Execution time:\s*([0-9.]+)", output)
    return float(match.group(1)) if match else None 

def run_test(size_mb, mem_mb, algo):
    """Runs the ./sorter executable.""" 
    cmd = [EXECUTABLE, INPUT_FILENAME, OUTPUT_FILENAME, str(mem_mb), algo]
    
    try:
        # check=True raises an error if C++ returns != 0
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        time_taken = parse_time(result.stdout)
        
        if time_taken is None:
            print(f"Failed to parse time. Output: {result.stdout}")
            return None
            
        throughput = size_mb / time_taken # MB/s
        return time_taken, throughput
        
    except subprocess.CalledProcessError as e:
        print(f"Error executing {algo}: {e.stderr}")
        return None, None

def plot_results(df, x_col, filename, title, xlabel):
    """Generates and saves plots to the results/ directory."""
    filepath = os.path.join(RESULTS_DIR, filename)
    plt.figure(figsize=(12, 5))
    
    # Plot 1: Execution Time
    plt.subplot(1, 2, 1)
    for algo in ALGOS:
        subset = df[df['algorithm'] == algo]
        plt.plot(subset[x_col], subset['time_sec'], marker='o', label=f'{algo}')
    
    plt.title(f"{title} - Tempo Total")
    plt.xlabel(xlabel)
    plt.ylabel("Segundos (menor é melhor)")
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.legend()
    
    # Plot 2: I/O Throughput
    plt.subplot(1, 2, 2)
    for algo in ALGOS:
        subset = df[df['algorithm'] == algo]
        plt.plot(subset[x_col], subset['throughput_mbs'], marker='s', linestyle='--', label=f'{algo}')
        
    plt.title(f"{title} - I/O Throughput")
    plt.xlabel(xlabel)
    plt.ylabel("MB/s (Higher is better)")
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.legend()
    
    plt.tight_layout()
    plt.savefig(filepath)
    print(f"Plot saved: {filepath}")

def main():
    compile_cpp()
    results = []
    
    print("\n--- TEST 1: SCALABILITY (Increasing File Size) ---")
    for size in SCENARIO_SIZES:
        generate_data(size)
        for algo in ALGOS:
            print(f"-> Size={size}MB | Algo={algo}...", end=" ", flush=True)
            t, tp = run_test(size, FIXED_MEMORY, algo)
            if t:
                print(f"OK! {t:.2f}s ({tp:.0f} MB/s)")
                results.append({
                    "scenario": "scalability", "file_size_mb": size,
                    "memory_mb": FIXED_MEMORY, "algorithm": algo,
                    "time_sec": t, "throughput_mbs": tp
                })
    
    print("\n--- TEST 2: MEMORY IMPACT (Fixed 500MB File) ---") 
    generate_data(FIXED_FILE_SIZE) 
    for mem in SCENARIO_MEMORY:
        for algo in ALGOS:
            print(f"-> Mem={mem}MB | Algo={algo}...", end=" ", flush=True)
            t, tp = run_test(FIXED_FILE_SIZE, mem, algo)
            if t:
                print(f"OK! {t:.2f}s ({tp:.0f} MB/s)")
                results.append({
                    "scenario": "memory_impact", "file_size_mb": FIXED_FILE_SIZE,
                    "memory_mb": mem, "algorithm": algo,
                    "time_sec": t, "throughput_mbs": tp
                })

    df = pd.DataFrame(results)
    df.to_csv(RESULTS_CSV, index=False)

    print(f"\n **DATA SAVED IN** {RESULTS_CSV}")

    if not df.empty:
        df_scale = df[df['scenario'] == 'scalability']
        if not df_scale.empty:
            plot_results(df_scale, 'file_size_mb', 'plot_scalability.png', 
                        'Scalability', 'File Size (MB)')

        df_mem = df[df['scenario'] == 'memory_impact']
        if not df_mem.empty:
            plot_results(df_mem, 'memory_mb', 'plot_memory_impact.png', 
                        'Memory Impact', 'Memory Limit (MB)')

    if os.path.exists(INPUT_FILENAME): 
        os.remove(INPUT_FILENAME)
    if os.path.exists(OUTPUT_FILENAME): 
        os.remove(OUTPUT_FILENAME)

if __name__ == "__main__":
    main()