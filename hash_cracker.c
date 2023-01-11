/* File:     hash_cracker.c
 * Ostermann's example solution
 * Mar 24, 2022
 */

// echo "andpepper Xl9dddHVkCrdG3vTJkuEBK9ecqr/aZT5wJFKqz/PRJC  clangCLANK  4" | mpiexec --hostfile cslab-hosts --display-allocation  ./hash_cracker
//                 3iFgDy/8wKDROxyEcte018Aq6t1ZXtJRalWpZExWP84

//Hash 1: echo "foobar 	 3iFgDy/8wKDROxyEcte018Aq6t1ZXtJRalWpZExWP84  0123456789-=  2"  	 	| mpiexec -n 4 ./hash_cracker

// echo "Li8dmN8vLwjZ9hgz   3iFgDy/8wKDROxyEcte018Aq6t1ZXtJRalWpZExWP84  '0123456789\!\@\#\%\^\&\*\(\)\_\+\-\='  2" | mpiexec -n 2 ./hash_cracker

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <mpi.h>


int debug = 0;  // must be turned OFF for github grading

const int bufsize=256;


// routine definitions
void Get_input(int my_rank, int comm_sz, char *salt, char *hash, char *alphabet, int *ppwd_len);
void ix_to_password(char *alphabet, int pwd_length, int ix, char *guess);
int FindHash(int my_rank, char *salt, char *hash, char *alphabet, int pwd_length, int first_pwd_ix, int num_pwds);
int power(int base, int exponent);
void f(char** out, int n, char* p, char* alphabet, int passlen);
char *trim(char *str);

// You shouldn't need to change anything about this routine
char *MakeSha256( char *salt,  char *pwd_guess) {
   char command[1024];
   char result[1024];
   FILE *file_result;

   // create the shell command to use
   // for example: 
   // BSH:Saru> openssl passwd -5 -salt foobar mypwd
   // $5$foobar$6WS2Np/pNB83FbxzS7a5fGaJO1PMtdldjMSWCiBio05
   snprintf(command, sizeof(command), "openssl passwd -5 -salt '%s' '%s'\n", salt, pwd_guess);

   if (debug>1)
      fprintf(stderr,"Running command %s\n", command);

   // run the shell command
   file_result = popen(command,"r");

   fgets(result, sizeof(result), file_result);

   pclose(file_result);

   //printf("Digest: '%s'", result);
   char *ptr = result;
   ptr=index(result+1,'$');   // find the second $
   ptr=1+index(ptr+1,'$');   // find the third $
   //printf("Hash: '%s'", ptr);

   return(strdup(ptr));
}


int main(void) {
   int my_rank;   // my rank, CPU number
   int comm_sz;   // number of CPUs in the group
   char salt[bufsize];       // the salt used to generate the hash (a string, like "foobar")
   char hash[bufsize];       // the password hash we're trying to find (a string, like "AiVIwiOWe.ZrPkJTJs30soPiP2dYmpbNm8faGkAMBr8")
   char alphabet[bufsize];   // the alphabet that the passwords are taken from (a string, like "0123456789")
   int password_length;       // how many characters (from that alphabet) are in the password
   int ix1;                   // the first password index that _I_ am supposed to start testing
   int ix2;                   // the last password index that _I_ am supposed to test
   int total_passwords;       // how many do we need to check?
   long my_answer_ix;          // the answer that I came up with
   int answer = 0;            // the best answer from anybody

   
   /* Let the system do what it needs to start up MPI */
   MPI_Init(NULL, NULL);

   /* Get my process rank */
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

   /* Find out how many processes are being used */
   MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

   /* Print the status */
   if (debug && my_rank == 0) {
      printf("Comm size: %d\n", comm_sz);
   }

   Get_input(my_rank, comm_sz, salt, hash, alphabet, &password_length);

   /* Print the status */
   if (debug && my_rank == 0) {
      printf("Target hash: '%s'\n", hash);      
      printf("Salt used: '%s'\n", salt);
      printf("Password alphabet: %s\n", alphabet);
      printf("Password Length: %d\n", password_length);
   }
   
   int length = strlen(alphabet);

   // how many total passwords do we need to check?
   // you will need to calculate this!
   total_passwords = power(length, password_length);

   // Print the workload
   if (debug && my_rank == 0) {
      printf("Total passwords: %d\n", total_passwords);
      printf("Passwords per rank: %d\n", total_passwords / comm_sz);
   }

   // what work does THIS cpu need to do
   // you will need to calculate this
   int pass_per_rank = total_passwords/comm_sz;
   int remainder = total_passwords % comm_sz;

    if(remainder == 0){
        ix1 = my_rank * pass_per_rank;                    // first password index to check
        ix2 = ix1 + pass_per_rank;                        // last password to check
    }else{
        total_passwords = total_passwords + (comm_sz - remainder);
        ix1 = my_rank * total_passwords/comm_sz;
        ix2 = ix1 + total_passwords/comm_sz;
    }
      
   if(debug && my_rank == 0){
       printf("Index 1: %d\n", ix1);
       printf("Index 2: %d\n", ix2);
   }

   // perform the assigned work
   my_answer_ix = FindHash(my_rank, salt, hash, alphabet, password_length, ix1, ix2);

   if (debug)
      printf("CPU %d returns answer %ld\n", my_rank, my_answer_ix);

   // int totalchars = my_answer_ix*password_length;
   // gather up the max password ix found */
   // you will need to finish this
   // you need to use MPI_Reduce to find the MAXIMUM value of my_answer_ix of all the CPUs
   MPI_Reduce(&my_answer_ix, &answer, 1, MPI_LONG, MPI_MAX, 0, MPI_COMM_WORLD);
   //printf("MPI_Reduce not finished\n");make 

   /* Print the result */
   if (my_rank == 0) {
      if (answer >= 0) {
         char guess[bufsize];
         ix_to_password(alphabet, password_length, answer, guess);
         if (debug) {
            printf("Found Answer: %d (%s)\n", answer, guess);
         } else { 
            printf("%d: '%s'\n", answer, guess);
         }
      } else {
            printf("not found\n");
      }
   }
   /* Shut down MPI */
   MPI_Finalize();

   return 0;
} /*  main  */


// you will need to finish this routine
void Get_input(int my_rank, int comm_sz, char *salt, char *hash, char *alphabet, int *ppwd_len) {

   if (my_rank == 0) {
      int ret;
      fprintf(stderr,"Enter salt, hash, alphabet, and passwdlength\n");
      ret = scanf("%s %s %s %d", salt, hash, alphabet, ppwd_len);
      if (ret != 4) {
         fprintf(stderr,"Invalid arguments provided (%d)\n", ret);
         exit(0);
      }
   }
   

   // you will need to complete the 4 MPI_Bcast calls needed to share those 4 parameters with everybody...
    MPI_Bcast(salt, bufsize, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(hash, bufsize, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(alphabet, bufsize, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(ppwd_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
   // printf("MPI_Bcast not finished\n");
} 




// you need to write this routine VERY carefully
// its purpose is to fill in the array "guess" with the ix'th password from the given alphabet
void ix_to_password(char *alphabet, int pwd_length, int ix, char *guess) {
    guess[pwd_length] = '\00';
    int index = ix*pwd_length;

    char* combinations = (char*)calloc(index, pwd_length);
    char* comb = combinations;
    char buf[pwd_length];
    
    f(&comb, pwd_length, buf, alphabet, pwd_length);

    for(int i = 0; i < pwd_length; i++){
        guess[i] = combinations[index];
        index++;
    }

    free(combinations);
}

// you need to finish this routine
// it needs to loop and generate each password from first_pwd_ix to last_pwd_ix
// for each one, compare against "target_hash" to check for a match
// if it matches, you should immediately return the ix of the matching password
// if none found, return -1
int FindHash(int my_rank, char *salt, char *target_hash, char *alphabet, int pwd_length, int first_pwd_ix, int last_pwd_ix) {
    char guess[pwd_length+2];
    int alphalen = strlen(alphabet);
    char* combinations = (char*)calloc(power(alphalen, pwd_length)*pwd_length, pwd_length);
    char* comb = combinations;
    char buf[bufsize];
    f(&comb, pwd_length, buf, alphabet, pwd_length);
    

    for (int ix = first_pwd_ix; ix < last_pwd_ix*pwd_length; ++ix){

        strncpy(guess, &combinations[ix], pwd_length);
        guess[pwd_length] = '\0';

        if(ix % pwd_length == 0){
            

            char *genHash = NULL; 
            
            genHash = MakeSha256(salt, guess);
            trim(genHash);

            if(debug == 1){
                printf("Guess: %d %s\n", ix/pwd_length, guess);
                printf("Hash: %s\n", genHash);
                printf("Target Hash: %s\n", target_hash);
            }


            if(strcmp(target_hash, genHash) == 0){
                free(combinations);
                return ix/pwd_length;
            }
        }
    }
    free(combinations);
    return(-1);
}

int power(int base, int exponent){
    int result = 1;

    while(exponent != 0){
        result *= base;
        --exponent;
    }

    return result;
}

void f(char** out, int n, char* p, char* alphabet, int passlen)
{
    if (n == 0) {
        strncpy(*out, p - passlen, passlen);
        *out += passlen;
        return;
    }
    for (int i = 0; i < strlen(alphabet); ++i) {
        *p = alphabet[i];
        f(out, n - 1, p + 1, alphabet, passlen);
    }
}

char *trim(char *str)
{
    size_t len = 0;
    char *frontp = str;
    char *endp = NULL;

    if( str == NULL ){
        return NULL; 
    }

    if( str[0] == '\0' ){ 
        return str; 
    }

    len = strlen(str);
    endp = str + len;

    while( isspace((unsigned char) *frontp) ) { ++frontp; }
    if( endp != frontp ){
        while( isspace((unsigned char) *(--endp)) && endp != frontp ) {}
    }

    if( frontp != str && endp == frontp ){
        *str = '\0';
    }
    else if( str + len - 1 != endp ){
        *(endp + 1) = '\0';
    }

    endp = str;
    if( frontp != str ){
            while( *frontp ){ 
                *endp++ = *frontp++; 
            }
            *endp = '\0';
    }

    return str;
}