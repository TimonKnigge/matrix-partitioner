#!/bin/bash
#SBATCH -t 5-00:00:00
#SBATCH -n 24

RUNNAME=my-run
INSET=my-matrices
declare -a SOLVER=("bin/my-solver")
EPS=0.03
TL=86400

echo "Run ${RUNNAME} no. ${SLURM_ARRAY_TASK_ID}."
cd ${HOME}/Knigge

# Find all matrices and prepare results directory.
CNT=$(ls -1 matrices/${INSET}/ | wc -l)
FILES=(matrices/${INSET}/*.tar.gz)
mkdir -p ${TMPDIR}/${RUNNAME}
mkdir -p results/${RUNNAME}

# Running 24 instances at once, for each problem set.
for PROG in "${SOLVER[@]}"
do
	PROG_NAME=$(basename -- "${PROG}")
	echo "Considering solver ${PROG_NAME}."
	cp ${PROG} ${TMPDIR}/${PROG_NAME}

	for i in {0..23}; do
	(
		for j in {0..4}; do
		{
			# Custom calculation - redo for each run!!
			id=$((${SLURM_ARRAY_TASK_ID}+14*(${i} + 24*${j})))
			if (( $id < $CNT ))
			then
				MTX=${FILES[${id}]}
				tar xzf ${MTX} -C ${TMPDIR}/

				MTX_NAME=$(basename -- "${MTX}")
				MTX_NAME="${MTX_NAME%.tar.gz}"
				M_IN=${TMPDIR}/${MTX_NAME}/${MTX_NAME}.mtx
				M_OUT=${TMPDIR}/${RUNNAME}/${MTX_NAME}__${PROG_NAME}.out
				M_DEB=${TMPDIR}/${RUNNAME}/${MTX_NAME}__${PROG_NAME}.debug

				echo "  Matrix ${MTX_NAME} by node ${i}, run ${j}."
				CMD="${TMPDIR}/${PROG_NAME} ${EPS} ${TL} <${M_IN} >${M_OUT} 2>${M_DEB}"
				eval ${CMD}
				wait

				cp ${M_OUT} results/${RUNNAME}
				cp ${M_DEB} results/${RUNNAME}

				rm -rf ${TMPDIR}/${MTX_NAME}/
			fi
		}
		done
	)&
	done

	# Wait until all runs are done.
	wait
done
