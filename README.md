# Password Storage Benchmark

This project is a **File Organization** benchmark application designed to measure and compare the performance impacts of storing passwords using different methods. The project is written in C.

## How It Works

When executed, the program sequentially performs the following 3 main steps:

1. **Data Preprocessing:** It reads raw password datasets from the `RawData` directory, cleans up duplicate and invalid entries, and writes the refined data into the `Processed` directory.
2. **Storage Allocation:** The processed passwords are saved into the `Storage` directory using 3 different formats:
   - **Raw:** As plain text (`passwords.txt`)
   - **Hashed:** Encrypted using the MD5 hashing algorithm (`passwords_md5.txt`)
   - **Compressed:** Compressed using the Zlib library (`passwords.zlib`)
3. **Benchmarking:** It randomly selects 100 passwords from the processed data and searches for these passwords across all 3 storage formats to measure search times. The results and performance metrics are saved to `Results/results_summary.txt`.

## Project Structure

* **Code/**: Contains the C source code (`.c`, `.h`), `Makefile`, and external libraries (md5, miniz).
* **RawData/**: Directory containing the raw password datasets to be processed.
* **Processed/**: Directory where cleaned and processed passwords are kept.
* **Storage/**: Directory where passwords are stored in 3 different formats (Raw, Hashed, Compressed).
* **Results/**: Directory where search benchmarks and performance test results are saved.
* **Report/**: Contains the project report (`Report.pdf`).

## How to Run

1. Use the `Makefile` or `compile.bat` file inside the `Code` folder to compile the project.
2. Run the generated executable file (`Project.exe`) in the root directory.
3. Once the execution is complete, check the `Results/results_summary.txt` file to review the benchmark results.
2. Run the generated executable file (`Project.exe`) in the root directory.
3. Once the execution is complete, check the `Results/results_summary.txt` file to review the benchmark results.
