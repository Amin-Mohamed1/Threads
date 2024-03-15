#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#define RESET "\033[0m"
#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"

#define MAX 20

struct entry{
    int row;
    int col;
};

// input and output matrices
int input_matrix_a[MAX][MAX], input_matrix_b[MAX][MAX], output_matrix_c_per_matrix[MAX][MAX], output_matrix_c_per_row[MAX][MAX], output_matrix_c_per_element[MAX][MAX];

// file pointers for dealing with files
FILE *fptr_a, *fptr_b;
FILE* fptr_c[3];
// dimensions of the input matrices
int num_of_rows_first_matrix, num_of_cols_first_matrix, num_of_rows_second_matrix, num_of_cols_second_matrix;

// var to check if input is the default case i.e. a.txt, b.txt, c.txt
int is_default;

// Reading the input matrices and their dimensions from file
void read_matrices_from_file(int matrix[MAX][MAX], FILE* fptr, int is_matrix_a) {
    int row, col;
    if (is_matrix_a == 1) {
        fscanf(fptr, "row=%d col=%d\n", &num_of_rows_first_matrix, &num_of_cols_first_matrix);
        for (int i = 0; i < num_of_rows_first_matrix; i++) {
            for (int j = 0; j < num_of_cols_first_matrix; j++) {
                fscanf(fptr, "%d", &matrix[i][j]);
                if (j < num_of_cols_first_matrix - 1) {
                    fscanf(fptr, " ");
                }
            }
            fscanf(fptr, "\n");
        }
    } else {
        fscanf(fptr, "row=%d col=%d\n", &num_of_rows_second_matrix, &num_of_cols_second_matrix);
        for (int i = 0; i < num_of_rows_second_matrix; i++) {
            for (int j = 0; j < num_of_cols_second_matrix; j++) {
                fscanf(fptr, "%d", &matrix[i][j]);
                if (j < num_of_cols_second_matrix - 1) {
                    fscanf(fptr, " ");
                }
            }
            fscanf(fptr, "\n");
        }
    }
}

// writing the output matrices into file
void write_matrices_in_file(int matrix[MAX][MAX], FILE *fptr){
    fprintf(fptr,"row=%d col=%d\n", num_of_rows_first_matrix, num_of_cols_second_matrix);
    for(int i = 0; i < num_of_rows_first_matrix; i++){
        for(int j = 0; j < num_of_cols_second_matrix; j++){
            fprintf(fptr, "%d ", matrix[i][j]);
        }
        fprintf(fptr, "\n");
    }
    fclose(fptr);
}

// normal matrix multiplication (one thread per matrix)
void* thread_per_entire_matrix(){
    for(int i = 0; i < num_of_rows_first_matrix; i++){
        for(int j = 0; j < num_of_cols_second_matrix; j++){
            output_matrix_c_per_matrix[i][j] = 0;
            for(int k = 0; k < num_of_cols_first_matrix; k++){
                output_matrix_c_per_matrix[i][j] += (input_matrix_a[i][k] * input_matrix_b[k][j]); 
            }
        }
    }
}


// thread for each row 
void *thread_per_row(void *row_index){
    long i_index = (long)row_index;
    for(int j = 0; j < num_of_cols_second_matrix; j++){
        output_matrix_c_per_row[i_index][j] = 0;
        for(int k = 0; k< num_of_rows_second_matrix ;k++)
            output_matrix_c_per_row[i_index][j] += input_matrix_a[i_index][k] * input_matrix_b[k][j];
    }
}

// thread for each element
void *thread_per_element(void *element){
    struct entry *element_indeces;
    element_indeces = (struct entry *) element;
    int r = element_indeces->row, c = element_indeces->col;
    output_matrix_c_per_element[r][c] = 0;
    for(int k = 0; k < num_of_rows_second_matrix ;k++)
        output_matrix_c_per_element[r][c] += input_matrix_a[r][k] * input_matrix_b[k][c];
}

void construct_thread_per_entire_matrix(){
    struct timeval stop, start;
    gettimeofday(&start, NULL);
    thread_per_entire_matrix();
    gettimeofday(&stop, NULL);

    long long time_taken_sec = stop.tv_sec - start.tv_sec;
    long long time_taken_usec = stop.tv_usec - start.tv_usec;
    printf(GREEN "Seconds: %lld.%06lld\n", time_taken_sec, time_taken_usec);
    printf(BLUE "Microseconds: %lld\n", time_taken_sec * 1000000 + time_taken_usec);
    printf(MAGENTA "Number of Threads Created: 1 Thread\n");
    printf("\n");
    fprintf(fptr_c[0], "Method: A thread per matrix\n");
    write_matrices_in_file(output_matrix_c_per_matrix,fptr_c[0]);
}

void construct_thread_per_row(){
    struct timeval stop, start;
    gettimeofday(&start, NULL);
    pthread_t threads[num_of_rows_first_matrix];

    for(int i = 0; i < num_of_rows_first_matrix; i++)
        pthread_create(&threads[i], NULL, thread_per_row, (void *)(long) i);

    for(int i = 0; i < num_of_rows_first_matrix; i++)
        pthread_join(threads[i], NULL);

    gettimeofday(&stop, NULL);
    long long time_taken_sec = stop.tv_sec - start.tv_sec;
    long long time_taken_usec = stop.tv_usec - start.tv_usec;
    printf(GREEN "Seconds: %lld.%06lld\n", time_taken_sec, time_taken_usec);
    printf(BLUE "Microseconds: %lld\n", time_taken_sec * 1000000 + time_taken_usec);
    printf(MAGENTA "Number of Threads Created: %d Thread\n",num_of_rows_first_matrix);
    printf("\n");
    fprintf(fptr_c[1], "Method: A thread per row\n");
    write_matrices_in_file(output_matrix_c_per_row,fptr_c[1]);
}

void construct_thread_per_element(){
    struct timeval stop, start;
    gettimeofday(&start, NULL);
    pthread_t threads[num_of_rows_first_matrix][num_of_cols_second_matrix];
    for(int i = 0; i < num_of_rows_first_matrix; i++)
        for(int j = 0; j < num_of_cols_second_matrix; j++) {
            struct entry *item = malloc(sizeof(struct entry));
            item->row = i;
            item->col = j;
            pthread_create(&threads[i][j], NULL, thread_per_element, (void *) item);
        }
    for(int i = 0; i < num_of_rows_first_matrix; i++)
        for(int j = 0;j < num_of_cols_second_matrix; j++)
            pthread_join(threads[i][j], NULL);

    gettimeofday(&stop, NULL);

    long long time_taken_sec = stop.tv_sec - start.tv_sec;
    long long time_taken_usec = stop.tv_usec - start.tv_usec;
    printf(GREEN "Seconds: %lld.%06lld\n", time_taken_sec, time_taken_usec);
    printf(BLUE "Microseconds: %lld\n", time_taken_sec * 1000000 + time_taken_usec);
    printf(MAGENTA "Number of Threads Created: %d Threads\n",num_of_rows_first_matrix * num_of_cols_second_matrix);
    printf(WHITE "\n");
    fprintf(fptr_c[2], "Method: A thread per element\n");
    write_matrices_in_file(output_matrix_c_per_element,fptr_c[2]);
}


void initialize_files(char* names[]) {

    char fname[1000];

    if (!is_default) {
        snprintf(fname, sizeof(fname), "%s.txt", names[1]);
        fptr_a = fopen(fname, "r");

        snprintf(fname, sizeof(fname), "%s.txt", names[2]);
        fptr_b = fopen(fname, "r");

        snprintf(fname, sizeof(fname), "%s_per_matrix.txt", names[3]);
        fptr_c[0] = fopen(fname, "w+");

        snprintf(fname, sizeof(fname), "%s_per_row.txt", names[3]);
        fptr_c[1] = fopen(fname, "w+");

        snprintf(fname, sizeof(fname), "%s_per_element.txt", names[3]);
        fptr_c[2] = fopen(fname, "w+");

    } else {

        fptr_a = fopen("a.txt", "r");
        fptr_b = fopen("b.txt", "r");
        fptr_c[0] = fopen("c_per_matrix.txt", "w+");
        fptr_c[1] = fopen("c_per_row.txt", "w+");
        fptr_c[2] = fopen("c_per_element.txt", "w+");

    }
}


int main(int argC, char* args[]){   
    is_default = (argC != 4);
    initialize_files(args);

    if(!fptr_a || !fptr_b){
        printf(RED "Cannot find files :(\n");
    }
    else{
        read_matrices_from_file(input_matrix_a, fptr_a, 1);
        read_matrices_from_file(input_matrix_b, fptr_b, 0);
        if(num_of_cols_first_matrix != num_of_rows_second_matrix){
            printf(RED "these matrices can't be multiplied!");
        }
        else {
            construct_thread_per_entire_matrix();
            construct_thread_per_row();
            construct_thread_per_element();
        }
    }
}
