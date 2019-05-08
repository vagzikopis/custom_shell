#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <errno.h>

#define BUFSIZE 512
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDBLUE   "\033[1m\033[34m"      /* Bold Blue */
#define RED   "\033[0;31m"
#define GRN   "\033[0;32m"
#define YEL   "\033[33m"
#define BLU   "\033[34m"
#define MAG   "\033[0;35m"
#define RESET "\033[0m"


void execCommands(char **command, int symbols[], int counter);
int symbolCounter(int symbols[], char input[]);
int specialSymbolSearch(char **command, int i);
char **Parser(char **toBeParsed, int i, char *delimeter);
int redirectionOne(char **command, int i, int basic_symbol, int exit_status);
int redirectionTwo(char **command, int i, int basic_symbol, int exit_status);
int simpleCommand(char **command, int i, int basic_symbol, int exit_status);
int pipeFunction(char **command, int i, int basic_symbol, int exit_status);
void readKeyboard();
void readFile();
int validInput(char *input);
void addNULL(char **array, int length);
void stringRead(char *input);
int quit(char *input);

int main(int argc, char *argv[])
{	
	printf(BOLDBLUE"\n\n\t\t\t Welcome to my custom Shell."RESET"\n\n");
	printf(GRN"\t\t\t Type your commands Carefully!\n\t\t\t Single & is not accepted.\n\t\t\t Maximum command length is 512 characters.\n\t\t\t Waiting for your commands.\n\t\t\t Enjoy your stay!"RESET"\n\n");
	if (argc == 1){
		readKeyboard();	// enable ineractive mode and get instructions from the command line //
	}else{
		readFile(argv[1]); // enable batch file mode and read the file given as argument //
	}
	return 0;
}

void readFile(char *file){
	char *line = NULL;
	char **input = malloc(sizeof(char *));
	int w = 0;
	size_t len = 0;
	FILE *fp;
	fp = fopen(file, "r"); // open file //
	while (getline(&line, &len, fp) != -1){ // read lines and store them into input //
		input = realloc(input, ++w * sizeof * input);
		input[w-1] = malloc(strlen(line)+1);
		strcpy(input[w-1], line);
	}
	for(int i=0; i<w; i++){ //this for loop goes through every line wirtten in the file //
		if(strcmp(input[i],"quit") == 0 || strcmp(input[i],"quit\n")==0 || strcmp(input[i],"quit \n")==0){ // if you read quit terminate program //
			break;
		}else if(input[i][0]=='#'){
			continue; // these are comment lines. Avoid their execution //
		} else if (strlen(input[i]) == strspn(input[i], " \r\n\t")){
			continue;
		}else{
			stringRead(input[i]); // pass the line to stringRead function //
		}
	}
	if(w==0){
		printf(RED"\t\t\t Empty File."RESET"\n"); // empty file case //
	}
	printf(RED"\t\t\t Quitting shell. See ya later!\n"RESET); 
	free(input); 
	free(line);
	fclose(fp);
}

void readKeyboard(){
	while(1)
	{
		char input[BUFSIZE];
		printf(BOLDGREEN"zikopis_8808> "RESET);		
		fgets(input, BUFSIZE, stdin);
		while (strcmp(input, "\n")==0){
			printf("\x1B[32m" "zikopis_8808> " "\x1B[0m");		
			fgets(input, BUFSIZE, stdin); // read input from the command line //
		}
		if(quit(input) == 1)
		{
			printf(RED"\t\tQuitting shell. See ya later!\n"RESET); // if you read quit terminate the program //
			break;
		}
		if (strlen(input)<=512){
			stringRead(input);	// call stringRead function //
		}
	}	
}
int quit(char *input){
	int quit = 0;
	for (int i=0; i<(strlen(input)-3); i++){
		if(input[i] == 'q' && input[i+1] == 'u' && input[i+2] == 'i' && input[i+3] == 't'){
			quit = 1;
			for (int j=0; j<i; j++){
				if(input[j] != ' ' && input[j] != '\t'){
					quit = 0;
				}
			}
			for (int j=i+4; j<strlen(input)-2; j++){
				if(input[j] != ' ' && input[j] != '\t'){
					quit = 0;
				}
			}
		}
	}
	return quit;
}

void stringRead(char *input){
	char **command = malloc(sizeof( *command)); // this array will contain the commands given, separated from ; and &&
	char *token;
	int basic_symbols[BUFSIZE]; // 1 = ; , 2 = && //
	int n = 0;
	int check = 0;
	int counter; // symbols counted (; &&) //
	check = validInput(input); // check for input validation //
	if(check == 0){
		counter = symbolCounter(basic_symbols, input); // how many ; and && i have //
		basic_symbols[counter+1] = 0;	
		token = strtok(input, ";&&\n\t\r"); 	//	remove ; and && from input and store the result into the lines of command array//
		while(token!=NULL){			
			command = realloc(command, ++n * sizeof * command);			
			command[n-1] = malloc(strlen(token) +1);
			strcpy(command[n-1], token); 
			token = strtok(NULL, ";&&\n");
		}
	execCommands(command, basic_symbols, counter); // give an array without ; and && and execute commands //	
	} else {
		return;
	}	
}

int validInput(char *input){
	char *temp;
	if(input[0] == ';'){
		printf("bash: syntax error. Command starts with ';'.\n");
		return 1;
	}else if(input[0] == '&'){
		printf("bash: syntax error. Command starts with '&'.\n");
		return 1;
	}else if ( input[strlen(input)-2] == '&'){
		if(input[strlen(input)-3] == '&'){
			printf("bash: sytax error. Command ends with '&&'.\n");
		}else{
			printf("bash: syntax error. Command ends with '&'.\n");	
		}
		
		return 1;
	}
	int index;
	int flag = 0;
	for (int i=0; i<strlen(input); i++){ // find ; or && that dont separate commands i.e "ls && ; echo shell" //
		if (input[i] == ';' || input[i] == '&'){
			if (flag == 1){
				if(input[i]==';'){
					printf("bash: syntax error near unexpected token '%c' \n", input[i]);
					return 1;					
				} else {
					if (index != i-1){
						printf("bash: syntax error near unexpected token '&%c' \n", input[i]);
						return 1;
					}
				}

			}else{
				index = i;
				flag = 1;
			} 
		} else if (input[i] == ' '){
			continue;
		}else{
			flag = 0;
		}
	}
	for (int i=0; i<strlen(input); i++){ // find commands that end with && //
		if(input[i] == '&' && input[i+1] == '&'){
			for (int j=i+2; j<strlen(input); j++){
				if (input[j] != ' ' && input[j] != '\n'){
					break;
				}else if(input[j] == '\n'){
					printf("bash: sytax error. Command ends with '&&'.\n");
					return 1;
				}
			}
		}

		if(input[i]=='&' && input[i-1]!='&' && input[i+1]!='&' ){ // find single & //
			printf("bash: syntax error , single '&' inserted \n");
			return 1;
		}
	}
	return 0;
}

void execCommands(char **command, int basic_symbols[], int counter){
	int exit_status = 0; // initialize exit_status //
	
	for(int i=0; i<=counter; i++){
		int special_symbol = specialSymbolSearch(command, i);; // 3 = > , 4 = < , 5 = | , else default value = 0 // 	
		if (exit_status == 0){
			if (special_symbol == 0){				// no redirection or pipe //
				exit_status = simpleCommand(command, i, basic_symbols[i], exit_status);	
			}else if (special_symbol == 3) {
				exit_status = redirectionOne(command, i, basic_symbols[i], exit_status);		//implement > redirection case //
			}else if (special_symbol == 4){
				exit_status = redirectionTwo(command, i, basic_symbols[i], exit_status);		//implement < redirection case //
			}else if (special_symbol == 5){
				exit_status = pipeFunction(command, i, basic_symbols[i], exit_status);						//implement | pipe case // 
			}
		} else {
			if (basic_symbols[i] != 2){					// if the previous command failed and the next symbol is ; re-initialize exit_status //
				exit_status = 0;			
			}
		}
	}
	return;
}

/* Parser gets a char** array and splits it into the final array. 
The split depends on the delimeter in the input. 
i variable is used to split commands based on the way they where typed.
A char** pointer is returned from the Parser function. */
char **Parser(char **toBeParsed, int i, char *delimeter){
	char *token;
	char **final = malloc(sizeof(char * )); // this will be returned //
	int w = 0;
	token = strtok(toBeParsed[i], delimeter); // break toBeParsed[i] based on the delimeter and store result in lines of **final //
	while(token!=NULL){
		final = realloc(final, ++w * sizeof * final);
		final[w-1] = malloc(strlen(token) +1); 
		strcpy(final[w-1], token);
		token = strtok(NULL, delimeter);
	}
	if(strcmp(delimeter, " ") == 0){
		addNULL(final, w);		
	}
	return final;
}

int pipeFunction(char **command, int i, int basic_symbol, int exit_status){
	int pipefd[2];
	pid_t p1, p2;
	int status1, status2,status;
	char **temp; 
	char **command_one; // first command //
	char **command_two; // second command //
	temp = Parser(command, i, "|"); // parse input //
	command_one = Parser(temp, 0, " "); // first command without " " and with NULL at the end //
	command_two = Parser(temp, 1, " "); // second command without " " and with NULL at the end //
	if (pipe(pipefd) < 0){ // pipe //
		printf("\n Pipe could not be initialized\n");
		return 1;
	}
	p1 = fork(); // fork for the first command //
	if (p1 < 0){
		printf("\n Fork Error \n");
		return 1;
	}
	if (p1 == 0){
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		
		execvp(command_one[0], command_one); // execute first command and send standard output to pipe //
		perror(command_one[0]);
		exit(EXIT_FAILURE);
	} else {
		wait(&status); // wait for the first command to be executed //
		//exit_status = status;
		close(pipefd[1]);
		p2 = fork(); // fork for the second command //

		if (p2 < 0){
			printf("\n Fork Error \n");
			return 1;
		}

		if (p2 == 0){
			close(pipefd[1]);
			dup2(pipefd[0], STDIN_FILENO);
			
			execvp(command_two[0], command_two); // get standard input from the pipe and execute second command //
			perror(command_two[0]);
			exit(EXIT_FAILURE);
		} else {
			
			wait(&status); // wait for the second command to be executed //
			close(pipefd[0]);
			
			exit_status = status; // exit status to be returned //
			if (basic_symbol == 1 | basic_symbol == 0 && exit_status != 0){
				exit_status = 0; // if symbol == ; then return exit_status = 0 //
			}
		}
	}
	return exit_status;
}

int simpleCommand(char **command, int i, int basic_symbol, int exit_status){
	char **ready;	
	pid_t pid;
	int status;
	ready = Parser(command, i, " "); // ready contains the commands to be executed without " " and with NULL at the end //
	pid = fork();
	if (pid < 0){
		perror("Fork Failed \n");
	}else if(pid == 0){
		execvp(ready[0], ready); // execute command //
		perror(ready[0]);
		exit(EXIT_FAILURE);
	}
	waitpid(pid, &status, 0);
	exit_status = WEXITSTATUS(status); // get execvp's exit_status // 
	if ( basic_symbol == 1 | exit_status == 0 && exit_status != 0){
		exit_status = 0; // if symbol == ; then return exit_status = 0 //
	}
	return exit_status;
}

int redirectionOne(char **command, int i, int basic_symbol, int exit_status){
	char **ready ; 
	char **bracket_free ;	
	char *filename;
	int fd;
	pid_t pid;
	int status;

	bracket_free = Parser(command, i, ">"); // command[i] splitted from > symbol //
	filename = strtok(bracket_free[1], " "); // remove " " from filename //
	ready = Parser(bracket_free, 0, " "); // ready contains the commands to be given to execvp, splitted from blank spaces and with NULL at the end //

	pid = fork();
	if (pid < 0){
		perror("Fork Failed\n");
	}else if(pid == 0){
		fd = creat(filename, 0644);	//create file if file is not existed // 
		dup2(fd, STDOUT_FILENO);
		close(fd);
		execvp(ready[0], ready); // execute command and send standard output to file //
		perror(ready[0]);
		exit(EXIT_FAILURE);
	}
	waitpid(pid, &status, 0);
	exit_status = WEXITSTATUS(status); // get execvp's exit_status //
	if ( basic_symbol == 1 | exit_status == 0 && exit_status != 0){
		exit_status = 0; // if symbol == ; then return exit_status = 0 //
	}
	return exit_status;
}

int redirectionTwo(char **command, int i, int basic_symbol, int exit_status){
	char **ready ; 
	char **bracket_free ;	
	char *filename;
	int fd;
	pid_t pid;
	int status;

	bracket_free = Parser(command, i, "<"); // command[i] splitted from < symbol //
	filename = strtok(bracket_free[1], " "); // ready contains the commands to be given to execvp, splitted from blank spaces and with NULL at the end //
	ready = Parser(bracket_free, 0, " "); // remove " " from filename //
	pid = fork();
	if (pid < 0){
		perror("Fork Failed\n");
	}else if(pid == 0){
		fd = open(filename, O_RDONLY); // open file //
		dup2(fd, STDIN_FILENO);
		close(fd);
		execvp(ready[0], ready); // read standard input from file and execute command //
		perror(ready[0]);
		exit(EXIT_FAILURE);
	}
	waitpid(pid, &status, 0);
	exit_status = WEXITSTATUS(status); // get execvp's exit_status //
	if ( basic_symbol == 1 | exit_status == 0 && exit_status != 0){
		exit_status = 0; // if symbol == ; then return exit_status = 0 //
	}
	return exit_status;
}

int symbolCounter(int symbols[], char input[]){
	int counter = 0;
	for(int i=0; i<strlen(input); i++){
		if(input[i]==';'){
			int accept = 0; // if ; is detected , accept it if only it is followed by characters //
			for (int j=i+1; j<strlen(input); j++){
				if (input[j] != ' ' && input[j] != '\n' && input[j] != '\t'){
					accept = 1;					
				}
			}
			if (accept == 1){
				symbols[counter] = 1;
				counter++;				
			}

		} else if ((input[i] == '&')&&(i!=strlen(input)-1)&&(input[i+1]=='&')) {
			symbols[counter] = 2; // count && //
			counter++;
		}
	}
	return counter;
}

int specialSymbolSearch(char **command, int i){
	int ret = 0;
	for(int j=0; j<strlen(command[i]); j++){
		if (command[i][j] == '>'){	 // return 3 if > is detected //
			ret = 3;
		} else if (command[i][j] == '<') {	// return 4 if < is detected //
			ret = 4;
		} else if (command[i][j] == '|') {	// return 5 if | is detected //
			ret = 5;
		}
	}
	return ret;
}

void addNULL(char **array, int length){
	array[length] = NULL; // add null at the last line of the array //
}
