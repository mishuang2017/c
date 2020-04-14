clean:
	find . -name *.ko -exec rm {} +
	find . -name *.o -exec rm {} +
	find . -name Module.symvers -exec rm {} +
	find . -name modules.order -exec rm {} +
	find . -name *.mod.c -exec rm {} +
# 	find . -name ".*" -exec rm -rf {} +
