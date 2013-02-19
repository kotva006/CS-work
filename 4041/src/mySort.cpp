

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>

using namespace std;

int main (int argc, char **argv) {

  string line;

  const char *flag = argv[1];
  if (argc != 3) {

      printf("MySort: Sorts an input file\n");
      printf("\nUsage: MySort <flag> <file_name>\n");
      printf("\nFlags:\nh - Heap Sort is used.\n");
      printf("i - Insertion Sort is used.\nm - Merge Sort is used.\n");
      printf("q - Quick Sort is used.\n");
      return 0;

  }
  ifstream inFile (argv[2]);
  if (inFile.fail()) {
      printf("File not Found\n");
      return 0;
  }
  int success = 0;
  int length = 1;
  int *unsorted = (int *) malloc( sizeof(int) * 100500);
  while(getline(inFile, line)) {
       unsorted[length] = atoi(line.c_str());
       length++;
  }


/* This is the implementation of the Insertion sort */

  if (flag[0] == 'i') {
      printf("Insertion Sort\n");
      int j = 1;
      for (j=1; j<length; j++) {

          int temp = unsorted[j];
          int i = j - 1;
          while (i>0 && unsorted[i] > temp) {
              unsorted[i+1] = unsorted[i];
              i = i - 1;
          }
          unsorted[i+1] = temp;
      }
      success = 1;
  }
  else if (flag[0] == 'm') {
      printf("Merge Sort");
      success = 1;
  }
  
  else if (flag[0] == 'h') {
      printf("Heap Sort");
      success = 1;
  }

  else if (flag[0] == 'q') {
      printf("Quick Sort");
      success = 1;
  }


  if (success == 1) {
      ofstream outFile;
      int i = 1;
      outFile.open("output.txt", ios::trunc);
      while(i < length) {
          outFile << unsorted[i];
          outFile << "\n";
          i++;
      }
      outFile.close();
      return 0;
  }

  cout << "Invalid sort method" << endl << endl;
  cout << "The supported Methods are:" << endl;
  cout << "Insertion i" << endl << "Merge m" << endl;
  cout << "Heap h" << endl << "Quick q" << endl;

  inFile.close();
  return 0;

}
