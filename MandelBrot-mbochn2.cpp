//
// Michal Bochnak
// Netid: mbochn2
// Homework #4
// Nov 16, 2017
//
// MandelBrot-mbochn2.cpp
//

//
// include header with functions
//
#include "mandelHeader.h"

//
// function prototypes
//
void whenExited();
void sigint_handler(int);
void sigchld_handler(int);
void cleanResources();

//
// globals
//
int     msg_queue_1_id,
        msg_queue_2_id,
        sh_mem_id,
        child_pid_1,
        child_pid_2,
        wchild_status_1,
        wchild_status_2;


int main (int argc, char** argv) {

    //
    // pipes
    //
    int     pipe1[2],
            pipe2[2];

    //
    // show info, set signals
    //
    showAuthorInfo();
    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);
    atexit(whenExited);

    //
    // check if pipes were successful
    //
    if (pipe(pipe1) == -1) {
        perror("pipe1");
        exit(-1);
    }
    if (pipe(pipe2) == -1) {
        perror("pipe2");
        exit(-1);
    }

    //
    // Message queues
    //
    if ((msg_queue_1_id = msgget(IPC_PRIVATE,
                                 0600 | IPC_CREAT)) < 0) {
        perror("msgget error");
        exit(-2);
    }
    if ((msg_queue_2_id = msgget(IPC_PRIVATE,
                                 0600 | IPC_CREAT)) < 0) {
        perror("msgget error");
        exit(-2);
    }

    //
    // Shared memory
    //
    if ((sh_mem_id = shmget(IPC_PRIVATE, 100000,
                            0600 | IPC_CREAT)) < 0) {
        perror("shmget error");
        exit(-3);
    }

    //
    // Fork child 1
    //
    // fork child 1 and check if successfull
    if ((child_pid_1 = fork()) < 0) {
        perror("child 1 error");
        exit(-4);
    }
    // executed by child 1
    else if (child_pid_1 == 0){

        // close unused pipes
        close(pipe1[1]);
        close(pipe2[0]);

        // redirect the input and output to pipe
        if ((dup2(pipe1[0], STDIN_FILENO)) < 0) {
            perror("dup2 error");
            exit(-5);
        }
        if ((dup2(pipe2[1], STDOUT_FILENO)) < 0) {
            perror("dup2 error");
            exit(-5);
        }

        // close pipes that were redirected
        close(pipe1[0]);
        close(pipe2[1]);

        // prepare command line arguments for child 1
        char* child_argv[] = { convStringToCharArray("./mandelCalc"),
                               convIntToCharArr(sh_mem_id),
                               convIntToCharArr(msg_queue_1_id),
                               NULL };

        // evec mandelCalc - child 1
        execvp(child_argv[0], child_argv);
        exit(-3);
    }

    //
    // Fork child 2
    //
    // fork child 2 and check if succesfull
    if ((child_pid_2 = fork()) < 0) {
        perror("child 2 error");
        exit(-4);
    }
        // executed by child 2
    else if (child_pid_2 == 0) {

        // close unused pipe ends
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[1]);

        // redirect input
        if ((dup2(pipe2[0], STDIN_FILENO)) < 0) {
            perror("dup2 error");
            exit(-5);
        }
        // close pipe that were redirected
        close(pipe2[0]);

        // prepare command line arguments for child 2
        char* child_argv[] = { convStringToCharArray("./mandelDisplay"),
                               convIntToCharArr(sh_mem_id),
                               convIntToCharArr(msg_queue_1_id),
                               convIntToCharArr(msg_queue_2_id),
                               NULL };

        // evec mandelDispkay
        execvp(child_argv[0], child_argv);
        exit(-4);
    }

    //
    // Executed by parent
    //
    // close unused pipe ends
    close(pipe1[0]);
    close(pipe2[0]);
    close(pipe2[1]);

    //
    // additional needed variables
    //
    // file pointer for pipe
    FILE* pointer_pipe1 = fdopen(pipe1[1], "w");
    char*   filename;       // filename to be send to mandelDisplay
    double  xMin, xMax, yMin, yMax;
    int     nRows, nCols, maxIters;
    struct msg_struct msg;  // message struct for message queues
    msg.mtype = 1;          // message type is 1

    while (true) {

        // get the problem info from user
        getProblemInfo(xMin, xMax, yMin, yMax,
                nRows, nCols, maxIters);

        // get filename
        filename = convStringToCharArray(getFilename());
        strcpy(msg.mtext, filename);

        // send problem info via pipe to mandelCalc
        fprintf(pointer_pipe1, "%lf %lf %lf %lf %d %d %d\n",
                xMin, xMax, yMin, yMax, nRows, nCols, maxIters);
        fflush(pointer_pipe1);

        // send the message with filename to mandelDisplay
        if ((msgsnd(msg_queue_2_id, &msg,
                    (sizeof(msg_struct) - sizeof(msg_struct.mtype)),
                    IPC_NOWAIT)) < 0) {
            perror("msgsnd error");
            exit(-6);
        }

        // wait for message from mandelCalc
        while ((msgrcv(msg_queue_1_id, &msg,
                       (sizeof(msg_struct)) - (sizeof(long)),
                       1, IPC_NOWAIT)) < 0);

        // wait for message from mandelDisplay
        while ((msgrcv(msg_queue_1_id, &msg,
                       (sizeof(msg_struct)) - (sizeof(long)),
                       1, IPC_NOWAIT)) < 0);

      // break if user is done
        if (done())
            break;
    }


    //
    // send SIGUSR1 to both children
    //
    kill(child_pid_1, SIGUSR1);
    kill(child_pid_2, SIGUSR1);
    pid_t wpid_1 = waitpid(child_pid_1, &wchild_status_1, WUNTRACED);
    pid_t wpid_2 = waitpid(child_pid_2, &wchild_status_2, WUNTRACED);

    // report children exit statuses
    if (WIFEXITED(wchild_status_1)) {
        printf("Child 1 - child %d terminated with the status %d\n" ,
               child_pid_1, WEXITSTATUS(wchild_status_1));
    }
    if (WIFEXITED(wchild_status_2)) {
        printf("Child 2 - child %d terminated with the status %d\n" ,
               child_pid_2, WEXITSTATUS(wchild_status_2));
    }

    // Report final results
    cout << "There were " << WEXITSTATUS(wchild_status_1)
         << " images processed." << endl;

    // clean resources
    cleanResources();

    // done
    cout << "\n   |------|\n   | Done |\n   |------|\n" << endl;
    return 0;
}


//
// Clean the resources if program exited
//
void whenExited() {
    cleanResources();
}

//
// SIGINT handler - clear resources
//
void sigint_handler(int sig) {
    cleanResources();
    exit(-1);
}

//
// SIGCHLD handler
//
void sigchld_handler(int sig) {
    // wait for children
    int     wpid_1 = waitpid(child_pid_1, &wchild_status_1, WNOHANG);
    int     wpid_2 = waitpid(child_pid_2, &wchild_status_2, WNOHANG);
    int     exit_st_ch_1 = (int8_t)WEXITSTATUS(wchild_status_1);
    int     exit_st_ch_2 = (int8_t)WEXITSTATUS(wchild_status_2);

    // report exit statuses
    cout << endl << "Child " << child_pid_1 << " exited with exit status of "
         << exit_st_ch_1 << endl;
    cout << "Child " << child_pid_2 << " exited with exit status of "
         << exit_st_ch_2 << endl;

    // child 1 terminated
    if (wpid_1 > 0) {
        // terminate other child
        kill(child_pid_2, 9);
    }
    // child 2 terminated
    if (wpid_2 > 0) {
        // terminate other child
        kill(child_pid_1, 9);
    }

    // Report final results
    if ((exit_st_ch_1 < 0) || (exit_st_ch_2 < 0)) {
        showErrorInfo(exit_st_ch_1, exit_st_ch_2);
    }
    else {
        cout << "There were " << exit_st_ch_2
             << " images processed." << endl;
    }

    // clear resources
    cleanResources();

    // done
    cout << "\n   |------|\n   | Done |\n   |------|\n" << endl;
    exit(-1);
}

//
// Clean resources: shared memory, messages queues
//
void cleanResources() {
    msgctl(msg_queue_1_id, IPC_RMID, NULL);
    msgctl(msg_queue_2_id, IPC_RMID, NULL);
    shmctl(sh_mem_id, IPC_RMID, NULL);
}























