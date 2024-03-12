#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>


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


// reading the input matrices and their dimensions from file
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

//calculate one row
void *thread_per_row(void *row_index){
    long i_index = (long)row_index;
    for(int j = 0; j < num_of_cols_second_matrix; j++){
        output_matrix_c_per_row[i_index][j] = 0;
        for(int k = 0; k< num_of_rows_second_matrix ;k++)
            output_matrix_c_per_row[i_index][j] += input_matrix_a[i_index][k] * input_matrix_b[k][j];
    }
}

//calculate one element
void *thread_per_element(void *element){
    struct entry *element_indeces;
    element_indeces = (struct entry *) element;
    int r = element_indeces->row, c = element_indeces->col;
    output_matrix_c_per_element[r][c] = 0;
    for(int k=0; k< num_of_rows_second_matrix ;k++)
        output_matrix_c_per_element[r][c] += input_matrix_a[r][k] * input_matrix_b[k][c];
}

//making thread for case1
void construct_thread_per_entire_matrix(){
    struct timeval stop, start;
    gettimeofday(&start, NULL);
    thread_per_entire_matrix();
    gettimeofday(&stop, NULL);

    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    printf("Number of Threads Created: 1 Thread\n");
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
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    printf("Number of Threads Created: %d Thread\n",num_of_rows_first_matrix);
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
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    printf("Number of Threads Created: %d Threads\n",num_of_rows_first_matrix * num_of_cols_second_matrix);
    fprintf(fptr_c[2], "Method: A thread per element\n");
    write_matrices_in_file(output_matrix_c_per_element,fptr_c[2]);
}


void files(char* names[]) {
    if (!is_default) {
        for (int i = 1; i < 4; i++) {
            char filename[1024];
            snprintf(filename, sizeof(filename), "%s%s", names[i], (i == 3) ? "_per_matrix.txt" : ".txt");
            fptr_c[i-1] = fopen(filename, (i == 1) ? "r" : "w+");
        }
    } else {
        fptr_a = fopen("a.txt", "r");
        fptr_b = fopen("b.txt", "r");
        for (int i = 1; i < 4; i++) {
            fptr_c[i-1] = fopen((i == 1) ? "c_per_matrix.txt" : (i == 2) ? "c_per_row.txt" : "c_per_element.txt", "w+");
        }
    }
}


int main(int argC, char* args[]){   
    is_default = !(argC == 4);
    files(args);

    if(!fptr_a || !fptr_b){
        printf("Cannot find files :(\n");
    }
    else{
        read_matrices_from_file(input_matrix_a, fptr_a, 1);
        read_matrices_from_file(input_matrix_b, fptr_b, 0);
        if(num_of_cols_first_matrix != num_of_rows_second_matrix){
            printf("these matrices can't be multiplied!");
        }
        else {
            construct_thread_per_entire_matrix();
            construct_thread_per_row();
            construct_thread_per_element();
        }
    }
}