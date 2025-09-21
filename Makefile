all:
	echo "Please specify a target (dimensions, samples or trace)

%.mtp: %.c
	gcc $< -o $@ -lm -fopenmp

clean:
	rm -f *.mtp

.PHONY: all clean
