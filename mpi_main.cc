#define _USE_MPI_
#define _VERBOSE_
#define _USE_FFTW_
#define _USE_COMPLEX_
#define _USE_CFITSIO_

#include "config.h"
#include "libfits.h"
#include "libmp.h"

#ifndef _RLUCY_
#include "RLucy.h"
#endif

#include <mpi.h>
#include <sys/stat.h>

extern communications comms;

int main(int argc, char*argv[]){

    MPI_Status status;
    int rank = 0;
    int count = 0;
    int MPI_thread_support = 0;

    char DBUFFER[BUFSZ];
    size_t chunksize = 1;
    size_t imagesize = 1;

    MPI_Init_thread(nullptr, nullptr, MPI_THREAD_FUNNELED, &MPI_thread_support);
    MPI_Comm_size(MPI_COMM_WORLD, &PROCESS::TOTAL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    time_t start;
    time_t end;
    double execution_time;

    FILE *console = rank == 0 ? stdout : fopen("/dev/null","wb");
    if(argc < 2){
      fprintf(console, "#. Config file required!\n#. Usage: mpirun -np# ./mpi_main <config_file>\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }

    fprintf(console, "1. Reading file: %s", argv[1]);
    if(config::parse(argv[1]) == EXIT_FAILURE){
      comms.print(INFO, "\t\tUnable to open config file", "main():");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
    fprintf(console, " - Done\n");

    array<double> stokes;
    array<double> space_variant_psf;
    array<double> space_variant_psf_transpose;

    vector_t psfshape;
    vector_t psfshape_transpose;
    vector_t fftw_dims;
    vector_t datashape;
    vector_t imageshape;
    vector_t chunkshape;

    fprintf(console, "2. Reading file: %s", config::PSF_FILE.c_str());

    if(fitsio<double>::read(config::PSF_FILE.c_str(), &space_variant_psf) == EXIT_FAILURE)            //   The third is an bool flag to set verbose logging
      MPI_Abort(MPI_COMM_WORLD, 1);

    fprintf(console, " - Done\n");
    if(transpose(space_variant_psf, space_variant_psf_transpose) == EXIT_FAILURE)
      MPI_Abort(MPI_COMM_WORLD, 1);

    psfshape           = space_variant_psf.dims();
    psfshape_transpose = space_variant_psf_transpose.dims();

    array<double> fftw_image;
    array<complex_t> fftw_image_fourier;

//------------------------------------------------------------------------------
// MASTER BLOCK ----------------------------------------------------------------

    if(rank==0){

      sprintf(DBUFFER, "\t\tConfig: %s", argv[1]);
      comms.print(INFO, DBUFFER, "main():");
      sprintf(DBUFFER, "\t\tPSF: %s", config::PSF_FILE.c_str());
      comms.print(INFO, DBUFFER, "main():");

      std::string log_to_file = "master.log";
      FILE *logfile = fopen(log_to_file.c_str(), "wb");

      size_t lambda_completed = 0;
      size_t lambda = config::LAMBDAL, lambda_counter = 0;

      vector_t process_lambda_quota;
      vector_t process_lambda_map;
      vector_t lambda_progress;

      fprintf(console, "3. Reading file: %s", config::DATA_FILE.c_str());
      fflush(console);

      sprintf(DBUFFER, "\t\tData: %s", config::DATA_FILE.c_str());
      comms.print(INFO, DBUFFER, "main():");
      if(fitsio<double>::read(config::DATA_FILE.c_str(), &stokes) == EXIT_FAILURE)
        MPI_Abort(MPI_COMM_WORLD, 1);

      datashape = stokes.dims();
      if(datashape.size() != config::DATA_RANK){
        fprintf(console, "\n - 3D FITS file required!\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
      }

      chunkshape = datashape;
      imageshape = vector_t(chunkshape.begin()+1, chunkshape.end());
      imagesize  = nelements(imageshape);
      fprintf(console, " - Done\n");

      if(config::LAMBDAU == -1 || config::LAMBDAU > datashape[0])
        config::LAMBDAU = datashape[0];

      if(config::LAMBDAL < 0 || config::LAMBDAL > config::LAMBDAU)
        config::LAMBDAL = 0;
//------------------------------------------------------------------------------
//    Load FFT wisdom (if available), and initialize plans

      fftw_dims    = imageshape;
      fftw_dims[0] = psfshape[1];
      fftw_image.init(fftw_dims);
      fftw_image_fourier.init(fftw_dims);

      fftw_import_wisdom_from_filename(config::FFTWISDOM.c_str());
      fftw_plans::forward = fftw_plan_dft_r2c_2d(fftw_dims[0], fftw_dims[1],\
                                                 fftw_image.data,\
                                                 reinterpret_cast<fftw_complex*>(fftw_image_fourier.data),\
                                                 FFTW_MEASURE);

      fftw_plans::reverse = fftw_plan_dft_c2r_2d(fftw_dims[0], fftw_dims[1],\
                                                 reinterpret_cast<fftw_complex*>(fftw_image_fourier.data),\
                                                 fftw_image.data, FFTW_MEASURE);
      fftw_export_wisdom_to_filename(config::FFTWISDOM.c_str());
//------------------------------------------------------------------------------

      fprintf(console, "4. Selecting ranks");
      for(int pid = 1; pid < PROCESS::TOTAL; pid++){
        if(pid > config::LAMBDAU - config::LAMBDAL){
          MPI_Send(imageshape.data(), imageshape.size(), MPI_LONG, pid, CMDS::SHUTDOWN, MPI_COMM_WORLD);
          PROCESS::DELETED++;                                                   // Shutdown excess processes.
        }else{
          MPI_Send(imageshape.data(), imageshape.size(), MPI_LONG, pid, CMDS::KEEPALIVE, MPI_COMM_WORLD);
        }
      }
      PROCESS::ACTIVE = PROCESS::TOTAL - PROCESS::DELETED;                      // Update the number of active processes

      lambda_progress.resize(config::LAMBDAU - config::LAMBDAL);                // Array indicates the allocation of wavelength indices. 1 = Done, 0 = Pending
      process_lambda_map.resize(PROCESS::ACTIVE);                               // Array specifies the wavelength that a process is working on, array[process_id] = lambda;
      process_lambda_quota.resize(PROCESS::ACTIVE);                             // Array specifies how many wavelengths have been allocated to a single process
      fprintf(console, " - Done\n");
      sprintf(DBUFFER, "\t\tRoot PID: %d", rank);
      comms.print(INFO, DBUFFER, "main():");
      sprintf(DBUFFER, "\t\tNumber of active processes: %d", PROCESS::ACTIVE);
      comms.print(INFO, DBUFFER, "main():");

      config::LAMBDAU = ((config::LAMBDAU >= datashape[0]) || (config::LAMBDAU < 0)) ? datashape[0] : config::LAMBDAU;

      if(PROCESS::TOTAL == 1){
          start  = time(nullptr);
          while(lambda < config::LAMBDAU){
            chunkshape    = datashape;
            chunkshape[0] = (lambda+config::THREADS) < config::LAMBDAU ? config::THREADS : (config::LAMBDAU - lambda);
            array<double> chunk;
            chunk.init(chunkshape);
            memcpy(chunk.data, stokes.data3D[lambda][0], sizeof(double)*chunk.nelements());
            fprintf(console, " - Processing wavelength %ld\n", lambda);
            if(RLucy_pthread_init(&chunk, &space_variant_psf, &space_variant_psf_transpose, logfile) == EXIT_FAILURE)
              MPI_Abort(MPI_COMM_WORLD, 1);
            lambda += chunkshape[0];
	          fflush(console);
          }
          end  = time(nullptr);
	        fprintf(console, " - Execution time(s) %lf\n", difftime(end, start));
      }else{

          sprintf(DBUFFER, "\t\tUsing %ld MPI processes and %ld thread(s)", PROCESS::ACTIVE, config::THREADS);
          comms.print(INFO, DBUFFER, "main():");

          start = time(nullptr);
          for(int pid = 1; pid < PROCESS::ACTIVE; pid++){
            chunkshape[0] = (lambda+config::THREADS) < config::LAMBDAU ? config::THREADS : (config::LAMBDAU - lambda);
            MPI_Send(chunkshape.data(), chunkshape.size(), MPI_LONG, pid, CMDS::KEEPALIVE, MPI_COMM_WORLD);
            MPI_Send(stokes.data3D[lambda][0], nelements(chunkshape), MPI_DOUBLE, pid, CMDS::KEEPALIVE, MPI_COMM_WORLD);
            fprintf(logfile, "main(): Sent lambda: %0.4ld   to rank %0.2d\n", lambda, pid);
            process_lambda_map[pid]   = lambda;                                 // Store the starting wavelength allocated to pid in process_lambda_map[pid]
            process_lambda_quota[pid] = chunkshape[0];                          // Store the number of wavelength indices allocated to pid in process_lambda_quota[pid]
            lambda += chunkshape[0];                                            // Update wavelength to the next pending index
          }

          fflush(logfile);
          fprintf(console, "\r5. Progress: %0.2lf %%", lambda_counter/(config::LAMBDAU-config::LAMBDAL)*100.0);
          fflush(console);

          while(lambda_counter < config::LAMBDAU - config::LAMBDAL){      // lambda_counter keeps tracks of how many wavelength indices have been processed
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);    // MPI_Probe listens within MPI_COMM_WORLD to processes that have completed execution and
            MPI_Get_count(&status, MPI_DOUBLE, &count);                         // are queueing to send data.
            if(status.MPI_TAG == PMSG::ERROR)
                MPI_Abort(MPI_COMM_WORLD, 1);

            lambda_completed = process_lambda_map[status.MPI_SOURCE];           // Retrieve the lambda assigned to the process from its ID and the process_lambda_map
            chunksize = process_lambda_quota[status.MPI_SOURCE]*imagesize;      // Retrieve the size of the chunk sent to the process from process_lambda_quota and imagesize
            MPI_Recv(stokes.data3D[lambda_completed][0], chunksize,\
                     MPI_DOUBLE, status.MPI_SOURCE, MPI_ANY_TAG,\
                     MPI_COMM_WORLD, &status);
            MPI_Recv(&execution_time, 1, MPI_DOUBLE, status.MPI_SOURCE,\
                     MPI_ANY_TAG, MPI_COMM_WORLD, &status);                     // Receive the RLucy() iteration time for data chunk
            lambda_counter += process_lambda_quota[status.MPI_SOURCE];         // Update lambda_counter the number of wavelength indices that were allocated to the process
            sprintf(DBUFFER, "\t\tRecd lambda %0.4ld from rank %0.2d", lambda_completed, status.MPI_SOURCE);
            comms.print(INFO, DBUFFER, "main():");
            fprintf(console, "\r5. Progress: %0.2lf %%", 100.0*lambda_counter/(config::LAMBDAU-config::LAMBDAL));
            fflush(console);
            if(lambda < config::LAMBDAU){
              chunkshape[0] = (lambda + config::THREADS) < config::LAMBDAU ? config::THREADS : (config::LAMBDAU - lambda);
              chunksize     = nelements(chunkshape);
              MPI_Send(chunkshape.data(), chunkshape.size(), MPI_LONG, status.MPI_SOURCE, CMDS::KEEPALIVE, MPI_COMM_WORLD);
              MPI_Send(stokes.data3D[lambda][0], chunksize, MPI_DOUBLE, status.MPI_SOURCE, CMDS::KEEPALIVE, MPI_COMM_WORLD);
              fprintf(logfile, "main(): Sent lambda: %0.4ld   to rank %0.2d\n", lambda, status.MPI_SOURCE);
//              fprintf(console, " - Sent lambda: %0.4ld to rank %0.2d\n", lambda, status.MPI_SOURCE);
              process_lambda_map[status.MPI_SOURCE]   = lambda;
              process_lambda_quota[status.MPI_SOURCE] = chunkshape[0];
              lambda += chunkshape[0];
            }
            fflush(logfile);
          }
	        end = time(nullptr);
          fprintf(console, " - Execution time: %lf\n", difftime(end,start));
          fprintf(console, "6. Shutting down rank(s)\n");
          comms.print(INFO, "\t\tShutting down rank(s)", "\nmain():");
          for(int pid=1; pid < PROCESS::ACTIVE; pid++)
            MPI_Send(nullptr, 0, MPI_CHAR, pid, CMDS::SHUTDOWN, MPI_COMM_WORLD);
      }
      if(config::SAVE){
        fprintf(console, "7. Writing to file: %s\n", config::OUTPUT_FILE.c_str());
        fitsio<double>::write(config::OUTPUT_FILE.c_str(), &stokes, config::CLOBBER);
      }else{
        std::cout << "7. Save skipped" << std::endl;
      }
      fclose(logfile);
      fftw_destroy_plan(fftw_plans::forward);
      fftw_destroy_plan(fftw_plans::reverse);
      fftw_cleanup();
//------------------------------------------------------------------------------
// PROCESS BLOCK ----------------------------------------------------------------

    }else if(rank){

      console = stdout;

      MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      MPI_Get_count(&status, MPI_DOUBLE, &count);
      imageshape.resize(count); imagesize = nelements(imageshape);
      MPI_Recv(imageshape.data(), imageshape.size(), MPI_LONG, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      fftw_dims    = imageshape;
      fftw_dims[0] = psfshape[1];
      fftw_image.init(fftw_dims);
      fftw_image_fourier.init(fftw_dims);

      fftw_import_wisdom_from_filename(config::FFTWISDOM.c_str());
      fftw_plans::forward = fftw_plan_dft_r2c_2d(fftw_dims[0], fftw_dims[1],\
                                                 fftw_image.data,\
                                                 reinterpret_cast<fftw_complex*>(fftw_image_fourier.data),\
                                                 FFTW_MEASURE);

      fftw_plans::reverse = fftw_plan_dft_c2r_2d(fftw_dims[0], fftw_dims[1],\
                                                 reinterpret_cast<fftw_complex*>(fftw_image_fourier.data),\
                                                 fftw_image.data, FFTW_MEASURE);
      fftw_export_wisdom_to_filename(config::FFTWISDOM.c_str());

      while(status.MPI_TAG == CMDS::KEEPALIVE){

        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_DOUBLE, &count);
        chunkshape.resize(count);

        if(status.MPI_TAG != CMDS::SHUTDOWN){
          MPI_Recv(chunkshape.data(), count, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
          array<double> chunk(chunkshape);
          MPI_Recv(chunk.data, chunk.nelements(), MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

          start = time(nullptr);
          if(RLucy_pthread_init(&chunk, &space_variant_psf, &space_variant_psf_transpose, console) == EXIT_FAILURE)
            MPI_Abort(MPI_COMM_WORLD, 1);
          end = time(nullptr);
          execution_time = difftime(end, start);

          MPI_Send(chunk.data, nelements(chunkshape), MPI_DOUBLE, 0, PMSG::READY, MPI_COMM_WORLD);
          MPI_Send(&execution_time, 1, MPI_DOUBLE, 0, PMSG::READY, MPI_COMM_WORLD);
        }
        else{
          MPI_Recv(nullptr, 0, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
          fftw_destroy_plan(fftw_plans::forward);
          fftw_destroy_plan(fftw_plans::reverse);
          fftw_cleanup();
        }
      }
    }

    MPI_Finalize();
}
