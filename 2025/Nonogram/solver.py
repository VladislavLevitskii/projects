#!/usr/bin/python3

import subprocess
import sys

def append_to_lines(line, lines):
    absolute_sum = sum(abs(i) for i in line)
    if absolute_sum > height:
        return
    while len(line) < height and absolute_sum < height:
        line.append(-1)
        absolute_sum += 1
    if line not in lines:
        lines.append(line)

def combinations_without_big_gaps(line, lines):
    append_to_lines(line.copy(), lines)
    length_of_line = 0
    for i in line:
        length_of_line += abs(i)
    for i in range(height - length_of_line):
        line.insert(i, -1)
        append_to_lines(line.copy(), lines)

def solve_line(line, colrow, line_dnfs):
    lines = []
    sum_of_line = sum(line) + len(line) - 1
    if sum_of_line > height:
        raise Exception("Over the limit")

    i = 1
    for _ in range(len(line) - 1):
        line.insert(i, -1)
        i += 2

    combinations_without_big_gaps(line.copy(), lines)

    neg_idx = []
    if len(line) != height:
        neg_idx = [i for i, x in enumerate(line) if x == -1]

    def generate(i, current):
        if i == len(neg_idx):
            if sum(abs(x) for x in current) <= height:
                combinations_without_big_gaps(current.copy(), lines)
            return
        for v in range(1, height + 1):
            current[neg_idx[i]] = -v
            generate(i + 1, current)

    generate(0, line.copy())

    dnf_for_line = []
    for possibility in lines:
        line_of_matrix = []

        if colrow > height:
            for i in range(height):
                line_of_matrix.append(matrix[colrow - height - 1][i])
        else:
            for i in range(height):
                line_of_matrix.append(matrix[i][colrow - 1])

        m = 0
        for i in possibility:
            if (1 != abs(i)):
                if (i < 0):
                    negative = True
                else:
                    negative = False
                possibility.remove(i)
                for _ in range(abs(i)):
                    if (negative):
                        possibility.insert(m, -1)
                    else:
                        possibility.insert(m, 1)
            m += 1

        for i in range(len(possibility)):
            if possibility[i] == -1:
                line_of_matrix[i] = -line_of_matrix[i]

        dnf_for_line.append(line_of_matrix)

    line_dnfs.append(dnf_for_line)

def matrix_initialization():
    matrix = [[0 for _ in range(height)] for _ in range(height)]

    i = 1
    for row in range(height):
        for col in range(height):
            matrix[row][col] = i
            i += 1
    return matrix

def handle_input():
    entry = []
    l = 1
    print("Input columns: ")
    for _ in range(height + height):
        if l == height + 1:
            print("Now rows")
        entry_line = input().strip()
        if entry_line == "0" or entry_line == "":
            entry.append([])
        else:
            entry.append(list(map(int, entry_line.split())))
        l += 1
    return entry

def calculateCNF():
    cnf = []
    aux_counter = height * height

    for group in lines:
        aux_vars = []
        for variant in group:
            aux_counter += 1
            a = aux_counter
            aux_vars.append(a)

            for lit in variant:
                cnf.append([-a, lit])

        cnf.append(aux_vars)

        for i in range(len(aux_vars)):
            for j in range(i + 1, len(aux_vars)):
                cnf.append([-aux_vars[i], -aux_vars[j]])
    return cnf, aux_counter

def writeFile():
    with open("formula.cnf", "w") as file:
        file.write(f"p cnf {num_vars} {num_clauses}\n")
        for clause in cnf:
            file.write(" ".join(map(str, clause)) + " 0\n")

def draw_grid(line, height):
    values = [int(x) for x in line.split()[1:-1]]
    values = values[:height * height]

    for r in range(height):
        print('+---'*height + '+')
        row_vals = values[r*height:(r+1)*height]
        row_str = '| ' + ' | '.join('■' if v > 0 else ' ' for v in row_vals) + ' |'
        print(row_str)
    print('+---'*height + '+')

if __name__ == '__main__':

    test = 0
    if "--help" in sys.argv or "-h" in sys.argv:
        print("Usage: ./solver.py [-v] [-h] [-t TEST]\n")
        print(
            "Input size refers to the height and width of the nonogram. \n"
            "The first n column numbers are listed from top to bottom, \n"
            "and the row numbers are listed from left to right.\n"
        )

        print("More info about the project: https://gitlab.mff.cuni.cz/levitsv/nonogram-projekt \n  ")
        print("Optional arguments:")
        print("   -h, --help       Show this help message and exit")
        print("   -t, --test TEST  Run the solver in test mode using the specified test file")
        print("   -v, --verbose adds additional information")
        exit(0)

    if "--test" in sys.argv or "-t" in sys.argv:
        test = 1
        if "--test" in sys.argv:
            index = sys.argv.index("--test")
        else:
            index = sys.argv.index("-t")
        try:
            test_file = sys.argv[index + 1]
        except IndexError:
            print("Invalid file for --test")

    if "--verbose" in sys.argv or "-v" in sys.argv:
        test = 2

    if test == 0:
        height = int(input("Size: "))
        matrix = matrix_initialization()

        entry = handle_input()

    elif test == 1 or test == 2:
        with open(test_file, "r") as file:
            height = int(file.readline())
            matrix = matrix_initialization()
            entry = []
            for _ in range(height + height):
                entry_line = file.readline().strip()
                if entry_line == "0" or entry_line == "":
                    entry.append([])
                else:
                    entry.append(list(map(int, entry_line.split())))

    lines = []
    for m, line in enumerate(entry, start=1):
        solve_line(line, m, lines)

    cnf, num_vars = calculateCNF()
    num_clauses = len(cnf)

    writeFile()

    result = subprocess.run(["./glucose-syrup", "-model", "formula.cnf"], capture_output=True, text=True)

    lines = result.stdout.strip().split("\n")

    last_line = lines[-1]

    if test == 2:
        for i in lines[:-1]:
            print(i)

    if ("UNSATISFIABLE" in last_line):
        print("No result")
    else:
        print("One of the models: ")
        draw_grid(last_line, height)