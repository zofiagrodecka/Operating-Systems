#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#define MAX_LINE_LENGTH 70

struct pixel{
    int x;
    int y;
    int value;
};

struct counter_element{
    int counter;
    struct pixel *pixels;
};

struct values_arguments{
    int **image;
    struct pixel *segment;
    int n_pixels;
};

void *calculate_negative_values(void *args){
    clock_t start, end;

    start = clock();
    while(start == (end = clock()));
    start = end;

    struct values_arguments *arguments = args;

    for(int i=0; i<arguments->n_pixels; i++){
        arguments->image[arguments->segment[i].y][arguments->segment[i].x] = 255 - arguments->image[arguments->segment[i].y][arguments->segment[i].x];
    }

    end = clock();

    long *time = malloc(sizeof(long));
    if(time == NULL){
        perror("Malloc error");
        exit(5);
    }

    *time = (end - start) * (1000000 / CLOCKS_PER_SEC);

    pthread_exit(time);
}

struct block_arguments{ // block_arguments to calculate_negative_blocks
    int **image;
    int start;
    int end;
    int height;
};

void *calculate_negative_blocks(void *args){
    clock_t start, end;

    start = clock();
    while(start == (end = clock()));
    start = end;

    struct block_arguments *arguments = args;
    for(int i=0; i<arguments->height; i++){
        for(int j=arguments->start; j <= arguments->end; j++){
            arguments->image[i][j] = 255 - arguments->image[i][j];
        }
    }

    end = clock();

    long *time = malloc(sizeof(long));
    if(time == NULL){
        perror("Malloc error");
        exit(5);
    }

    *time = (end - start) * (1000000 / CLOCKS_PER_SEC);

    pthread_exit(time);
}

void counting_sort(struct counter_element *tab, int size, int max){
    struct counter_element tmp[size];
    int C[max+1];
    for(int i=0; i<max+1; i++){
        C[i] = 0;
    }
    for(int i=0; i<size; i++){
        C[tab[i].counter]++;
    }
    for(int i=1; i < max+1; i++){
        C[i] += C[i-1];
    }
    for(int i=size-1; i >= 0; i--){
        C[tab[i].counter]--;
        tmp[C[tab[i].counter]] = tab[i];
    }
    for(int i=0; i<size; i++){
        tab[i] = tmp[i];
    }
}

void add(struct counter_element *tab, int index, struct pixel *pixel_tab, int start){
    for(int i=0; i<tab[index].counter; i++){
        pixel_tab[start] = tab[index].pixels[i];
        start++;
    }
}

int main(int argc, char *argv[]) {
    if(argc != 5){
        perror("Wrong number of invocation block_arguments");
        exit(1);
    }

    int n = (int)strtol(argv[1], NULL, 10);

    FILE *input_img = fopen(argv[3], "r");
    if(input_img == NULL){
        perror("Cannot open file");
        exit(2);
    }

    FILE *output_img = fopen(argv[4], "w");
    if(output_img == NULL){
        perror("Cannot open file");
        exit(2);
    }

    FILE *time_file = fopen("Times.txt", "a");
    if(output_img == NULL) {
        perror("Cannot open file");
    }

    char str[MAX_LINE_LENGTH];

    if(fscanf(input_img, "%s", str) == 0){
        perror("Error while reading file");
        exit(3);
    }

    if(strcmp(str, "P2") != 0){
        perror("Wrong image type");
        exit(1);
    }
    if(fprintf(output_img, "%s\n", str) == 0){
        perror("Error while writing to file");
        exit(1);
    }

    int width, height, max_pixel_value;
    if(fscanf(input_img, "%d %d %d", &width, &height, &max_pixel_value) <= 0){
        perror("Error while reading file");
    }
    if(fprintf(output_img, "%d %d\n%d\n", width, height, max_pixel_value) == 0){
        perror("Error while writing to file");
        exit(1);
    }

    printf("w: %d, h: %d, m: %d\n", width, height, max_pixel_value);
    fprintf(time_file, "Wymiary obrazu:\nw: %d, h: %d, m: %d\n", width, height, max_pixel_value);
    fprintf(time_file, "Liczba watkow: %d\n", n);

    struct counter_element counter[max_pixel_value+1];
    for(int i=0; i<max_pixel_value+1; i++){
        counter[i].counter = 0;
        counter[i].pixels = calloc(width * height, sizeof(struct pixel));
    }

    int **image;
    image = calloc(height, sizeof(int *));
    for(int i=0; i<height; i++){
        image[i] = calloc(width, sizeof(int));
    }

    struct pixel pixel;
    for(int i = 0; i < height; i++){
        for(int j = 0; j<width; j++){
            if(fscanf(input_img, "%d", &image[i][j]) <= 0){
                perror("Error while reading file");
            }

            pixel.value = image[i][j];
            pixel.x = j;
            pixel.y = i;
            counter[image[i][j]].pixels[counter[image[i][j]].counter] = pixel;
            counter[image[i][j]].counter++;

            //printf("%d \n", image[i][j]);
        }
        //printf("\n");
    }

    // tworzenie negatywu
    pthread_t threads[n];
    struct block_arguments *args = calloc(n, sizeof(struct block_arguments));
    struct values_arguments *vargs = calloc(n, sizeof(struct values_arguments));
    struct pixel **segments = calloc(n, sizeof(struct pixel*));
    for(int i=0; i<n; i++){
        segments[i] = calloc(width*height, sizeof(struct pixel));
    }
    int *n_pixels;
    n_pixels = calloc(n, sizeof(int));
    if(n_pixels == NULL){
        perror("Error while allocating memory for n_pixels");
        exit(100);
    }
    int thread_ind = 0;
    int visited[max_pixel_value+1];
    for(int i=0; i<max_pixel_value+1; i++){
        visited[i] = 0;
    }

    clock_t t0, t1;
    t0 = clock();
    while(t0 == (t1 = clock()));
    t0 = t1;

    if(strcmp(argv[2], "block") == 0){
        int start;
        int end;

        for(int i=1; i<=n; i++){ // numeruje watki od 1
            start = ceil((i-1)*width/n);
            end = ceil(i*width/n) - 1;
            args[i-1].start = start;
            args[i-1].end = end;
            args[i-1].height = height;
            args[i-1].image = image;
            if((pthread_create(&threads[i - 1], NULL, calculate_negative_blocks, &args[i - 1])) != 0){
                perror("Error while creating a thread");
                exit(10);
            }
        }
    }
    else if(strcmp(argv[2], "numbers") == 0){
        int average_n_pixels_for_segment = (width * height) / n;
        counting_sort(counter, max_pixel_value+1, width * height);

        int ind = max_pixel_value; // indeks w counterze
        int segment_ind = 0; // przepisuje piksele do tablicy segment
        int sum = 0;

        // dodaje ostatni element countera do 0 segmentu
        sum = counter[ind].counter;
        add(counter, ind, segments[thread_ind], segment_ind);
        n_pixels[thread_ind] = counter[ind].counter;
        segment_ind = n_pixels[thread_ind] - 1;
        visited[ind] = 1;
        ind--;

        while(ind >=0 && thread_ind < n && counter[ind].counter > 0){

            if(sum + counter[ind].counter < average_n_pixels_for_segment){ // dodaje do tego samego segmentu
                sum += counter[ind].counter;
                add(counter, ind, segments[thread_ind], segment_ind);
                n_pixels[thread_ind] = sum;
                segment_ind = n_pixels[thread_ind] - 1;
                visited[ind] = 1;
                vargs[thread_ind].segment = segments[thread_ind];
                vargs[thread_ind].n_pixels = n_pixels[thread_ind];
                vargs[thread_ind].image = image;
            }
            else{ // dodaje do kolejnego segmentu

                vargs[thread_ind].segment = segments[thread_ind];
                vargs[thread_ind].n_pixels = n_pixels[thread_ind];
                vargs[thread_ind].image = image;

                thread_ind++;
                if(thread_ind < n) {
                    sum = counter[ind].counter;
                    segment_ind = 0;
                    add(counter, ind, segments[thread_ind], segment_ind);
                    n_pixels[thread_ind] = counter[ind].counter;
                    segment_ind = n_pixels[thread_ind] - 1;
                    visited[ind] = 1;
                }
            }

            ind--;
        }

       int i = 0;
       int j = 0;
       while(i < max_pixel_value+1 && !visited[i]) {
            add(counter, i, segments[j], n_pixels[j]);
            n_pixels[j] += counter[i].counter;
            visited[i] = 1;
            vargs[j].segment = segments[j];
            vargs[j].n_pixels = n_pixels[j];

           j++;
           if(j == n){
               j = 0;
           }

           i++;
       }

    }
    else{
        perror("Wrong segmentation type");
        exit(15);
    }

   if(strcmp(argv[2], "numbers") == 0){
        for(int i=0; i<n; i++){
            if ((pthread_create(&threads[i], NULL, calculate_negative_values, &vargs[i])) != 0) {
                perror("Error while creating a thread");
                exit(10);
            }
        }
   }

    long *status;
    for(int i=0; i<n; i++){
        if((pthread_join(threads[i], (void *) &status)) != 0){
            perror("Cannot join thread");
            exit(6);
        }
        //printf("exit status: %ld\n", *status);
        fprintf(time_file, "Thread time: %ld us\n", *status);
        free(status);
    }

    t1 = clock();
    long *total_time = malloc(sizeof(long));
    if(total_time == NULL){
        perror("Malloc error");
        exit(5);
    }

    *total_time = (t1 - t0) * (1000000 / CLOCKS_PER_SEC);
    fprintf(time_file, "Total time: %ld us\n", *total_time);
    printf("Time: %ld us\n", *total_time);


    for(int i = 0; i < height; i++){
        for(int j = 0; j<width; j++){
            if(fprintf(output_img, "%d ", image[i][j]) <= 0){
                perror("Error while writing to file");
            }
            //printf("%d ", image[i][j]);
        }
        fprintf(output_img, "\n");
        //printf("\n");
    }

    fprintf(time_file, "---------------------------\n");

    if(fclose(input_img) == EOF){
        perror("Cannot close file");
        exit(2);
    }
    if(fclose(output_img) == EOF){
        perror("Cannot close file");
        exit(2);
    }
    if(fclose(time_file) == EOF){
        perror("Cannot close file");
        exit(2);
    }

    for(int i=0; i<max_pixel_value+1; i++){
        free(counter[i].pixels);
    }

    for(int i=0; i<height; i++){
        free(image[i]);
    }
    free(image);
    free(args);
    free(vargs);

    for (int i = 0; i < n; i++) {
        free(segments[i]);
    }
    free(segments);
    free(n_pixels);
    free(total_time);

    return 0;
}
