all: simulator

clean:
	-rm tvm379

simulator: tvm379.c tvm379.h
	gcc tvm379.c -o tvm379
