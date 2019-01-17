#ifndef _LIBMP_
#define _LIBMP_
#endif

/*
struct MPI_Status {
  MPI_SOURCE - id of processor sending the message
  MPI_TAG - the message tag
  MPI_ERROR - error status
};
*/

class CMDS {
public:
  static int KEEPALIVE;
  static int SHUTDOWN;
};

int CMDS::KEEPALIVE = 1;
int CMDS::SHUTDOWN  = 0;

class PMSG {
public:
  static int READY;
  static int ERROR;
  static int WARNING;
};

int PMSG::READY = 0;
int PMSG::ERROR = 1;
int PMSG::WARNING = 2;

class PROCESS {
public:
  static int TOTAL;
  static int ACTIVE;
  static int DELETED;
};

int PROCESS::TOTAL = 0;
int PROCESS::ACTIVE = 0;
int PROCESS::DELETED = 0;
