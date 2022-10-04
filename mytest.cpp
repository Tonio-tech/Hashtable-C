// CMSC 341 - Fall 2021 - Project 4
#include "hash.h"
#include <iostream>
#include <random>
#include <vector>
using namespace std;
enum RANDOM {UNIFORM, NORMAL};
class Random {
public:
    Random(int min, int max, RANDOM type=UNIFORM) : m_min(min), m_max(max), m_type(type)
    {
        if (type == NORMAL){
            m_generator = std::mt19937(m_device());
            //the data set will have the mean of 50 and standard deviation of 20 
            m_normdist = std::normal_distribution<>(50,20);
        }
        else{
            // Using a fixed seed value generates always the same sequence
            // of pseudorandom numbers, e.g. reproducing scientific experiments
            // here it helps us with testing since the same sequence repeats
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_unidist = std::uniform_int_distribution<>(min,max);
        }
    }

    int getRandNum(){
        int result = 0;
        if(m_type == NORMAL){
            //returns a random number in a set with normal distribution
            //we limit random numbers by the min and max values
            result = m_min - 1;
            while(result < m_min || result > m_max)
                result = m_normdist(m_generator);
        }
        else{
            //this will generate a random number between min and max values
            result = m_unidist(m_generator);
        }
        return result;
    }
    
    private:
    int m_min;
    int m_max;
    RANDOM m_type;
    std::random_device m_device;
    std::mt19937 m_generator;
    std::normal_distribution<> m_normdist;//normal distribution
    std::uniform_int_distribution<> m_unidist;//uniform distribution

};

// The hash function used by HashTable class
unsigned int hashCode(const string str);

class Tester{ // Tester class to implement test functions
   public:
   Tester(){}
   ~Tester(){}
   void testInsertion();
   void testGetFile();
   void testRemove();
   void testRehash();
   private:
};

int main(){
   // This program presents a sample use of the class HashTable 
   // It does not represent any rehashing
   Random diskBlockGen(DISKMIN,DISKMAX);
   int tempDiskBlocks[50] = {0};
   HashTable aTable(MINPRIME,hashCode);
   int temp = 0;
   int secondIndex = 0;
   for (int i=0;i<50;i++){
      temp = diskBlockGen.getRandNum();
      if (i%3 == 0){//this saves 17 numbers from the index range [0-49]
         tempDiskBlocks[secondIndex] = temp;
         cout << temp << " was saved for later use." << endl;
         secondIndex++;
      }
      cout << "Insertion # " << i << " => " << temp << endl;
      if (i%3 != 0)
         aTable.insert(File("test.txt", temp));
      else
         // these will be deleted
         aTable.insert(File("driver.cpp", temp));
   }

   cout << "Message: dump after 50 insertions in a table with MINPRIME (101) buckets:" << endl;
   aTable.dump();

   for (int i = 0;i<14;i++)
         aTable.remove(File("driver.cpp", tempDiskBlocks[i]));
   cout << "Message: dump after removing 14 buckets," << endl;
   aTable.dump();

   {
        cout << endl << endl << endl << "BEGINNING TESTS:::" << endl;
        Tester tester1;
        tester1.testInsertion();
        tester1.testGetFile();
        tester1.testRemove();
        tester1.testRehash();
   }
   
   return 0;
}

unsigned int hashCode(const string str) {
   unsigned int val = 0 ;
   const unsigned int thirtyThree = 33 ;  // magic number from textbook
   for ( unsigned int i = 0 ; i < str.length(); i++)
      val = val * thirtyThree + str[i] ;
   return val ;
}


void Tester::testInsertion() {
    // Test the insertion operation in the hash table. The following presents a sample algorithm to test the normal insertion operation:
        // There is some non-colliding data points in the hash table.
        // Insert multiple non-colliding keys.
        // Check whether they are inserted in the correct bucket (correct index).
        // Check whether the data size changes correctly.

    cout << endl << "TESTING INSERTION:::" << endl;
    Random diskBlockGen(DISKMIN,DISKMAX);
    HashTable aTable(MINPRIME,hashCode);
    int temp = 0;

    cout << "m_size before:  table 1: " << aTable.numEntries(TABLE1) << " table 2: " << aTable.numEntries(TABLE2) << endl;

    for (int i=0;i<10;i++){
      temp = diskBlockGen.getRandNum();
      aTable.insert(File("mytest.cpp", temp));
   }

    //when i insert into table1, it rehases, but table 2 is still empty
    cout << "m_size after:  table 1: " << aTable.numEntries(TABLE1) << " table 2: " << aTable.numEntries(TABLE2) << endl;

    aTable.dump();
}


void Tester::testGetFile() {
    // Test the find operation (getFile() function) with a few non-colliding keys. This also tests whether the insertion works correctly.
    // Test the find operation (getFile() function) with a number of colliding keys without triggering a rehash. This also tests whether the insertion works correctly.
    cout << endl << endl << "TESTING GETFILE" << endl;
    HashTable bTable(MINPRIME,hashCode);

    //non colliding keys
    File file1("mytest.cpp", 321435);
    File file2("mytest.cpp", 598765);
    File file3("mytest.cpp", 634566);
    File file4("mytest.cpp", 968563);
    File file5("mytest.cpp", 111234);

    bTable.insert(file1);
    bTable.insert(file2);
    bTable.insert(file3);
    bTable.insert(file4);
    bTable.insert(file5);

    if(!(bTable.getFile(file1.key(), file1.diskBlock()) == EMPTY)) {
        cout << "file 1 has been found" << endl;
    }

    if(!(bTable.getFile(file2.key(), file2.diskBlock()) == EMPTY)) {
        cout << "file 2 has been found" << endl;
    }

    if(!(bTable.getFile(file3.key(), file3.diskBlock()) == EMPTY)) {
        cout << "file 3 has been found" << endl;
    }

    if(!(bTable.getFile(file4.key(), file4.diskBlock()) == EMPTY)) {
        cout << "file 4 has been found" << endl;
    }

    if(!(bTable.getFile(file5.key(), file5.diskBlock()) == EMPTY)) {
        cout << "file 5 has been found" << endl;
    }

    //colliding keys
    File file6("mytest.cpp", 554687);
    File file7("mytest.cpp", 342084);
    File file8("mytest.cpp", 129487);

    bTable.insert(file6);
    bTable.insert(file7);
    bTable.insert(file8);

    if(!(bTable.getFile(file6.key(), file6.diskBlock()) == EMPTY)) {
        cout << "file 6 has been found" << endl;
    }

    if(!(bTable.getFile(file7.key(), file7.diskBlock()) == EMPTY)) {
        cout << "file 7 has been found" << endl;
    }

    if(!(bTable.getFile(file8.key(), file8.diskBlock()) == EMPTY)) {
        cout << "file 8 has been found" << endl;
    }
}


void Tester::testRemove() {
    // Test the remove operation with a few non-colliding keys.
    // Test the remove operation with a number of colliding keys without triggering a rehash.
    cout << endl << endl << "TESTING REMOVE" << endl;
    HashTable cTable(MINPRIME,hashCode);

    File file1("mytest.cpp", 783921);
    File file2("mytest.cpp", 127382);
    File file3("mytest.cpp", 400019);
    File file4("mytest.cpp", 634271);
    File file5("mytest.cpp", 324555);
    File file6("mytest.cpp", 836329);
    File file7("mytest.cpp", 293873);
    File file8("mytest.cpp", 197353);
    File file9("mytest.cpp", 100233);
    File file10("mytest.cpp", 200321);

    cTable.insert(file1);
    cTable.insert(file2);
    cTable.insert(file3);
    cTable.insert(file4);
    cTable.insert(file5);
    cTable.insert(file6);
    cTable.insert(file7);
    cTable.insert(file8);
    cTable.insert(file9);
    cTable.insert(file10);

    cout << "BEFORE REMOVE" << endl;
    cTable.dump();

    cTable.remove(file8);
    cTable.remove(file2);
    cTable.remove(file6);
    cTable.remove(file3);
    cTable.remove(file1);

    cout << "AFTER REMOVE" << endl;
    cTable.dump();

}


void Tester::testRehash() {
    // Test the rehashing is triggered after a descent number of colliding keys insertion.
    // Test the rehashing is triggered after a descent number of colliding keys removal.

    cout << endl << endl << "TESTING REHASH" << endl;
    HashTable cTable(MINPRIME,hashCode);
    Random diskBlockGen(DISKMIN,DISKMAX);
    int tempDiskBlocks[50] = {0};

    int temp = 0;

    cout << "testing rehash insertion" << endl;
    for (int i = 0; i < 51; i++){
      temp = diskBlockGen.getRandNum();
      cTable.insert(File("mytest.cpp", temp));
   }

    cTable.dump();

    cout << "testing rehash removal" << endl;
    for (int i = 0; i < 42; i++) {
        cTable.remove(File("mytest.cpp", tempDiskBlocks[i]));
    }

    cTable.dump();


}
