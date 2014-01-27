#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sys/time.h>

using namespace std;


long long operationCounter;
//Heap sort algorithm
void maxHeapify(int *inArray, int index, int heapSize) {
  int left = 2*index;
  int right = (2*index) + 1;
  int largest = 0;
  operationCounter++;
  if (left <= heapSize && inArray[left] > inArray[index]) {
    largest = left;}
  else {
    largest = index;}
  operationCounter++;
  if (right <= heapSize and inArray[right] > inArray[largest]) {
    largest = right;}
  if (largest != index) {
    int temp = inArray[largest];
    inArray[largest] = inArray[index];
    inArray[index] = temp;
    maxHeapify(inArray, largest, heapSize);
  }
}

void heapSort(int *inArray, int length){
  int heapSize = length;
  int i = 0;
  for (i = length/2; i >=1; i--) {
    maxHeapify(inArray, i, heapSize);
  }
  for (i = length; i >= 2; i--) {
     int temp = inArray[1];
     inArray[1] = inArray[i];
     inArray[i] = temp;
     heapSize--;
     maxHeapify(inArray, 1, heapSize);
  }
}
//end headp sort    

//merge sort
void mergeSort(int *inArray, int lowEnd, int highEnd) {
    if (lowEnd < highEnd) {
        int temp = (lowEnd + highEnd)/2;
        mergeSort(inArray, lowEnd, temp);
        mergeSort(inArray, temp+1, highEnd);
        int temp2 = temp - lowEnd + 1;
        int temp3 = highEnd - temp;
        int i = 0;
        int left[temp2+1];
        int right[temp3+1];
        for (i = 1; i < temp2+1; i++) {
            left[i] = inArray[lowEnd+i-1];
        }
        for (i=1; i<temp3+1; i++) {
            right[i] = inArray[temp+i];
        }
        left[temp2+1] = 2147480000;
        right[temp3+1] = 2147480000;
        i = 1;
        int j = 1;
        int count = 0;
        for (count = lowEnd; count < highEnd +1; count++) {
            operationCounter++;
            if (left[j] < right[i]) {
                inArray[count] = left[j];
                j++;
            }else{
                inArray[count] = right[i];
                i++;
            }
        }
     }
}

void quickSort(int *inArray, int lowEnd, int highEnd) {
    if (lowEnd < highEnd) {
        int temp = inArray[highEnd];
        int i = lowEnd -1;
        int j = 0;
        for (j = lowEnd; j < highEnd; j++) {
          operationCounter++;
          if (inArray[j] <= temp) {
             i++;
             swap(inArray[i], inArray[j]);
          }
        }
        swap(inArray[i+1], inArray[highEnd]);
        quickSort(inArray, lowEnd, i);
        quickSort(inArray, i+2, highEnd);
    }
}
        

int main (int argc, char **argv) {

  operationCounter = 0;
  struct timeval timeBegin;
  struct timeval timeEnd;


  string line;
  //Error checking
  if (argc != 3) {
      printf("MySort: Sorts an input file\n");
      printf("\nUsage: MySort <flag> <file_name>\n");
      printf("\nFlags:\nh - Heap Sort is used.\n");
      printf("i - Insertion Sort is used.\nm - Merge Sort is used.\n");
      printf("q - Quick Sort is used.\n");
      return 0;
  }

  ifstream inFile (argv[2]);
  if (inFile.fail()) {printf("File not Found\n"); return 0;}
  //Prepping the inputs for an output file  
  const char *flag = argv[1];
  string flagString(flag, 1);
  const char *file = argv[2];
  int counter = 0;
  while(file[0] != '\0'){
      counter++;
      file++;
  }
  string fileString(argv[2], counter);

  int success = 0;
  int length = 1;
  int *unsorted = (int *) malloc( sizeof(int) * 100500);
  while(getline(inFile, line)) {
       unsorted[length] = atoi(line.c_str());
       length++;
  }


/* This is the implementation of the Insertion sort */

  if (flag[0] == 'i') {
      //printf("Insertion Sort\n");
      //insertion
      gettimeofday(&timeBegin, 0);
      int j = 1;
      for (j=1; j<length; j++) {

          int temp = unsorted[j];
          int i = j - 1;
          operationCounter++;
          while (i>0 && unsorted[i] > temp) {
              operationCounter++;
              unsorted[i+1] = unsorted[i];
              i = i - 1;
          }
          unsorted[i+1] = temp;
      }
      success = 1;
      gettimeofday(&timeEnd, 0);
  }
  else if (flag[0] == 'm') {
      //printf("Merge Sort\n");
      gettimeofday(&timeBegin, 0);
      mergeSort(unsorted, 1, length-1);
      gettimeofday(&timeEnd, 0);
      success = 1;
  }
  
  else if (flag[0] == 'h') {
      //printf("Heap Sort\n");
      gettimeofday(&timeBegin, 0);
      heapSort(unsorted, length-1);
      gettimeofday(&timeEnd, 0);
      success = 1;
  }

  else if (flag[0] == 'q') {
      //printf("Quick Sort\n");
      gettimeofday(&timeBegin, 0);
      quickSort(unsorted, 1, length-1);
      gettimeofday(&timeEnd, 0);
      success = 1;
  }

//Outputs the sorted data into a file
//Also the statistics
  if (success == 1) {
      ofstream outFile;
      ofstream dataFile;
      string outPut = flagString + "_" + fileString + ".sorted";
      string dataName = flagString + "_" + fileString + ".stats";
      outFile.open(outPut.c_str(), ios::trunc);
      dataFile.open(dataName.c_str(), ios::trunc);
      int i = 1;
      while(i < length) {
          outFile << unsorted[i];
          outFile << "\n";
          i++;
      }
      dataFile << operationCounter << "\n";
      dataFile << (((timeEnd.tv_sec - timeBegin.tv_sec) * 1000) +  ((timeEnd.tv_usec - timeBegin.tv_usec ) / 1000))<< " milliseconds\n";
      outFile.close();
      dataFile.close();
      return 0;
  }

  cout << "Invalid sort method" << endl << endl;
  cout << "The supported Methods are:" << endl;
  cout << "Insertion i" << endl << "Merge m" << endl;
  cout << "Heap h" << endl << "Quick q" << endl;

  inFile.close();
  return 0;
}
