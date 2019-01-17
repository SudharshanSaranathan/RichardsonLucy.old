#include <mpi.h>
#include <time.h>
#include <cstdio>
#include <stdlib.h>
#include <iostream>

#include "types.h"
#include "libmp.h"
#include "config.h"
#include "libmem.h"
#include "types.hpp"
#include "fitsio.h"
#include "libfits.h"
#include "libtools.h"
#include "libfits.hpp"

#define THREADS 2
#define DATA_DIMENSIONS 3
#define PSF_DIMENSIONS 3
#define BUFFERL 400000
#define MAX(X,Y) (X > Y) ? X : Y
#define MIN(X,Y) (X < Y) ? X : Y

int main(int argc, char*argv[]){

    int rank = 0, pthreads;
    MPI_Init_thread(nullptr, nullptr, THREADS, &pthreads);
    MPI_Comm_size(MPI_COMM_WORLD, &PROCESS::TOTAL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

//------------------------------------------------------------------------------
// MASTER BLOCK ----------------------------------------------------------------

    if(rank==0){

      if(argc < 3){
        fprintf(stdout, "Usage: ./main <filename> <filename>\n");
        error.set_params(true, "Usage: ./main <filename> <filename>", "main():");
        error.flush();
        return EXIT_FAILURE;
      }

      MPI_Status status;

      int count;
      int SENDTAG;
      char READBUF[BUFFERL];
      char FILEBUF[BUFFERL];
      long datasize = 1;
      long slicesize = 1;
      long position_completed = 0;
      long position_new = 0, progress_counter = 0;

      std::vector<int> process_status;
      std::vector<int> position_progress;
      std::vector<int> process_position_map;

      array<double> stokes;
      std::vector<long> datashape;
      std::vector<long> sliceshape;

      fprintf(stdout, "1. Reading FITS file");
      sprintf(FILEBUF, "\t\t\tReading FITS file");
      info.set_params(true, FILEBUF, "main():"); info.flush();

      if(fitsio<double>::read(argv[1], &stokes) == EXIT_SUCCESS){

        datashape = stokes.dims();
        datasize = stokes.size();

        if(datashape.size() != DATA_DIMENSIONS){
          std::cout << "\n - 3D FITS file required" <<std::endl;
          std::cout << " - Program exit" << std::endl;
          return(EXIT_FAILURE);
        }

        sliceshape = std::vector<long>(datashape.begin()+1, datashape.end());
        slicesize = sliceshape.size();

        sprintf(FILEBUF, "\t\t\tFITS file: sst.i.sub.mod.fits");
        info.set_params(true, FILEBUF, "main():"); info.flush();

        sprintf(FILEBUF, "\t\t\tDimensions: %ld %ld %ld", datashape[0], datashape[1], datashape[2]);
        info.set_params(true, FILEBUF, "main():"); info.flush();
      }
      else{
        sprintf(FILEBUF, "Read-Error (check comms.error)");
        error.set_params(true, FILEBUF, "main():"); error.flush();
        exit(EXIT_FAILURE);
      }

      fprintf(stdout, " - Done\n");
      fprintf(stdout, "2. Selecting process(es)");

      process_status.resize(PROCESS::TOTAL);
      position_progress.resize(datashape[0]);
      std::fill(process_status.begin(), process_status.end(), 1);
      std::fill(position_progress.begin(), position_progress.end(), 0);

      // If processes > number of wavelength points, shut down excess processes
      for(int init = 1; init < PROCESS::TOTAL; init++){
        MPI_Recv(READBUF, BUFFERL, MPI::BOOL, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(status.MPI_SOURCE > datashape[0]){
          MPI_Send(nullptr, 0, MPI::BOOL, status.MPI_SOURCE, CMDS::SHUTDOWN, MPI_COMM_WORLD);
          process_status[status.MPI_SOURCE] = 0;
          PROCESS::DELETED++;
        }else{
          MPI_Send(sliceshape.data(), sliceshape.size(), MPI::LONG, status.MPI_SOURCE, CMDS::KEEPALIVE, MPI_COMM_WORLD);
        }
      }
      PROCESS::ACTIVE = PROCESS::TOTAL - PROCESS::DELETED; // Update active processes
      // If processes cannot initialize memory, remove from list of active processes
      for(int init = 1; init < PROCESS::ACTIVE; init++){
        MPI_Recv(FILEBUF, BUFFERL, MPI::BOOL, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(status.MPI_TAG == PMSG::ERROR){
          fprintf(stdout, " - Process %0.3d reported error\n", status.MPI_SOURCE);
          process_status[status.MPI_SOURCE] = 0;
          PROCESS::DELETED++;
          PROCESS::ACTIVE--;
        }
      }
      process_position_map.resize(PROCESS::ACTIVE);        // Update
      fprintf(stdout, " - Done\n");
      sprintf(FILEBUF, "\tRoot PID: %d", rank);
      mpi_comms.set_params(true, FILEBUF, "main():"); mpi_comms.flush();
      sprintf(FILEBUF, "\tNumber of active processes: %d", PROCESS::ACTIVE);
      mpi_comms.set_params(true, FILEBUF, "main():"); mpi_comms.flush();

      fprintf(stdout, "3. Beginning parallelization\n");
      mpi_comms.set_params(true, "\tBeginning parallelization\n", "main():");
      mpi_comms.flush();

      for(int pid = 1; pid < PROCESS::ACTIVE; pid++){
        MPI_Send(stokes.data + position_new*slicesize, slicesize, MPI_DOUBLE, pid, CMDS::KEEPALIVE, MPI_COMM_WORLD);
        process_position_map[pid] = position_new;
        pid++;
        position_new += 1;        // position_new += number_of_pthreads, when using pthreads
      }

      while(progress_counter < datashape[0]){
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	      MPI_Get_count(&status, MPI_DOUBLE, &count);
        if(status.MPI_TAG != PMSG::ERROR){
          position_completed = process_position_map[status.MPI_SOURCE];
          position_progress[position_completed] = 1;
          progress_counter++;
          MPI_Recv(stokes.data + position_completed*slicesize, slicesize, MPI_DOUBLE, status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
          if(position_new < datashape[0]){
            MPI_Send(stokes.data + position_new*slicesize, slicesize, MPI_DOUBLE, status.MPI_SOURCE, CMDS::KEEPALIVE, MPI_COMM_WORLD);
            process_position_map[status.MPI_SOURCE] = position_new;
//            fprintf(stdout, " - Received %ld bytes from process %0.3d\n", count*sizeof(double), status.MPI_SOURCE);
//            fprintf(stdout, " - Assigned position %0.4ld to process %0.3d\n", position_new, status.MPI_SOURCE);
            position_new++;
          }
        }else{
          // Decide how the error needs to be handled
        }
      }

      fprintf(stdout, "4. Task complete, shutting down process(s)");
      sprintf(FILEBUF, "Task complete. Shutting down %d process(s)",  PROCESS::ACTIVE);
      mpi_comms.set_params(true, FILEBUF, "\nmain():"); mpi_comms.flush();

      for(int pid=1; pid<PROCESS::ACTIVE; pid++){
        MPI_Send(nullptr, 0, MPI::BOOL, pid, CMDS::SHUTDOWN, MPI_COMM_WORLD);
      }

      fprintf(stdout, " - Done\n");
      fprintf(stdout, "6. Writing to file: %s", argv[2]);
      if(fitsio<double>::write(argv[2], &stokes, true) == EXIT_SUCCESS){
        fprintf(stdout, " - Done\n");
        sprintf(FILEBUF, "Writing to file: %s", argv[2]);
        mpi_comms.set_params(true, FILEBUF, "\nmain():"); mpi_comms.flush();
      }else{
        sprintf(FILEBUF, " Writing to file failed (check comms.error)");
        mpi_comms.set_params(true, FILEBUF, "\nmain():"); mpi_comms.flush();
      }

//------------------------------------------------------------------------------
// PROCESS BLOCK ----------------------------------------------------------------

    }else if(rank){

      MPI_Status status;

      int bytes = 0;
      int count = 0;
      bool alive = true;
      char FILEBUF[BUFFERL];
      std::vector<long> sliceshape;

      MPI_Send(nullptr, 0, MPI::BOOL, 0, PMSG::READY, MPI_COMM_WORLD);
      MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      MPI_Get_count(&status, MPI_LONG, &count);
      sliceshape.resize(count);

      MPI_Recv(sliceshape.data(), count, MPI::LONG, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      alive = (status.MPI_TAG == CMDS::KEEPALIVE);
      double *data = memory<double>::allocate(sliceshape.size());

      if(alive == true){
        if(data == nullptr){
          MPI_Send(nullptr, 0, MPI::BOOL, 0, PMSG::ERROR, MPI_COMM_WORLD);
          fprintf(stdout, "Process %0.3d: Memory-Error\n", rank);
          mpi_comms.set_params(true, FILEBUF, "Process()");
        }else{
          MPI_Send(nullptr, 0, MPI::BOOL, 0, PMSG::READY, MPI_COMM_WORLD);
        }
      }

      while(alive){
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_DOUBLE, &count);
        if(status.MPI_TAG != CMDS::SHUTDOWN){
          MPI_Recv(data, count, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
          sprintf(FILEBUF, "Process %0.3d received %ld bytes", rank, sizeof(double)*count);
          mpi_comms.set_params(true, FILEBUF, "process():"); mpi_comms.flush();

          MPI_Send(data, count, MPI_DOUBLE, 0, PMSG::READY, MPI_COMM_WORLD);
        }
        else{
          MPI_Recv(FILEBUF, BUFFERL, MPI::BOOL, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
          alive = false;
        }
      }
      memory<double>::deallocate(data);

    }
    MPI_Finalize();
}
