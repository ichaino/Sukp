#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

typedef struct {
    int id;
    double weight;          
    double efficiency;      
} Element;

typedef struct {
    int id;
    double profit;
    int *required_elements;         // elementos necesarios para completar el item
    int element_count;              // cantidad de elementos que componen el item
    double profit_per_element;
} Item;

void leer_instancia(const char *filename, int *m, int *n, double *capacity, 
                   double **item_profits, double **element_weights, 
                   int ***matrix, int **element_counts) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error al abrir archivo\n");
        exit(1);
    }
    
    fscanf(file, "%d", m); // m items
    fscanf(file, "%d", n); // n elementos
    fscanf(file, "%lf", capacity); // capacidad maxima
    
    *item_profits = (double*)malloc(*m * sizeof(double)); // leo las ganancias de los items
    for (int i = 0; i < *m; i++) {
        fscanf(file, "%lf", &(*item_profits)[i]);
    }
    
    *element_weights = (double*)malloc(*n * sizeof(double)); // leo los pesos de los elementos
    for (int i = 0; i < *n; i++) {
        fscanf(file, "%lf", &(*element_weights)[i]);
    }
    
    *matrix = (int**)malloc(*m * sizeof(int*)); // creo matriz para identificar que elementos pertenecen a cada item
    *element_counts = (int*)malloc(*m * sizeof(int)); 
    
    // cuento los elementos de cada item
    for (int i = 0; i < *m; i++) {
        (*matrix)[i] = (int*)malloc(*n * sizeof(int));
        (*element_counts)[i] = 0;
        
        for (int j = 0; j < *n; j++) {
            fscanf(file, "%d", &(*matrix)[i][j]);
            if ((*matrix)[i][j] == 1) {
                (*element_counts)[i]++;
            }
        }
    }
    
    fclose(file);
}

// inicializacion de items y elementos + calculo de eficiencia
void initialize_items_and_elements(int m, int n, double *item_profits, double *element_weights, 
                                 int **matrix, int *element_counts, Element *elements, Item *items) {
    
    // inicializo los elementos
    for (int j = 0; j < n; j++) {
        elements[j].id = j;
        elements[j].weight = element_weights[j];
        elements[j].efficiency = 0.0;
    }
    
    // inicializo items y calculo ganancia por elemento
    for (int i = 0; i < m; i++) {
        items[i].id = i;
        items[i].profit = item_profits[i];
        items[i].element_count = element_counts[i];
        items[i].required_elements = (int*)malloc(n * sizeof(int));
        
        int count = 0;
        double total_weight = 0.0;

        for (int j = 0; j < n; j++) {
            if (matrix[i][j] == 1) {
                items[i].required_elements[count++] = j; // guardo el indice del elemento
                total_weight += elements[j].weight; // sumo el peso total del item
                elements[j].efficiency += item_profits[i] / element_counts[i]; // distribuyo la ganancia en cada uno de los elementos del item
            }
        }
        
        items[i].profit_per_element = (total_weight > 0) ? item_profits[i] / total_weight : 0; // si el peso total > 0, calculo la ganancia del elemento
    }
    
    // elijo el elemento de mayor eficiencia con respecto al peso
    for (int j = 0; j < n; j++) {
        if (elements[j].weight > 0) {
            elements[j].efficiency /= elements[j].weight;
        }
    }
}

// =========== ALGORITMO DETERMINISTA =========== 
double greedy_knapsack(int m, int n, Element *elements, Item *items, 
                               double capacity, int *solution) {
    
    double current_weight = 0.0;
    double total_profit = 0.0;

    // veo que elemento es seleccionado
    for (int j = 0; j < n; j++) {
        solution[j] = 0;
    }
    
    // ordeno los items de mayor a menor eficiencia (bubblesort)
    int *item_order = (int*)malloc(m * sizeof(int));
    for (int i = 0; i < m; i++) item_order[i] = i;
    
    for (int i = 0; i < m - 1; i++) {
        for (int k = i + 1; k < m; k++) {
            if (items[item_order[i]].profit_per_element < items[item_order[k]].profit_per_element) {
                int temp = item_order[i];
                item_order[i] = item_order[k];
                item_order[k] = temp;
            }
        }
    }
    
    // intento completar los items en orden
    for (int i = 0; i < m; i++) {
        int item_id = item_order[i];
        
        double additional_weight = 0.0;
        int can_add = 1; // se asume que se puede agregar
        
        // verifico si cabe 
        for (int e = 0; e < items[item_id].element_count; e++) {
            int elem_id = items[item_id].required_elements[e];
            if (solution[elem_id] == 0) { // veo si el elemento no esta seleccionado
                additional_weight += elements[elem_id].weight;
                if (current_weight + additional_weight > capacity) {
                    can_add = 0; // no cabe
                    break;
                }
            }
        }
        
        // agrego el item si es posible
        if (can_add && additional_weight > 0) {
            // seleccino los elementos faltantes
            for (int e = 0; e < items[item_id].element_count; e++) {
                int elem_id = items[item_id].required_elements[e];
                if (solution[elem_id] == 0) {
                    solution[elem_id] = 1; // marco el elemento como seleccionado
                    current_weight += elements[elem_id].weight;
                }
            }
            total_profit += items[item_id].profit; // sumo la ganancia del item completo
        }
    }
    
    free(item_order);
    return total_profit;
}

// =========== ALGORITMO PROBABILISTICO =========== 
double probabilistic_algorithm(int m, int n, Element *elements, Item *items, 
                               double capacity, int *solution, int seed) {
    srand(seed); // distintas semillas
    
    double current_weight = 0.0;
    
    for (int j = 0; j < n; j++) {
        solution[j] = 0;
    }
    
    // // ordeno los items de mayor a menor eficiencia (bubblesort)
    int *item_order = (int*)malloc(m * sizeof(int));
    for (int i = 0; i < m; i++) item_order[i] = i;
    
    for (int i = 0; i < m - 1; i++) {
        for (int k = i + 1; k < m; k++) {
            if (items[item_order[i]].profit_per_element < items[item_order[k]].profit_per_element) {
                int temp = item_order[i];
                item_order[i] = item_order[k];
                item_order[k] = temp;
            }
        }
    }
    
    for (int i = 0; i < m; i++) {
        int item_id = item_order[i];
        
        if ((double)rand() / RAND_MAX < 0.7) { // genero un numero entre 0 y 1 -> si es menor a 0.7, lo agrego
            double additional_weight = 0.0;    //                              -> si es mayor a 0.7, salta el item
            int can_add = 1;
            
            // veo cuanto peso adicional necesito
            for (int e = 0; e < items[item_id].element_count; e++) {
                int elem_id = items[item_id].required_elements[e];
                if (solution[elem_id] == 0) { // si el elemento no es seleccionado
                    additional_weight += elements[elem_id].weight;
                    if (current_weight + additional_weight > capacity) {
                        can_add = 0; // no cabe
                        break;
                    }
                }
            }
            
            if (can_add && additional_weight > 0) {
                // agrego los elementos faltantes
                for (int e = 0; e < items[item_id].element_count; e++) {
                    int elem_id = items[item_id].required_elements[e];
                    if (solution[elem_id] == 0) {
                        solution[elem_id] = 1; // marco como seleccionado
                        current_weight += elements[elem_id].weight;
                    }
                }
            }
        }
    }
    
    free(item_order);
    
    // evaluo si los items estan completos
    double total_profit = 0.0;
    for (int i = 0; i < m; i++) {
        int is_complete = 1;
        for (int e = 0; e < items[i].element_count; e++) {
            if (solution[items[i].required_elements[e]] == 0) {
                is_complete = 0; // item incompleto
                break;
            }
        }
        if (is_complete) {
            total_profit += items[i].profit; // solo sumo los items completos
        }
    }
    
    return total_profit;
}

// =========== ALGORITMO LNS =========== 
double lns_algorithm(int m, int n, Element *elements, Item *items, 
                     double capacity, int seed, int *best_solution, 
                     double destroy_rate, int max_iter) {
    srand(seed);
    int *current_solution = (int*)malloc(n * sizeof(int));
    double best_profit = greedy_knapsack(m, n, elements, items, capacity, current_solution);
    memcpy(best_solution, current_solution, n * sizeof(int));

    int *candidate_solution = (int*)malloc(n * sizeof(int));

    for (int iter = 0; iter < max_iter; iter++) { // bucle con n iteraciones para el lns
        memcpy(candidate_solution, best_solution, n * sizeof(int)); // copio la mejor solucion actual

        // cuento la cantidad de elementos seleccionados
        int selected_count = 0; 
        for (int j = 0; j < n; j++) {
            if (candidate_solution[j]) {
                selected_count++;
            }
        }
        
        // destruccion de los elementos
        int to_remove = (int)(destroy_rate * selected_count);
        for (int k = 0; k < to_remove; k++) {
            int id;
            do { id = rand() % n; } while (candidate_solution[id] == 0); // elijo un elemento aleatoreo que este seleccionado
            candidate_solution[id] = 0; // lo elimino
        }

        // calculo el peso de los elementos seleccionados luego de la destruccion
        double current_weight = 0.0;
        for (int j = 0; j < n; j++){
            if (candidate_solution[j]){
                current_weight += elements[j].weight;
            }
        }

        // reordeno items degun la eficiencia (bubblesort)
        int *item_order = (int*)malloc(m * sizeof(int));
        for (int i = 0; i < m; i++) {
            item_order[i] = i;
        }

        for (int i = 0; i < m - 1; i++) { 
            for (int k = i + 1; k < m; k++) {
                if (items[item_order[i]].profit_per_element < items[item_order[k]].profit_per_element) {
                    int temp = item_order[i];
                    item_order[i] = item_order[k];
                    item_order[k] = temp;
                }
            }
        }

        // reconstruccion para añadir los items
        for (int i = 0; i < m; i++) {
            int item_id = item_order[i];
            if (((double)rand() / RAND_MAX) > 0.6){ // 40% probabilidad de añadir el item
                continue;
            }
            
            // veo si el item cabe
            double add_weight = 0.0;
            int can_add = 1;
            for (int e = 0; e < items[item_id].element_count; e++) {
                int elem = items[item_id].required_elements[e];
                if (!candidate_solution[elem]) { // si el elemento no esta seleccionado sumo su peso
                    add_weight += elements[elem].weight;
                    if (current_weight + add_weight > capacity) { // si excedo la capacidad total
                        can_add = 0; // no lo agrego
                        break;
                    }
                }
            }
            if (can_add) { // lo agrego si cabe
                for (int e = 0; e < items[item_id].element_count; e++) {
                    int elem = items[item_id].required_elements[e];
                    candidate_solution[elem] = 1; // marco los elementos como seleccionados
                }
                current_weight += add_weight; // actualizo el peso
            }
        }

        free(item_order);

        //compruebo para cada item si todos los elementos son seleccionados 
        double total_profit = 0.0;
        for (int i = 0; i < m; i++) {
            int complete = 1;
            for (int e = 0; e < items[i].element_count; e++) { // si algun elemento no fue seleccionado (item incompleto)
                if (candidate_solution[items[i].required_elements[e]] == 0) {
                    complete = 0;
                    break;
                }
            }
            if (complete){ //sumo la ganancia si esta ccompleto
                total_profit += items[i].profit;  
            } 
        }

        // lo reemplazo si hay una mejora
        if (total_profit > best_profit) {
            best_profit = total_profit;
            memcpy(best_solution, candidate_solution, n * sizeof(int));
        }
    }

    free(candidate_solution);
    free(current_solution);
    return best_profit;
}

int main() {
    char filename[256];
    int opcion;

    int m, n;
    double capacity;
    double *item_profits = NULL, *element_weights = NULL;
    int **matrix = NULL, *element_counts = NULL;
    Element *elements = NULL;
    Item *items = NULL;

    printf("=== ALGORITMO SUKP ===\n");

    while (1) {
        // si la instancia no esta cargada pido el archivo
        if (item_profits == NULL) {
            printf("\nIngrese el nombre del archivo de instancia (ej: 100_100_0.10_0.75.txt):\n> ");
            scanf("%s", filename);

            leer_instancia(filename, &m, &n, &capacity, &item_profits, &element_weights, &matrix, &element_counts);

            elements = (Element*)malloc(n * sizeof(Element));
            items = (Item*)malloc(m * sizeof(Item));
            initialize_items_and_elements(m, n, item_profits, element_weights, matrix, element_counts, elements, items);

            printf("\nInstancia cargada: %s (%d items, %d elementos, capacidad %.2f)\n", filename, m, n, capacity);
        }

        // menu
        printf("\n===========================================\n");
        printf("Archivo actual: %s\n", filename);
        printf("===========================================\n");
        printf("1) Ejecutar algoritmo Determinista\n");
        printf("2) Ejecutar algoritmo Probabilístico\n");
        printf("3) Ejecutar algoritmo LNS\n");
        printf("4) Cambiar instancia\n");
        printf("5) Salir\n");
        printf("Seleccione una opción: ");
        scanf("%d", &opcion);

        if (opcion == 1) {
            printf("\n=== ALGORITMO DETERMINISTA ===\n");
            int *sol = (int*)malloc(n * sizeof(int));
            double profit = greedy_knapsack(m, n, elements, items, capacity, sol);
            printf("Ganancia total: %.2f\n", profit);
            free(sol);
        } 
        else if (opcion == 2) {
            printf("\n=== ALGORITMO PROBABILÍSTICO ===\n");
            printf("Semilla  | Ganancia Real\n");
            printf("---------|---------------\n");

            double max_profit = 0.0, sum_profit = 0.0;
            int best_seed = 0;
            for (int seed = 1; seed <= 30; seed++) {
                int *sol = (int*)malloc(n * sizeof(int));
                double profit = probabilistic_algorithm(m, n, elements, items, capacity, sol, seed);
                printf("%8d | %13.2f\n", seed, profit);
                if (profit > max_profit) { max_profit = profit; best_seed = seed; }
                sum_profit += profit;
                free(sol);
            }
            double avg_profit = sum_profit / 30.0;
            printf("\nMejor semilla: %d - Ganancia: %.2f\n", best_seed, max_profit);
            printf("Ganancia promedio: %.2f\n", avg_profit);
        } 
        else if (opcion == 3) {
            double destroy_rate;
            int max_iter;
            printf("\nIngrese el porcentaje de destrucción (0.0 - 1.0): ");
            scanf("%lf", &destroy_rate);
            printf("Ingrese la cantidad de iteraciones: ");
            scanf("%d", &max_iter);

            printf("\n=== ALGORITMO LNS ===\n");
            printf("Semilla  | Ganancia Real\n");
            printf("---------|---------------\n");

            double max_profit = 0.0, sum_profit = 0.0;
            int best_seed = 0;
            for (int seed = 1; seed <= 30; seed++) {
                int *sol = (int*)malloc(n * sizeof(int));
                double profit = lns_algorithm(m, n, elements, items, capacity, seed, sol, destroy_rate, max_iter);
                printf("%8d | %13.2f\n", seed, profit);
                if (profit > max_profit) { max_profit = profit; best_seed = seed; }
                sum_profit += profit;
                free(sol);
            }
            double avg_profit = sum_profit / 30.0;
            printf("\nMejor semilla: %d - Ganancia: %.2f\n", best_seed, max_profit);
            printf("Ganancia promedio: %.2f\n", avg_profit);
        } 
        else if (opcion == 4) {
            // libero la instancia y solicito otra
            for (int i = 0; i < m; i++) free(matrix[i]);
            free(matrix);
            free(items);
            free(elements);
            free(item_profits);
            free(element_weights);
            free(element_counts);
            item_profits = NULL; // marcar que no hay instancia cargada
            printf("\nCambiando instancia...\n");
            continue;
        } 
        else if (opcion == 5) {
            printf("\nPrograma terminado.\n");
            break;
        } 
        else {
            printf("\nOpción no válida.\n");
        }
    }

    // Liberar recursos si quedan cargados
    if (item_profits) {
        for (int i = 0; i < m; i++) free(matrix[i]);
        free(matrix);
        free(items);
        free(elements);
        free(item_profits);
        free(element_weights);
        free(element_counts);
    }

    return 0;
}