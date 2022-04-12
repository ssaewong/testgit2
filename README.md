# longest-compound
This solution is for the longest compound word quiz in https://gist.github.com/bobbae/b2433df38fe1da05c1807e4ca99af237. 

It is written in C program language compiled by GCC in Linux environment. 


## How to run the program
At Linux terminal, run the script as follows. 

***./run.sh***

This will check for the file "words.txt". If the file does not exist, the script will download the file from https://gist.github.com/bobbae/4ca309a1857158d5766d4ede4235cae0. 

To test the program with different input text file, run the script as follows. 

***./run.sh filename***
 
## Algorithm Explanation

At first, all strings in the file will be copied to the linked list (container) and reordered in the increasing order of the string length. Processing the string in this order will optimize the searching algorithm. It will ensure that all dictionry words, whose length are less than the processing string's length, will always be included in the dictionary before the look up. 

The program use trie data structure for the dictionary, which will only contain strings that are not uncompund words.

For each string in the container, it will look up the dictionary for the matched word in the dictionary. 
If the matched word is found, it will recursively look up the dictionary for the rest of the string. 
Once all alphabets of the string are processed, it can determine if the string is the compound word. 
If it is not the compound word, the string will be added to the dictionary. 
If it is, the number of compound words will be increased and the end result will be updated if the string has the longest length. 

The function for lookup the dictionary recursively is function **search_dictionary_trie(wordlink_s *wordlink, unsigned int offset, unsigned word_count)**. 

During the search, we maintain only one copy of the string in the container. All fragmented string are analyzed from the offset. 
This will optimize time for unneccessary memory copies and optimize memory usage. 

Note that, the algorithm currently does not use any thread. However, from the design, we can create multiple working threads to invoke  **search_dictionary_trie()** in parallel. 

## Algorithm Limitation 
The algrithm assumes the following. 
  - The longest string length in the file is MAX_STRING_LENGTH (128) 
  - The longest dictionary word is MAX_WORD_LENGTH (64). 
  - The number of results with the longest length is MAX_RESULTS (128)
If the overlimit is encountered, the program will prompt the user to modify the limit accordingly. 

## Example in details
For example, assume that the dictionary contains the following words
  > *do
  > cat
  > dog
  > dogso*

To check the string **dogsocat**, 
The first round of "search_dictionary_trie" will get **do**, **dog**, **dogso**. 

The second round for **do** will search the rest string, which is **gsocat** and it will be terminated because it cannot find more matched word. 

The second round for **dog** will search the rest string, which is **socat** and it will be terminated because it connot find more matched word. 

The second round for **dogso** will search the rest string, which is **cat** and it will get **cat**. 


## Testing the programs
Some unit test for each function has been performed, including the function **test_inspect_container()** and **inspect_dictionary()**. Some small sample files, included in the package, are used to validate the algorithm. 


