import os
import subprocess
import time
import struct
import random
import re
import pandas as pd
import numpy as np

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

ALGOS = ["std", "radix"]

# Number of times we are running the experiment
NUM_RUNS = 10 

# Regex to extract number of runs generated 
RUN_SUMMARY_RE = re.compile(
    r"RUN_SUMMARY\s+"
    r"runs=(\d+)\s+"
    r"max_buffer_elems=(\d+)"
)


def compile_cpp():
    """Calls the root Makefile to ensure the binary exists and is up to date."""
    print("Checking compilation...")
    try:
        subprocess.check_call(["make", "sorter"], stdout=subprocess.DEVNULL)
        print("Binary up to date!")
    except subprocess.CalledProcessError:
        print("Compilation error via Makefile.") 
        exit(1)


def generate_data(size_mb):
    """This function generates random data for the tests."""
    random.seed(size_mb)

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


def parse_runs(output):
    summary = dict()
    for line in output.splitlines():
        m = RUN_SUMMARY_RE.search(line)
        if not m:
            continue
        summary["total_runs"] = int(m.group(1))
        summary["max_buffer_elems"] = int(m.group(2))

    return summary

def run_test(size_mb, mem_mb, algo):
    """Runs the ./sorter executable.""" 
    cmd = [EXECUTABLE, INPUT_FILENAME, OUTPUT_FILENAME, str(mem_mb), algo]
    

    result = subprocess.run(cmd, capture_output=True, text=True, check=True)
    time_taken = parse_time(result.stdout)
    runs_summary = parse_runs(result.stdout)
           
    throughput = size_mb / time_taken # MB/s
    
    return {
        "time_sec": time_taken,
        "total_runs": runs_summary.get("total_runs", 0),
        "max_buffer_elems": runs_summary.get("max_buffer_elems"),
        "throughput_mb_s": throughput
    }


def run_scalability():
    """Varies file size with fixed memory."""
    print("\n--- TEST 1: SCALABILITY (Increasing File Size) ---")

    results = []
    for size in SCENARIO_SIZES:
        generate_data(size)
        for algo in ALGOS:
            stats = run_batch_execution("scalability", size, FIXED_MEMORY, algo)
            if stats:
                results.extend(stats)

    return results


def run_memory_impact():
    """Varies memory limit with fixed file size."""
    print("\n--- TEST 2: MEMORY IMPACT (Fixed 500MB File) ---")

    generate_data(FIXED_FILE_SIZE)
    results = []
    for mem in SCENARIO_MEMORY:
        for algo in ALGOS:
            stats = run_batch_execution("memory_impact", FIXED_FILE_SIZE, mem, algo)
            if stats:
                results.extend(stats)

    return results


def run_batch_execution(scenario, size_mb, mem_mb, algo):
    """Runs N times and returns raw per-run measurements."""
    print(f"  -> {algo.upper()}: [Size={size_mb}MB, Mem={mem_mb}MB]:")
    
    rows = []
    for run_idx in range(NUM_RUNS):
        drop_cache()

        res = run_test(size_mb, mem_mb, algo)
        if res is None:
            continue

        rows.append({
            "scenario": scenario,
            "file_size_mb": size_mb,
            "memory_mb": mem_mb,
            "algorithm": algo,
            "run_id": run_idx + 1,
            "time_sec": res["time_sec"],
            "throughput_mb_s": res["throughput_mb_s"],
            "total_runs": res["total_runs"],
            "max_buffer_elems": res["max_buffer_elems"]
        })

        print(f"    Run {run_idx + 1}/{NUM_RUNS}: "
            f"{res['time_sec']:.3f}s "
            f"(Throughput: {res['throughput_mb_s']:.2f} MB/s)") 
    
    print(" Done.")
    return rows


def drop_cache():
    subprocess.run(["sudo", "sh", "-c", "echo 3 > /proc/sys/vm/drop_caches"], check=True)
    time.sleep(1)


def run_benchmark():
    """Runs External Merge Sort benchmark.

    This function runs the External Merge Sort algorithm and performs a
    benchmark comparing the External Merge Sort + Radix with the vanilla
    External Merge Sort.
    """
    res_scale = run_scalability()
    res_mem = run_memory_impact()
    
    df = pd.DataFrame(res_scale + res_mem)
    df.to_csv(RESULTS_CSV, index=False)
    print(f"\n[OK] Results saved to {RESULTS_CSV}.")

    if df.empty:
        raise RuntimeError("Error while executing the benchmark.")


def main():
    compile_cpp()
    run_benchmark()


if __name__ == "__main__":
    main()
