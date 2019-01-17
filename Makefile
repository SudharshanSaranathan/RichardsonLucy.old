CC     =  mpicc
CSTD   = -std=c++11

IFLAGS = -I${I_MPI_ROOT}/include64 -I${FFTW_HOME}/include -I/ptmp/ssaran/opt/local/include
LFLAGS = -L${I_MPI_ROOT}lib64 -L${FFTW_HOME}/lib -L/ptmp/ssaran/opt/local/lib

OFLAGS = -g3 -O3 -Ofast -ffast-math
LIBS   = -lcfitsio -lmpi -lpthread -lfftw3 -lm -lcurl -lstdc++

DEPS = config.h libmp.h types.h libmem.h libfits.h RLucy.h libtools.h operators.h
OBJS  = mpi_main.o config.o types.o libmem.o libfits.o RLucy.o libtools.o libconvolve.o operators.o

%.o: %.cc ${DEPS}
	${CC} -c -o $@ $< ${DFLAGS} ${IFLAGS} ${LFLAGS} ${OFLAGS} ${CSTD}

make: ${OBJS}
	${CC} -o mpi_main ${OBJS} ${IFLAGS} ${LFLAGS} ${LIBS}

clean:
	rm -rf mpi_main ${OBJS}
