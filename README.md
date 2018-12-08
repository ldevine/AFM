# Analogical Frame Mapper

Analogical Frame Mapper (AFM) is a program for finding analogical relations in word embeddings.

Vector files should be in word2vec binary format.

Example execution:

	./afmfind -vecs my_vecs.bin -frames frames.txt -extends2 1 -extends3 1 -nns 15 -ext-nns 40
	-p 0.3 -max-vecs 10000 -threads 8
	


