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

OverallStats studentStatistics;

void* processStudent(void* arg) {
    Student* student = (Student*)arg;
    int sum = 0;
    
    for (int i = 0; i < student->numGrades; i++) {
        sum += student->grades[i];
        
        sem_wait(&studentStatistics.statsSemaphore);
        if (student->grades[i] >= PASS_THRESHOLD) {
            studentStatistics.passingPerQuestion[i]++;
        }
        if (student->grades[i] > studentStatistics.highestGrade) {
            studentStatistics.highestGrade = student->grades[i];
        }
        if (student->grades[i] < studentStatistics.lowestGrade) {
            studentStatistics.lowestGrade = student->grades[i];
        }
        sem_post(&studentStatistics.statsSemaphore);
    }
    
    student->average = (double)sum / student->numGrades;
    student->passed = (student->average >= PASS_THRESHOLD);
    
    sem_wait(&studentStatistics.statsSemaphore);
    if (student->passed) {
        studentStatistics.totalPassing++;
    }
    sem_post(&studentStatistics.statsSemaphore);
    
    return NULL;
}

int main() {
    FILE *input = fopen("./input2/input.txt", "r");
    if (!input) {
        printf("Error opening input file\n");
        return 1;
    }
    
    fscanf(input, "%d", &studentStatistics.numStudents);
    fscanf(input, "%d", &studentStatistics.numGrades);
    
    studentStatistics.students = malloc(studentStatistics.numStudents * sizeof(Student));
    studentStatistics.passingPerQuestion = calloc(studentStatistics.numGrades, sizeof(int));
    studentStatistics.highestGrade = 0;
    studentStatistics.lowestGrade = 100;
    studentStatistics.totalPassing = 0;
    
    sem_init(&studentStatistics.statsSemaphore, 0, 1);
    
    pthread_t* threads = malloc(studentStatistics.numStudents * sizeof(pthread_t));
    
    for (int i = 0; i < studentStatistics.numStudents; i++) {
        studentStatistics.students[i].grades = malloc(studentStatistics.numGrades * sizeof(int));
        studentStatistics.students[i].numGrades = studentStatistics.numGrades;
        
        fscanf(input, "%d", &studentStatistics.students[i].id);
        for (int j = 0; j < studentStatistics.numGrades; j++) {
            fscanf(input, "%d", &studentStatistics.students[i].grades[j]);
        }
        
        pthread_create(&threads[i], NULL, processStudent, &studentStatistics.students[i]);
    }
    
    fclose(input);
    
    for (int i = 0; i < studentStatistics.numStudents; i++) {
        pthread_join(threads[i], NULL);
    }
    
    FILE *output = fopen("./input2/results1.txt", "w");
    if (!output) {
        printf("Error opening output file\n");
        return 1;
    }
    
    for (int i = 0; i < studentStatistics.numStudents; i++) {
        fprintf(output, "%d %.2f %s\n", 
                studentStatistics.students[i].id,
                studentStatistics.students[i].average,
                studentStatistics.students[i].passed ? "Passed" : "Failed");
    }
    
    fprintf(output, "\n--- Overall Statistics ---\n");
    fprintf(output, "Number of students passing each question:\n");
    for (int i = 0; i < studentStatistics.numGrades; i++) {
        fprintf(output, "Question %d: %d students passed.\n", 
                i + 1, studentStatistics.passingPerQuestion[i]);
    }
    
    fprintf(output, "Total number of students who passed overall: %d\n", 
            studentStatistics.totalPassing);
    fprintf(output, "Highest grade: %d\n", studentStatistics.highestGrade);
    fprintf(output, "Lowest grade: %d\n", studentStatistics.lowestGrade);
    
    fclose(output);
    
    sem_destroy(&studentStatistics.statsSemaphore);
    for (int i = 0; i < studentStatistics.numStudents; i++) {
        free(studentStatistics.students[i].grades);
    }
    free(studentStatistics.students);
    free(studentStatistics.passingPerQuestion);
    free(threads);
    
    return 0;
}
