/* mxtool.c
Private implementation of mxtool.h. It is a command line program for XML.  
Created: October 2nd, 2014
Author: Michael Thai - Student Number (0808957)
Contact: mthai@mail.uoguelph.ca
*/
#define _POSIX_C_SOURCE 1
#include "mxutil.h"
#include "mxtool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <regex.h>
#include <termios.h>
#include <assert.h>

char *toUpper(char *str){
    char *newstr = str;
    newstr[0] = newstr[0] - 32;

    return newstr;
}

int compare(const void *a, const void *b);
void sortRecs( XmElem *collection, const char *keys[] );

/* In the main, arguments are taken from the command line and functions from mxtool.c and mxutil.c are used in order to
 * satisfy the input*/ 
int main(int arc,char const *argv[]){
  
  if (arc < 2){
    fprintf(stderr, "Invalid amount of arguments provided.\n");
    fprintf(stderr, "Try ./mxtool -review <input.xml> output.xml\n");
    fprintf(stderr, "Try ./mxtool -cat file1.xml <file2.xml> output.xml\n");
    fprintf(stderr, "Try cat file.xml | ./mxtool -keep/-discard    a/t/p=information > output.xml\n");
    exit(0);
  }
  
  //Variable Declaration
  XmElem *top,*top2;
  FILE *concatFile; 
  xmlSchemaPtr schema = NULL;
  schema = mxInit( getenv("MXTOOL_XSD") );
  int mf; 
  char *pattern;
  
  
  mf = mxReadFile( stdin, schema, &top);
  
  if (mf != 0){ 
    fprintf(stderr, "Could not validate schema file.\n");
    exit(0);
  }

  if ( strcmp(argv[1], "-review") == 0 ){   
    review(top, stdout);
    mxCleanElem(top);
  }
  
  else if ( strcmp(argv[1], "-cat") == 0){
    concatFile = fopen(argv[2], "r");
    mf = mxReadFile( concatFile, schema, &top2 );
    if (mf != 0){
      fprintf(stderr, "Could not validate schema file.\n");
      exit(0);
    }
    mf = concat(top,top2,stdout);
    fclose(concatFile);
    if (mf == -1){
      fprintf(stderr, "Concatenation Failed. Exiting.");
      exit(0);
    }
    mxCleanElem(top);
    mxCleanElem(top2);
  }
  
  else if ( strcmp(argv[1], "-keep") == 0){
    
      pattern = malloc(sizeof(char)*strlen(argv[2]) + 1);
      assert(pattern);
      strcpy(pattern, argv[2]);
      mf = selects(top, KEEP, pattern, stdout);
      
      if (mf ==  EXIT_FAILURE){
        fprintf(stderr, "Failed to keep | %s | in records. Exiting. ", pattern);
	exit(0);
      }
      free(pattern);
      mxCleanElem(top);
  }
  
  else if ( strcmp(argv[1], "-discard") == 0){
    
    pattern = malloc(sizeof(char)*strlen(argv[2]) + 1);
    assert(pattern);
    strcpy(pattern, argv[2]);
    mf = selects(top, DISCARD, pattern, stdout);
    
    if (mf == EXIT_FAILURE){
        fprintf(stderr, "Failed to keep | %s | in records. Exiting. ", pattern);
	exit(0);
      }
      free(pattern);    
      mxCleanElem(top);
  }
  
  else if ( strcmp(argv[1], "-lib") == 0 ){


    libFormat(top, stdout);

  }
  
  else if ( strcmp(argv[1], "-bib") == 0){
    printf("not done yet");
  }
  
  
  else{
    fprintf(stderr, "Invalid Arguments given\n");
    fprintf(stderr, "Try ./mxtool -review <input.xml> output.xml\n");
    fprintf(stderr, "Try ./mxtool -cat file1.xml <file2.xml> output.xml\n");
    fprintf(stderr, "Try cat file.xml | ./mxtool -keep/-discard    a/t/p=information > output.xml\n");    
    exit(0);
  }
  mxTerm(schema);
  
  return 0;
}
/* Allow the user to select which records from an open file to a new file*/
int review( const XmElem *top, FILE *outfile ){
  //Variable Declaration
  XmElem *toWrite = malloc(sizeof(XmElem));
  assert(toWrite);
  BibData bibinfo;
  char choice; 
  struct termios config, newConfig; 
  FILE *input = fopen("/dev/tty", "r");
  FILE *output = fopen("/dev/tty", "w"); 
  int i, j;
  int counter = 0;
  
  //Set "Collection" record properties before writing
  toWrite->tag = top->tag;
  toWrite->nattribs = top->nattribs;
  toWrite->attrib = top->attrib;
  toWrite->isBlank = top->isBlank;
  toWrite->text = top->text;
  toWrite->subelem = malloc(sizeof(XmElem)*top->nsubs);
  assert(toWrite->subelem);    

  
  if (!output || !input){
    fprintf(stderr, "Unable to open /dev/tty. Exiting"); 
    fclose(input);
    fclose(output);
    return(EXIT_FAILURE); 
  }
  //Set terminal settings to skip new lines
  tcgetattr(fileno(input), &config);
  tcgetattr(fileno(input), &newConfig); 
  newConfig.c_lflag &= ~ECHO;
  newConfig.c_lflag &= ~ICANON;
  newConfig.c_cc[VMIN] = 1;
  newConfig.c_cc[VTIME] = 0;
  tcsetattr(fileno(input), TCSANOW,&newConfig); 
  
  
  fprintf(output, "Press enter to keep record, space to skip, d to stop choosing record, "); 
  fprintf(output, "k to keep current records and add the rest in the file.\n"); 
  
  for (i = 0; i<top->nsubs; i++){
    marc2bib( (*top->subelem)[i], bibinfo); 
    //print out records for the user
    if ( (bibinfo[AUTHOR])[strlen( (bibinfo[AUTHOR]) +1 )] == '.'){
      fprintf(output,"%d. %s", i+1, bibinfo[AUTHOR] );
    }
    else{
      fprintf(output, "%d. %s.", i+1, bibinfo[AUTHOR]);
    }
    
    
    if ( (bibinfo[TITLE])[strlen( (bibinfo[TITLE]) +1 )] == '.'){
      fprintf(output, " %s", bibinfo[TITLE] );
    }
    else{
      fprintf(output, " %s.", bibinfo[TITLE]);
    }    
    
    
    if ( (bibinfo[PUBINFO])[strlen( (bibinfo[PUBINFO]) +1 )] == '.'){
      fprintf(output, " %s\n", bibinfo[PUBINFO] );
    }
    else{
      fprintf(output, " %s.\n", bibinfo[PUBINFO]);
    }    
    
    //Write/Skip Records decisions
    choice = fgetc(input); 
    if(choice == '\n'){
      fprintf(output, "<Enter key pressed>\n");
      (*toWrite->subelem)[counter] = (*top->subelem)[i];
      counter++;
    }
    else if (choice == 'd'){ //skip the rest of the records
      fprintf(output, "<\"d\" key pressed>\n");         
      i = top->nsubs;
    }
    else if (choice == 'k'){ //write selected records + rest of the records
      fprintf(output, "<\"k\" key pressed>\n");      
      for (j = i; j<top->nsubs; j++){
	(*toWrite->subelem)[counter] = (*top->subelem)[j];
	counter++;
      }
      i = top->nsubs;      
    }
    else if (choice == ' '){
      fprintf(output, "<Spacebar pressed>\n");      
    }
    else{
      fprintf(output, "\nPress enter to keep record, space to skip, d to stop choosing record, "); 
      fprintf(output, "k to keep current records and add the rest in the file.\n");  
      i--;
    }

  }
  toWrite->nsubs = counter;  
  toWrite->subelem = realloc( toWrite->subelem, toWrite->nsubs * sizeof( XmElem * ) );
  assert(toWrite->subelem);  

  int check = mxWriteFile(toWrite, outfile);
  tcsetattr(fileno(input), TCSANOW,&config); //return terminal settings back to original
  fclose(input);
  fclose(output);
  free(toWrite->subelem);
  free(toWrite);  
  if (check <= 0){
    return(EXIT_FAILURE); 
  }
  return (check);
}

/* Combine records from 2 files into one*/ 
int concat( const XmElem *top1, const XmElem *top2, FILE *outfile ){
  //Variable Declaration
  XmElem *toWrite = (XmElem*)malloc(sizeof(XmElem));
  assert(toWrite);
  int counter = 0;
  int i;
  int results;
  //Copy Collection properties
  toWrite->tag = top1->tag;
  toWrite->text = top1->text;
  toWrite->isBlank = top1->isBlank;
  toWrite->nattribs = top1->nattribs;
  toWrite->nsubs = top1->nsubs + top2->nsubs;
  toWrite->subelem = malloc(sizeof(XmElem*)*toWrite->nsubs);
  assert(toWrite->subelem);
  
  for (i = 0; i<top1->nsubs; i++){ //Add records from file 1
    
    (*toWrite->subelem)[counter] = (*top1->subelem)[i];
    counter++;
    
  }
  
  for (i = 0; i<top2->nsubs; i++){//Add records from file 2
    
    (*toWrite->subelem)[counter] = (*top2->subelem)[i];
    counter++;
    
  }

  results = mxWriteFile(toWrite, outfile);
  free(toWrite);
  free(toWrite->subelem);
  
  if (results == -1){
    
    fprintf(stderr, "Failed to combine records from both files.");
    return(EXIT_FAILURE);
    
  }
  
  return EXIT_SUCCESS;
}

/* Check to see if the regex is a substring of the data given and return true or false */
int match( const char *data, const char *regex ){
  //int regcomp(regex_t *preg, const char *regex, int cflags);
  //int regexec(const regex_t *preg, const char *string, size_t nmatch, regmatch_t pmatch[], int eflags);
  
  regex_t preg;
  int value = regcomp(&preg, regex, 0);
  
  if (value != 0){
    regfree(&preg);
    return(0);
  }
  
  value = regexec(&preg, data, 0, NULL, 0);
  
  if (value == 0){
    regfree(&preg);
    return(1);
  }
  else{
    regfree(&preg);
    return(0);
  }
}



/* Gets the author/title/pubinfo/callnumber from specific tags in a specific record */
void marc2bib( const XmElem *mrec, BibData bdata ){
  int field, subField;
  const char *data;
  char *string1, *string2, *string3, *string4; //temporary storage
  char *author, *pInfo, *title, *cnum;
  int flag = 0; //variable used to check if alternate tags need to be checked
  
  /* Try to find Author*/
  field = mxFindField(mrec, 100);
  if (field > 0){
    flag = 1;
    
    subField = mxFindSubfield(mrec, 100, 1, 'a');
    if (subField > 0){
      data = mxGetData(mrec, 100, 1, 'a', 1);
      author = malloc(sizeof(char)*strlen(data)+1);
      assert(author);
      strcpy( author, data );
      bdata[AUTHOR] = author;
      
    }
    else{
      
      author = malloc(sizeof(char)*3);
      assert(author);
      strcpy( author, "na" );
      bdata[AUTHOR] = author;
      
    } 
  }
  //Try field 130 if 100 doesn't exist
  field = mxFindField(mrec, 130);
  if ( (field > 0) && (flag==0) ){
    
    subField = mxFindSubfield(mrec, 130, 1, 'a');
    if (subField > 0){
      
      data = mxGetData(mrec, 130, 1, 'a', 1);
      author = malloc(sizeof(char)*strlen(data)+1);
      assert(author);
      strcpy( author, data );
      bdata[AUTHOR] = author;
      
    }
    else{ //couldn't find subfield under tag 100 or 130
      
      author = malloc(sizeof(char)*3);
      assert(author);
      strcpy( author, "na" );
      bdata[AUTHOR] = author;
      
    } 
  }
  else if(flag == 0){ //couldnt find tags 100 or 130
    
    author = malloc(sizeof(char)*3);
    assert(author);
    strcpy( author, "na" );
    bdata[AUTHOR] = author;
    
  }  

  /* Try to find Title in tag 245 $a, $p, $c*/
  field = mxFindField(mrec, 245);
  if (field > 0){
    
    subField = mxFindSubfield(mrec, 245, 1, 'a');
    
    if (subField > 0){
      
      data = mxGetData(mrec, 245, 1, 'a', 1);
      string1 = malloc(sizeof(char)*strlen(data)+1);
      assert(string1);
      strcpy(string1, data);
      
    }
    else{
      
      string1 = malloc(sizeof(char)*2);
      assert(string1);
      strcpy(string1, "");
      
    }
    
    subField = mxFindSubfield(mrec, 245, 1, 'p');
    
    if (subField > 0){
      
      data = mxGetData(mrec, 245, 1, 'p', 1);
      string2 = malloc(sizeof(char)*strlen(data)+1);
      assert(string2);
      strcpy(string2, data);
      
    }
    else{
      
      string2 = malloc(sizeof(char)*2);
      assert(string2);
      strcpy(string2, "");
      
    }
    
    subField = mxFindSubfield(mrec, 245, 1, 'b');
    
    if (subField > 0){
      
      data = mxGetData(mrec, 245, 1, 'b', 1);
      string3 = malloc(sizeof(char)*strlen(data)+1);
      assert(string3);
      strcpy(string3, data);
      
    }
    else{
      
      string3 = malloc(sizeof(char)*2);
      assert(string3);
      strcpy(string3, "");
      
    }    
    
    title = malloc(sizeof(char)*(strlen(string1) + strlen(string2) + strlen(string3) + 1) );
    assert(title);
    strcpy(title, string1);
    strcat(title, string2);
    strcat(title, string3); 
    bdata[TITLE] = title;
    free(string1);
    free(string2);
    free(string3);   
  }
  else{ //Couldn't find a subfield under tag 245, or couldn't find tag 245
    
    title = malloc(sizeof(char)*3);
    assert(title);
    strcpy(title, "na");
    bdata[TITLE] = title;
    
  }

  /* Try to find Publication Info (pinfo) */ 
  flag = 0; // flag here will be used to indicate tag $250a exists and was retrieved
  field = mxFindField(mrec, 260);
  if (field > 0){
    
    subField = mxFindSubfield(mrec, 260, 1, 'a');
    
    if (subField > 0){
      
      data = mxGetData(mrec, 260, 1, 'a', 1);
      string1 = malloc(sizeof(char)*strlen(data)+1);
      assert(string1);
      strcpy(string1, data);
      
    }
    else{
      
      string1 = malloc(sizeof(char)*2);
      assert(string1);
      strcpy(string1, "");
      
    }
    
    subField = mxFindSubfield(mrec, 260, 1, 'b');
    
    if (subField > 0){
      
      data = mxGetData(mrec, 260, 1, 'b', 1);
      string2 = malloc(sizeof(char)*strlen(data)+1);
      assert(string2);
      strcpy(string2, data);
      
    }
    else{
      
      string2 = malloc(sizeof(char)*2);
      assert(string2);
      strcpy(string2, "");
      
    }
    
    subField = mxFindSubfield(mrec, 260, 1, 'c');
    
    if (subField > 0){
      
      data = mxGetData(mrec, 260, 1, 'c', 1);
      string3 = malloc(sizeof(char)*strlen(data)+1);
      assert(string3);
      strcpy(string3, data);
      
    }
    else{
      
      string3 = malloc(sizeof(char)*2);
      assert(string3);
      strcpy(string3, "");
      
    }    
    //Check whether field 250 exists
    field = mxFindField(mrec, 250); 
    if (field > 0){
      subField = mxFindSubfield(mrec, 250, 1, 'a');
      
      if (subField > 0){
	
	data = mxGetData(mrec, 250, 1, 'a', 1); 
	string4 = malloc(sizeof(char)*strlen(data)+1);
	assert(string4);
	strcpy(string4, data);
	pInfo = malloc(sizeof(char)*(strlen(string1) + strlen(string2) + strlen(string3) + strlen(string4) + 1) );
	assert(pInfo);
	strcpy(pInfo, string1);
	strcat(pInfo, string2);
	strcat(pInfo, string3); 
	strcat(pInfo, string4);
	bdata[PUBINFO] = pInfo;
	free(string1);
	free(string2);
	free(string3);  	
	free(string4);
	flag = 1;
      }    
    }
    if (flag == 0){ //Flag set so it only runs if $250 was found
    pInfo = malloc(sizeof(char)*(strlen(string1) + strlen(string2) + strlen(string3) + 1) );
    assert(pInfo);
    strcpy(pInfo, string1);
    strcat(pInfo, string2);
    strcat(pInfo, string3); 
    bdata[PUBINFO] = pInfo;
    free(string1);
    free(string2);
    free(string3);   
    }
  }
  else{//Couldn't find tag 260 or tag 250
    
    pInfo = malloc(sizeof(char)*3);
    assert(pInfo);
    strcpy(pInfo, "na");
    bdata[PUBINFO] = pInfo;
    
  }

  /* Try to find Call Number (cnum) */
  flag = 0;
  field = mxFindField(mrec, 90);
  
  if (field>0){
    flag = 1;
    subField = mxFindSubfield(mrec, 90, 1, 'a');
    if (subField > 0){
      
      data = mxGetData(mrec, 90, 1, 'a', 1); 
      string1 = malloc(sizeof(char)*strlen(data)+1);
      assert(string1);
      strcpy(string1,data); 
      
    }
    else{
      
      string1 = malloc(sizeof(char)*2);
      assert(string1);
      strcpy(string1,"");
      
    }
    
    subField = mxFindSubfield(mrec, 90, 1, 'b');
    if (subField > 0){
      
      data = mxGetData(mrec, 90, 1, 'b', 1); 
      string2 = malloc(sizeof(char)*strlen(data)+1);
      assert(string2);
      strcpy(string2,data); 
      
    }
    else{
      
      string2= malloc(sizeof(char)*2);
      assert(string2);
      strcpy(string2,"");
      
    }   
    cnum = malloc(sizeof(char)*( strlen(string1) + strlen(string2) +1));
    assert(cnum);
    strcpy(cnum, string1);
    strcat(cnum, string2);
    bdata[CALLNUM] = cnum;
    free(string1);
    free(string2);  
  }
  //Try Tag 50 is tag 90 doesn't exist
  field = mxFindField(mrec, 50);
  if ( (field>0) && (flag==0) ){
    
    subField = mxFindSubfield(mrec, 50, 1, 'a');
    if (subField > 0){
      
      data = mxGetData(mrec, 50, 1, 'a', 1); 
      string1 = malloc(sizeof(char)*strlen(data)+1);
      assert(string1);
      strcpy(string1,data); 
      
    }
    else{
      
      string1 = malloc(sizeof(char)*2);
      assert(string1);
      strcpy(string1,"");
      
    }
    
    subField = mxFindSubfield(mrec, 50, 1, 'b');
    if (subField > 0){
      
      data = mxGetData(mrec, 50, 1, 'b', 1); 
      string2 = malloc(sizeof(char)*strlen(data)+1);
      assert(string2);
      strcpy(string2,data); 
      
    }
    else{
      
      string2 = malloc(sizeof(char)*2);
      assert(string2);
      strcpy(string2,"");
      
    }   
    cnum = malloc(sizeof(char)*( strlen(string1) + strlen(string2) +1));
    assert(cnum);
    strcpy(cnum, string1);
    strcat(cnum, string2);
    bdata[CALLNUM] = cnum;
    free(string1);
    free(string2);  
  }
  else if (flag == 0){// tag 50 and 90 both don't exist
    
    cnum = malloc(sizeof(char)*3);
    assert(cnum);
    strcpy(cnum, "na");
    bdata[CALLNUM] = cnum;
    
  }  
}
/* Allow the user to indicate which records he wants to keep based on keywords given under author, publication information
 * or title. */
int selects( const XmElem *top, const enum SELECTOR sel, const char *pattern, FILE *outfile ){
  //Variable Declaration
  BibData bibinfo;
  char *letter, *string, *temp;
  int i;
  int counter = 0;
  temp = malloc(sizeof(char)*strlen(pattern)+1);
  assert(temp);
  strcpy(temp, pattern);
  XmElem *toWrite = malloc(sizeof(XmElem));
  assert(toWrite);
  toWrite->subelem = malloc(sizeof(XmElem)*top->nsubs);
  assert(toWrite->subelem);

  letter = strtok(temp, "=");
  string = strtok(NULL, "\n");
  
  if ( pattern[1] != '='){
    fprintf(stderr, "No '=' was given.\n");
    return(EXIT_FAILURE);    
  }
  //Check if the syntax for selects is proper
  if ( (letter[0] != 'a') && (letter[0] != 'p') && (letter[0] != 't') ){
    fprintf(stderr, "Error. No indication of author, publication info, or title was given.\n");
    return(EXIT_FAILURE);
  }
  //Keep or Discard pinfos with pattern in it
  if ( letter[0] == 'p'){
    
    for (i = 0; i<top->nsubs; i++){
      
      marc2bib( (*top->subelem)[i], bibinfo);
      if ( sel == KEEP ){
	
	if ( match( bibinfo[PUBINFO], string) != 0 ){
	  (*toWrite->subelem)[counter] = (*top->subelem)[i];
	  counter++;
	}
	
      }
      else if ( sel == DISCARD ){
	
	if ( match( bibinfo[PUBINFO], string) == 0){
	  (*toWrite->subelem)[counter] = (*top->subelem)[i];
	  counter++;	  
	}
      }
      
    }
  }   
  //Keep or Discard authors with pattern in it
  if ( letter[0] == 'a'){
    
    for (i = 0; i<top->nsubs; i++){
      
      marc2bib( (*top->subelem)[i], bibinfo);
      if ( sel == KEEP ){
	
	if ( match( bibinfo[AUTHOR], string) != 0 ){
	  (*toWrite->subelem)[counter] = (*top->subelem)[i];
	  counter++;
	}
	
      }
      else if ( sel == DISCARD ){
	
	if ( match( bibinfo[AUTHOR], string) == 0){
	  (*toWrite->subelem)[counter] = (*top->subelem)[i];
	  counter++;	  
	}
      }
      
    }
  }
  //Keep or Discard titles with pattern in it
  if ( letter[0] == 't'){
    
    for (i = 0; i<top->nsubs; i++){
      
      marc2bib( (*top->subelem)[i], bibinfo);
      if ( sel == KEEP ){
	
	if ( match( bibinfo[TITLE], string) != 0 ){
	  (*toWrite->subelem)[counter] = (*top->subelem)[i];
	  counter++;
	}
	
      }
      else if ( sel == DISCARD ){
	
	if ( match( bibinfo[TITLE], string) == 0){
	  (*toWrite->subelem)[counter] = (*top->subelem)[i];
	  counter++;	  
	}
      }
      
    }
  }
  toWrite->tag = top->tag;
  toWrite->nattribs = top->nattribs;
  toWrite->attrib = top->attrib;
  toWrite->isBlank = top->isBlank;
  toWrite->text = top->text;
  toWrite->nsubs = counter;
  toWrite->subelem = realloc( toWrite->subelem, toWrite->nsubs*sizeof(XmElem) );
  int check = 0;
  check = mxWriteFile(toWrite, outfile); 
  
  if (check == -1){
    fprintf(stderr, "writing failed in selects");
    return(EXIT_FAILURE);
  }
  
  free(toWrite->subelem);
  free(toWrite);
  
  return(EXIT_SUCCESS);

    
  }
/* Find if the key given is for authors or for call numbers. The function sorts the keys and records based on what is in keys*/
void sortRecs( XmElem *collection, const char *keys[] ){
  BibData bibinfo; 
  int i, j;
  int counter = 0;
  XmElem *toWrite = malloc(sizeof(XmElem));
  assert(toWrite);
  toWrite->tag = collection->tag;
  toWrite->nattribs = collection->nattribs;
  toWrite->attrib = collection->attrib;
  toWrite->isBlank = collection->isBlank;
  toWrite->text = collection->text;
  toWrite->nsubs = collection->nsubs;  
  toWrite->subelem = malloc(sizeof(XmElem)*collection->nsubs);
  assert(toWrite->subelem);

  marc2bib((*collection->subelem)[0], bibinfo);
  //sort records by author
  if ( strcmp(bibinfo[AUTHOR], keys[0]) == 0 ){
    
    qsort(keys, collection->nsubs, sizeof(char*), compare);
    
    for (i = 0; i < collection->nsubs; i++){
      
      for (j = 0; j < collection->nsubs; j++){
	marc2bib( (*collection->subelem)[j] , bibinfo);
	if ( strcmp( bibinfo[AUTHOR], keys[i] ) == 0){
	  (*toWrite->subelem)[counter] = (*collection->subelem)[j]; //Copy over an element that matches the key to a seperate
	  counter++;                                                  //XmElem
	}	
      }     
    }    
  }
  //sort records by callnumber
  else if ( strcmp(bibinfo[CALLNUM], keys[0]) == 0){
    qsort(keys, collection->nsubs, sizeof(char*), compare);
    
    for (i = 0; i < collection->nsubs; i++){
      
      for (j = 0; j < collection->nsubs; j++){
	marc2bib( (*collection->subelem)[j] , bibinfo);
	if ( strcasecmp( bibinfo[CALLNUM], keys[i] ) == 0){
	  (*toWrite->subelem)[counter] = (*collection->subelem)[j];
	  counter++;
	}	
      }     
    }    
  }
  
  for (i = 0; i < collection->nsubs; i++){
    
    (*collection->subelem)[i] = (*toWrite->subelem)[i]; //Copy the newly allocated XmElem with the sorted records
  }                                                     //back into the original XmElem since you cannot return in a void function
  free(toWrite->subelem);                               
  free(toWrite);
  
}
/* Compares 2 strings with strcmp and acts as a function qsort can point to*/
int compare( const void *a, const void *b){
  return (strcasecmp(*(char**)a, *(char**)b) );
}
/* Pass in the call numbers for keys and call sortRecs in order to sort the records by callnumber*/
int libFormat( const XmElem *top, FILE *outfile ){
    const char *keys[top->nsubs];
    BibData bibinfo;
    //Set the keys as call numbers
    for (int i = 0; i<top->nsubs; i++){
      marc2bib( (*top->subelem)[i], bibinfo);
      keys[i] = (bibinfo[CALLNUM]);
    }
    sortRecs((XmElem*)top, keys); 
    
   for(int i = 0; i<top->nsubs; i++){ //Output the sorted records to the terminal
      marc2bib((*top->subelem)[i], bibinfo);
      if ( (bibinfo[CALLNUM])[strlen( (bibinfo[CALLNUM]) +1 )] == '.'){
	printf("%s", bibinfo[CALLNUM] );
      }
      else{
	printf("%s.", bibinfo[CALLNUM]);
      }         
      
      if ( (bibinfo[AUTHOR])[strlen( (bibinfo[AUTHOR]) +1 )] == '.'){
	printf("%s", bibinfo[AUTHOR] );
      }
      
      else{
	printf("%s.", bibinfo[AUTHOR]);
      }
      
      if ( (bibinfo[TITLE])[strlen( (bibinfo[TITLE]) +1 )] == '.'){
	printf("%s\n\n", bibinfo[TITLE] );
      }
      else{
	printf("%s.\n\n", bibinfo[TITLE]);
      }    
    }  
    //mxWriteFile(top, outfile);
    return(EXIT_SUCCESS);
}
/* Sort the records by Author name by settings the keys to the authors */
int bibFormat( const XmElem *top, FILE *outfile ){
  
    const char *keys[top->nsubs];
    BibData bibinfo;
    
    for (int i = 0; i<top->nsubs; i++){
      marc2bib( (*top->subelem)[i], bibinfo);
      keys[i] = (bibinfo[AUTHOR]);
    }
    sortRecs((XmElem*)top, keys);
    
   for(int i = 0; i<top->nsubs; i++){
      marc2bib((*top->subelem)[i], bibinfo);
      
      if ( (bibinfo[AUTHOR])[strlen( (bibinfo[AUTHOR]) +1 )] == '.'){
	printf("%s", bibinfo[AUTHOR] );
      }
      
      else{
	printf("%s.", bibinfo[AUTHOR]);
      }      
      
      if ( (bibinfo[CALLNUM])[strlen( (bibinfo[CALLNUM]) +1 )] == '.'){
	printf("%s", bibinfo[CALLNUM] );
      }
      else{
	printf("%s.", bibinfo[CALLNUM]);
      }         
      
      if ( (bibinfo[TITLE])[strlen( (bibinfo[TITLE]) +1 )] == '.'){
	printf("%s\n\n", bibinfo[TITLE] );
      }
      else{
	printf("%s.\n\n", bibinfo[TITLE]);
      }    
    }  
    //mxWriteFile(top, outfile);
    return(EXIT_SUCCESS);  
  
}
