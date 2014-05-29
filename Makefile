
CXXFLAGS := -O3 -Wall

default: libminibpm.a test

libminibpm.a:	src/MiniBpm.o
	ar cr $@ $^
	ranlib $@

test:
	$(MAKE) -C tests

clean:
	rm -f libminibpm.a src/MiniBpm.o
