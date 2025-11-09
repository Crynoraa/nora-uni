#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>


//user defines
#define CHUNK_SIZE 1024
#define AVERAGE_RUNS 10


void size() //shows size of the created file from the makefile; change the file size in the makefile directly
{
    FILE *file = fopen("file.txt", "r");
    fseek(file, SEEK_CUR - 1, SEEK_END); // starts from 0th Byte till EOF
    double size = ftell(file);

    if(size / (1024.0*1024.0) >= 0.5 )
    {
        printf("Size of File in MB: %.3f \n", size/(1024.0*1024.0));
    }
    else if (size / (1024.0) >= 0.5 )
    {
        printf("Size of File in KB: %.3f \n", size/(1024.0));
    }
    else
    {
        printf("Size of File in Bytes: %f \n", size);
    }
    printf("//////////////////////////////////////\n");
    return;
}

double single_byte_syscall() //single byte latency using syscalls
{
    struct timeval start, end;
    int fd = open("file.txt", O_RDONLY);
    if (fd < 0)
    {
        perror("Error Opening File");
        return -1;
    }
    char a;
    gettimeofday(&start, NULL);

    read(fd, &a, sizeof(a));

    gettimeofday(&end, NULL);
    double latency_byte = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) /1000000.0;

    close(fd);
    return latency_byte;
}

double file_per_byte_syscall() //time for reading the whole file in single bytes via syscalls
{
    struct timeval start, end;
    int fd = open("file.txt", O_RDONLY);
    if (fd < 0)
    {
        perror("Error Opening File");
        return -1;
    }
    char a;
    int x;
    gettimeofday(&start, NULL);

    x = read(fd, &a, sizeof(a));

    while (x != 0)
    {
        x = read(fd, &a, sizeof(a));
    }

    gettimeofday(&end, NULL);
    double latency_byte = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) /1000000.0;

    close(fd);
    return latency_byte;
}

double single_chunk_syscall() //single chunk latency using syscalls
{
    struct timeval start, end;
    int fd = open("file.txt", O_RDONLY);
    if (fd < 0)
    {
        perror("Error Opening File");
        return -1;
    }
    char a[CHUNK_SIZE];
    gettimeofday(&start, NULL);

    read(fd, a, sizeof(a));

    gettimeofday(&end, NULL);
    double latency_chunk = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) /1000000.0;

    close(fd);
    return latency_chunk;
}

double file_per_chunk_syscall() //time for reading the whole file in single chunks (1024 Bytes) via syscalls
{
    struct timeval start, end;
    int fd = open("file.txt", O_RDONLY);
    if (fd < 0)
    {
        perror("Error Opening File");
        return -1;
    }
    char a[CHUNK_SIZE];
    int x;
    gettimeofday(&start, NULL);

    x = read(fd, a, sizeof(a));

    while (x != 0)
    {
        x = read(fd, a, sizeof(a));
    }

    gettimeofday(&end, NULL);
    double latency_chunk = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) /1000000.0;

    close(fd);
    return latency_chunk;
}

double single_byte_stdio() //single time latency using stdio
{
    struct timeval start, end;

    FILE *file = fopen("file.txt", "r");
    if (file == NULL)
    {
        perror("Error Opening File");
        return -1;
    }

    gettimeofday(&start, NULL);

    fgetc(file);

    gettimeofday(&end, NULL);

    double latency_byte = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) /1000000.0;

    fclose(file);
    return latency_byte;
}

double file_per_byte_stdio() //time for reading the whole file in single bytes via stdio
{
    struct timeval start, end;
    FILE *file = fopen("file.txt", "r");
    int ch;
    if (file == NULL)
    {
        perror("Error Opening File");
        return -1;
    }

    gettimeofday(&start, NULL);

    ch = fgetc(file);

    while (ch != EOF)
    {
        ch = fgetc(file);
    }

    gettimeofday(&end, NULL);

    double latency_total = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) /1000000.0;

    fclose(file);
    return latency_total;
}

double single_chunk_stdio() //single chunk latency using stdio
{
    struct timeval start, end;
    char a[1024];
    FILE *file = fopen("file.txt", "r");
    if (file == NULL)
    {
        perror("Error Opening File");
        return -1;
    }

    gettimeofday(&start, NULL);

    fread(a, sizeof(char), CHUNK_SIZE, file);

    gettimeofday(&end, NULL);

    double latency_chunk = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) /1000000.0;

    fclose(file);
    return latency_chunk;
}

double file_per_chunk_stdio() //function for timing for reading the whole file in single chunks via syscalls
{
    struct timeval start, end;
    char a[1024];
    FILE *file = fopen("file.txt", "r");
    if (file == NULL)
    {
        perror("Error Opening File");
        return -1;
    }
    int x;

    gettimeofday(&start, NULL);

    x = fread(a, sizeof(char), CHUNK_SIZE, file);
    while (x > 0)
    {
        x = fread(a, sizeof(char), CHUNK_SIZE, file);
    }
    gettimeofday(&end, NULL); 

    double latency_byte = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) /1000000.0;

    fclose(file);
    return latency_byte;
}

void average(char a, int size) //runs the following command for a given size (default: 10)
{
    double avg = 0;

    if (a == 'a')
    {
        for (int i = 0; i < size; i++)
        {
            avg = avg + single_byte_stdio();
        }
        printf("Average of %d runs of single byte Latency (sdtio): %f\n", size, avg/size);
    }
    else if (a == 'b')
    {
        for (int i = 0; i < size; i++)
        {
            avg = avg + file_per_byte_stdio();
        }
        printf("Average of %d runs of file in bytes time (stdio): %f\n", size, avg/size);
    }
    else if (a == 'c')
    {
        for (int i = 0; i < size; i++)
        {
            avg = avg + single_chunk_stdio();
        }
        printf("Average of %d runs of single chunk latency (stdio): %f\n", size, avg/size);
    }
    else if (a == 'd')
    {
        for (int i = 0; i < size; i++)
        {
            avg = avg + file_per_chunk_stdio();
        }
        printf("Average of %d runs of file in chunks time (stdio): %f\n", size, avg/size);
    }
    else if (a == 'e')
    {
        for (int i = 0; i < size; i++)
        {
            avg = avg + single_byte_syscall();
        }
        printf("Average of %d runs of single byte latency (syscall): %f\n", size, avg/size);
    }
    else if (a == 'f')
    {
        for (int i = 0; i < size; i++)
        {
            avg = avg + file_per_byte_syscall();
        }
        printf("Average of %d runs of file in bytes time (syscall): %f\n", size, avg/size);
    }
    else if (a == 'g')
    {
        for (int i = 0; i < size; i++)
        {
            avg = avg + single_chunk_syscall();
        }
        printf("Average of %d runs of single chunk latency (syscall): %f\n", size, avg/size);
    }
    else if (a == 'h')
    {
        for (int i = 0; i < size; i++)
        {
            avg = avg + file_per_chunk_syscall();
        }
        printf("Average of %d runs of file in chunks time (syscall): %f\n", size, avg/size);
    }
    else
    {
        perror("Input invalid method");
        return;
    }

    return;
}

void stdio(int runs) //takes in the amount of runs for the average function; also has most formating
{
    double i = 0;
    printf("Read File via stdio\n");
    printf("//////////////////////////////////////\n");
    printf("-:- Byte-by-Byte\n");
    printf("//////////////////////////////////////\n");
    i = single_byte_stdio();
    printf("Latency for single byte: %f seconds\n", i);
    average('a',runs);
    i = file_per_byte_stdio();
    printf("Time for whole file (in bytes): %f seconds\n", i);
    average('b',runs);
    printf("//////////////////////////////////////\n");
    printf("-:- in 1024 KB-Chunks\n");
    printf("//////////////////////////////////////\n");
    i = single_chunk_stdio();
    printf("Latency for single chunk: %f seconds\n", i);
    average('c',runs);
    i = file_per_chunk_stdio();
    printf("Time for whole file (in chunks): %f seconds\n", i);
    average('d',runs);
    printf("//////////////////////////////////////\n");
    return;
}

void syscalls(int runs) //takes in the amount of runs for the average function; also has most formating
{
    double i = 0;
    printf("Read File via syscalls\n");
    printf("//////////////////////////////////////\n");
    printf("-:- Byte-by-Byte\n");
    printf("//////////////////////////////////////\n");
    i = single_byte_stdio();
    printf("Latency for single byte: %f seconds\n", i);
    average('e',runs);
    i = file_per_byte_stdio();
    printf("Time for whole file (in bytes): %f seconds\n", i);
    average('f',runs);
    printf("//////////////////////////////////////\n");
    printf("-:- in 1024 KB-Chunks\n");
    printf("//////////////////////////////////////\n");
    i = single_chunk_stdio();
    printf("Latency for single chunk: %f seconds\n", i);
    average('g',runs);
    i = file_per_chunk_stdio();
    printf("Time for whole file (in chunks): %f seconds\n", i);
    average('h',runs);
    printf("//////////////////////////////////////\n");
    return;
}

int main()
{
    size();
    stdio(AVERAGE_RUNS);
    syscalls(AVERAGE_RUNS);
    exit (0);
}