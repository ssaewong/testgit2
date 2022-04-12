#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

/* This is for case insensitive only */ 

#define MAX_WORD_LENGTH      64
#define MAX_STRING_LENGTH    128 
#define MAX_RESULTS          128 

#define DEFAULT_INPUT "words.txt"

char filename_input[256]; 
unsigned int maxstring_len;    // the length of the longest compound word 
unsigned int maxstring_count;  // the number of words that are longest 
char         maxstring[MAX_RESULTS][MAX_STRING_LENGTH]; // array  of the results
unsigned int compound_word_count; // the number of compound words in the list

typedef struct wordlink_ss {
    char                *word;                    // pointer to array to store the word after adjustment
    char                *word_orig;               // pointer to array to store the original word 
    bool                processed;                // flag indicating if the word has been processed
    bool                compound;                 // flag indicating if the word is the compound word. 
    struct wordlink_ss  *next_word;               // pointer to next word
} wordlink_s;

/* container to store words with the given number of alphabets */ 
typedef struct container_ss {
    unsigned int        num_alphabets;              // number of alphabets 
    wordlink_s          *wordlink[26];              // the link list for each character (a-z)
    wordlink_s          *end_wordlink[26];          // the link list for each character (a-z)
    struct container_ss *next_container;            // the pointer to next containter w/ different number of alphabets
} container_s; 

container_s *container_head = NULL; 

typedef struct dictionary_tries_ss {
    struct dictionary_tries_ss    *alpha[26];    // array to contain the tries link for all alphabets
    bool                          is_leaf;          // flag to indicate if the node is the leaf node
} dictries_s; 

dictries_s dictionary_head; 

/* Get the new dictionary trie node */ 
dictries_s *get_dictionary_trie(dictries_s *src, char alphabet) {
    dictries_s          *new_trie; 
    unsigned int        index; 

    if (src == NULL) {
        printf("Error! get_dictionary_trie, src should not be NULL \n"); 
        exit(1); 
    }
    if (src->alpha[alphabet - 'a'] == NULL) {
        new_trie = (dictries_s *) malloc(sizeof(dictries_s)); 
        for (index = 0; index < 26; index++) {
            new_trie->alpha[index] = NULL; 
        }
        new_trie->is_leaf = false; 
        /* link the new one */ 
        src->alpha[alphabet - 'a'] = new_trie; 
        return new_trie; 
    }
    return src->alpha[alphabet - 'a']; 
}

/* Add the new dictionary trie node */ 
void add_dictionary(char *string) {
    dictries_s      *trace; 
    unsigned int    index; 

    trace = &dictionary_head; 
    for (index = 0; index < strlen(string); index++) {
        trace = get_dictionary_trie(trace, string[index]); 
    }
    trace->is_leaf = true; 
}

/* Inspect the dictionary trie*/ 
void inspect_dictionary_trie(dictries_s *trace, char *str, unsigned int depth) {
    unsigned int    index; 
    char            my_str[MAX_STRING_LENGTH + 1]; 

    strncpy(my_str, str, MAX_STRING_LENGTH); 
    if (trace->is_leaf == true) {
        /* this is the dictionary word */ 
        printf("found %s \n", my_str); 
    }
    for (index = 0; index < 26; index++) {
        if (trace->alpha[index] != NULL) {
            my_str[depth] = index + 'a'; 
            // printf("depth %d , str %s , inspect %c \n", depth, my_str, my_str[depth]); 
            inspect_dictionary_trie(trace->alpha[index], my_str, (depth + 1));  
        }
    }
}

void inspect_dictionary(void) {
    char        inspect_string[MAX_STRING_LENGTH]; 
    dictries_s  *dictionary; 

    dictionary = &dictionary_head; 
    printf("inspect dictionary \n"); 
    memset(inspect_string, 0, MAX_STRING_LENGTH); 
    inspect_dictionary_trie(dictionary, inspect_string, 0); 
}

/* Search the dictionary */ 
void search_dictionary_trie(wordlink_s *wordlink, unsigned int offset, unsigned int word_count) {
    dictries_s    *trace; 
    unsigned int  trace_index; 
    unsigned int  pos; 
    char          word_copy[MAX_STRING_LENGTH]; 
    unsigned int  tmp_word_count = 0; 

    // printf("**** search_dictionary_trie %s , offset %d word_count %d\n", wordlink->word, offset, word_count); 
    trace = &dictionary_head; 
    trace_index = offset; 
    while (trace != NULL) {
        strcpy(word_copy, wordlink->word); 
        word_copy[trace_index + 1] = 0; 
        // printf("index %d %s \n", trace_index, &word_copy[offset]); 
        if (trace->is_leaf == true) {
            /* found the matching dictionary word */ 
            tmp_word_count++;  
            strcpy(word_copy, wordlink->word); 
            word_copy[trace_index] = 0; 
            // printf("### found index %d trace_index %d %s , num_words = %d \n", offset, trace_index, word_copy, (word_count + 1)); 
            /* recusive search next */ 
            if (trace_index < strlen(wordlink->word)) {
                search_dictionary_trie(wordlink, trace_index, (word_count + 1)); 
            } 
        }
        if (trace_index >= strlen(wordlink->word)) {
            if (((word_count + tmp_word_count) > 1) && (trace->is_leaf == true)) {
                /* this is the compound word */
                wordlink->compound = true;  
                // printf("^^^ compound word %s, num_words = %d \n", wordlink->word, (word_count + tmp_word_count)); 
            } else {
                wordlink->compound = false; 
            }
            return;
        }

        /* continue to search */ 
        pos = wordlink->word[trace_index] - 'a'; 
        if (trace->alpha[pos] != NULL) {
            trace = trace->alpha[pos]; 
            trace_index++; 
        } else {
            return; 
        }
    }
}



/* The function will change string to case-insensitive and copy to *des. 
 * All whitespace will be removed for both *src and *des.
 * The function returns the final length of the string after removing the whitespaces.
 */
int process_string(char *src, char *des, int length) {
    unsigned int        index; 

    /* convert to all small alphabets */ 
    for (index = 0; index < length; index++) {
        des[index] = tolower(src[index]);
        /* remove white space */ 
        if (isspace(des[index]) != 0) {
            des[index] = 0; 
            src[index] = 0; 
            return index; 
        }
        if (isalpha(des[index]) == 0) {
            printf("We support only alphabets , not digits! \n"); 
            printf("Error string : %s  \n", src); 
            exit(1); 
        }
    }
    des[index] = 0; 
    return index; 
}

container_s *get_size_container(int size) {
    container_s         *trace, *trace_prev;
    container_s         *new_container;  
    unsigned int        index; 

    trace = container_head; 
    trace_prev = NULL; 
    while (trace != NULL) {
        if (trace->num_alphabets == size) {
            /* found the container */ 
            return trace; 
        }
        if (trace->num_alphabets > size) {
            /* create the new container for this */ 
            new_container = (container_s *) malloc(sizeof(container_s)); 
            new_container->num_alphabets = size; 
            for (index = 0; index < 26; index++) {
                new_container->wordlink[index] = NULL; 
                new_container->end_wordlink[index] = NULL; 
            }
            /* link it to the container list */ 
            if (trace_prev != NULL) {
                trace_prev->next_container = new_container; 
            }
            /* update the container head accordingly */ 
            if (trace == container_head) {
                container_head = new_container; 
            }
            new_container->next_container = trace; 
            return new_container; 

        }
        trace_prev = trace; 
        trace = trace->next_container; 
    }

    new_container = (container_s *) malloc(sizeof(container_s)); 
    new_container->num_alphabets = size; 
    for (index = 0; index < 26; index++) {
        new_container->wordlink[index] = NULL; 
        new_container->end_wordlink[index] = NULL; 
    }
    new_container->next_container = NULL; 
    if (container_head == NULL) {
        container_head = new_container; 
    }
    if (trace_prev != NULL) {
        trace_prev->next_container = new_container; 
    }
    return new_container;  
}

void test_inspect_container(void) {
    container_s  *trace; 
    wordlink_s   *trace_word; 
    int          count; 
    int          index; 
    
    printf("test_inspect_container \n"); 
    trace = container_head; 
    while (trace != NULL) {
        printf("container %p, size %d \n", trace, trace->num_alphabets); 
        for (index = 0; index < 26; index++) {
            trace_word = trace->wordlink[index]; 
            count = 0; 
            while (trace_word != NULL) {
                printf("%s %s \n", trace_word->word, trace_word->word_orig); 
                trace_word = trace_word->next_word; 
                count++; 
            }
            if (count != 0) {
                printf("Alphabet %c : total %d words\n", ('a' + index), count); 
            }
        }
        trace = trace->next_container; 
    }
}

void construct_container(char *file_input) {
    ssize_t     read;
    size_t      len; 
    char        *line, *process_line; 
    int         process_len; 
    FILE        *fp; 
    container_s *container;  
    wordlink_s  *wordlink, *word_trace;  
    int         pos; 


    if ((fp = fopen(file_input, "r")) == NULL) {
        printf("Error opening file %s \n", file_input); 
        exit(1); 
    }

    line = (char *) malloc(MAX_STRING_LENGTH); 
    process_line = (char *) malloc(MAX_STRING_LENGTH); 
    len = MAX_STRING_LENGTH; 

    while ((read = getline(&line, &len, fp)) != -1) {
        process_len = process_string(line, process_line, read); 
        container = get_size_container(process_len);  
        /* allocate stroage for this string */ 
        wordlink = (wordlink_s *) malloc(sizeof(wordlink_s)); 

        if (process_len >= MAX_STRING_LENGTH) {
            printf("Error! the string in the file is oversized, please consider to readjust MAX_STRING_LENGTH accordingly \n"); 
            printf("#define MAX_STRING_LENGTH %d \n", MAX_STRING_LENGTH); 
            exit(1); 
        }

        /* put the string in wordlink and initialize its structure */ 
        wordlink->word = process_line; 
        wordlink->word_orig = line; 
        wordlink->processed = false; 
        wordlink->compound = false; 
        wordlink->next_word = NULL; 

        pos = wordlink->word[0] - 'a'; 
        word_trace = container->wordlink[pos]; 
        if (word_trace != NULL) {
            container->end_wordlink[pos]->next_word = wordlink; 
            container->end_wordlink[pos] = wordlink; 
        } else {
            container->wordlink[pos] = wordlink; 
            container->end_wordlink[pos] = wordlink; 
        }

        /* done , process to the next line , prepare new memory */ 
        line = (char *) malloc(MAX_STRING_LENGTH); 
        process_line = (char *) malloc(MAX_STRING_LENGTH); 
        len = MAX_STRING_LENGTH; 
    }

    printf("Complete the container construction \n"); 
    fclose(fp); 
}

int main(int argc, char *argv[]) {
    unsigned int index; 
    container_s  *trace; 
    wordlink_s   *trace_word; 

    /* read the input file name and construct the word container in order of the number of alphabets in the word */ 
    if (argc >= 2) {
		strcpy(filename_input, argv[1]); 
	} else {
		strcpy(filename_input, DEFAULT_INPUT); 		
	}
    printf("Open file %s successfully\n", filename_input); 

    /* Construct containers */ 
    construct_container(filename_input); 
#if DBG 
    test_inspect_container(); 
#endif 

    /* Initialize dictionary */
    for (index = 0; index < 26; index++) {
        dictionary_head.alpha[index] = NULL; 
        dictionary_head.is_leaf = false; 
    }

    /* Initialize the result */ 
    maxstring_len = 0; 
    maxstring_count = 0; 
    for (index = 0; index < MAX_RESULTS; index++) {
        memset(maxstring[index], 0, MAX_STRING_LENGTH); 
    }
    compound_word_count = 0; 

    /* Start processing */ 
    trace = container_head; 
    while (trace != NULL) {
        // printf(" trace %p num_alphabets = %d \n", trace, trace->num_alphabets);  
        for (index = 0; index < 26; index++) {
            trace_word = trace->wordlink[index]; 
            while (trace_word != NULL) {
                /* Initialize result */ 
                trace_word->processed = true; 
                search_dictionary_trie(trace_word, 0, 0); 
                if (trace_word->compound == false) {
                    /* add the word to the dictionary */ 
                    // printf(">>>>> add to dictionary %s \n", trace_word->word); 
                    add_dictionary(trace_word->word); 
                } else {
                    compound_word_count++; 
                    // printf(">>>>> found compound word %s, count %d \n", trace_word->word, compound_word_count);
                    if (strlen(trace_word->word) == maxstring_len) {
                        /* found the same result, add it to the result list */ 
                        if (maxstring_count < MAX_RESULTS) {
                            strcpy(maxstring[maxstring_count], trace_word->word);   
                        } else { 
                            // printf("the number of results are more than the limit \n"); 
                            // printf("Need to adjust MAX_RESULTS %d \n", MAX_RESULTS); 
                            // exit(1);  
                        }
                        maxstring_count++; 
                        // printf("@@@ found more longer compound word len %d %s \n", maxstring_len, trace_word->word); 
                    } else if (strlen(trace_word->word) > maxstring_len) {
                        /* found the new result */ 
                        maxstring_len = strlen(trace_word->word); 
                        maxstring_count = 1; 
                        strcpy(maxstring[0], trace_word->word); 
                        // printf("@@@ found longer compound word len %d %s \n", maxstring_len, trace_word->word); 
                    } 
                } 
                trace_word = trace_word->next_word; 
            }
        }
        trace = trace->next_container; 
    }

#if DBG 
    inspect_dictionary(); 
#endif 

    /* show the result */ 
	printf("\n\n---------------------------------------\n"); 
    printf("The longest compound word's length = %d \n", maxstring_len);
   	printf("Answers: \n"); 	
    for (index = 0; index < maxstring_count; index++) {
        printf("%d: %s \n", index + 1, maxstring[index]); 
    }
    printf("Total number of compound words = %d \n", compound_word_count); 

}



