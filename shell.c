    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/wait.h>

    char * pwd(void); // pwd command
    void append_redirect(char * arg); // used for the append option ">>"
    void overwrite_redirect(char * arg);
    void printerr(void);
    int cmd_case(char ** input); 
    void my_pipe(char ** child, char ** parent);
    //file descriptors when a program is created
    // 0 - STDIN - all scanf read from FD 0
    // 1 - STDOUT - all your printf got to FD 100
    // 2 - STDERR
    int main()
    {

            char my_string[sizeof(char) * 1024];
            char * input[1024];
            // holds the elements of the input
            char * first[1024];
            char * second[1024];
            char * third[1024];

            char *valid;
            int status;
            for(;;){ // this loop runs the shell until exit has been entered

                    printf("mysh> ");

                    valid = fgets(my_string, sizeof(my_string) , stdin);
                    char *token;
                    //printf( "my_string: %s\n", my_string);
                    //printf( "valid: %s\n", valid);

                    if (valid == NULL){
                            perror(NULL);
                            continue;
                    }
                    if (my_string[0] == '\n'){
                            continue;
                    }
                    else{


                            /* get the first token */
                            int i = 0;
                            token = strtok(my_string, " \t\n");
                            input[i] = token;
                            i++;
                            while(token != NULL){


                                    token = strtok(NULL, " \t\n");
                                    input[i] = token;
                                    //printf("%s 1", input[i]);
                                    i++;
                            }
                            input[i+1] = NULL; // null terminate the parsed input

                            int case_no = -1; //hold case number
                            case_no = cmd_case(input);
                            //printf( "%s\n", token );

                            //////////////////
                            // EXIT command //
                            //////////////////
                            if(strcmp(input[0], "exit") == 0){

                                    return 0;
                            }
                            /////////////////
                            // PWD command //
                            /////////////////
                            else if(strcmp(input[0], "pwd") == 0){

                                    printf("%s\n", pwd());
                            }

                            ////////////////
                            // CD command //
                            ////////////////
                            else if(strcmp(input[0], "cd") == 0){
                                    int error; // if the change directory does not work
                                    char * homePath; // hold the homepath
                                    // get path if there is one

                                    // if no path has been specified then change to home directory
                                    if(input[1] == NULL){

                                            homePath = getenv("HOME"); // get the homePath

                                            if(homePath == NULL){
                                                    printf("Home directory not found\n");
                                            }
                                            else{
                                                    error = chdir(homePath);
                                                    if (error != 0){
                                                            printf("no such directory\n");
                                                    }
                                            }
                                    }
                                    else{ // change to the dir that is specified

                                            error = chdir(input[1]);
                                            if (error != 0){
                                                    printf("no such directory\n");
                                            }

                                    }
                            } // end CD command


                            ///////////////////////
                            // running a program //
                            ///////////////////////
                            else if(case_no != -1){



                                    int counter;
                                    int case_no; //hold case number
                                    case_no = cmd_case(input);
                                    // run a program with or with out args

                                    // "Case 2"  :  a [args] > .txt 
                                    if(case_no == 2){
                                            char * filename;

                                            for(counter = 0; input[counter] != NULL; counter++){
                                                    if(strcmp(input[counter], ">") == 0){
                                                            filename = input[counter + 1];
                                                    }
                                            }
                                            pid_t pid;

                                            pid = fork();

                                            if(pid == 0){
                                                    overwrite_redirect(filename);
                                                    i = 0;
                                                    i = execvp(*input, input);
                                                    if (i == -1){
                                                            printf("Unknown command: [%s]\n", input[0]);
                                                            return 0; //exit child
                                                    }
                                            }
                                            else if(pid > 0) waitpid(pid,&status, WNOHANG);
                                    }
                                    // "Case 3"  :  a [args] >> .txt
                                    else if(case_no == 3){
                                            char * filename;

                                            for(counter = 0; input[counter] != NULL; counter++){
                                                    if(strcmp(input[counter], ">>") == 0){
                                                            filename = input[counter + 1];
                                                    }
                                            }
                                            pid_t pid;

                                            pid = fork();

                                            if(pid == 0){
                                                    append_redirect(filename);
                                                    i = 0;
                                                    i = execvp(*input, input);
                                                    if (i == -1){
                                                            printf("Unknown command: [%s]\n", input[0]);
                                                            return 0; //exit child
                                                    }
                                            }
                                            else if(pid > 0) waitpid(pid,&status,WNOHANG);

                                    }
                                    // "Case 4"  :  a [args] | b [args]
                                    else if(case_no == 4){

                                            for (counter = 0; strcmp(input[counter], "|") != 0; counter ++){
                                                    first[counter] = input[counter];

                                            }
                                            first[counter + 1] = NULL;
                                            counter++;
                                            int x;
                                            for (x = 0; input[counter] != NULL; counter ++, x++){
                                                    second[x] = input[counter];

                                            }
                                            second[x + 1] = NULL;


                              int pipes[2];
                              pipe(pipes); 


                              if (fork() == 0)
                                {


                                  dup2(pipes[1], 1);
                                  close(pipes[0]);
                                  close(pipes[1]);


                                  execvp(first[0], first);
                                }
                              else
                                {

                                  if (fork() == 0)
                                  {
                                      dup2(pipes[0], 0);
                                      close(pipes[0]);
                                      close(pipes[1]);


                                      execvp(second[0], second);
                                 }


                               }

                                      close(pipes[0]);
                                      close(pipes[1]);
                                      for (i = 0; i < 2; i++){
                                        wait(&status);
                                    }
                            }
                            // "Case 5"  :  a [args] | b [args] | c [args]
                            else if(case_no == 5){

                                    int x,y;
                                    for (counter = 0; strcmp(input[counter], "|") != 0; counter ++){
                                                    first[counter] = input[counter];

                                    }
                                    first[counter+1] = NULL;

                                    counter++;


                                    for (x = 0; strcmp(input[counter], "|") != 0; counter ++, x++){
                                                    second[x] = input[counter];

                                    }
                                            second[x + 1] = NULL;
                                            counter++;
                                    for (y = 0; input[counter] != NULL; counter ++, y++){
                                                    third[y] = input[counter];

                                    }
                                            third[y+1] = NULL; 


                                      int status;
                                      int i;

                                      int pipes[4];
                                      pipe(pipes); 
                                      pipe(pipes + 2);


                                      if (fork() == 0)
                                        {


                                          dup2(pipes[1], 1);



                                          close(pipes[0]);
                                          close(pipes[1]);
                                          close(pipes[2]);
                                          close(pipes[3]);

                                          execvp(*first, first);
                                        }
                                      else
                                        {


                                          if (fork() == 0)
                                            {


                                              dup2(pipes[0], 0);



                                              dup2(pipes[3], 1);



                                              close(pipes[0]);
                                              close(pipes[1]);
                                              close(pipes[2]);
                                              close(pipes[3]);

                                              execvp(*second, second);
                                            }
                                          else
                                            {


                                              if (fork() == 0)
                                                {


                                                  dup2(pipes[2], 0);



                                                  close(pipes[0]);
                                                  close(pipes[1]);
                                                  close(pipes[2]);
                                                  close(pipes[3]);

                                                  execvp(*third, third);
                                                }
                                            }
                                        }



                                      close(pipes[0]);
                                      close(pipes[1]);
                                      close(pipes[2]);
                                      close(pipes[3]);

                                      for (i = 0; i < 3; i++){
                                        wait(&status);
                                    }

                            }
                            // "Case 8"  :  a [args] % b [args]
                            else if(case_no == 8){

                                    int x;
                                    for (counter = 0; strcmp(input[counter], "%") != 0; counter ++){
                                                    first[counter] = input[counter];

                                    }
                                    first[counter+1] = NULL;

                                    counter++;


                                    for (x = 0; input[counter] != NULL; counter ++, x++){
                                                    second[x] = input[counter];

                                    }
                                            second[x + 1] = NULL;


                                      int status;
                                      int i;

                                      int pipes[4];
                                      pipe(pipes); 
                                      pipe(pipes + 2); 


                                      if (fork() == 0)
                                        {


                                          dup2(pipes[1], 1);



                                          close(pipes[0]);
                                          close(pipes[1]);
                                          close(pipes[2]);
                                          close(pipes[3]);

                                          execvp(*first, first);
                                        }
                                      else
                                        {


                                          if (fork() == 0)
                                            {


                                              dup2(pipes[0], 0);



                                              dup2(pipes[3], 1);



                                              close(pipes[0]);
                                              close(pipes[1]);
                                              close(pipes[2]);
                                              close(pipes[3]);

                                            char * arg[2];
                                            arg[0] = "mytee";
                                            arg[1] = "\0";
                                              execvp(*arg,  arg);
                                            }
                                          else
                                            {


                                              if (fork() == 0)
                                                {


                                                  dup2(pipes[2], 0);



                                                  close(pipes[0]);
                                                  close(pipes[1]);
                                                  close(pipes[2]);
                                                  close(pipes[3]);

                                                  execvp(*second, second);
                                                }
                                            }
                                        }



                                      close(pipes[0]);
                                      close(pipes[1]);
                                      close(pipes[2]);
                                      close(pipes[3]);

                                      for (i = 0; i < 3; i++){
                                        wait(&status);
                                    }

                            }

                                    // "Case 9"  :  a [args] | b [args] > .txt 
                            else if(case_no == 9){
                                            int x,y;

                                            for (counter = 0; strcmp(input[counter], "|") != 0; counter ++){
                                                    first[counter] = input[counter];

                                            }

                                            counter++;

                                            for (x = 0; strcmp(input[counter], "|") != 0; counter ++, x++){
                                                    second[counter] = input[counter];

                                            }


                                            char * filename;

                                            for(counter = 0; second[counter] != NULL; counter++){
                                                    if(strcmp(second[counter], ">>") == 0){
                                                            filename = second[counter + 1];
                                                    }
                                            }

                              int pipes[2];
                              pipe(pipes); // sets up 1st pipe


                              if (fork() == 0)
                                {


                                  dup2(pipes[1], 1);



                                  close(pipes[0]);
                                  close(pipes[1]);


                                  execvp(first[0], first);
                                }
                              else
                                {

                                  if (fork() == 0)
                                  {


                                      dup2(pipes[0], 0);



                                      close(pipes[0]);
                                      close(pipes[1]);

                                      overwrite_redirect(filename);

                                      execvp(second[0], second);
                                 }


                               }


                                      close(pipes[0]);
                                      close(pipes[1]);


                                      for (i = 0; i < 2; i++){
                                        wait(&status);
                                    }
                            }
                                    else{
                                            int pid;

                                            pid = fork();
                                            if(pid == 0){

                                                    i = 0;
                                                    i = execvp(*input, input);
                                                    if (i == -1){
                                                            printf("Unknown command: [%s]\n", input[0]);

                                                    }
                                            }
                                            else if(pid > 0) wait(&status);
                                    }
                            }
                    }
            }


            return 0;
    }

    char * pwd(void){
            char path[1024];
            return getcwd(path, (sizeof(path)*1024));
    }
    void printerr(void){
            printf("ERROR!");
    }




    void append_redirect(char * arg){
            /* 
            * function to append the out put of a process to
            * a file that is specified in args.
            * param: is the command line input
            */

            close(1);
            int fd, ret;
            fd = open(arg, O_CREAT | O_APPEND | O_RDWR, 0644);
            if(fd < 0) printf("redirect failed\n");
    }

    void overwrite_redirect(char * arg){
            /* 
            * function to overwrite the out put of a process to
            * a file that is specified in args.
            * param: is the command line input
            */

            close(1);
            int fd;
            fd = open(arg, O_CREAT | O_TRUNC | O_RDWR, 0644);
            if(fd < 0) printf("redirect failed\n");

    }



    int cmd_case(char ** input){

            // Cases:
            // "Case 1"  :  a [args] 
            // "Case 2"  :  a [args] > .txt 
            // "Case 3"  :  a [args] >> .txt
            // "Case 4"  :  a [args] | b [args] 
            // "Case 5"  :  a [args] | b [args] | c [args]
            // "Case 6"  :  a [args] | b [args] | c [args] > .txt
            // "Case 7"  :  a [args] | b [args] | c [args] >> .txt
            // "Case 8"  :  a [args] % b [args]
            // "Case 9"  :  a [args] | b [args] > .txt 
            // "Case 10" :  a [args] | b [args] >> .txt 
            // "Case 11" :  a [args] % b [args] > .txt
            // "Case 12" :  a [args] % b [args] >> .txt
            // returns -1 on failure

            // flags for parsing input & determining case
            // initializing them all to 0; 1 indicates that that operator is present

            int PIPE1, PIPE2, TEE, OW, AP;
            PIPE1 = PIPE2 = TEE = OW = AP = 0;
            //Walk input strings, find operators, set flags accordingly
            char * c; //holder string
            int g; //counter variable
            int case_no = -1; // holds the case number to return 

            for (g = 0; input[g] != NULL; g++){

                    c = input[g];
                    //printf("input[%d] :%s\n" , g, input[g]);
                    if(strcmp(input[g], "|") == 0){

                            if (PIPE1 == 0){
                                    PIPE1 = 1;
                            }
                            else{
                                    PIPE2 = 1;
                            }

                    }
                    else if (strcmp(input[g], "%") == 0){
                            TEE = 1;
                    }
                    else if (strcmp(input[g], ">") == 0){
                            OW = 1;
                    }
                    else if(strcmp(input[g], ">>") == 0){
                            AP = 1;
                    }

            }
            //printf("PIPE1 :%d, PIPE2 :%d, OW :%d, AP :%d, TEE :%d \n", PIPE1,PIPE2,OW,AP,TEE);

            //Case 1 is taken care of.
            //Case 2: OW
            if ((OW == 1) && (PIPE1 == 0) && (PIPE2 == 0) && (TEE == 0) && (AP  == 0)){
                    //printf("Case 2!\n");
                    case_no = 2;

            }
            //Case 3: AP
            else if ((OW == 0) && (PIPE1 == 0) && (PIPE2 == 0) && (TEE == 0) && (AP  == 1)){
                    //printf("Case 3!\n");
                    case_no = 3;

            }
            //Case 4: 1 pipe
            else if ((OW == 0) && (PIPE1 == 1) && (PIPE2 == 0) && (TEE == 0) && (AP  == 0)){
                    //printf("Case 4!\n");
                    case_no = 4;

            }
            //Case 9: 1 pipe + OW (moved so all cases with 1 pipe are together)
            else if ((OW == 1) && (PIPE1 == 1) && (PIPE2 == 0) && (TEE == 0) && (AP  == 0)){
                    //printf("Case 9!\n");
                    case_no = 9;

            }
            //Case 10: 1 pipe + AP
            else if ((OW == 0) && (PIPE1 == 1) && (PIPE2 == 0) && (TEE == 0) && (AP  == 1)){
                    //printf("Case 10!\n");
                    case_no = 10;

            }
            //Case 5: 2 pipes
            else if ((OW == 0) && (PIPE1 == 1) && (PIPE2 == 1) && (TEE == 0) && (AP  == 0)){
                    //printf("Case 5!\n");
                    case_no = 5;

            }
            //Case 6: 2 pipes + OW
            else if ((OW == 1) && (PIPE1 == 1) && (PIPE2 == 1) && (TEE == 0) && (AP  == 0)){
                    //printf("Case 6!\n");
                    case_no = 6;

            }
            //Case 7: 2 pipes + AP
            else if ((OW == 0) && (PIPE1 == 1) && (PIPE2 == 1) && (TEE == 0) && (AP  == 1)){
                    //printf("Case 7!\n");
                    case_no = 7;
            }
            //Case 8: Tee
            else if ((OW == 0) && (PIPE1 == 0) && (PIPE2 == 0) && (TEE == 1) && (AP  == 0)){
                    //printf("Case 8!\n");
                    case_no = 8;

            }
            //Case 11: Tee + OW (remember we stuck 9 and 10 further up)
            else if ((OW == 1) && (PIPE1 == 0) && (PIPE2 == 0) && (TEE == 1) && (AP  == 0)){
                    //printf("Case 11!\n");
                    case_no = 11;

            }
            //Case 12: Tee + AP
            else if ((OW == 0) && (PIPE1 == 0) && (PIPE2 == 0) && (TEE == 1) && (AP  == 1)){
                    //printf("Case 12!\n");
                    case_no = 12;

            }
            else{
                    //printf("Case 1\n");
                    case_no = 1;
            }

            return case_no;



    }
    /*
    
        more possible piping options in the works
        
        
                                    // "Case 5"  :  a [args] | b [args] | c [args]
                                    else if(case_no == 5){

                                    for (counter = 0; strcmp(input[counter], "|") != 0; counter ++){
                                                    first[counter] = input[counter];

                                            }

                                            counter++;

                                            for (; strcmp(input[counter], "|") != 0; counter ++){
                                                    second[counter] = input[counter];

                                            }
                                            counter++;
                                            for (; input[counter] != NULL; counter ++){
                                                    third[counter] = input[counter];

                                            }

                                            char * filename;

                                            for(counter = 0; third[counter] != NULL; counter++){
                                                    if(strcmp(third[counter], ">>") == 0){
                                                            filename = third[counter + 1];
                                                    }
                                            }


                                            int pid;
                                            int pipes1[2];
                                            int pipes2[2];
                                            pipe(pipes1);
                                            pipe(pipes2);

                                            pid = fork(); // fork 1


                                            pipes1[0];
                                            pipes1[1];
                                            pipes2[0];
                                            pipes2[1];

                                            if(pid == 0){

                                                    close(pipes1[1]); // close stdout and copies stdin to that spot
                                                    dup2(pipes1[0], 0); // "write end" // write to stdin

                                                    execvp(first[0], first);

                                            }
                                            else if(pid > 0){
                                                    wait(&status);

                                            }

                                            pid = fork(); // fork 2

                                            if(pid == 0){ // the middle child, needs 2 pipes

                                                    close(pipes1[0]); //close stdin and copies stdout to that spot
                                                    dup2(pipes1[1], 0); // "read end" // read from stdout of pipe1

                                                    close(pipes2[1]); // close stdout and copies stdin to that spot
                                                    dup2(pipes2[0], 0); // "write end"

                                                    execvp(second[0], second);

                                            }
                                            else if(pid > 0){
                                                    wait(&status);

                                            }

                                            pid = fork(); // fork 3

                                            if(pid == 0){

                                                    close(pipes2[0]); //close stdin and copies stdout to that spot
                                                    dup2(pipes2[1], 0); // "read end" // read from stdout

                                                    execvp(third[0], third);

                                            }
                                            else if(pid > 0){
                                                    wait(&status);
                                            }

                                    }
                                    // "Case 6"  :  a [args] | b [args] | c [args] > .txt
                                    else if(case_no == 6){


                                    for (counter = 0; strcmp(input[counter], "|") != 0; counter ++){
                                                    first[counter] = input[counter];

                                            }

                                            counter++;

                                            for (; strcmp(input[counter], "|") != 0; counter ++){
                                                    second[counter] = input[counter];

                                            }
                                            counter++;
                                            for (; input[counter] != NULL; counter ++){
                                                    third[counter] = input[counter];

                                            }

                                            char * filename;

                                            for(counter = 0; third[counter] != NULL; counter++){
                                                    if(strcmp(third[counter], ">>") == 0){
                                                            filename = third[counter + 1];
                                                    }
                                            }


                                            int pid;
                                            int pipes1[2];
                                            int pipes2[2];
                                            pipe(pipes1);
                                            pipe(pipes2);

                                            pid = fork(); // fork 1


                                            pipes1[0];
                                            pipes1[1];
                                            pipes2[0];
                                            pipes2[1];

                                            if(pid == 0){

                                                    close(pipes1[1]); // close stdout and copies stdin to that spot
                                                    dup2(pipes1[0], 0); // "write end"

                                                    execvp(first[0], first);

                                            }
                                            else if(pid > 0){
                                                    wait(&status);

                                            }

                                            pid = fork(); // fork 2

                                            if(pid == 0){ // the middle child, needs 2 pipes

                                                    close(pipes1[0]); //close stdin and copies stdout to that spot
                                                    dup2(pipes1[1], 0); // "read end" // read from stdout of pipe1

                                                    close(pipes2[1]); // close stdout and copies stdin to that spot
                                                    dup2(pipes2[0], 0); // "write end"

                                                    execvp(second[0], second);

                                            }
                                            else if(pid > 0){
                                                    wait(&status);

                                            }

                                            pid = fork(); // fork 3

                                            if(pid == 0){

                                                    close(pipes2[0]); //close stdin and copies stdout to that spot
                                                    dup2(pipes2[1], 0); // "read end" // read from stdout

                                                    char * filename;

                                                    for(counter = 0; input[counter] != NULL; counter++){
                                                    if(strcmp(input[counter], ">") == 0){
                                                            filename = input[counter + 1];
                                                    }

                                                    overwrite_redirect(filename);
                                                    execvp(third[0], third);

                                            }
                                    }
                                            else if(pid > 0){
                                                    wait(&status);
                                            }

                                    }
                                    // "Case 7"  :  a [args] | b [args] | c [args] >> .txt
                                    else if(case_no == 7){

                                            for (counter = 0; strcmp(input[counter], "|") != 0; counter ++){
                                                    first[counter] = input[counter];

                                            }

                                            counter++;

                                            for (; strcmp(input[counter], "|") != 0; counter ++){
                                                    second[counter] = input[counter];

                                            }
                                            counter++;
                                            for (; input[counter] != NULL; counter ++){
                                                    third[counter] = input[counter];

                                            }

                                            char * filename;

                                            for(counter = 0; third[counter] != NULL; counter++){
                                                    if(strcmp(third[counter], ">>") == 0){
                                                            filename = third[counter + 1];
                                                    }
                                            }


                                            int pid;
                                            int pipes1[2];
                                            int pipes2[2];
                                            pipe(pipes1);
                                            pipe(pipes2);

                                            pid = fork(); // fork 1


                                            pipes1[0];
                                            pipes1[1];
                                            pipes2[0];
                                            pipes2[1];

                                            if(pid == 0){

                                                    close(pipes1[1]); // close stdout and copies stdin to that spot
                                                    dup2(pipes1[0], 0); // "write end"

                                                    execvp(first[0], first);

                                            }
                                            else if(pid > 0){
                                                    wait(&status);

                                            }

                                            pid = fork(); // fork 2

                                            if(pid == 0){ // the middle child, needs 2 pipes

                                                    close(pipes1[0]); //close stdin and copies stdout to that spot
                                                    dup2(pipes1[1], 0); // "read end" // read from stdout of pipe1

                                                    close(pipes2[1]); // close stdout and copies stdin to that spot
                                                    dup2(pipes2[0], 0); // "write end"

                                                    execvp(second[0], second);

                                            }
                                            else if(pid > 0){
                                                    wait(&status);

                                            }

                                            pid = fork(); // fork 3

                                            if(pid == 0){

                                                    close(pipes2[0]); //close stdin and copies stdout to that spot
                                                    dup2(pipes2[1], 0); // "read end" // read from stdout

                                                    char * filename;

                                                    for(counter = 0; input[counter] != NULL; counter++){
                                                    if(strcmp(input[counter], ">") == 0){
                                                            filename = input[counter + 1];
                                                    }

                                                    append_redirect(filename);
                                                    execvp(third[0], third);

                                            }
                                            }
                                            else if(pid > 0){
                                                    wait(&status);
                                            }

                                    }
                                    // "Case 8"  :  a [args] % b [args]
                                    else if(case_no == 8){


                                    for (counter = 0; strcmp(input[counter], "%") != 0; counter ++){
                                                    first[counter] = input[counter];

                                            }
                                            counter++;
                                            for (; input[counter] != NULL; counter ++){
                                                    second[counter] = input[counter];

                                            }


                                            int pid;
                                            int pipes1[2];
                                            int pipes2[2];
                                            pipe(pipes1);
                                            pipe(pipes2);

                                            pid = fork(); // fork 1


                                            pipes1[0];
                                            pipes1[1];
                                            pipes2[0];
                                            pipes2[1];

                                            if(pid == 0){

                                                    close(pipes1[1]); // close stdout and copies stdin to that spot
                                                    dup2(pipes1[0], 0); // "write end"

                                                    execvp(first[0], first);

                                            }
                                            else if(pid > 0){
                                                    wait(&status);
                                                    printf("Print statement 1\n");

                                            }

                                            pid = fork(); // fork 2
                                            printf("Print statement 6\n");
                                            if(pid == 0){ // mytee, needs 2 pipes


                                                    close(pipes1[0]); //close stdin and copies stdout to that spot

                                                    dup2(pipes1[1], 0); // "read end" // read from stdout of pipe1
                                                    close(pipes2[1]); // close stdout and copies stdin to that spot

                                                    dup2(pipes2[0], 0); // "write end"


                                                    char * args[2];
                                                    args[0] = "mytee";
                                                    args[1] = NULL;

                                                    execvp(args[0], args);
                                                    printf("Print statement 9\n");
                                            }
                                            else if(pid > 0){
                                                    wait(&status);
                                                    printf("Print statement 2\n");
                                            }

                                            pid = fork(); // fork 3

                                            if(pid == 0){

                                                    close(pipes2[0]); //close stdin and copies stdout to that spot
                                                    dup2(pipes2[1], 0); // "read end" // read from stdout

                                                    execvp(second[0], second);

                                            }
                                            else if(pid > 0){
                                                    wait(&status);
                                                    printf("Print statement 3\n");
                                            }


                                    }
                                    // "Case 9"  :  a [args] | b [args] > .txt 
                                    else if(case_no == 9){

                                            for (counter = 0; strcmp(input[counter], "|") != 0; counter ++){
                                                    first[counter] = input[counter];

                                            }

                                            counter++;

                                            for (; input[counter] != NULL; counter ++){
                                                    second[counter] = input[counter];

                                            }

                                            int pid;
                                            int pipes[2];
                                            pipe(pipes);

                                            pid = fork();


                                            pipes[0];
                                            pipes[1];

                                            if(pid == 0){

                                                    close(pipes[1]); // close stdout and copies stdin to that spot
                                                    dup2(pipes[0], 0); // "write end"

                                                    execvp(first[0], first);

                                            }
                                            else if(pid > 0){
                                                    wait(&status);

                                            }

                                            pid = fork();
                                            if(pid == 0){

                                                    close(pipes[0]); //close stdin and copies stdout to that spot
                                                    dup2(pipes[1], 1); // "read end"

                                                    char * filename;

                                                    for(counter = 0; input[counter] != NULL; counter++){
                                                    if(strcmp(input[counter], ">") == 0){
                                                            filename = input[counter + 1];
                                                    }

                                                    overwrite_redirect(filename);

                                                    execvp(second[0], second);

                                            }
                    }
                                            else if(pid > 0){
                                                    wait(&status);
                                            }

                                    }
                                    // "Case 10" :  a [args] | b [args] >> .txt
                                    else if(case_no == 10){

                                            for (counter = 0; strcmp(input[counter], "|") != 0; counter ++){
                                                    first[counter] = input[counter];

                                            }

                                            counter++;

                                            for (; input[counter] != NULL; counter ++){
                                                    second[counter] = input[counter];

                                            }

                                            int pid;
                                            int pipes[2];
                                            pipe(pipes);

                                            pid = fork();


                                            pipes[0];
                                            pipes[1];

                                            if(pid == 0){

                                                    close(pipes[1]); // close stdout and copies stdin to that spot
                                                    dup2(pipes[0], 0); // "write end"

                                                    execvp(first[0], first);

                                            }
                                            else if(pid > 0){
                                                    wait(&status);

                                            }

                                            pid = fork();
                                            if(pid == 0){

                                                    close(pipes[0]); //close stdin and copies stdout to that spot
                                                    dup2(pipes[1], 1); // "read end"

                                                    char * filename;

                                                    for(counter = 0; input[counter] != NULL; counter++){
                                                    if(strcmp(input[counter], ">") == 0){
                                                            filename = input[counter + 1];
                                                    }

                                                    append_redirect(filename);

                                                    execvp(second[0], second);

                                            }

                                            }
                                            else if(pid > 0){
                                                    wait(&status);
                                            }

                                    }
                                    // "Case 11" :  a [args] % b [args] > .txt
                                    else if(case_no == 11){

                                    }
                                    // "Case 12" :  a [args] % b [args] >> .txt
                                    else if(case_no == 12){

                                    }*/
    