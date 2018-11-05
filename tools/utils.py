#!/usr/bin/python3

import sys

def imbalance(matrix):
	ceq = lambda i: sum(sum(val[0] == i for _, val in row) for row in matrix)
	d, f = abs(ceq('1') - ceq('2')), ceq('3')
	return d-f if d >= f else (f-d)%2

def read_matrix(filename):
	""" Reads a matrix (in MM format) from the given file. Returns (R, C, NZ, M),
		with M in row-major order. """
	with open(filename, 'r') as f:
		lines = [line.rstrip('\n') for line in f]

	# Verify the header is MM format.
	header = lines[0].split(' ')
	if header[0] != "%%MatrixMarket" or len(header) != 5:
		raise ValueError("Invalid MatrixMarket header.")
	if header[1] != "matrix" or header[2] != "coordinate":
		raise ValueError("File does not describe a sparse matrix.")

	# How many units does each matrix element consist of.
	elems = 1
	if header[3] == "complex":
		elems = 2
	elif header[3] == "pattern":
		elems = 0

	# Is the matrix symmetric?
	sym = header[4] in {"symmetric", "hermitian", "skew-symmetric"}

	# Disregard comments.
	i = 1
	while i < len(lines) and lines[i][0] == '%':
		i += 1
	lines = lines[i:]

	R, C, NZ = map(int, lines[0].split(' '))
	index = dict()
	for line in lines[1:]:
		tokens = line.split(' ')
		r, c, vs = int(tokens[0])-1, int(tokens[1])-1, tuple(tokens[2:])
		if len(vs) != elems:
			raise ValueError("Matrix element with incorrect size.")
		index[(r, c)] = vs
		if sym:
			index[(c, r)] = vs
	NZ = len(index)

	M = [[] for _ in range(R)]
	for r, c in index:
		M[r].append((c, index[(r, c)]))
	return (R, C, NZ, [list(sorted(row, key=lambda tup: tup[0])) for row in M])
