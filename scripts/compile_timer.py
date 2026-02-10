#!/usr/bin/env python3
"""
Compile-time benchmark wrapper for CMake CXX_COMPILER_LAUNCHER.
Measures compilation time and memory usage.
"""
import subprocess
import sys
import time
import os
import json
import shutil

def find_time_command():
    """Find the GNU time command."""
    for cmd in ['/usr/bin/time', 'gtime', 'time']:
        path = shutil.which(cmd)
        if path and cmd != 'time':  # Avoid shell builtin
            return path
    return None

def run_with_time(cmd, time_cmd):
    """Run command with time wrapper and parse output."""
    if time_cmd:
        full_cmd = [time_cmd, '-v'] + cmd
    else:
        full_cmd = cmd
    
    start = time.perf_counter()
    result = subprocess.run(full_cmd, capture_output=True, text=True)
    elapsed = time.perf_counter() - start
    
    return result, elapsed

def parse_time_output(stderr):
    """Parse GNU time -v output for memory usage."""
    max_rss = None
    for line in stderr.split('\n'):
        if 'Maximum resident set size' in line:
            parts = line.split()
            if len(parts) >= 6:
                try:
                    max_rss = int(parts[-2])  # Value in KB
                except ValueError:
                    pass
    return max_rss

def main():
    if len(sys.argv) < 2:
        print("Usage: compile_timer.py <compiler> [args...]", file=sys.stderr)
        return 1
    
    compiler = sys.argv[1]
    args = sys.argv[2:]
    cmd = [compiler] + args
    
    time_cmd = find_time_command()
    
    result, elapsed = run_with_time(cmd, time_cmd)
    
    mem_kb = None
    if time_cmd:
        mem_kb = parse_time_output(result.stderr)
    
    # Output JSON for parsing
    output = {
        "compile_time_ms": round(elapsed * 1000, 1),
        "compiler_memory_kb": mem_kb,
        "return_code": result.returncode,
        "command": ' '.join(cmd[:3]) + ' ...' if len(cmd) > 3 else ' '.join(cmd)
    }
    
    print(json.dumps(output), file=sys.stderr)
    
    # Also print human-readable to stdout for CMake
    print(f"COMPILE_TIME_MS={output['compile_time_ms']:.1f}")
    if mem_kb:
        print(f"COMPILE_MEM_KB={mem_kb}")
    else:
        print("COMPILE_MEM_KB=N/A")
    
    # Forward compiler output
    if result.stdout:
        print(result.stdout)
    if result.stderr:
        print(result.stderr, file=sys.stderr)
    
    return result.returncode

if __name__ == '__main__':
    sys.exit(main())