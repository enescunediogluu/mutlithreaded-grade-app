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

OverallStats studentPerformance;

void* processStudent(void* arg) {
    Student* student = (Student*)arg;
    int sum = 0;
    
    for (int i = 0; i < student->numGrades; i++) {
        sum += student->grades[i];
        
        sem_wait(&studentPerformance.statsSemaphore);
        if (student->grades[i] >= PASS_THRESHOLD) {
            studentPerformance.passingPerQuestion[i]++;
        }
        if (student->grades[i] > studentPerformance.highestGrade) {
            studentPerformance.highestGrade = student->grades[i];
        }
        if (student->grades[i] < studentPerformance.lowestGrade) {
            studentPerformance.lowestGrade = student->grades[i];
        }
        sem_post(&studentPerformance.statsSemaphore);
    }
    
    student->average = (double)sum / student->numGrades;
    student->passed = (student->average >= PASS_THRESHOLD);
    
    sem_wait(&studentPerformance.statsSemaphore);
    if (student->passed) {
        studentPerformance.totalPassing++;
    }
    sem_post(&studentPerformance.statsSemaphore);
    
    return NULL;
}

int main() {
    FILE *input = fopen("./input2/input.txt", "r");
    if (!input) {
        printf("Error opening input file\n");
        return 1;
    }
    
    fscanf(input, "%d", &studentPerformance.numStudents);
    fscanf(input, "%d", &studentPerformance.numGrades);
    
    studentPerformance.students = malloc(studentPerformance.numStudents * sizeof(Student));
    studentPerformance.passingPerQuestion = calloc(studentPerformance.numGrades, sizeof(int));
    studentPerformance.highestGrade = 0;
    studentPerformance.lowestGrade = 100;
    studentPerformance.totalPassing = 0;
    
    sem_init(&studentPerformance.statsSemaphore, 0, 1);
    
    pthread_t* threads = malloc(studentPerformance.numStudents * sizeof(pthread_t));
    
    for (int i = 0; i < studentPerformance.numStudents; i++) {
        studentPerformance.students[i].grades = malloc(studentPerformance.numGrades * sizeof(int));
        studentPerformance.students[i].numGrades = studentPerformance.numGrades;
        
        fscanf(input, "%d", &studentPerformance.students[i].id);
        for (int j = 0; j < studentPerformance.numGrades; j++) {
            fscanf(input, "%d", &studentPerformance.students[i].grades[j]);
        }
        
        pthread_create(&threads[i], NULL, processStudent, &studentPerformance.students[i]);
    }
    
    fclose(input);
    
    for (int i = 0; i < studentPerformance.numStudents; i++) {
        pthread_join(threads[i], NULL);
    }
    
    FILE *output = fopen("./input2/results1.txt", "w");
    if (!output) {
        printf("Error opening output file\n");
        return 1;
    }
    
    for (int i = 0; i < studentPerformance.numStudents; i++) {
        fprintf(output, "%d %.2f %s\n", 
                studentPerformance.students[i].id,
                studentPerformance.students[i].average,
                studentPerformance.students[i].passed ? "Passed" : "Failed");
    }
    
    fprintf(output, "\n--- Overall Statistics ---\n");
    fprintf(output, "Number of students passing each question:\n");
    for (int i = 0; i < studentPerformance.numGrades; i++) {
        fprintf(output, "Question %d: %d students passed.\n", 
                i + 1, studentPerformance.passingPerQuestion[i]);
    }
    
    fprintf(output, "Total number of students who passed overall: %d\n", 
            studentPerformance.totalPassing);
    fprintf(output, "Highest grade: %d\n", studentPerformance.highestGrade);
    fprintf(output, "Lowest grade: %d\n", studentPerformance.lowestGrade);
    
    fclose(output);
    
    sem_destroy(&studentPerformance.statsSemaphore);
    for (int i = 0; i < studentPerformance.numStudents; i++) {
        free(studentPerformance.students[i].grades);
    }
    free(studentPerformance.students);
    free(studentPerformance.passingPerQuestion);
    free(threads);
    
    return 0;
}
