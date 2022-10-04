// CMSC 341 - Fall 2021 - Project 4
//SIZE NOT INCREASING IN INSERT???????
#include "hash.h"
#include "math.h"
HashTable::HashTable(unsigned size, hash_fn hash){
    // The alternative constructor, size is an unsigned integer to specify the length of hash table, and hash is a function pointer to a hash function. 
    // The type of hash is defined in hash.h.
    // The table size must be a prime number between MINPRIME and MAXPRIME. 
    // If the user passes a size less than MINPRIME, the capacity must be set to MINPRIME. 
    // If the user passes a size larger than MAXPRIME, the capacity must be set to MAXPRIME. 
    // If the user passes a non-prime number the capacity must be set to the smallest prime number greater than user's value.
    // Moreover, the constructor creates memory for table 1 and initializes all member variables. And, it initializes m_newTable to TABLE1.

    m_hash = hash;
    m_newTable = TABLE1;
    if(isPrime(size)) {
        if(size >= MINPRIME && size <= MAXPRIME) {
            m_capacity1 = size;
        }
        if(size < MINPRIME) {
            m_capacity1 = MINPRIME;
        }
        if(size > MAXPRIME) {
            m_capacity1 = MAXPRIME;
        }
    } else {
        m_capacity1 = findNextPrime(size);
    }
    m_size1 = 0;
    m_numDeleted1 = 0;
    m_table1 = new File[m_capacity1]; 

    if(isPrime(size)) {
        if(size >= MINPRIME && size <= MAXPRIME) {
            m_capacity2 = size;
        }
        if(size < MINPRIME) {
            m_capacity2 = MINPRIME;
        }
        if(size > MAXPRIME) {
            m_capacity2 = MAXPRIME;
        }
    } else {
        m_capacity2 = findNextPrime(size);
    }
    m_size2 = 0;
    m_numDeleted2 = 0;
    m_table2 = nullptr;
    
}

HashTable::~HashTable(){
    //Destructor, deallocates the memory for both table 1 and table 2.
    if(m_table1 != nullptr) {
        delete [] m_table1;
    }

    if(m_table2 != nullptr) {
        delete [] m_table2;
    }
    
}

File HashTable::getFile(string name, unsigned int diskBlock){
    //This function looks for the File object with name and diskBlock in the hash table, if the object is found the function returns it, 
        //otherwise the function returns empty object. 
    //If there are two hash tables at the time, the function needs to look into both tables.

    if(m_size1 != 0) {
        for(unsigned int i = 0; i < m_capacity1; i++) {
            if(m_table1[i].key() == name && m_table1[i].diskBlock() == diskBlock) {
                return m_table1[i];
            }
        }
        return EMPTY;
    } else {
        return EMPTY;
    }

    if(m_size2 != 0) {
        for(unsigned int i = 0; i < m_capacity2; i++) {
            if(m_table2[i].key() == name && m_table2[i].diskBlock() == diskBlock) {
                return m_table2[i];
            }
        }
        return EMPTY;
    } else {
        return EMPTY;
    }
    
    
}

bool HashTable::insert(File file){
    //This function inserts an object into the hash table. 
    //The insertion index is determined by applying the hash function m_hash that is set in the HashTable constructor call 
        //and then reducing the output of the hash function modulo the table size. 
    //A hash function is provided in the sample driver.cpp file to be used in this project.
    
    // Hash collisions should be resolved using the quadratic probing policy. We insert into the table indicated by m_newTable.
        // index = ((Hash(key) % TableSize) + (i x i)) % TableSize. 
    //After every insertion we need to check for the proper criteria, and if it is required, we need to rehash the entire table incrementally into a new table. 
        //rehash once the load factor exceeds 50%.
        //rehash once the deleted ratio exceeds 80%.
    //The incremental transfer proceeds with 25% of the nodes at a time. 
    //Once we transferred 25% of the nodes for the first time, the second 25% will be transferred at the next operation (insertion or removal). 
    //Once all data is transferred to the new table, the old table will be removed, and its memory will be deallocated.
    
    // If the "file" object is inserted, the function returns true, otherwise it returns false. A File object can only be inserted once. 
    //The hash table does not contain duplicate objects. Moreover, the disk block value should be a valid one falling in the range [DISKMIN-DISKMAX]. 
    //Every File object is a unique object carrying the file's name and the disk block number. The file's name is the key which is used for hashing.
    int index = 0;
    unsigned int  * size = 0;
    unsigned int capacity = 0;
    File * table = nullptr;
    File * otherTable = nullptr;
  
    index = m_hash(file.key()) % tableSize(m_newTable);

    if(m_newTable == TABLE1) {
        table = m_table1;
        otherTable = m_table2;
        size = &m_size1;
        capacity = m_capacity1;
    } else {
        table = m_table2;
        otherTable = m_table1;
        size = &m_size2;
        capacity = m_capacity2;
    }
    
    //check if file has alreadybeen inserted, if getfil return asn empty file, that means that the file has NOT already been inserted
    if(getFile(file.key(), file.diskBlock()) == EMPTY) {
        //if the table spot with the index is empty, then insert the file
        if(table[index].key() == "" && table[index].diskBlock() == 0) {
            table[index] = file;
            *size = *size + 1;
        } else {
            //if its not empty, then do quadratic probing
            for(unsigned int i = 0; i < capacity; i++) {
                //if the index your probing to is empty, put the file in
                index = (index + (i*i)) % tableSize(m_newTable);
                if(table[index].key() == "" && table[index].diskBlock() == 0) {
                    table[index] = file; 
                    *size = *size + 1;
                    break;
                }
            }
        }

        if(lambda(m_newTable) > 0.5) {
            rehash(otherTable, table);
        }

        if(deletedRatio(m_newTable) > 0.8) {
            cout << "Time to rehash in deleted ratio" << endl;
            rehash(otherTable, table);
        }

        return true;
    } else {
        //file cannot be inserted twice
        return false;
    }

    return false;
}

void HashTable::rehash(File * to, File * from) {

    unsigned int size = tableSize(m_newTable); 

    //25 % of the size
    unsigned int block1 = size * .25;
    unsigned int block2 = size * .50;
    unsigned int block3 = size * .75;
    unsigned int block4 = size * 1;


    //only wanna allocate table's data once, so do it when counter is 1
    if(counter == 1) {
        //increase the table capcity to 4 times the other table's, first unallocate the table tho
        if(to == m_table2) {
            m_table2 = new File[findNextPrime(4 * m_size1)];
            to = m_table2;
            m_size2 = 0;
            m_capacity2 = findNextPrime(4 * m_size1);
            m_newTable = TABLE2;
        } else {
            m_table1 = new File[findNextPrime(4 * m_size2)];
            to = m_table1;
            m_size1 = 0;
            m_capacity1 = findNextPrime(4 * m_size2);
            m_newTable = TABLE2;
        }
        //if from isnt delted or deleted, insert it using from
        for(unsigned int i = 0; i < block1; i++) {
            if(!(from[i] == EMPTY) && !(from[i] == DELETED)) {
                File temp = from[i];
                from[i] = EMPTY;
                insert(temp);
            } 
        }
        counter++;
        return;
    }

    if(counter == 2) {
        for(unsigned int i = block1; i < block2; i++) {
            if(!(from[i] == EMPTY) && !(from[i] == DELETED)) {
                File temp = from[i];
                from[i] = EMPTY;
                insert(temp);
            } 
        }
        counter++;
        return;
    }

    if(counter == 3) {
        for(unsigned int i = block2; i < block3; i++) {
            if(!(from[i] == EMPTY) && !(from[i] == DELETED)) {
                File temp = from[i];
                from[i] = EMPTY;
                insert(temp);
            } 
        }
        counter++;
        return;
    }
   
   if(counter == 4) {
        for(unsigned int i = block3; i < block4; i++) {
            if(!(from[i] == EMPTY) && !(from[i] == DELETED)) {
                File temp = from[i];
                from[i] = EMPTY;
                insert(temp);
            }  
        }
        counter++;
        return;
    }

    if(counter >= 5) {
        return;
    }

}

bool HashTable::remove(File file){
    //This function removes a data point from the hash table. In a hash table we do not empty the bucket, we only tag it as deleted. 
    //To tag a removed bucket we assign the DELETED object to the bucket. The DELETED object is defined in hash.h. 
    //To find the bucket of the object we should use the quadratic probing policy.

    // After every deletion we need to check for the proper criteria, and if it is required, we need to rehash the entire table incrementally into a new table. 
    //The incremental transfer proceeds with 25% of the nodes at a time. 
    //Once we transferred 25% of the nodes for the first time, the second 25% will be transferred at the next operation (insertion or removal). 
    //Once all data is transferred to the new table, the old table will be removed, and its memory will be deallocated.

    // If the "file" object is found and is deleted, the function returns true, otherwise it returns false.

    //Treat deleted element as empty when inserting.
    //Treat deleted element as occupied when searching.

    //if the file in found in the table then delete it.

    int index = 0;
    unsigned int * deleted = 0;
    index = m_hash(file.key()) % tableSize(m_newTable);

    File * table = nullptr;
    File * otherTable = nullptr;

    if(m_newTable == TABLE1) {
        table = m_table1;
        otherTable = m_table2;
        deleted = &m_numDeleted1;
    } else {
        table = m_table2;
        otherTable = m_table1;
        deleted = &m_numDeleted2;
    }
    

    for(unsigned int i = 0; i < m_capacity1; i++) {
        index = (index + (i*i)) % tableSize(m_newTable);
        if(table[index].key() == file.key() && table[index].diskBlock() == file.diskBlock()) {
            table[index] = DELETED; 
            table[index].key() = DELETEDKEY; 
            *deleted = *deleted + 1;
            break;
        }
    }


    if(lambda(m_newTable) > 0.5) {
        //newTable would be the table (table 1), and other table would be table 2
        rehash(otherTable, table);
    }

    if(deletedRatio(m_newTable) > 0.8) {
        rehash(otherTable, table);
    }

    return true;

}

float HashTable::lambda(TABLENAME tablename) const {
    //This function returns the load factor of the hash table. The load factor is the ratio of occupied buckets to the table capacity.
        // The load factor is the number of occupied buckets divided by the table size. 
        //The number of occupied buckets is the total of available data and deleted data. 
    //The number of occupied buckets is the total of available buckets and deleted buckets. 
    //The parameter tablename specifies the table that should be used for the calculation.

    float loadFactor = 0.0;
    //occupied buckets / table size
    //m_size1 is current number of entries in TABLE1, includes deleted entries 
    if(tableSize(tablename) == m_capacity1) {
        loadFactor = static_cast< float >(m_size1) / static_cast< float >(m_capacity1);
    } else {
        loadFactor = static_cast< float >(m_size2) / static_cast< float >(m_capacity2);
    }

    return loadFactor;
}

float HashTable::deletedRatio(TABLENAME tableName) const {
    //This function returns the ratio of the deleted buckets to the total number of occupied buckets . 
        //The deleted ratio is the number of deleted buckets divided by the number of occupied buckets.
    //The parameter tablename specifies the table that should be used for the calculation.
  
    float deletedRatio = 0.0;
    
    if(tableSize(tableName) == m_capacity1) {
        if(m_size1 != 0) {
            deletedRatio = static_cast< float >(m_numDeleted1) / static_cast< float >(m_size1);
        } else {
            deletedRatio = 0.0;
        }
    } else {
        if(m_size2 != 0) {
            deletedRatio = static_cast< float >(m_numDeleted2) / static_cast< float >(m_size2);
        }  else {
            deletedRatio = 0.0;
        }
    }

    return deletedRatio;
}

void HashTable::dump() const {
    cout << "Dump for table 1: " << endl;
    if (m_table1 != nullptr)
        for (unsigned int i = 0; i < m_capacity1; i++) {
            cout << "[" << i << "] : " << m_table1[i] << endl;
        }
    cout << "Dump for table 2: " << endl;
    if (m_table2 != nullptr)
        for (unsigned int i = 0; i < m_capacity2; i++) {
            cout << "[" << i << "] : " << m_table2[i] << endl;
        }
}

bool HashTable::isPrime(int number){
    bool result = true;
    for (int i = 2; i <= number / 2; ++i) {
        if (number % i == 0) {
            result = false;
            break;
        }
    }
    return result;
}

int HashTable::findNextPrime(int current){
    //we always stay within the range [MINPRIME-MAXPRIME]
    //the smallest prime starts at MINPRIME
    if (current < MINPRIME) current = MINPRIME-1;
    for (int i=current; i<MAXPRIME; i++) { 
        for (int j=2; j*j<=i; j++) {
            if (i % j == 0) 
                break;
            else if (j+1 > sqrt(i) && i != current) {
                return i;
            }
        }
    }
    //if a user tries to go over MAXPRIME
    return MAXPRIME;
}