# Analogical Frame Mapper

Analogical Frame Mapper (AFM) is a program for finding analogical relations in word embeddings. 

It is the software associated with the paper: [Unsupervised Mining of Analogical Frames by Constraint Satisfaction](http://alta2018.alta.asn.au/alta2018-draft-proceedings.pdf#page=44)

An analogical frame is like a multi-dimensional proportional analogy.

Given a word embedding file in word2vec binary format, the program **afmfind** will search the word embeddings, attempting to discover analogical frames. The discovery process is specified by a number of command line arguments.

The code should compile with any C++11 conformant compiler. The code depends on the C++ linear algebra library Eigen, a copy of which is included in the source of this repository.

Example execution:

	./afmfind -vecs my_vecs.bin -frames frames.txt -extends2 1 -extends3 1 -nns 15 -ext-nns 40
	-p 0.3 -max-vecs 10000 -threads 8
	
Parameters:

	frames    the text file produced containing the frames.
	max-vecs  the maximum number of vectors (vocab) to include in the search.
	threads   the number of threads to search with.
	p         the value of the parallel constraint. Larger is more constrained, ie. fewer frames.
	nns       the number of vector neighbours to consider when assigning a value to an adjacent
               variable in the frame.
	ext-nns   the number of neighbours to consider when assigning values to adjacent variables
               when extending a frame.
        
        
Note that the effectiveness of increasing the number of threads depends on the values of other parameters. This is a result of how the computation is decomposed and assigned to worker tasks. Improvements in how work is decomposed will be included in future versions of the code.

