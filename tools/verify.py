#!/usr/bin/python3

import sys
import utils

def flatten(ls):
	return [item for l in ls for item in l]

def verify(O, P, eps=0.03, debug=True):
	OR, OC, ONZ, OM = O
	PR, PC, PNZ, PM = P
	if OR != PR or OC != PC or ONZ != PNZ:
		raise ValueError("Matrices have different sizes.")
	elif debug:
		print(str.format("Read {}x{} matrix with {} nonzeros.", OR, OC, ONZ))

	OI, PI = set(), set()
	for r, row in enumerate(OM):
		for c, v in row:
			OI.add((r, c))
	side = [0, 0, 0]
	for r, row in enumerate(PM):
		for c, v in row:
			PI.add((r, c))
			if len(v) != 1 or int(v[0]) < 1 or int(v[0]) > 3:
				raise ValueError("Partitioned matrix has non-{1,2,3} value.")
			side[int(v[0])-1] += 1
	if len(PI) != PNZ:
		raise ValueError("Number of nonzeros in partitioned matrix does not line up.")
	elif len(OI) != ONZ:
		raise ValueError("Number of nonzeros in original matrix does not line up.")
	for r, c in PI:
		if (r, c) not in OI:
			raise ValueError("Nonzeros in partitioned matrix not the same as in original.")

	if debug:
		print(str.format("Partition sizes: {} red, {} blue and {} unassigned.", side[0], side[1], side[2]))
	d2 = [max(0, min(side[2], side[1] - side[0])), max(0, min(side[2], side[0] - side[1]))]
	side = [side[0]+d2[0], side[1]+d2[1], side[2]-d2[0]-d2[1]]
	side[0] += (side[2] + 1) // 2
	side[1] += (side[2] + 0) // 2
	if debug:
		print(str.format("After redistribution this would amount to {} vs. {} nonzeros.", side[0], side[1]))
	imb = max(side[0], side[1]) / ((ONZ + 1) // 2) - 1.0
	if debug:
		print(str.format("This ammounts to an imbalance of eps={}", imb))
	if imb > eps:
		raise ValueError("Partition imbalance is too large.")

def main():
	if len(sys.argv) < 3:
		raise ValueError("Give the original and partitioned matrices as arguments.")
	O, P = utils.read_matrix(sys.argv[1]), utils.read_matrix(sys.argv[2])
	verify(O, P)

if __name__ == "__main__":
	main()
