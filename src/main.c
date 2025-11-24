#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <unistd.h>
#include <sys/stat.h>
#define MAXTOKENS 100;

size_t tokenizeCommands(char* buf, char** tokens){
  size_t tokens_len = 0;
  int is_string = 0;
  char tmptoken[256];
  tmptoken[0] = '\0';
  size_t len = 0;

  while(*buf != '\0' && tokens_len < 100){
    if(*buf == '\''){
      is_string = !is_string;
    }else if(*buf == ' '){
      if(len > 0 && !is_string){
        tmptoken[len] = '\0';
        tokens[tokens_len] = strdup(tmptoken);
        tmptoken[0] = '\0';
        tokens_len++;
        len = 0;
      }
      if(is_string){
        tmptoken[len++] = ' ';
        tmptoken[len] = '\0';
      }
    }else{
      tmptoken[len++] = *buf;
      tmptoken[len] = '\0';
    }
    buf++;
  }
  if(len > 0){
    tmptoken[len] = '\0';
    tokens[tokens_len] = strdup(tmptoken);
    tokens_len++;
  }



  // char* token = strtok(buf, " ");
  //
  // while(token != NULL){
  //   tokens[tokens_len] = strdup(token);
  //   tokens_len++;
  //   token = strtok(NULL, " ");
  // }
  return tokens_len;
}


char *valid_commands[] = {"exit", "type", "echo", "clear", "pwd"};

int isValidCommand(char* command){
  size_t size = sizeof(valid_commands)/sizeof(char*);

  for(int i = 0; i < size; i++){
    if(strcmp(command, valid_commands[i]) == 0){
      return 1;
    }
  }
  return 0;
}

int doesFileExist(char* filepath){
  return access(filepath, X_OK) == 0;
}

char* isValidPathFile(char* name){
  char* path = getenv("PATH");
  if(!path) return NULL;
  char *path_copy = strdup(path);
  if(!path_copy) return NULL;
  char* directory = strtok(path_copy, ":");
  int c = 0;
  char* directories[100];
  while(directory != NULL && c < 100){
    directories[c] = directory;
    c++;
    directory = strtok(NULL, ":");
  }


  for(int i = 0; i < c; i++){
    char filepath[256];
    filepath[0] = '\0';
    strcat(filepath, directories[i]);
    strcat(filepath, "/");
    strcat(filepath, name);

    if(doesFileExist(filepath)){
      free(path_copy);
      return strdup(filepath);
    }
  }
  free(path_copy);
  return NULL;
}

int isValidDirectory(char* path){
  struct stat statData;

  int result = stat(path, &statData);

  if(stat(path, &statData) == 0){
    return 1;
  };
  return 0;
}




int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  char buf[4094];

  char* user = getenv("USER");
  char* home = getenv("HOME");

  while(1){
    printf("$ ");
    if(!fgets(buf, sizeof(buf), stdin)){
      printf("\n");
      break;
    }

    size_t len = strlen(buf);

    if(len > 0 && buf[len -1] == '\n'){
      buf[len - 1] = '\0';
    }else{
      int c;
      while((c = getchar()) != '\n' && c != EOF){}
    }

    if(buf[0] == '\0') continue;

    char** tokens = malloc(100 * sizeof(char*));
    size_t tokens_len = tokenizeCommands(buf, tokens);

    if(tokens_len < 1) continue;

    // CLEAR COMMAND
    if(strcmp(tokens[0], "clear") == 0){
      if(tokens_len == 1){
        write(1, "\033[2J\033[H", 7);
      }else{
        printf("clear: Unknown arguments provided; try 'clear' instead\n");
      }
      continue;
    }


    if(argc > 1 && strcmp(argv[1], "debug") == 0){
      for(int i = 0; i < tokens_len; i++){
        printf("token[%d] = %s\n", i, tokens[i]);
      }
    }
    // EXIT COMMAND
    if(strcmp(tokens[0], "exit") == 0 && tokens_len == 1){
      break;
    }

    // CAT COMMAND
    if(strcmp(tokens[0], "cat") == 0){
      if(tokens_len == 1) continue;
      char cmd[256] = "cat ";
      for(int i = 1; i < tokens_len; i++){
        strcat(cmd, "'");
        strcat(cmd, tokens[i]);
        strcat(cmd, "'");
        if(i!=tokens_len-1) strcat(cmd, " ");
      }
      system(cmd);
      continue;
    }

    // ECHO COMMAND
    if(strcmp(tokens[0], "echo") == 0){
      for(size_t i = 1; i < tokens_len; i++){
        if(i!=1) printf(" ");
        printf("%s", tokens[i]);
      }
      printf("\n");
      continue;
    }

    if(strcmp(tokens[0], "type") == 0){
      if(tokens_len == 1){
        continue;
      }
      for(size_t i =1; i < tokens_len; i++){
        char* filePath = isValidPathFile(tokens[i]);
        if(isValidCommand(tokens[i])){
          printf("%s is a shell builtin\n", tokens[i]);
        }else if(filePath){
          printf("%s is %s\n", tokens[i], filePath);
        }else{
          printf("%s: not found\n", tokens[i]);
        }
        free(filePath);
      }
      continue;
    }

    if(strcmp(tokens[0], "pwd") == 0){
      char output[300];
      if(getcwd(output, sizeof(output))!= NULL){
        printf("%s\n", output);
      }
      continue;
    }


    if(isValidPathFile(tokens[0])){
      char output[1024];
      char command[300];
      command[0] = '\0';
      strcat(command, tokens[0]);
      for(int i = 1; i < tokens_len; i++){
        strcat(command, " ");
        strcat(command, tokens[i]);
      }

      FILE *fp = popen(command, "r");
      while(fgets(output, sizeof(output), fp) != NULL){
        printf("%s", output);
      }
      continue;
    }


    if(strcmp(tokens[0], "cd") == 0){
      if(tokens_len == 1 || *tokens[1] == '~'){
        chdir(home);
      }
      else if(isValidDirectory(tokens[1])) chdir(tokens[1]);
      else printf("cd: %s: No such file or directory\n", tokens[1]);
      continue;
    }
    printf("%s: command not found\n", tokens[0]);

    free(tokens);
  }

  return 0;
}
