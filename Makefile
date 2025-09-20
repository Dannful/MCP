all:
	echo "Please specify a target (dimensions, samples or trace)

%.mtp: %.c
	gcc -lm -fopenmp $< -o $@

clean:
	rm -f *.mtp

.PHONY: all clean
