#!/usr/bin/python3

import sys
import utils
import verify

# Tries to limit picture size to ~MAX by MAX pixels.
MAX = 1000

def side_to_color(s):
	if s == 1:
		return '#ff0000'
	elif s == 2:
		return '#0000ff'
	elif s == 3:
		return '#ffff00'
	else:
		return '#000000'

def main():
	if len(sys.argv) < 3:
		raise ValueError("Give the partitioned matrix and target file as arguments.")
	P = utils.read_matrix(sys.argv[1])
	verify.verify(P, P, debug=False)

	R, C, NZ, M = P
	F = min(10, max(1, MAX // max(R, C)))
	svg = open(sys.argv[2], 'w')
	svg.write('<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n')
	svg.write(str.format('<svg xmlns:svg="http://www.w3.org/2000/svg" '
			'xmlns="http://www.w3.org/2000/svg" version="1.0" '
			'width="{}" height="{}" id="svg2">\n', C*F, R*F))

	svg.write('\t<g>\n')
	svg.write(str.format('\t\t<rect width="{}" height="{}" x="0" y="0" id="bkg" '
			 'style="fill:#ffffff;fill-opacity:1;" />\n', C*F, R*F))

	for r, row in enumerate(M):
		for c, v in row:
			svg.write(str.format(
				'\t\t\t<rect width="{}" height="{}" x="{}" y="{}" '
				'id="r{}-{}" style="fill:{};fill-opacity:1;" />\n',
				F, F, c*F, r*F, c, r, side_to_color(int(v[0]))))

	svg.write('\t</g>\n</svg>\n')
	svg.close()

if __name__ == "__main__":
	main()
