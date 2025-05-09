#!/bin/bash

################################################################################
# Unpack the file A2.tar.gz (which should be on the format specified in the
# instructions of assignment 2 in Parallel and Distributed Programming.
# Check that the required files exist and can be built respectively.
# Run the binary for different number of processes. Exit with exit code 1 if
# the file structure is not the required one or the program computes the wrong
# result. Then also print a message saying what's wrong. If everything seems OK,
# print a message saying that the file is ready for submission, and exit with
# exit code 0.
################################################################################

# Extract tar, enter directory, build (clean first and check that files were removed)
echo "Extracting and entering A2 directory"
tar -xzf A2.tar.gz || exit 1
cd A2 || exit 1
report=`find . -maxdepth 1 -name "A2_Report.pdf" | wc -l`
if [ 1 != "$report" ]; then
	echo "A2_Report.pdf is missing!"
	exit 1
fi
echo "Building"
make clean || exit 1
binary=`find . -maxdepth 1 -name "stencil" | wc -l`
if [ 0 != "$binary" ]; then
	echo "make clean does not work as expected!"
	exit 1
fi
make || exit 1
binary=`find . -maxdepth 1 -name "stencil" | wc -l`
if [ 1 != "$binary" ]; then
	echo "stencil not built by 'make all'!"
	exit 1
fi
echo "OK"

echo "Checking result of serial runs"
applications=( 1 )
for appl in ${applications[@]}; do
	./stencil ../test_data/input96.txt test_output96.txt $appl > /dev/null
	diff --ignore-all-space test_output96.txt ../test_data/output96_${appl}_ref.txt
	if [ 0 != $? ] ; then
		echo "Wrong result of $appl application(s) on 96 elements!"
		exit 1
	fi
	rm test_output96.txt
done
echo "OK"

echo "Checking result of parallel runs"
pe=( 4 )
for p in ${pe[@]}; do
	output_lines=`mpirun --bind-to none -np $p ./stencil ../test_data/input96.txt test_output96.txt 4 | wc -l`
	if [ 1 -lt $output_lines ]; then
		echo "Your program doesn't seem to be parallelized!"
		exit 1
	fi
	diff --ignore-all-space test_output96.txt ../test_data/output96_4_ref.txt
	if [ 0 != $? ]; then
		echo "Wrong results of parallel run ($p processes)!"
		exit 1
	fi
	rm test_output96.txt
done
echo "OK"

make clean || exit 1
echo "Your file is ready for submission. Well done!"
