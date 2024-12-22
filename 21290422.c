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
    int numGrades;
} Student;

typedef struct {
    int highestGrade;
    int lowestGrade;
    int* passingPerQuestion;
    int totalPassing;
    int numStudents;
    int numGrades;
    Student* students;
    sem_t statsSemaphore;
} OverallStats;

OverallStats stats;

void* processStudent(void* arg) {
    Student* student = (Student*)arg;
    int sum = 0;
    
    for (int i = 0; i < student->numGrades; i++) {
        sum += student->grades[i];
        
        sem_wait(&stats.statsSemaphore);
        if (student->grades[i] >= PASS_THRESHOLD) {
            stats.passingPerQuestion[i]++;
        }
        if (student->grades[i] > stats.highestGrade) {
            stats.highestGrade = student->grades[i];
        }
        if (student->grades[i] < stats.lowestGrade) {
            stats.lowestGrade = student->grades[i];
        }
        sem_post(&stats.statsSemaphore);
    }
    
    student->average = (double)sum / student->numGrades;
    student->passed = (student->average >= PASS_THRESHOLD);
    
    sem_wait(&stats.statsSemaphore);
    if (student->passed) {
        stats.totalPassing++;
    }
    sem_post(&stats.statsSemaphore);
    
    return NULL;
}

int main() {
    FILE *input = fopen("./input2/input.txt", "r");
    if (!input) {
        printf("Error opening input file\n");
        return 1;
    }
    
    fscanf(input, "%d", &stats.numStudents);
    fscanf(input, "%d", &stats.numGrades);
    
    stats.students = malloc(stats.numStudents * sizeof(Student));
    stats.passingPerQuestion = calloc(stats.numGrades, sizeof(int));
    stats.highestGrade = 0;
    stats.lowestGrade = 100;
    stats.totalPassing = 0;
    
    sem_init(&stats.statsSemaphore, 0, 1);
    
    pthread_t* threads = malloc(stats.numStudents * sizeof(pthread_t));
    
    // Read student data and create threads
    for (int i = 0; i < stats.numStudents; i++) {
        stats.students[i].grades = malloc(stats.numGrades * sizeof(int));
        stats.students[i].numGrades = stats.numGrades;
        
        fscanf(input, "%d", &stats.students[i].id);
        for (int j = 0; j < stats.numGrades; j++) {
            fscanf(input, "%d", &stats.students[i].grades[j]);
        }
        
        pthread_create(&threads[i], NULL, processStudent, &stats.students[i]);
    }
    
    fclose(input);
    
    // Wait for all threads to complete
    for (int i = 0; i < stats.numStudents; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Write results to output file
    FILE *output = fopen("./input2/results1.txt", "w");
    if (!output) {
        printf("Error opening output file\n");
        return 1;
    }
    
    // Write individual results
    for (int i = 0; i < stats.numStudents; i++) {
        fprintf(output, "%d %.2f %s\n", 
                stats.students[i].id,
                stats.students[i].average,
                stats.students[i].passed ? "Passed" : "Failed");
    }
    
    // Write statistics
    fprintf(output, "\n--- Overall Statistics ---\n");
    fprintf(output, "Number of students passing each question:\n");
    for (int i = 0; i < stats.numGrades; i++) {
        fprintf(output, "Question %d: %d students passed.\n", 
                i + 1, stats.passingPerQuestion[i]);
    }
    
    fprintf(output, "Total number of students who passed overall: %d\n", 
            stats.totalPassing);
    fprintf(output, "Highest grade: %d\n", stats.highestGrade);
    fprintf(output, "Lowest grade: %d\n", stats.lowestGrade);
    
    fclose(output);
    
    // Cleanup
    sem_destroy(&stats.statsSemaphore);
    for (int i = 0; i < stats.numStudents; i++) {
        free(stats.students[i].grades);
    }
    free(stats.students);
    free(stats.passingPerQuestion);
    free(threads);
    
    return 0;
}