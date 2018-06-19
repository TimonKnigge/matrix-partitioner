#!/usr/bin/env

############################
## DEFINE EVERTHING BELOW ##

## Uncomment for bash output.
#set -e -x

## Name the run (will be used as a directory name, so only
## alphanumerics!!).
NAME=run_identifier

## List of binaries to test (with various configurations).
## Note: the binaries will be id'd by their name, so make
## sure this can be a valid part of a filename.
declare -a SOLVER=("./mp1", "./mp2")

## Consider all matrices (zipped as .tar.gz) in this directory
## (recursively). We assume the matrices are downloaded from
## the Suite Sparse Matrix Collection (sparse.tamu.edu) and
## therefore consiste of a file NAME.tar.gz containing only
## NAME/NAME.mtx. For other formats, the code below should
## be trivial to rewrite.
DIR=tars/

## Timelimit for every matrix (in seconds) and epsilon.
TL=300
EPS=0.03

## Whether or not to save the output matrix (otherwise only
## debug is stored).
SOL=false

## DEFINE EVERYTHING ABOVE ##
#############################

echo "Run '${NAME}', with TL=${TL}, EPS=${EPS}."

# Make a directory to store results, and one for unzips.
OUTDIR=runs/${NAME}
mkdir -p ${OUTDIR}
mkdir -p tmp

# Loop over all the given matrices.
find ${DIR} -type f -name '*.tar.gz' | while read MTX; do
	# Get the name of the matrix.
	MTX_NAME=$(basename -- "${MTX}")
	MTX_NAME="${MTX_NAME%.tar.gz}"
	echo "Considering ${MTX_NAME} at ${MTX} now."

	# Unzip and run over all binaries.
	tar xzf ${MTX} -C tmp/

	for PROG in "${SOLVER[@]}"
	do
		# Get the name of the solver.
		PROG_NAME=$(basename -- "${PROG}")

		# stdin, stdout and stderr
		MIN=tmp/${MTX_NAME}/${MTX_NAME}.mtx
		MOUT=/dev/null
		DOUT=${OUTDIR}/${MTX_NAME}__${PROG_NAME}.debug
		if ${SOL} ; then MOUT=${OUTDIR}/${MTX_NAME}__${PROG_NAME}.mtx; fi

		# Compose the CMD.
		CMD="${PROG} ${EPS} ${TL} < ${MIN} > ${MOUT} 2> ${DOUT}"
		echo " Running '${CMD}'."
		eval ${CMD}
	done

	# Remove the matrix again.
	rm -r "tmp/${MTX_NAME}"
done

rm -r tmp/
