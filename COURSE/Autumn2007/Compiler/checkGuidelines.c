/**
 * This code is to check whether a given peice of c code
 * follows the specified guidelines or not. 
 *
 * @author Udit Sajjanhar (sajjanharudit@gmail.com)
 *
 * licensed under GPL
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE  1
#define FALSE 0

const char SPACE = ' ';
const char TAB = '\t';
const char OPENBRACE = '{';
const char CLOSEBRACE = '}';
const char *OPERATORS[] = { "==", "*=", "/=", "%=", "+=", "-=", "<<=",
                         ">>=", "&=", "^=", "|=", "?", ":", "||", "&&",
			 "|", "^", "!=", "=", "!=", "<=", ">=",
                         "<<", ">>", ">", "<", "+", "-", "/"};
const char *DROPCHECK[] = { "++", "--", "->", "\\\"", "\\\'" };
const char *KEYWORDS[] = { "if", "while", "else", "switch", "else if",
                          "for", ","};
const char *VALIDENDSTRINGS[] = { "}", "{", ";" };

const int LINEWRAP = 80;
const int INDENT = 2;
int lineNumber = 0;
int braceCheck = 1;
int openBraceCount = 0;
int changeIndent = 0;
int curReqIndent = 0;
int opSeen = 0;
int singleSeen = 0;
int doubleSeen = 0;
int braceFound = 0;
int singDobChanged = 0;
int validEnd = 1;

/**
 * The function does the following things:
 * 1. checks whether C code file to be checked for consistancy has been
 *    provided as command line input
 * 2. If (1) above is TRUE then it checks whether the file exists or not
 * 3. if (2) above is TRUE it returns the file pointer to the C code file
 * 4. if any of (1) or (2) is FALSE, the code exists with a proper message
 *
 * @param numArgs the number of input arguments given at runtime
 * @param argArray An array f string containg all the arguments 
 *
 * @returns A file pointer to the opened file
 */
FILE * CheckValidFilename(int numArgs, char **argArray) {
  
  FILE *cCodeFile;
  
  /* check if the number of arguments is atleast 2 */
  if (numArgs < 2) {
    printf("\nToo less input arguments.\n");
    printf("Please provide the C code to be checked as a command");
    printf("line input\n\n");
    exit(0);
  }
  
  /* check whether the filenmae provided exists or not */
  cCodeFile = fopen(argArray[1], "r");
  if (cCodeFile == NULL) { 
    printf("\nError in opening the %s file.\n", argArray[1]);
    printf("Please check if it exists.\n\n");
    exit(0);
  } 

  return cCodeFile;
}

/**
 * Reads one line from the current position of file pointer and 
 * finds out the
 * 1. current indentation level of the line
 * 2. whether tabs are used
 *
 * @param cCode The pointer to the C Code file
 * @param curLine The pointer to the char array in which the line is to stored
 * @param indLevel The pointer to hold the value of the indent level of the 
 *                current line being read
 * @param isTabPresent The pointer to a int which indicates whether a
 *                     tab was used to indent or not
 *
 * @returns The length of the current line of C Code
 */
int GetOneLine(FILE *cCode, char **curLine, int *indLevel, int *isTabPresent) {
  
  int hasFirstCharCome = FALSE;
  int isTab = FALSE;
  
  char tempChar, line[400];
  
  int count = 0, curIndLevel = 0; 
  int i;
 
  if (fgets(line, 400, cCode) != NULL) {
    count = strlen(line);
  }
  if (count == 400) {
    printf("Too big line. PLease shorten all your line lengths. Exiting.. \n");
    exit(0);
  }
  
  i = 0;
  tempChar = line[i];
  while ( (tempChar != '\n')) {
    
    if (!hasFirstCharCome) {
      if (tempChar == SPACE || tempChar == TAB) {
        curIndLevel += 1;
        if (tempChar == TAB)
          isTab = TRUE;
      } else {
        hasFirstCharCome = TRUE;
      }
    }
    tempChar = line[++i];
  } 
  
  *isTabPresent = isTab;

  /* check if this is not a empty line */
  if (count != 0) {
    *indLevel = curIndLevel;
    *curLine = (char *)malloc(count * sizeof(char));
    strncpy(*curLine, line, count - 1);
  } else {
    *isTabPresent = FALSE;
    *indLevel = -1;
    *curLine = NULL;
  }

  lineNumber++;
  singDobChanged = 0;
  braceFound = 0;
  changeIndent = 0; 
  return count - 1;
	    
}


/**
 * checks whether a given string starts with a particular string
 *
 * @param base The string which has to be checked
 * @param search The string which has to be searched 
 *
 * @returns TRUE, if the base starts with the search string,
 *          otherwise FALSE
 */
int StartsWith(char *base, char *search) {

  int searchLen = 0;
  int i = 0;

  searchLen = strlen(search);
  
  if (strlen(base) < searchLen) {
    return FALSE;
  } else {
    for (i = 0; i < searchLen; i++) {
      if (base[i] != search[i]) {
        return FALSE;
      }
    }
    return TRUE;
  }
}


/**
 * checks whether a given string ends with a particular string.
 * Trailing whitespaces are ignored.
 *
 * @param base The string which has to be checked
 * @param search The string which has to be searched 
 *
 * @returns TRUE, if the base ends with the search string,
 *          otherwise FALSE
 */
int EndsWith(char *base, char *search) {

  int searchLen = 0;
  int i = 0, j = 0;

  searchLen = strlen(search);
  
  if (strlen(base) < searchLen) {
    return FALSE;
  } else {
    for (i = searchLen - 1, j = strlen(base) - 1; i >= 0 && j >= 0; j--) {
      
      if ((base[j] == TAB) || (base[j] == SPACE || (base[j] < 30)))
        continue;
     
      //printf("%c, %c, %d, %d", base[j], search[i], j, i) ;
      if (base[j] != search[i]) {
        return FALSE;
      }

      i--;
    } 

    if (i >= 0)
      return FALSE;
    else 
      return TRUE;
  }
}

/**
 * seeks to the end of a comment. If the comment extends over to
 * many lines all the remaing comment lines are read. 
 *
 * If anything follows in the same line where the comment is closed,
 * then it raises an error and makes the program to exit 
 *
 * @param line The char array containg the line
 * @param pattern the string pattern to be expected in the
 *                begining of a multiline comment
 * @param isWrong The int variable indicating the comment is 
 *                not proper and hence doesn't check for the pattern.
 * @param cCode The pointer to the C code file
 */
void GoToEndOfComment(char *line, char *pattern, int isWrong, FILE *cCode) {
  
  int newLineIndent;
  int bolTemp;
  
  while (1) {
    if (strstr(line, "*/") == NULL) { 
      GetOneLine(cCode, &line, &newLineIndent, &bolTemp);
      if (!isWrong) {
        if (StartsWith(pattern, "!")) {
          if (StartsWith(line + newLineIndent, pattern + 1)) {
            printf("Error(line %d): %s not ", lineNumber, pattern + 1);
            printf("expected at the start of comment\n");
          }
        } else {
          if (!StartsWith(line + newLineIndent, pattern)) {
            printf("Error(line %d): %s ", lineNumber, pattern);
            printf("expected at the start of comment\n");
          }
        }
      }
    } else {
      /* check if anything else follows the closing of comment */
      if (!EndsWith(line, "*/")) {
        printf("Error(line %d): The comment ", lineNumber);
        printf("should not be followed by any character. Exitting......\n");
        exit(0);
      } else {
        return ;
      }
    }
    /* since the the C code has compiled, every comment will be 
       closed and and thus thw while loop will terminate */
  }    
}

/**
 * Checks the indentation level of the current line.
 * 
 * It also updates the indentation level , in case the 
 * current line has "{" or "}"
 *
 * @param curIndent The indentation of the current line
 * @param isTabPresent The integer variabel indicating whether a
 *                     tab was used to indent the current line or not
 */
void CheckIndentOfCurrentLine(int curIndent, int isTabPresent) {

  if (!braceCheck || singleSeen || doubleSeen || braceFound
      || singDobChanged) {
    return;
  }
  if (changeIndent < 0) {
    curReqIndent -= INDENT;
  }
  
  if (isTabPresent) {
    printf("Error(line %d): Indentation incorrect. ", lineNumber);
    printf("TABS were used.\n");
    			    
  } else if (curIndent != curReqIndent && validEnd) {
    printf("Error(line %d): Indentation incorrect \n", lineNumber);
  }
  
  if (changeIndent == 1 || changeIndent == -2) {
    curReqIndent += INDENT;
  }
}

/**
 * Checks whether the given line is comment or not. All
 * the three types (/**,/* and //) are checked for. If the
 * line is found to be a multi-line comment then the 
 * subsequent comment lines are also read 
 *
 * @param base The char array containg the current line
 * @param indent The integer variable representing the indent
 *               level of the line
 * @param length The length of the line
 * @param cCode the pointer to the C Code file
 * @param isTabPresent integer variable indicating whether a tab was
 *                     used to indent the current line or not
 *
 * @returns TRUE, if the line is found to be a comment otherwise
 *          FALSE
 */
int CheckForComment(char *base, int indent, int length, FILE *cCode,
                    int isTabPresent) {

  int isWrong = FALSE;
  int i = 0, EXPRLEN = 0;
 
  /* check for block comments */
  if (StartsWith(base + indent, "/**")) {
    EXPRLEN = 3;
    
    /* check if there are other characters in the same line */
    for (i = indent + EXPRLEN; i < length; i++) {
      if ( (base[i] != SPACE) && (base[i] != TAB) ) {
        isWrong = TRUE;
        break;
      }	      
    }

    if (isWrong) {
      /* The comment starts with "/**" but has other characters in the same
         same line. Log an error */
      printf("Error(line %d): The block comment ", lineNumber);
      printf("should only have /** in the first line\n");

      GoToEndOfComment(base, "", isWrong, cCode);
    } else {
      GoToEndOfComment(base, "*", isWrong, cCode);
    }
  } else if (StartsWith(base + indent, "/*")) {

    /* The comment doesnt start with "/**" but starts with "/*". */
    GoToEndOfComment(base, "!*", isWrong, cCode);
    
  } else if (StartsWith(base + indent, "//")) {

    /* This is the wrong style of commenting. Log an error */
    printf("Error(line %d): Wrong Style of commenting\n", lineNumber); 
  } else if (StartsWith(base + indent, "#")) {
    
    /* this will take care of the # includes and the # defines */
  } else {
    return FALSE;
  } 
  
  opSeen = 0;
  return TRUE;
}

/**
 * Checks for opening and closing paranthesis. It raises error if:
 * 1. opening paranthesis are not the last character of a line
 * 2. Closing characters are not the first and only character of a line
 *
 * However if the line contains the token "enum" or is an assignment 
 * list like "= {a,b,c,d}" then the checking is ignored
 *
 * Also an exception is made for the closing brace of structure, as 
 * it can have some variable or semicolon following it.
 *
 * All the above exceptions are filteresd by the fact that closing 
 * paranthesis in the exceptional cases always have a trailing semicolon
 * which is not poresent otherwise. Anyone got better ideas???
 *
 * @param base The Char array containg the line
 * @param index The indewx position of the character to be checked
 * @param length The length of the currenmt line
 * @param indent The current indenttation level of the line 
 */
void CheckForBraces(char *base, int index, int length, int indent) {

  char *tempString;
  int i = 0;
 
  /* initialize tempString */
  tempString = (char*)malloc(index * sizeof(char));
  for (i = 0; i < index; i++) {
    tempString[i] = ' ';
  }
	  
  /* Check if the brace checking is allowed for the current charact */
  if (!braceCheck) {
    if (base[index] == CLOSEBRACE) {
      braceCheck = 1;
      braceFound = 1;
    }
    return;
  }

  /* check if the current line contains an "enum" identifier */
  if (strstr(base, "enum") != NULL || strstr(base, "struct") != NULL) {
    braceCheck = 0;
    return;
  }
  
  /* check for opening paranthesis */	
  if (base[index] == OPENBRACE) {
  
    opSeen = 0;
    
    /* A block has been opened, increa the indent level */
    if (changeIndent == -1) {
      changeIndent = -2;
    } else {
      changeIndent = 1;
    }
    
    /* check if brace is the first character in the line */
    if (StartsWith(base + indent, "{")) {
      printf("Error(line %d): The openeing brace should be in", lineNumber);
      printf(" the same line where the compound statement started\n");
      return;
    }
    
    /* check if the line is of the form "= {a,b,c,d}" to ignore */
    for (i = 0; i < index; i++) {
      tempString[i] = base[i];
    }
    if (EndsWith(tempString, "=")) {
      braceCheck = 0;
      return;
    }
    
    /* check if some text follows the opening brace */
    if (!EndsWith(base, "{")) {
      printf("Error(line %d,%d): The opening brace ", lineNumber, index);
      printf("should be the last character in the line %s\n",base);
      return;
    }
  } else if (base[index] == CLOSEBRACE) {

    opSeen = 0;
    
    /* The brace has been closed, decrease the indent level */
    if (changeIndent == 1) {
      changeIndent = 2;
    } else {
      changeIndent = -1;
    }
    
    /* check if the current line ends with semicolon */
    if (EndsWith(base, ";") || (strstr(base, "else if") != NULL) ||
       (strstr(base, "else") != NULL) || (strstr(base, "while") != NULL) ) {
      return;
    }

    /* check if closing brace is not the first character of the line */
    if (!StartsWith(base + indent, "}")) {
      printf("Error(line %d): The closing brace ", lineNumber);
      printf("should be the first and only character in a line \n");
      return;
    }

    /* check if there are any other characters than the cloing brace */
    if (!EndsWith(base, "}")) {
      printf("Error(line %d): The closing brace ", lineNumber);
      printf("should be the first and only character in a line \n");
      return;
    }
  }
}

void CheckValidEnd(char *base) {
  
  int numElements, i;
  numElements = sizeof(VALIDENDSTRINGS) / sizeof(VALIDENDSTRINGS[0]);
  for (i = 0; i < numElements; i++) {
    if (EndsWith(base, (char *)VALIDENDSTRINGS[i])) {
      validEnd = 1;
    } else {
      validEnd = 0;
    }
  }
}

/**
 * checks whether the the current character is one of the operators.
 * if yes, then it checks for spaces immediately preceding and following it
 *
 * @param base The Char array containg the line
 * @param index The indewx position of the character to be checked
 * @param length The length of the currenmt line
 * @param indent The indent position of the current line
 */
int CheckForAdjoiningSpaces(char *base, int index, int length, int indent) {

  int numElements, i; 	 

  numElements = sizeof(DROPCHECK) / sizeof(DROPCHECK[0]);
  for (i = 0; i < numElements; i++) {
    if (StartsWith(base + index, (char *)DROPCHECK[i])) {
      opSeen = 0;
      return strlen(DROPCHECK[i]);
    }
  }
  
  numElements = sizeof(OPERATORS) / sizeof(OPERATORS[0]);
  
  /* go through the list of the key words */
  for (i = 0; i < numElements; i++) {
    if (StartsWith(base + index, (char *)OPERATORS[i]) && !opSeen) {
      if (!StartsWith(base + index + strlen(OPERATORS[i]), " ") && 
          !(EndsWith(base, (char *)OPERATORS[i])) ) {
        /* The key word is matched but there is no space after the keyword */
        printf("Error(line %d, char %d): ", lineNumber, index);
        printf("A space should precede and follow %s token\n", OPERATORS[i]);
      } 
      if ((index != 0) && (index != indent) && (base[index - 1] != ' '))  {
        printf("Error(line %d, char %d): ", lineNumber, index);
        printf("A space should precede and follow %s token\n", OPERATORS[i]);
      }
      opSeen = 1;
      return strlen(OPERATORS[i]); 
    }  
  }	
  return 0;
}


/**
 * checks whether the current characters is one of the condionals or
 * iterators. If yes, it checks for a space following it.
 *
 * @param base The Char array containg the line
 * @param index The indewx position of the character to be checked
 * @param length The length of the currenmt line
 */
int CheckForSingleSpace(char *base, int index, int length) {
 
  int numElements, i; 	 

  numElements = sizeof(DROPCHECK) / sizeof(DROPCHECK[0]);
  for (i = 0; i < numElements; i++) {
    if (StartsWith(base + index, (char *)DROPCHECK[i])) {
      opSeen = 0;
      return strlen(DROPCHECK[i]);
    }
  }
  
  numElements = sizeof(KEYWORDS) / sizeof(KEYWORDS[0]);
  
  /* go through the list of the key words */
  for (i = 0; i < numElements; i++) {
    if (StartsWith(base + index, (char *)KEYWORDS[i])) {
      if (!StartsWith(base + index + strlen(KEYWORDS[i]), " ") && 
          !(EndsWith(base, (char *)KEYWORDS[i])) ) {
        /* The key word is matched but there is no space after the keyword */
        printf("Error(line %d, %d): ", lineNumber, index);
        printf("A space should follow %s token\n", KEYWORDS[i]);
      }
      opSeen = 0;
      return strlen(KEYWORDS[i]);
    }  
  }	  

  return 0;
}


int CheckSingleDouble(char *base, int i) {

  if (base[i] == '\'' && !(i != 0 && base[i - 1] == '\\')) {
    if (singleSeen == 1) {
      singleSeen = 0;
    } else {
      singleSeen = 1;
    }
    return 1;
  } 

  if (base[i] == '"' && !(i != 0 && base[i - 1] == '\\')) {
    if (doubleSeen == 1) {
      doubleSeen = 0;
    } else {
      doubleSeen = 1;
    }
    return 1;
  }

  if (doubleSeen || singleSeen) {
    return 1;
  }

  return 0;
}


/**
 * checks the individual characters in a line, in case the line is not
 * a comment. 
 *
 * @param base The char array containg the current line
 * @param indent The integer variable representing the indent
 *               level of the line
 * @param length The length of the line
 *
 * @returns TRUE, if the line is found to be a comment otherwise
 *          FALSE
 */
void CheckIndividualCharacters(char *base, int indent, int length) {

  int i=0, val1 = 0, val2 = 0;
  
  /* iterate through all the characters of the line */
  while (i < length) {
 
   
    /* check for quotation marks */
    if (CheckSingleDouble(base, i)) {
      i = i + 1;
      continue;
    }
    
    /* check for brace presence */
    CheckForBraces(base, i, length, indent);

    /* check for space after if... and fors... */
    val1 = CheckForSingleSpace(base, i, length);

    /* check for spaces in operators */
    val2 = CheckForAdjoiningSpaces(base, i, length, indent);

    if (!val1 && !val2) {
      if (base[i] != SPACE && base[i] != TAB) {
        opSeen = 0;
      }	
      i = i + 1;
    } else if (val1 > val2) {
      i = i + val1;
    } else {
      i = i + val2;
    }
  }
}

int main(int argc, char **argv) {

  /* indicates whether a int was used to indent */
  int isTabPresent = FALSE;
  
  /* array containing the current line of C Code */
  char *curLine;
  
  /* keeps the track of indentation */
  int curIndent = 0;

  /* length of the current input line */
  int curLineLength = 0; 
	
  /* The file pointer to the C code file */
  FILE *cCode;
 

  
  
  /* check for a valid filename */
  cCode = CheckValidFilename(argc, argv);

  /* read the code line by line */
  while (!feof(cCode)) {

    curLineLength = GetOneLine(cCode, &curLine, &curIndent, &isTabPresent);   
    
    /* check the length of the current line */
    if (curLineLength > LINEWRAP) {
      printf("Error(line %d):", lineNumber);
      printf("The line length is more than %d characters\n", LINEWRAP);
    }
    /* Do the various checks on line if it is not empty and contains
       some other characters than whiotespaces */
    if ( (curLineLength != 0) && (curLineLength != curIndent) ) {

      /* check whether the given line is comment or not */
      if (CheckForComment(curLine,curIndent,curLineLength, cCode,
                          isTabPresent)) {
        continue;
      } else {

        /* check individual characters */
        CheckIndividualCharacters(curLine, curIndent, curLineLength);

	/* Check the indentation of the current line */
	CheckIndentOfCurrentLine(curIndent, isTabPresent);	
        
	/* check whether this line ends validly */
	CheckValidEnd(curLine);
	
      }
    }
  }
  return 0;
}
