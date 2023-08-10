#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <signal.h>
#define MAX_USERHOST_LEN 1024
#define LIMIT_FOR_DIR 2048
#define PATH_SIZE 1000
#define SIZE_OF_INPUTS 500
#define RESET_COLOR "\e[m" 
#define MAKE_GREEN "\e[32m" 
#define MAKE_BLUE "\e[36m"
/* DONE
cd . .. ~ - xxxxx
ls . .. -a, -l, -l -a, -a -l, -al, -la
echo
pwd
pinfo
history
*/
char* username;
char system_name[MAX_USERHOST_LEN];
char curr_dir[LIMIT_FOR_DIR];
char absolutepath[PATH_SIZE];
char old_dir[1200];
char hist[LIMIT_FOR_DIR];
char cout[LIMIT_FOR_DIR];
char *entry;
int entry_size = 2000;
int code_cd(char **command, int len)
{
    char extension1[] = ".";
    char extension2[] = "..";
    char extension3[] = "~";
    char extension4[] = "-";
    if (len == 1 || (len == 2 && !strcmp(command[1], extension3))){ //cd ~
        getcwd(old_dir, 1200);
        chdir(absolutepath);
    }
    else if (len == 2 && !strcmp(command[1], extension4)){ // cd -
        printf("%s\n", old_dir);
    }
    else if ((len == 2 && !strcmp(command[1], extension1))){ //cd .
        getcwd(old_dir, 1200);
        char dir_req[1200];
        getcwd(dir_req, 1200);
        chdir(dir_req);
    }
    else if ((len == 2 && !strcmp(command[1], extension2))){ // cd ..
        getcwd(old_dir, 1200);
        char dir_req[1200];
        getcwd(dir_req, 1200);
        char *F_occurence;
        char seperator = '/';
        F_occurence = strrchr(dir_req, seperator);
        char ifn[25];
        int i = 0, j = 0;
        if(F_occurence){
            for (i = 0, j = 0; i <= (F_occurence - dir_req) - 1; i++, j++)
            {
                ifn[j] = dir_req[i];
            }
            ifn[j] = '\0';
        }
        chdir(ifn);
    }
    else if(getcwd(old_dir, 1200) && chdir(command[1]) != 0){
        perror("there is a problem in cd command arguments");
    }
    else{
        chdir(command[1]);
    }
    return 1;
}
int print_pro(char *path)
{
    struct stat buff;
    int blocks;
    //printf("%d\n", s);
    //printf("%s\n", path);
    if(stat(path, &buff) != -1){
        printf((S_ISDIR(buff.st_mode)) ? "d" : "-");
        printf((buff.st_mode & S_IRUSR) ? "r" : "-");
        printf((buff.st_mode & S_IWUSR) ? "w" : "-");
        printf((buff.st_mode & S_IXUSR) ? "x" : "-");
        printf((buff.st_mode & S_IRGRP) ? "r" : "-");
        printf((buff.st_mode & S_IWGRP) ? "w" : "-");
        printf((buff.st_mode & S_IXGRP) ? "x" : "-");
        printf((buff.st_mode & S_IROTH) ? "r" : "-");
        printf((buff.st_mode & S_IWOTH) ? "w" : "-");
        printf((buff.st_mode & S_IXOTH) ? "x " : "- ");
        printf("%li ", buff.st_nlink);
        struct passwd *pass;
        pass = getpwuid(buff.st_uid);
        if(pass == NULL){
            perror("getpwuid");
            printf("%d ", buff.st_uid);
        }
        else{
            printf("%s ", pass->pw_name);
        }
        struct group *grp;
        grp = getgrgid(buff.st_gid);
        if(grp == NULL){
            perror("getgrgid");
            printf("%d ", buff.st_gid);
        }
        else{
            printf("%s ", grp->gr_name);
        }
        printf("%ld ", buff.st_size);
        struct tm *time;
        char output[500];
        time = localtime(&(buff.st_mtime));
        if(time == NULL){perror("localtime"); exit(0);}
        strftime(output, sizeof(output), "%b %d %R", time);
        printf("%s ", output);
        blocks = buff.st_blocks;
    }
    return blocks;
}
int checkIfFileExists(const char* filename){
    struct stat buffer;
    int exist = stat(filename,&buffer);
    if(exist == 0){return 1;}
    else{return 0;}
}
int isDirectory(const char *path) {
   struct stat statbuf;
   if(stat(path, &statbuf) != 0){return 0;}
   return S_ISDIR(statbuf.st_mode);
}
int numberofones(int a[], int n)
{
    int count = 0;
    for(int i = 0; i < n; i++){
        if(a[i] == 1){count++;}
    }
    return count;
}
int ls_traverse(char *path)
{
    struct dirent *directory;
    DIR *curr_direc = opendir(path);
    if (curr_direc == NULL){
        printf("Cannot open current directory");
        return 0;
    }
    while ((directory = readdir(curr_direc)) != NULL){
        if (directory->d_name[0] != '.'){
            printf("%s\n", directory->d_name);
            char temp[1000];
            strcpy(temp, path);
            strcat(temp, "/");
            strcat(temp, directory->d_name);
            if(isDirectory(temp)){
                ls_traverse(temp);
            }
        }
    }
    closedir(curr_direc);
}
int code_ls(char **command, int len)
{
    int a[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    char al_case[9][10];
    char extension1[] = ".";    //a0
    char extension2[] = "..";   //a1
    char extension3[] = "-a";   //a2
    char extension4[] = "-l";   //a3
    char extension5[] = "-l -a";//a4
    char extension6[] = "-a -l";//a5
    char extension7[] = "-la";  //a6
    char extension8[] = "-al";  //a7
    char extension9[] = "~";    //a8

    strcpy(al_case[0], extension1); 
    strcpy(al_case[1], extension2); 
    strcpy(al_case[2], extension3); 
    strcpy(al_case[3], extension4);
    strcpy(al_case[4], extension5);
    strcpy(al_case[5], extension6);
    strcpy(al_case[6], extension7);
    strcpy(al_case[7], extension8);
    strcpy(al_case[8], extension9);

    struct dirent *directory;
    DIR *curr_direc;
    for (int i = 1; i < len; i++){
        for (int j = 0; j < 9; j++){
            if (!strcmp(command[i], al_case[j])){
                a[j] = 1;
            }
        }
    }

    if (len == 1 || ((len == 2) && (a[0] == 1))){ //ls ,ls .
onedot:
        curr_direc = opendir(".");
        if (curr_direc == NULL){
            printf("Cannot open current directory");
            return 0;
        }
        while ((directory = readdir(curr_direc)) != NULL){
            if (directory->d_name[0] != '.'){
                struct stat buffer;
                char tempo[1000];
                char *path_type = getcwd(tempo, 1000);
                strcat(path_type, "/");   
                strcat(path_type, directory->d_name);
                if(stat(path_type, &buffer) != -1){
                    if(S_ISDIR(buffer.st_mode)){
                        printf(MAKE_BLUE"%s\n" RESET_COLOR,directory->d_name);
                    }
                    else if(buffer.st_mode & S_IXUSR){
                        printf(MAKE_GREEN"%s\n" RESET_COLOR, directory->d_name);
                    }
                    else{
                        printf("%s\n", directory->d_name);
                    }
                }
            }
        }
        closedir(curr_direc);
        return 0;
    }
    else if ((len == 2) && (a[1] == 1)){ //ls ..
twodots:
        curr_direc = opendir("./..");
        if (curr_direc == NULL){
            printf("Cannot open current directory");
            return 0;
        }
        while ((directory = readdir(curr_direc)) != NULL){
            if (directory->d_name[0] != '.'){
                struct stat buffer;
                char tempo[1000];
                char *path_type = getcwd(tempo, 1000);
                strcat(path_type, "/");   
                strcat(path_type, directory->d_name);
                if(stat(path_type, &buffer) != -1){
                    if(S_ISDIR(buffer.st_mode)){
                        printf(MAKE_BLUE"%s\n" RESET_COLOR,directory->d_name);
                    }
                    else if(buffer.st_mode & S_IXUSR){
                        printf(MAKE_GREEN"%s\n" RESET_COLOR, directory->d_name);
                    }
                    else{
                        printf("%s\n", directory->d_name);
                    }
                }
            }
        }
        closedir(curr_direc);
        return 0;
    }
    else if ((len == 2) && (a[2] == 1)){ //ls -a
        curr_direc = opendir(".");
        if (curr_direc == NULL)
        {
            printf("Cannot open current directory");
            return 0;
        }
        while ((directory = readdir(curr_direc)) != NULL){
            struct stat buffer;
                char tempo[1000];
                char *path_type = getcwd(tempo, 1000);
                strcat(path_type, "/");   
                strcat(path_type, directory->d_name);
                if(stat(path_type, &buffer) != -1){
                    if(S_ISDIR(buffer.st_mode)){
                        printf(MAKE_BLUE"%s\n" RESET_COLOR,directory->d_name);
                    }
                    else if(buffer.st_mode & S_IXUSR){
                        printf(MAKE_GREEN"%s\n" RESET_COLOR, directory->d_name);
                    }
                    else{
                        printf("%s\n", directory->d_name);
                    }
                }
        }
        closedir(curr_direc);
        return 0;
    }
    else if ((len == 2) && (a[3] == 1)){ //ls -l
        curr_direc = opendir(".");
        char temp[1000];
        int total = 0;
        if (curr_direc == NULL){
            printf("Cannot open current directory");
            return 0;
        }
        while ((directory = readdir(curr_direc)) != NULL){
            if (directory->d_name[0] != '.'){
                char *path1 = getcwd(temp, 1000);
                strcat(path1, "/");   
                strcat(path1, directory->d_name);
                struct stat buf;
                if(directory->d_name[0] != '.'){
                    if(stat(path1, &buf) != -1){
                        total += buf.st_blocks;
                    }
                }
            }
        }
        printf("total %d\n", total/2);
        rewinddir(curr_direc);
        while ((directory = readdir(curr_direc)) != NULL){
            if (directory->d_name[0] != '.'){
                char *path = getcwd(temp, 1000);
                strcat(path, "/");   
                strcat(path, directory->d_name);
                print_pro(path);
                struct stat buffer;
                char tempo[1000];
                char *path_type = getcwd(tempo, 1000);
                strcat(path_type, "/");   
                strcat(path_type, directory->d_name);
                if(stat(path_type, &buffer) != -1){
                    if(S_ISDIR(buffer.st_mode)){
                        printf(MAKE_BLUE"%s\n" RESET_COLOR,directory->d_name);
                    }
                    else if(buffer.st_mode & S_IXUSR){
                        printf(MAKE_GREEN"%s\n" RESET_COLOR, directory->d_name);
                    }
                    else{
                        printf("%s\n", directory->d_name);
                    }
                }
            }
        }
        closedir(curr_direc);
        return 0;
    }
    else if ((len == 3 && a[2] == 1 & a[3] == 1) || ((len == 2) && a[6] == 1) ||((len == 2) && a[7] == 1) ){ //ls -l -a, ls -a -l, ls -al, ls -la
        curr_direc = opendir(".");
        char temp[1000];
        int total = 0;
        if (curr_direc == NULL){
            printf("Cannot open current directory");
            return 0;
        }
        while ((directory = readdir(curr_direc)) != NULL){
            if (directory->d_name[0] != '.'){
                char *path1 = getcwd(temp, 1000);
                strcat(path1, "/");   
                strcat(path1, directory->d_name);
                struct stat buf;
                if(stat(path1, &buf) != -1){
                    total += buf.st_blocks; 
                }
            }
        }
        printf("total %d\n", total/2);
        rewinddir(curr_direc);
        while ((directory = readdir(curr_direc)) != NULL){
            char *path = getcwd(temp, 1000);
            strcat(path, "/");   
            strcat(path, directory->d_name);
            print_pro(path);
            struct stat buffer;
                char tempo[1000];
                char *path_type = getcwd(tempo, 1000);
                strcat(path_type, "/");   
                strcat(path_type, directory->d_name);
                if(stat(path_type, &buffer) != -1){
                    if(S_ISDIR(buffer.st_mode)){
                        printf(MAKE_BLUE"%s\n" RESET_COLOR,directory->d_name);
                    }
                    else if(buffer.st_mode & S_IXUSR){
                        printf(MAKE_GREEN"%s\n" RESET_COLOR, directory->d_name);
                    }
                    else{
                        printf("%s\n", directory->d_name);
                    }
                }

        }
        closedir(curr_direc);
        return 0;
    }
    else if ((len == 2) && (a[8] == 1)){ //ls ~
tilda:
        curr_direc = opendir(absolutepath);
        printf("%s\n", absolutepath);
        if (curr_direc == NULL){
            printf("Cannot open current directory");
            return 0;
        }
        while ((directory = readdir(curr_direc)) != NULL){
            if (directory->d_name[0] != '.'){
                struct stat buffer;
                char tempo[1000];
                char *path_type = getcwd(tempo, 1000);
                strcat(path_type, "/");   
                strcat(path_type, directory->d_name);
                if(stat(path_type, &buffer) != -1){
                    if(S_ISDIR(buffer.st_mode)){
                        printf(MAKE_BLUE"%s\n" RESET_COLOR,directory->d_name);
                    }
                    else if(buffer.st_mode & S_IXUSR){
                        printf(MAKE_GREEN"%s\n" RESET_COLOR, directory->d_name);
                    }
                    else{
                        printf("%s\n", directory->d_name);
                    }
                }
            }
        }
        closedir(curr_direc);
        return 0;
    }

    else{ //ls <flags><directory_name_path/filename>
        char dir_req[1200];
        getcwd(dir_req, 1200);
        int decider = numberofones(a, 9);
loop:
        if(chdir(command[decider+1]) != 0){
            if(checkIfFileExists(command[decider+1])){printf("%s\n", command[decider+1]);}
            else{perror("there exist no such file/directory");}
            chdir(dir_req);
            strcat(dir_req, "/");
            strcat(dir_req, command[decider+1]);
        }
        else{
            chdir(dir_req);
            strcat(dir_req, "/");
            strcat(dir_req, command[decider+1]);
            ls_traverse(dir_req);
        }    
        if(a[0] == 1){
            printf("\n.:\n\n");
            goto onedot;
        }
        if(a[1] == 1){
            printf("\n..:\n\n");
            goto twodots;
        }
        if(a[8] == 1){
            printf("\n~:\n\n");
            goto tilda;
        }
        if(a[3] == 1){
            print_pro(dir_req);
            printf("%s\n", command[decider+1]);
        }
        if(len - (decider + 1) > 1){
            getcwd(dir_req, 1200);
            decider++;
            goto loop;
        }
        return 0;
    }
}
int code_pinfo(char **command, int p)
{
    char process_name[1000];
    char file_name[150];
    char name[50];
    char process_status;
    char buffer[1000];
    char exe_path[1000];
    int process_ID;
    int memory;
    int a, b;
    int path_length = strlen(absolutepath);
    strcpy(process_name, "/proc/");
    if (command[1] != NULL){
        strcat(process_name, command[1]);
    }
    else{
        strcat(process_name, "self");
    }
    strcpy(file_name, process_name);
    strcat(file_name, "/stat");
    FILE *file_pointer = fopen(file_name, "r");
    if (file_pointer == NULL){
        printf("Process not found\n");
    }
    fscanf(file_pointer, "%*s %*s %c %*s %d %*s %*s %d %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %d", &process_status, &a, &b, &memory);
    fscanf(file_pointer, "%d", &process_ID);
    printf("pid : %d\n", process_ID);
    printf("Process Status : %c", process_status);
    (a==b)?printf("+\n") : printf(" ");
    fclose(file_pointer);

    strcpy(file_name, process_name);
    strcat(file_name, "/exe");
    readlink(file_name, buffer, sizeof(buffer));

    int index = 0;
    while (index < path_length){
        if (absolutepath[index] != buffer[index])
            break;
        index++;
    }
    if (index != path_length){
        strcpy(exe_path, buffer);
        exe_path[strlen(buffer)] = '\0';
    }
    else{
        exe_path[0] = '~';
        exe_path[1] = '\0';
        strcat(exe_path, (const char *)&buffer[path_length]);
    }
    int i, j = 0;
    while (buffer[i]){
        j++;
        buffer[i++] = '\0';
    }
    if (strlen(exe_path))
        fprintf(stdout, "Executable Path : %s\n", exe_path);
    return 1;
}
int code_pwd(char **command)
{
    char dir_req[1200];
    getcwd(dir_req, 1200);
    printf("%s\n", dir_req);
    return 0;
}
int code_echo(char **command, int len)
{
    int count = 0;
    for (int i = 1; i < len - 1; i++){
        if(!strcmp(command[i], ">")){count++;}
        if(!strcmp(command[i], ">>")){count++;}
        if(!strcmp(command[i], "<")){count++;}
    }
    if(len != 1){
        for (int i = 1; i < len - 1; i++)
        {
            if(!strcmp(command[i], ">")){
                FILE *flp;
                flp =fopen(command[i + 1], "w");
                for(int k = 1; k < i; k++){
                    fprintf(flp, "%s ", command[k]);
                }
                fprintf(flp, "\n");
                fclose(flp);
                return 0;
            }
            if(!strcmp(command[i], ">>")){
                FILE *flp;
                flp =fopen(command[i + 1], "a");
                for(int k = 1; k < i; k++){
                    fprintf(flp, "%s ", command[k]);
                }
                fprintf(flp, "\n");
                fclose(flp);
                return 0;
            }
            if(!strcmp(command[i], "<")){printf("enter2");}
            if(count == 0){printf("%s ", command[i]);}
        }
        printf("%s", command[len - 1]);
    }
    printf("\n");
    return 0;
}
int history_storage(char **command, int len)
{
    strcpy(cout, absolutepath);
    strcat(cout, "/count.txt");   
    strcpy(hist, absolutepath);
    strcat(hist, "/history.txt");
    FILE *history = fopen(hist, "a");
    FILE *count1 = fopen(cout, "w");
    char count = fgetc(count1);
    count = count - '0';
    count = count + 1;
    fprintf(count1, "%d\n", count);
    for (int i = 0; i < len - 1; i++)
    {
        fprintf(history, "%s ", command[i]);
    }
    fprintf(history, "%s", command[len - 1]);
    fprintf(history,"\n");
    fclose(history);
    return 0;
}

int code_history(char **command, int len)
{
    strcpy(hist, absolutepath);
    strcat(hist, "/history.txt");
    FILE *history = fopen(hist, "r");
    char cursor;
    cursor = fgetc(history);
    while(cursor != EOF)
    {
        printf("%c", cursor);
        cursor = fgetc(history);
    }
    return 0;
}
void handle_sigchld(int signal)
{
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        fprintf(stderr,"\nprocess with pid %d exited normally\n", pid);
    }
}
int code_exec(char **command, int len)
{
    char *input_file_name = {0}, *output_file_name = {0};
    int sep1, sep2;
    FILE *f = fopen("jobs.txt", "a");
    //printf("%d\n", len);
    for(int i = 0; i < len; i++){
        if(!strcmp(command[i], "<")){
            sep1 = i;
            input_file_name = command[i + 1];
        }
        if(!strcmp(command[i], ">")){
            sep2 = i;
            output_file_name = command[i + 1];
        }
    }
    if(input_file_name != NULL || output_file_name != NULL){  
        command[1] = input_file_name;
        command[2] = '\0';
        command[3] = '\0';   
    }
    len = 2;
    if(output_file_name == NULL){
        pid_t pid;
        pid_t wpid;
        int status, back_ground = 0, valve;
        for(int i = 0; i < len; i++){
            for(int j = 0; j < 9; j++){
                if(command[i][j] == '&'){
                    command[i][j] = '\0';
                    back_ground = 1;
                    break;
                }
            }
        }
        if ((pid = fork()) < 0){
            perror("fork()");
            exit(0);
        }
        int initial = time(0);
        if (pid == 0){
            if(back_ground){
                if(execvp(*command, command) < 0){
                    perror("execvp()");
                    exit(0);
                }
            }
        }
        if(pid == 0){
            if(!back_ground){
                if(execvp(*command, command) < 0){
                    perror("execvp()");
                    exit(0);
                }
            }
        }
        else{
            if(back_ground){
                for (int i = 0; i < len - 1; i++)
                {
                    fprintf(f, "%s ", command[i]);
                }
                fprintf(f, "%s", command[len - 1]);
                fprintf(f,"\n");
                fclose(f);
                signal(SIGCHLD, handle_sigchld);
                return 0;
            }
            else{
                for (int i = 0; i < len - 1; i++)
                {
                    fprintf(f, "%s ", command[i]);
                }
                fprintf(f, "%s", command[len - 1]);
                fprintf(f,"\n");
                fclose(f);
                while(wait(&status) != pid);
                int final = time(0);
                int interval = final - initial;
                if(interval >= 1)
                    printf("Took %ds\n", interval);
            }
        }
        printf("[%d]  %d\n", status, pid);
    }
    if(output_file_name != NULL){
        int out = open(output_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        pid_t pid;
        pid_t wpid;
        int status, back_ground = 0, valve;
        for(int i = 0; i < len; i++){
            for(int j = 0; j < 10; j++){
                if(command[i][j] == '&'){
                    command[i][j] = '\0';
                    back_ground = 1;
                    break;
                }
            }
        }
        if ((pid = fork()) < 0){
            perror("fork()");
            exit(0);
        }
        int initial = time(0);
        if (pid == 0){
            if(back_ground){
                dup2(out, 1);
                if(execvp(*command, command) < 0){
                    perror("execvp()");
                    exit(0);
                }
            }
        }
        if(pid == 0){
            if(!back_ground){
                dup2(out, 1);
                if(execvp(*command, command) < 0){
                    perror("execvp()");
                    exit(0);
                }
            }
        }
        else{
            if(back_ground){
                for (int i = 0; i < len - 1; i++)
                {
                    fprintf(f, "%s ", command[i]);
                }
                fprintf(f, "%s", command[len - 1]);
                fprintf(f,"\n");
                fclose(f);
                signal(SIGCHLD, handle_sigchld);
                return 0;
            }
            else{
                for (int i = 0; i < len - 1; i++)
                {
                    fprintf(f, "%s ", command[i]);
                }
                fprintf(f, "%s", command[len - 1]);
                fprintf(f,"\n");
                fclose(f);
                while(wait(&status) != pid);
                int final = time(0);
                int interval = final - initial;
                if(interval >= 1)
                    printf("Took %ds\n", interval);
            }
        }
        printf("[%d]  %d\n", status, pid);
    }
    return 0;
}
int traverse(char *path)
{
    struct dirent *directory;
    DIR *curr_direc = opendir(path);
    if (curr_direc == NULL){
        printf("Cannot open current directory");
        return 0;
    }
    while ((directory = readdir(curr_direc)) != NULL){
        if (directory->d_name[0] != '.'){
            printf("%s/%s\n",path, directory->d_name);
            char temp[1000];
            strcpy(temp, path);
            strcat(temp, "/");
            strcat(temp, directory->d_name);
            if(isDirectory(temp)){
                traverse(temp);
            }
        }
    }
    closedir(curr_direc);
}
int traverse_dir(char *path)
{
    struct dirent *directory;
    DIR *curr_direc = opendir(path);
    if (curr_direc == NULL){
        printf("Cannot open current directory");
        return 0;
    }
    while ((directory = readdir(curr_direc)) != NULL){
        if (directory->d_name[0] != '.'){
            char temp[1000];
            strcpy(temp, path);
            strcat(temp, "/");
            strcat(temp, directory->d_name);
            if(isDirectory(temp)){
                printf("%s/%s\n",path, directory->d_name);
                traverse_dir(temp);
            }
        }
    }
    closedir(curr_direc);
}
int traverse_file(char *path)
{
    struct dirent *directory;
    DIR *curr_direc = opendir(path);
    if (curr_direc == NULL){
        printf("Cannot open current directory");
        return 0;
    }
    while ((directory = readdir(curr_direc)) != NULL){
        if (directory->d_name[0] != '.'){
            char temp[1000];
            strcpy(temp, path);
            strcat(temp, "/");
            strcat(temp, directory->d_name);
            if(!isDirectory(temp)){printf("%s/%s\n",path, directory->d_name);}
            if(isDirectory(temp)){
                traverse_dir(temp);
            }
        }
    }
    closedir(curr_direc);
}
int code_discover(char **command, int len)
{
    int a[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    char al_case[5][10];
    char extension1[] = ".";
    char extension2[] = "..";
    char extension3[] = "~";
    char extension4[] = "-d";
    char extension5[] = "-f";
    strcpy(al_case[0], extension1); //a0
    strcpy(al_case[1], extension2); //a1
    strcpy(al_case[2], extension3); //a2
    strcpy(al_case[3], extension4); //a3
    strcpy(al_case[4], extension5); //a4

    struct dirent *directory;
    DIR *curr_direc;
    for (int i = 1; i < len; i++){
        for (int j = 0; j < 5; j++){
            if (!strcmp(command[i], al_case[j])){
                a[j] = 1;
            }
        }
    }
    if (len == 1 || (len == 2 && a[0] == 1) || (a[3] == 1 && a[4] == 1)){ //discover, discover .
        char *path = ".";
        traverse(path);
        return 0;
    }
    else if (len == 2 && a[1] == 1){ // discover ..
        char *path = "..";
        traverse(path);
        return 0;   
    }
    else if (len == 2 && a[2] == 1){ //discover ~
        char *path = "~";
        traverse(path);
        return 0;
    }
    else if (len == 2 && a[3] == 1){ //discover -d
        char *path = ".";
        traverse_dir(path);
        return 0;
    }
    else if (len == 2 && a[4] == 1){ //discover -f
        char *path = ".";
        traverse_file(path);
        return 0;
    }
    int decider = numberofones(a, 5);
    if(decider == 0){
        char dir[1000];
        strcpy(dir, ".");
        strcat(dir, "/");
        strcat(dir, command[1]);
        if(!isDirectory(dir)){
            if(checkIfFileExists(dir)){printf("%s\n", dir);}
            else{perror("there exist no such directory");}
        }
        else{
            traverse(dir);
        }
    }
    if(len == 3){
        char dir[1000];
        int b[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
        for (int j = 0; j < 5; j++){
            if (!strcmp(command[2], al_case[j])){
                b[j] = 1;
            }
        }
        if(b[1] == 1){
            strcpy(dir, "..");
            strcat(dir, "/");
            strcat(dir, command[1]);
            if(!isDirectory(dir)){perror("there exist no such directory"); return 0;}
            else{
                traverse(dir);
            }
        }
        if(b[0] == 1 || (b[3] == 1 && b[4] == 1)){
            strcpy(dir, ".");
            strcat(dir, "/");
            strcat(dir, command[1]);
            if(!isDirectory(dir)){perror("there exist no such directory"); return 0;}
            else{
                traverse(dir);
            }
        }
        if(b[3] == 1){
            strcpy(dir, ".");
            strcat(dir, "/");
            strcat(dir, command[1]);
            if(!isDirectory(dir)){perror("there exist no such directory"); return 0;}
            else{
                traverse_dir(dir);
            }
        }
        if(b[4] == 1){
            strcpy(dir, ".");
            strcat(dir, "/");
            strcat(dir, command[1]);
            if(!isDirectory(dir)){perror("there exist no such directory"); return 0;}
            else{
                traverse_file(dir);
            }
        }
    }
    if(len > 3){
        char dir[1000];
        int b[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
        for(int i = 0; i < decider; i++){
            for (int j = 0; j < 5; j++){
                if (!strcmp(command[i+2], al_case[j])){
                    b[j] = 1;
                }
            }
        }
        if(b[1] == 1){
            strcpy(dir, "..");
            strcat(dir, "/");
            strcat(dir, command[1]);
            strcat(dir, command[decider+2]);
            if(!isDirectory(dir)){
                if(checkIfFileExists(dir)){printf("%s\n", dir);}
                else{perror("there exist no such directory");}
            }
            else{
                traverse(dir);
            }
        }
        if(b[0] == 1 || (b[3] == 1 && b[4] == 1)){
            strcpy(dir, ".");
            strcat(dir, "/");
            strcat(dir, command[1]);
            strcat(dir, command[decider+2]);
            if(!isDirectory(dir)){
                if(checkIfFileExists(dir)){printf("%s\n", dir);}
                else{perror("there exist no such directory");}
            }
            else{
                traverse(dir);
            }
        }
        if(b[3] == 1){
            strcpy(dir, ".");
            strcat(dir, "/");
            strcat(dir, command[1]);
            strcat(dir, command[decider+2]);
            if(!isDirectory(dir)){
                if(checkIfFileExists(dir)){printf("%s\n", dir);}
                else{perror("there exist no such directory");}
            }
            else{
                traverse_dir(dir);
            }
        }
        if(b[4] == 1){
            strcpy(dir, ".");
            strcat(dir, "/");
            strcat(dir, command[1]);
            strcat(dir, command[decider+2]);
            if(!isDirectory(dir)){
                if(checkIfFileExists(dir)){printf("%s\n", dir);}
                else{perror("there exist no such directory");}
            }
            else{
                traverse_file(dir);
            }
        }
    }
}
int code_exit(char **command, int len)
{
    exit(0);
}
void ctrlChandle()
{
    getcwd(absolutepath, PATH_SIZE);
    username = (char *)malloc((MAX_USERHOST_LEN + 1) * sizeof(char));
    cuserid(username);
    gethostname(system_name, HOST_NAME_MAX + 1);
    char *dir;
    if (getcwd(curr_dir, sizeof(curr_dir)) != NULL){
        char *len;
        len = strstr(curr_dir, absolutepath);
        if (len){
            dir = len + strlen(absolutepath);
            printf("<");
            printf(MAKE_GREEN"%s@%s" RESET_COLOR, username, system_name);
            printf(":");
            printf(MAKE_BLUE"~%s" RESET_COLOR, dir);
            printf(">");
        }
        else{
            dir = curr_dir;
            printf("<");
            printf(MAKE_GREEN"%s@%s" RESET_COLOR, username, system_name);
            printf(":");
            printf(MAKE_BLUE"~%s" RESET_COLOR, dir);
            printf(">");
        }
    }
    return ;
}
int code_jobs(char **command, int len)
{
    FILE *f = fopen("jobs.txt", "r");
    char cursor;
    cursor = fgetc(f);
    while(cursor != EOF)
    {
        printf("%c", cursor);
        cursor = fgetc(f);
    }
    return 0;
}
int main()
{
    getcwd(absolutepath, PATH_SIZE);
    username = (char *)malloc((MAX_USERHOST_LEN + 1) * sizeof(char));
    cuserid(username);
    gethostname(system_name, HOST_NAME_MAX + 1);
    while (1){
        char *dir;
        if (getcwd(curr_dir, sizeof(curr_dir)) != NULL){
            char *len;
            len = strstr(curr_dir, absolutepath);
            if (len){
                dir = len + strlen(absolutepath);
                printf("<");
                printf(MAKE_GREEN"%s@%s" RESET_COLOR, username, system_name);
                printf(":");
                printf(MAKE_BLUE"~%s" RESET_COLOR, dir);
                printf(">");
            }
            else{
                dir = curr_dir;
                printf("<");
                printf(MAKE_GREEN"%s@%s" RESET_COLOR, username, system_name);
                printf(":");
                printf(MAKE_BLUE"~%s" RESET_COLOR, dir);
                printf(">");
            }
        }
        if (getcwd(curr_dir, sizeof(curr_dir)) == NULL){
            perror("getcwd() error");
            exit(EXIT_FAILURE);
        }
        signal(SIGINT, ctrlChandle);
        // signal(SIGTSTP, ctrlChandle);
        entry = (char *)malloc(sizeof(char) * entry_size);
        assert(entry != NULL);
        if(strcmp(entry, "\n") == 0){continue;}
        fgets(entry, entry_size, stdin);
        if(strcmp(entry, "\n") == 0){continue;}
        char *partsofentry[SIZE_OF_INPUTS];
        int count = 0;
        char delimiters_phase1[] = ";";
        partsofentry[count] = strtok(entry, delimiters_phase1);
        while (partsofentry[count] != NULL){
            count++;
            partsofentry[count] = strtok(NULL, delimiters_phase1);
        }
        for (int i = 0; i < count; i++)
        {
        L:
            if (!strcmp(partsofentry[i], "\n")){
                i++;
                if (i < count){goto L;}
            }

            char *command[SIZE_OF_INPUTS];
            int j = 0;
            char delimiters_phase2[] = "',' '\t','\n'";
            command[j] = strtok(partsofentry[i], delimiters_phase2);
            while (command[j] != NULL){
                j++;
                command[j] = strtok(NULL, delimiters_phase2);
            }
            history_storage(command, j);
            if (!strcmp(command[0], "cd")){
                code_cd(command, j);
            }
            else if (!strcmp(command[0], "ls")){       
                code_ls(command, j);
            }
            else if (!strcmp(command[0], "pwd")){
                code_pwd(command);
            }
            else if (!strcmp(command[0], "pinfo")){
                code_pinfo(command, j);
            }
            else if (!strcmp(command[0], "echo")){
                code_echo(command, j);
            }
            else if (!strcmp(command[0], "history")){
                code_history(command, j);
            }
            else if (!strcmp(command[0], "discover")){
                code_discover(command, j);
            }
            else if (!strcmp(command[0], "jobs")){
                code_jobs(command, j);
            }
            else if (!strcmp(command[0], "exit")){
                code_exit(command, j);
            }
            else{
                code_exec(command, j);
            }
        }
    }
    return 0;
}