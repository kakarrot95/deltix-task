# Getting Started
================

To run this project, follow these steps:

### Prerequisites

* Place the `market_data.csv` and `user_data.csv` files in the same directory as the `main.cpp` file.

### Building and Running

1. Run the command `make` to build the project.
2. Run the command `./main` to execute the program.

### Output

After running the program, three CSV files (`bars-1h.csv`, `bars-1d.csv`, and `bars-30d.csv`) will be generated in the same directory.

## Background
============

Initially, I attempted to implement a similar solution using pandas in Python. However, despite my best efforts, the solution was not satisfactory. The estimated execution time for the medium-sized dataset was around 22 hours, and for the large dataset, it was around 60 hours.

I realized that just because others use pandas and Python to solve similar problems doesn't mean I have to. So, I decided to create a C++ solution, which ran in approximately 60 seconds on my computer (16 GB RAM, i7-10870H) for the large dataset. The problem statement allowed us to choose any language and optimize for speed, so I chose C++.