# ECE-GY 6913 – Computing Systems Architecture (Fall 2026)

C++ simulators for the course labs. Each lab folder is self-contained, with its own source, `Makefile`, input files, assignment PDF, and `expected_results/` for checking output.

| Lab | Folder | Description |
|-----|--------|---------|
| 1 | `Lab_1_Single_Cycle_MIPS` | Single-cycle MIPS processor simulator |
| 2 | `Lab_2_MIPS_5_Stage_Pipeline_Simulator` | 5-stage pipelined MIPS simulator (hazards, forwarding, stalls) |
| 3 | `Lab_3_Cache_Simulator` | Two-level cache simulator driven by a memory trace |
| 4 | `Lab_4_Branch_Prediction` | Branch predictor simulator driven by a branch trace |

## Requirements

- `g++` with C++11 support

## Build and run

Run all commands from inside the lab folder.

### Lab 1 – Single-cycle MIPS
```
make            # g++ MIPS.cpp -o MIPS
./MIPS          # reads imem.txt, dmem.txt
```
Outputs: `RFresult.txt`, `dmemresult.txt`. Compare with `expected_results/`. `make clean` removes the binary.

### Lab 2 – 5-stage pipeline
```
make            # g++ MIPS_pipeline.cpp -o MIPS_pipeline
./MIPS_pipeline # reads imem.txt, dmem.txt
```
Outputs: `RFresult.txt`, `dmemresult.txt`, `stateresult.txt`. Compare with `expected_results/`. `make clean` removes the binary.

Note: the register-file and state outputs are opened in append mode, so delete old result files before re-running.

### Lab 3 – Cache simulator
```
make            # g++ -g cachesimulator.cpp -o cachesimulator.out
./cachesimulator.out cacheconfig.txt trace.txt
```
Output: `trace.txt.out`. Compare with `expected_results/trace.txt.out.ans.txt`.

### Lab 4 – Branch prediction
```
make            # g++ -g branchsimulator.cpp -o branchsimulator.out
./branchsimulator.out config.txt trace.txt
```
Output: `trace.txt.out`. Compare with `expected_results/trace.txt.out.ans.txt`.
