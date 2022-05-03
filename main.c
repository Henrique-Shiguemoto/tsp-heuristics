#include <stdio.h>
#include <stdlib.h>
#include <string.h>                 //currently just for strcat, strtok and strcmp
#include <math.h>                   //just for sqrt
#include <stdbool.h>
#include <float.h>                  //just for DBL_MAX
#include <time.h>

#define HEADER_LINE_COUNT 6         //This is just the number of lines in the header of the input files

//GENETIC ALGORITHM PARATEMERS
#define GENE_SIZE         (75)        //Number of chromosomes per gene
#define MUTATION_RATE     (5)        //This number goes from 0 to 100
#define HOUR              3600.0
#define TIME_IN_SECONDS   (3.0)*HOUR
#define MAX_ITERATIONS    100

typedef struct Vertex{
    int id;
    double x;
    double y;
} Vertex;

typedef struct Cycle{
    double result;
    int cycle_size;
    Vertex vertex_cycle[];
} Cycle;

typedef struct Descendants{
    Cycle* descendant1;
    Cycle* descendant2;
}Descendants;

Cycle* initialize_cycle(int);
void destroy_cycle(Cycle*);
double calculate_distance(Vertex vertices[], int, int);
double calculate_cycle_result(Cycle*);
void tsp_2Opt_Optimal(Cycle*);
Cycle* two_opt_swap(Cycle*, int, int);
Cycle* tsp_2Opt_Greedy(Cycle*);
Cycle* tsp_NN(Vertex vertices[], int);
Cycle* tsp_NND(Vertex vertices[], int);
Descendants* cycle_crossover(Cycle*, Cycle*);
Descendants* position_based_crossover(Cycle*, Cycle*);
void mutate_cycle(Cycle*);
void print_vertex(Vertex);
void print_cycle(Cycle*);
void print_result(Cycle*);
void check_for_duplicates(Cycle*);
Cycle* get_best_cycle_from_gene(Cycle**, int);
Cycle* get_worst_cycle_from_gene(Cycle**, int);
void copy_cycle(Cycle*, Cycle*);

Cycle* initialize_cycle(int size){
    Cycle* cycle = malloc(sizeof(double) + sizeof(int) + sizeof(Vertex) * size);
    
    if(cycle == NULL){
        perror("Error: ");
        exit(EXIT_FAILURE);
    }

    cycle->result = 0;
    cycle->cycle_size = size;

    return cycle;
}

void destroy_cycle(Cycle* cycle){
    //Rule of thumb: ONE FREE PER MALLOC
    free(cycle);
    cycle = NULL;
}

double calculate_distance(Vertex vertices[], int i, int j){
    return sqrt(pow(vertices[i].x - vertices[j].x, 2) + pow(vertices[i].y - vertices[j].y, 2));
}

double calculate_cycle_result(Cycle* cycle){
    double result = 0;
    
    for (int i = 0; i < cycle->cycle_size - 1; i++)
    {
        result = result + calculate_distance(cycle->vertex_cycle, i, i + 1);
    }

    result = result + calculate_distance(cycle->vertex_cycle, cycle->cycle_size - 1, 0);
    
    return result;
}

void tsp_2Opt_Optimal(Cycle* input_cycle){
    Cycle* best_cycle = initialize_cycle(input_cycle->cycle_size);
    copy_cycle(best_cycle, input_cycle);

    int size = input_cycle->cycle_size;
    for(int i = 0; i < size - 1; i++){
        for(int j = i + 1; j < size; j++){            
            double gain = 0;
            double loss = 0;
            int i1 = i - 1;
            int i2 = i;
            int j1 = j;
            int j2 = j + 1;
            
            if(i1 == -1){
                i1 = input_cycle->cycle_size-1;
            }

            if(j2 == input_cycle->cycle_size){
                j2 = 0;
            }
            
            loss = calculate_distance(input_cycle->vertex_cycle, i1, i2) + calculate_distance(input_cycle->vertex_cycle, j1, j2);
            gain = calculate_distance(input_cycle->vertex_cycle, i1, j1) + calculate_distance(input_cycle->vertex_cycle, i2, j2);

            //If any of the and points of the two edges are equal to each other, then we just get out of the function
            if(i1 == j1 || i1 == j2 || i2 == j1 || i2 == j2){
                gain = loss;
            }

            double new_result = input_cycle->result + gain - loss;

            if(new_result < best_cycle->result){
                Cycle* aux_cycle = two_opt_swap(input_cycle, i, j);
                aux_cycle->result = new_result;
                
                //We NEED a deep copy here because we have to use destroy_cycle(cycle) when we get out of the scope inner
                // for loop scope
                copy_cycle(best_cycle, aux_cycle);
                destroy_cycle(aux_cycle);
            }
        }
    }

    copy_cycle(input_cycle, best_cycle);
    destroy_cycle(best_cycle);
}

Cycle* two_opt_swap(Cycle* current_cycle, int i, int j){
    Cycle* cycle = initialize_cycle(current_cycle->cycle_size);
    
    //In this function we are removing the edges (i - 1, i) and (j, j + 1) 
    // and adding the edges (i - 1, j) and (i, j + 1)

    for(int n = 0; n <= i - 1; n++){
        cycle->vertex_cycle[n] = current_cycle->vertex_cycle[n];
    }

    int temp = 0;
    for (int n = i; n <= j; n++)
    {
        cycle->vertex_cycle[n] = current_cycle->vertex_cycle[j - temp];
        temp++;
    }

    for(int n = j + 1; n < current_cycle->cycle_size; n++){
        cycle->vertex_cycle[n] = current_cycle->vertex_cycle[n];
    }

    return cycle;
}

Cycle* tsp_2Opt_Greedy(Cycle* input_cycle){
    Cycle* best_cycle = initialize_cycle(input_cycle->cycle_size);
    
    int allowed_times_improved = 10;
    int times_improved = 0;

    bool improved = true;
    while(improved && times_improved < allowed_times_improved){
        improved = false;
        best_cycle->result = input_cycle->result;
        int size = input_cycle->cycle_size;
        for(int i = 0; i < size - 1; i++){
            for(int j = i + 1; j < size; j++){
                
                double gain = 0;
                double loss = 0;
                int i1 = i - 1;
                int i2 = i;
                int j1 = j;
                int j2 = j + 1;
                
                if(i1 == -1){
                    i1 = input_cycle->cycle_size-1;
                }

                if(j2 == input_cycle->cycle_size){
                    j2 = 0;
                }
                
                loss = calculate_distance(input_cycle->vertex_cycle, i1, i2) + calculate_distance(input_cycle->vertex_cycle, j1, j2);
                gain = calculate_distance(input_cycle->vertex_cycle, i1, j1) + calculate_distance(input_cycle->vertex_cycle, i2, j2);

                //If any of the and points of the two edges are equal to each other, then we just get out of the function
                if(i1 == j1 || i1 == j2 || i2 == j1 || i2 == j2){
                    gain = loss;
                }

                double new_result = input_cycle->result + gain - loss;

                if(new_result < best_cycle->result){
                    Cycle* cycle = two_opt_swap(input_cycle, i, j);

                    //We NEED a deep copy here because we have to use destroy_cycle(cycle) when we get out of the scope inner
                    // for loop scope
                    best_cycle->result = best_cycle->result + gain - loss;
                    for(int k = 0; k < best_cycle->cycle_size; k++){
                        best_cycle->vertex_cycle[k] = cycle->vertex_cycle[k];
                    }
                    improved = true;
                    times_improved++;
                    destroy_cycle(cycle);
                }
            }
        }
        //Updating the input_cycle
        if(improved){
            input_cycle = best_cycle;
        }
    }
    return input_cycle;
}

Cycle* tsp_NN(Vertex vertices[], int vertex_list_size){
    Cycle* cycle = initialize_cycle(vertex_list_size);
    
    srand((unsigned) time(NULL));
    int starting_vertex_index = rand() % vertex_list_size;

    int sequence_length = 1; //we start with one vertex
    double current_distance = 0;

    bool* visited = calloc(vertex_list_size, sizeof(bool));
    visited[starting_vertex_index] = true; // we always start from the first vertex
    int cycle_iterator = 0;
    cycle->vertex_cycle[cycle_iterator] = vertices[starting_vertex_index]; //Starting the cycle build
    
    int current_vertex_index = starting_vertex_index;
    int next_index = 0;
    while (sequence_length < vertex_list_size){
        double min_distance = DBL_MAX;
        for (int i = 0; i < vertex_list_size; i++){
            // We only care about vertices that weren't visited yet
            if (!visited[i]){
                double distance = calculate_distance(vertices, current_vertex_index, i);
                if(distance < min_distance){
                    min_distance = distance;
                    next_index = i;
                }
            }
        }
        current_vertex_index = next_index;        

        visited[current_vertex_index] = true;
        cycle_iterator++;
        cycle->vertex_cycle[cycle_iterator] = vertices[current_vertex_index];
        
        current_distance = current_distance + min_distance;
        sequence_length++;
    }

    current_distance = current_distance + calculate_distance(vertices, starting_vertex_index, current_vertex_index);
    cycle->result = current_distance;

    free(visited);
    visited = NULL;

    return cycle;
}

Cycle* tsp_NND(Vertex vertices[], int vertex_list_size){
    Cycle* cycle = initialize_cycle(vertex_list_size);
    
    //srand(clock() % rand());
    int starting_vertex_index = rand() % vertex_list_size;

    int end = starting_vertex_index; 
    int start = starting_vertex_index; // Start and end points are the same in the beginning

    int sequence_length = 1;
    double current_distance = 0;
    
    bool* visited = calloc(vertex_list_size, sizeof(bool));
    visited[starting_vertex_index] = true; // we always start from the first vertex
    int cycle_iterator = 0;
    cycle->vertex_cycle[cycle_iterator] = vertices[starting_vertex_index];

    int next_index = 0;
    bool need_to_update_end_point = false; //Just a boolean so we can know if we need to update the end-point or the start-point (exclusive OR)

    while(sequence_length < vertex_list_size){
        double min_distance = DBL_MAX;
        //Searching for vertex which is closest to the start-point or the end-point
        for(int i = 0; i < vertex_list_size; i++){
            if(!visited[i]){
                //Checking the vertices adjacent to the end-point
                double distance_from_start = calculate_distance(vertices, start, i);
                double distance_from_end = calculate_distance(vertices, end, i);
                if(distance_from_start < min_distance){
                    min_distance = distance_from_start;
                    next_index = i;
                    need_to_update_end_point = false;
                }
                if(distance_from_end < min_distance){
                    min_distance = distance_from_end;
                    next_index = i;
                    need_to_update_end_point = true;
                }
            }
        }

        if(need_to_update_end_point){
            end = next_index;
            visited[end] = true;
            cycle_iterator++;
            cycle->vertex_cycle[cycle_iterator] = vertices[end];
        }else{
            start = next_index;
            visited[start] = true;
            cycle_iterator++;
            for(int i = cycle_iterator; i > 0; i--){
                cycle->vertex_cycle[i] = cycle->vertex_cycle[i - 1];
            }
            cycle->vertex_cycle[0] = vertices[start];
        }
        current_distance = current_distance + min_distance;
        sequence_length++;
    }

    //Connecting end-points
    current_distance = current_distance + calculate_distance(vertices, start, end);
    cycle->result = current_distance;

    free(visited);
    visited = NULL;

    return cycle;
}

Descendants* cycle_crossover(Cycle* father1, Cycle* father2){
    int size_of_cycle = father1->cycle_size;
    
    Cycle* child1 = initialize_cycle(size_of_cycle);
    Cycle* child2 = initialize_cycle(size_of_cycle);

    //Copying cycle from father2 to child1 (RESULT IS STILL NOT COPIED YET)
    for (int i = 0; i < size_of_cycle; i++)
    {
        child1->vertex_cycle[i] = father2->vertex_cycle[i];
    }

    int index = 0;
    do
    {
        //Copying vertex
        child1->vertex_cycle[index] = father1->vertex_cycle[index];
        //Index update
        for (int j = 0; j < size_of_cycle; j++)
        {
            if(father2->vertex_cycle[j].id == child1->vertex_cycle[index].id){
                child1->vertex_cycle[j] = father1->vertex_cycle[j];
                index = j;
                break;
            }
        }
    } while (index != 0);
    
    //Copying cycle from father1 to child2 (RESULT IS STILL NOT COPIED YET)
    for (int i = 0; i < size_of_cycle; i++)
    {
        child2->vertex_cycle[i] = father1->vertex_cycle[i];
    }

    index = 0;
    do
    {
        //Copying vertex
        child2->vertex_cycle[index] = father2->vertex_cycle[index];
        //Index update
        for (int j = 0; j < size_of_cycle; j++)
        {
            if(father1->vertex_cycle[j].id == child2->vertex_cycle[index].id){
                child2->vertex_cycle[j] = father2->vertex_cycle[j];
                index = j;
                break;
            }
        }
    } while (index != 0);

    //Updating children results
    child1->result = calculate_cycle_result(child1);
    child2->result = calculate_cycle_result(child2);

    Descendants* descendants = malloc(sizeof(child1) + sizeof(child2));
    descendants->descendant1 = child1;
    descendants->descendant2 = child2;
    return descendants;
}

Descendants* position_based_crossover(Cycle* father1, Cycle* father2){
    //Chosing random position
    #define POSITIONS_SIZE 5
    
    int pos[POSITIONS_SIZE] = {0};
    pos[0] = rand() % father1->cycle_size;
    pos[1] = rand() % father1->cycle_size;
    pos[2] = rand() % father1->cycle_size;
    pos[3] = rand() % father1->cycle_size;
    pos[4] = rand() % father1->cycle_size;    

    while(pos[0] == pos[1] || pos[0] == pos[2] || pos[0] == pos[3] || pos[0] == pos[4] || 
          pos[1] == pos[2] || pos[1] == pos[3] || pos[1] == pos[4] || 
          pos[2] == pos[3] || pos[2] == pos[4] || 
          pos[3] == pos[4]){
        pos[0] = rand() % father1->cycle_size;
        pos[1] = rand() % father1->cycle_size;
        pos[2] = rand() % father1->cycle_size;
        pos[3] = rand() % father1->cycle_size;
        pos[4] = rand() % father1->cycle_size;
    }

    Cycle* child1 = initialize_cycle(father1->cycle_size);
    Cycle* child2 = initialize_cycle(father1->cycle_size);

    copy_cycle(child1, father1);
    copy_cycle(child2, father2);

    for (int i = 0; i < POSITIONS_SIZE; i++)
    {
        int aux = child1->vertex_cycle[pos[i]].id;
        child1->vertex_cycle[pos[i]] = father2->vertex_cycle[pos[i]];
        for (int j = 0; j < father1->cycle_size; j++)
        {
            if(father1->vertex_cycle[j].id == aux){
                child1->vertex_cycle[j] = father1->vertex_cycle[pos[i]];
            }
        }

        int aux2 = child2->vertex_cycle[pos[i]].id;
        child2->vertex_cycle[pos[i]] = father1->vertex_cycle[pos[i]];
        for (int j = 0; j < father2->cycle_size; j++)
        {
            if(father2->vertex_cycle[j].id == aux2){
                child2->vertex_cycle[j] = father2->vertex_cycle[pos[i]];
            }
        }
    }

    //Updating children results
    child1->result = calculate_cycle_result(child1);
    child2->result = calculate_cycle_result(child2);

    Descendants* descendants = malloc(sizeof(child1) + sizeof(child2));
    descendants->descendant1 = child1;
    descendants->descendant2 = child2;

    return descendants;
}

void mutate_cycle(Cycle* cycle){
    int start =  (rand() % cycle->cycle_size) / 2;
    int finish = ((rand() % cycle->cycle_size) / 2) + (cycle->cycle_size / 2);

    while(start < finish){
        
        Vertex v_aux = cycle->vertex_cycle[start];
        cycle->vertex_cycle[start] = cycle->vertex_cycle[finish];
        cycle->vertex_cycle[finish] = v_aux;        
        
        start++;
        finish--;
    }

    cycle->result = calculate_cycle_result(cycle);
}

void print_vertex(Vertex v){
    printf("ID = %i\t", v.id);
    printf("X = %lf\t", v.x);
    printf("Y = %lf\n", v.y);
}

void print_cycle(Cycle* cycle){
    printf("cycle->cycle_size = %i\n", cycle->cycle_size);
    for(int i = 0; i < cycle->cycle_size; i++){
        printf("cycle->vertex_cycle[%i].id = %i\n", i, cycle->vertex_cycle[i].id);
    }
    printf("Result distance: %lf\n\n", cycle->result);
}

void print_result(Cycle* cycle){
    printf("Result = %lf\n", cycle->result);
}

void check_for_duplicates(Cycle* cycle){
    for (int i = 0; i < cycle->cycle_size; i++)
    {
        for (int j = 0; j < cycle->cycle_size; j++)
        {
            if(i != j){
                if(cycle->vertex_cycle[i].id == cycle->vertex_cycle[j].id){
                    printf("DUPLICATE\n");
                }
            }
        }   
    }
}

Cycle* get_best_cycle_from_gene(Cycle** gene, int gene_size){

    Cycle* best_cycle = gene[0];
    for (int i = 1; i < gene_size; i++)
    {
        if(best_cycle->result > gene[i]->result){
            best_cycle = gene[i];
        }
    }
    
    return best_cycle;
}

Cycle* get_worst_cycle_from_gene(Cycle** gene, int gene_size){

    Cycle* worst_cycle = gene[0];
    for (int i = 1; i < gene_size; i++)
    {
        if(worst_cycle->result < gene[i]->result){
            worst_cycle = gene[i];
        }
    }
    
    return worst_cycle;
}

void copy_cycle(Cycle* dest, Cycle* origin){
    dest->cycle_size = origin->cycle_size;
    dest->result = origin->result;
    for (int i = 0; i < origin->cycle_size; i++)
    {
        dest->vertex_cycle[i] = origin->vertex_cycle[i];
    }
}

int main(int argc, char *argv[]){
    
    if(argc < 2){
        printf("This program needs at least 2 arguments to run: <exe_name> <input_file>");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    
    char input_file_directory[30] = "data\\";    // Maybe could be a #define idk
    strcat(input_file_directory, argv[1]);       // data\<input_file>

    FILE *file = fopen(input_file_directory, "r"); // files from the data directory are read only files
    if (file == NULL){
        printf("Error while searching for the input file.\nCheck if the input file name is typed correctly or type the input file's directory.\n");
        return -1;
    }

    char line[100] = "";
    int line_count = 0;
    
    // Just counting the number of lines in the file (so we can get the amount of vertices)
    while (fgets(line, sizeof(line), file)){
        line_count++;
    }
    
    int vertex_count = line_count - 7; //Amount of lines that contain vertex data from input file
    Vertex* vertex_list = calloc(vertex_count, sizeof(Vertex));
    
    //Resetting file pointer to the start of file again
    fseek(file, 0, SEEK_SET);

    line_count = 0;
    // Fillig vertex_list with vertices (vertex structs) based on the input file data (id, x coordinate, y coordinate)
    while (fgets(line, sizeof(line), file))
    {
        // We don't care about the first 6 lines (they are only there for header information purposes) and neither the last one (EOF string)
        if (line_count >= HEADER_LINE_COUNT && strcmp(line, "EOF\n") != 0 && strcmp(line, "EOF \n") != 0 && strcmp(line, "EOF") != 0){
            // Separating id, x coordinate and y coordinate (strtok is destructive btw)
            char *first_piece = strtok(line, " ");
            char *second_piece = strtok(NULL, " ");
            char *third_piece = strtok(NULL, " ");
            char *err;

            Vertex vertex = {.id = atoi(first_piece), 
                            .x = strtod(second_piece, &err), 
                            .y = strtod(third_piece, &err)};
            vertex_list[vertex.id - 1] = vertex;
        }
        line_count++;
    }

    //Producing solutions 
    Cycle* gene[GENE_SIZE] = {0};
    
    printf("\nStarting gene creation with tsp_NND...\n");
    
    long start = time(NULL);
    for (int i = 0; i < GENE_SIZE; i++)
    {
        gene[i] = tsp_NND(vertex_list, vertex_count);
        // print_result(gene[i]);
    }
    
    print_result(get_best_cycle_from_gene(gene, GENE_SIZE));
    
    printf("\nStarting iterations...\n");
    long time_elapsed_in_seconds = 0;
    int iterations = 0;

    while(time_elapsed_in_seconds < TIME_IN_SECONDS){
        //Chromosome selection
        Cycle* parents[2];

        int parent_index[2] = {-1, -1};
        
        //Tournament to select both parents (3 contestants: k1, k2, k3)
        for (int i = 0; i < 2; i++)
        {
            int k1 = rand() % GENE_SIZE;
            int k2 = rand() % GENE_SIZE;
            int k3 = rand() % GENE_SIZE;
        
            while (k1 == k2 || k1 == k3 || k2 == k3 || k1 == parent_index[0] || k2 == parent_index[0] || k3 == parent_index[0])
            {
                k1 = rand() % GENE_SIZE;
                k2 = rand() % GENE_SIZE;
                k3 = rand() % GENE_SIZE;
            }

            if(gene[k1]->result > gene[k2]->result && gene[k1]->result > gene[k3]->result){
                parents[i] = gene[k1];
                parent_index[i] = k1;
            }else if(gene[k2]->result > gene[k1]->result && gene[k2]->result > gene[k3]->result){
                parents[i] = gene[k2];
                parent_index[i] = k2;
            }else{
                parents[i] = gene[k3];
                parent_index[i] = k3;
            }
        }

        //Crossover
        Descendants* d = cycle_crossover(parents[0], parents[1]);

        //Mutation
        if((rand() % 100) <= MUTATION_RATE){
            mutate_cycle(d->descendant1);
            mutate_cycle(d->descendant2);
        }
        
        //Local Search
        tsp_2Opt_Optimal(d->descendant1);
        tsp_2Opt_Optimal(d->descendant2);

        //Update
        Cycle* worst_cycle = get_worst_cycle_from_gene(gene, GENE_SIZE);
        if((worst_cycle->result > d->descendant1->result) && (worst_cycle->result > d->descendant2->result)){
            
            int i1 = rand() % GENE_SIZE;
            int i2 = rand() % GENE_SIZE;

            while(i1 == parent_index[0] && i2 == parent_index[1] && i1 == i2){
                i1 = rand() % GENE_SIZE;
                i2 = rand() % GENE_SIZE;
            }

            copy_cycle(gene[i1], d->descendant1);
            copy_cycle(gene[i2], d->descendant2);
        }

        destroy_cycle(d->descendant1);
        destroy_cycle(d->descendant2);
        free(d);

        if((iterations % 1500) == 0){
            printf("Iteration = %i\n", iterations);
            print_result(get_best_cycle_from_gene(gene, GENE_SIZE));
        }
        
        long finish = time(NULL);
        time_elapsed_in_seconds = finish - start;
        iterations++;
    }
    
    //Freeing memory
    for (int i = 0; i < GENE_SIZE; i++)
    {
        destroy_cycle(gene[i]);
    }
    
    free(vertex_list);
    vertex_list = NULL;
    
    fclose(file);
    return 0;
}