#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#define PASS_THRESHOLD 60
#define MAX_GRADES 100

typedef struct {
    int id;
    int* grades;
    double average;
    int passed;
    int num_grades;
} Student;

typedef struct {
    int highest_grade;
    int lowest_grade;
    int* passing_per_question;
    int total_passing;
    int num_students;
    int num_grades;
    Student* students;
    sem_t stats_semaphore;
} GlobalStats;

GlobalStats stats;

void* process_student(void* arg) {
    Student* student = (Student*)arg;
    int sum = 0;
    
    for (int i = 0; i < student->num_grades; i++) {
        sum += student->grades[i];
        
        sem_wait(&stats.stats_semaphore);
        if (student->grades[i] >= PASS_THRESHOLD) {
            stats.passing_per_question[i]++;
        }
        if (student->grades[i] > stats.highest_grade) {
            stats.highest_grade = student->grades[i];
        }
        if (student->grades[i] < stats.lowest_grade) {
            stats.lowest_grade = student->grades[i];
        }
        sem_post(&stats.stats_semaphore);
    }
    
    student->average = (double)sum / student->num_grades;
    student->passed = (student->average >= PASS_THRESHOLD);
    
    sem_wait(&stats.stats_semaphore);
    if (student->passed) {
        stats.total_passing++;
    }
    sem_post(&stats.stats_semaphore);
    
    return NULL;
}

int main() {
    FILE *input = fopen("input.txt", "r");
    if (!input) {
        printf("Error opening input file\n");
        return 1;
    }
    
    fscanf(input, "%d", &stats.num_students);
    fscanf(input, "%d", &stats.num_grades);
    
    stats.students = malloc(stats.num_students * sizeof(Student));
    stats.passing_per_question = calloc(stats.num_grades, sizeof(int));
    stats.highest_grade = 0;
    stats.lowest_grade = 100;
    stats.total_passing = 0;
    
    sem_init(&stats.stats_semaphore, 0, 1);
    
    pthread_t* threads = malloc(stats.num_students * sizeof(pthread_t));
    
    // Read student data and create threads
    for (int i = 0; i < stats.num_students; i++) {
        stats.students[i].grades = malloc(stats.num_grades * sizeof(int));
        stats.students[i].num_grades = stats.num_grades;
        
        fscanf(input, "%d", &stats.students[i].id);
        for (int j = 0; j < stats.num_grades; j++) {
            fscanf(input, "%d", &stats.students[i].grades[j]);
        }
        
        pthread_create(&threads[i], NULL, process_student, &stats.students[i]);
    }
    
    fclose(input);
    
    // Wait for all threads to complete
    for (int i = 0; i < stats.num_students; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Write results to output file
    FILE *output = fopen("results.txt", "w");
    if (!output) {
        printf("Error opening output file\n");
        return 1;
    }
    
    // Write individual results
    for (int i = 0; i < stats.num_students; i++) {
        fprintf(output, "%d %.2f %s\n", 
                stats.students[i].id,
                stats.students[i].average,
                stats.students[i].passed ? "Passed" : "Failed");
    }
    
    // Write statistics
    fprintf(output, "--- Overall Statistics ---\n");
    fprintf(output, "Number of students passing each question:\n");
    for (int i = 0; i < stats.num_grades; i++) {
        fprintf(output, "Question %d: %d students passed.\n", 
                i + 1, stats.passing_per_question[i]);
    }
    
    fprintf(output, "Total number of students who passed overall: %d\n", 
            stats.total_passing);
    fprintf(output, "Highest grade: %d\n", stats.highest_grade);
    fprintf(output, "Lowest grade: %d\n", stats.lowest_grade);
    
    fclose(output);
    
    // Cleanup
    sem_destroy(&stats.stats_semaphore);
    for (int i = 0; i < stats.num_students; i++) {
        free(stats.students[i].grades);
    }
    free(stats.students);
    free(stats.passing_per_question);
    free(threads);
    
    return 0;
}