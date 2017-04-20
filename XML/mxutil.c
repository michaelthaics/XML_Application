/* mxutil.c
Private implementation of mxutil.h. It is a library of utility functions for the MARCXML schema and XML documents. 
Created: September 15th, 2014 - Updated October 9th, 2014
Author: Michael Thai - Student Number (0808957)
Contact: mthai@mail.uoguelph.ca
*/

#define _POSIX_C_SOURCE 1
#include "mxutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

/*Write the whole XmElem collection to a file
  Remodeled based off of printElement()*/
int mxWriteFile( const XmElem *top, FILE *mxfile ){
 int static depth = 0;
 int static numRecs = 0;
 xmlChar *temp;

  if (depth == 0){
    fprintf(mxfile, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(mxfile, "<!-- Output by mxutil library Michael Thai -->\n");
    
  }

  for ( int i=0; i<depth; i++ ){ 
    fprintf(mxfile,  "\t" );
  }
  
  if ( strcmp(top->tag, "collection") !=0){
    fprintf(mxfile, "<marc:%s", top->tag );
  }
  
  if ( strcmp(top->tag, "record") == 0){
    numRecs++;
  }
 
  for ( int i=0; i < top->nattribs; i++ ){
    if (strcmp(top->tag, "collection") == 0){
      fprintf(mxfile, "<marc:collection xmlns:marc=\"http://www.loc.gov/MARC21/slim\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.loc.gov/MARC21/slim http://www.loc.gov/standards/marcxml/schema/MARC21slim.xsd\">\n");
      
    }
    else{
      fprintf(mxfile, " %s=\"%s\"", (*top->attrib)[i][0], (*top->attrib)[i][1] );
    }
  }
  if (top->isBlank != 1) {
    temp = xmlEncodeSpecialChars(NULL,(xmlChar*)top->text);
    fprintf(mxfile, ">%s", (char*)temp);
    xmlFree(temp);
  }
  else if (strcmp (top->tag , "collection") != 0){
    fprintf(mxfile, ">\n");
  }
  // print subelements
  for ( int i=0; i < top->nsubs; i++ ) {
    ++depth;
    mxWriteFile( (*top->subelem)[i] , mxfile );
    --depth;
  }
  if ( (strcmp(top->tag, "datafield") == 0) || (strcmp(top->tag, "record") == 0) ){
    
    for ( int i=0; i<depth; i++ ){ 
    fprintf(mxfile,  "\t" );
  }
  
    
  }
    fprintf(mxfile, "</marc:%s>\n",top->tag);

  if (numRecs == 0){
    return(-1);
  }
  if (numRecs>3){  
  return (numRecs/2);
  }
  else{
    return (numRecs);
  }
}

/* Initializes the MARC XML Schema 
   returns: schema pointer on success, null on failure
 */
xmlSchemaPtr mxInit( const char *xsdfile ){
  
  xmlSchemaPtr sp = NULL;

  LIBXML_TEST_VERSION;
  xmlLineNumbersDefault(1);
  xmlSchemaParserCtxtPtr ctp = xmlSchemaNewParserCtxt(xsdfile); //Pointer to the XSD file
  sp = xmlSchemaParse(ctp); //Parse the XSD File
  xmlSchemaFreeParserCtxt(ctp); //Free the pointer to the XSD File
  
  if (sp==NULL){ // If sp is invalid, return null
    
    return(NULL); 
    
  }
  
  else{ // Return sp if not null 
    
    return(sp);
    
  }
}
/* This function initializes the DOM tree from the XML file, and uses mxMakeElem to aid in copying the DOM tree to our XmElem structure
 * returns: 0 on success, 1 if failed to parse xml file, 2 if the xml file doesn't match the schema
 */
int mxReadFile( FILE *marcxmlfp, xmlSchemaPtr sp, XmElem **top ){

  /* Variable Declaration */
  xmlDocPtr dp = NULL; //initialize doc pointer
  int validate;
  xmlNodePtr node = NULL; //Initialize node pointer
  
  dp = xmlReadFd(fileno(marcxmlfp), "", NULL, 0); //Build the DOM tree from the xml file
  xmlSchemaValidCtxtPtr ctxt = xmlSchemaNewValidCtxt(sp);
  validate = xmlSchemaValidateDoc(ctxt, dp);
  
  if(validate == 0){ // If the xml document was valid with the schema, copy the DOM tree to the XmElem structure
    
    node = xmlDocGetRootElement(dp); //Set node to the root of the DOM tree
    *top = mxMakeElem(dp,node); //Make the XmElem tree from the DOM tree
    
    /* Free the pointers used */
    xmlSchemaFreeValidCtxt(ctxt);
    xmlFreeDoc(dp); 
    
    return (0);
  }
  
  else if(dp == NULL){ // Failed to parse XML file
    
    *top = NULL; 
    
    xmlSchemaFreeValidCtxt(ctxt);
    xmlFreeDoc(dp); 
    
    return(1);
  }
  
  else{ // Failed to validate the XML file with the Schema
   
    *top = NULL;
    
    xmlSchemaFreeValidCtxt(ctxt);
    xmlFreeDoc(dp); 
    
    return(2);
    
  }
}
/* Makes an element. Recursively called to copy the DOM tree starting from the root element 
 * returns the root element created
 */
XmElem *mxMakeElem( xmlDocPtr doc, xmlNodePtr node ){
  
  //Variable Declaration
  xmlNodePtr first;
  XmElem *elem = NULL;
  xmlAttrPtr ap = NULL;
  int i;
  
    while(node->type == XML_COMMENT_NODE){
      node = node->next; 
      
    }

    /* Allocate memory for the name of the element and copy it to the element's 'tag' property*/
    elem = (XmElem*)malloc(sizeof(XmElem)); 
    assert(elem);
    if (node->name != NULL){
      
    elem->tag = malloc(strlen((char*)node->name)+1); 
    assert(elem->tag);
    strcpy(elem->tag,(char*)node->name); 
    //printf("\n tags = %s", elem->tag);
    }

    elem->text = (char*)xmlNodeListGetString(doc,node->children,1);//Retrieve and assign the text from the DOM tree

    elem->isBlank = xmlIsBlankNode(node->children);
    
    /* Count the number of attributes in the element */
    ap = node->properties; //Set ap to the properties of the node in the DOM tree
    elem->nattribs = 0; 
    
    while(ap != NULL){ //Check to see how many attributes there are in the element and store in XmElem->nattribs
      
      elem->nattribs++;
      ap = ap->next;
      
    }

    /* Allocate memory for the given attributes in the element and assign them to XmElem->attrib */
    if (elem->nattribs > 0){ 
      
      ap = node->properties; //reset attribute pointer to the first attribute  
      elem->attrib = malloc(sizeof(char*[2])*elem->nattribs); 
      assert(elem->attrib);
      
      for (i = 0; i<elem->nattribs; i++){ //Assign the attributes of the element (name and value)
	
	(*elem->attrib)[i][0] = malloc((sizeof(char) * strlen((char*)ap->name))+1);
	strcpy((*elem->attrib)[i][0], (char*)ap->name); 
	(*elem->attrib)[i][1] = (char*)xmlGetProp(node, ap->name);
	ap = ap->next; 
      }    
    }
    
    if (elem->nattribs == 0){ 
      
      elem->attrib = NULL;
      
    }
    
    elem->nsubs = xmlChildElementCount(node); 
    
    if (elem->nsubs > 0){ 
      
      elem->subelem = malloc(sizeof(XmElem*)*elem->nsubs);
      assert(elem->subelem);
      
    }
    
    first = xmlFirstElementChild(node); //Point to the first sub element (child) 
    
    /* Recursively call mxMakeElem to create all the elements in the DOM tree from the root */
    for (i = 0; i<elem->nsubs;i++){ 
      
      if (first != NULL){ 
	
	(*elem->subelem)[i] = mxMakeElem(doc, first);
	
      }
      
      first = xmlNextElementSibling(first); //Point to the next sub element (child)
    }
  
  return (elem); //Returns the root of the newly built XmElem tree
}

/* Finds the amount of 'tags' in a given field
 * returns (above)
 */
int mxFindField( const XmElem *mrecp, int tag ){
  
  //Variable Declaration
  int counter = 0; //counts amount of time the given 'tag' shows up in the field
  int i,j;
  XmElem * subElement;
  if (mrecp){
    
  if (strcmp(mrecp->tag,"record") != 0){
    
    return(0);
    
  }
  }
  
  if (tag == 0){
    
    return(0);
    
  }
  
  /* Checks the 'record' element for the 'tag' given in the parameter */
  if (mrecp->nsubs > 0){ 
    
    for (i = 0; i<mrecp->nsubs; i++){ //Loop through the sub elements
      
      subElement = (*mrecp->subelem)[i]; 
      
      if (subElement->nattribs > 0){
	
        for (j = 0; j<subElement->nattribs; j++){ //Loop through the sub element's attributes to find element's with 'tag'
	  
          int compareTag = atoi((*subElement->attrib)[j][1]); 
	  
          if (tag == compareTag){ //Increment the counter if 'tag' is found
	    
            counter++; 
	    
          }         
        }        
      }      
    }  
  }
  return (counter);
}


/* Finds the amount of 'codes' or 'sub' in a given tnumth instance of 'tag'
 * returns (above)
 */
int mxFindSubfield( const XmElem *mrecp, int tag, int tnum, char sub ){
  //Variable Declaration
  int counter = 0; //Counts each occurence of tag
  int counter2 = 0;//Counts amount of times 'sub' appears in the appropriate field
  int i, j;
  int compareTag = 0;//used to compare the element's 'tag' to the given 'tag' from the parameter
  XmElem * subField;//The t-numth element will be stored in here to later check its attribs for 'sub'
  XmElem * subElement;
  char *compareSub;
  char compare;
  
  if (tnum > mxFindField(mrecp, tag) ){
    return (0);
  }
  /* Find the t-numth times tag was found and copy that element to check its attributes for 'sub' */
  if (mrecp->nsubs > 0){ 
    
    for (i = 0; i<mrecp->nsubs; i++){ //Loops through the sub elements

      subElement = (*mrecp->subelem)[i]; 
	
      for (j = 0; j<subElement->nsubs; j++){

        compareTag = atoi((*subElement->attrib)[0][1]); 
	  
        if (tag == compareTag){ //Check the element's tag for the 'tag' in the parameter
	    
          counter++; 
	    
          if (counter == tnum){ //check if tag was found t-numth times

	    subField = subElement; //If tag found t-numth times, assign the element for the attribs to be checked
	    break;
	      
	  }    
        }        
      }    
    }
    /*Check the found element for char 'sub' occurences*/
    if (subField != NULL){ 
      
      for (i = 0; i<subField->nsubs; i++){ //Loop through the sub elements
	
      subElement = (*subField->subelem)[i];
      
        for (j = 0; j<subElement->nattribs; j++){ //Loop through attributes for 'sub' occurences
	
          compareSub = (char*)(*subElement->attrib)[j][1];
          compare = compareSub[0];
	    
          if ( sub == compare ){ //if the attribute matches the given 'sub' increment the counter
	      
            counter2++; 
	      
          }
        }       
      }      
    }    
  }  
  return(counter2);
}



/*Finds the text from the tnumth instance of 'tag' and the snumth instance of 'code'
 * returns (above)
 */
const char *mxGetData( const XmElem *mrecp, int tag, int tnum, char sub, int snum ){
  //Variable Declaration
  int counter = 0; //counts amount of time the given 'tag' shows up in the field
  int counter2 = 0;
  int i,j;
  XmElem * data = NULL; //A XmElem storage for the tnumth instance of the element with given 'tag'  for snum checking
  XmElem * subElement;
  
  if ( (tnum > mxFindField(mrecp,tag) )  ){
    return(NULL);
  }

  /* Checks the 'record' element for the 'tag' given in the parameter */
  if (tnum >0){
    
    if (mrecp->nsubs > 0){ 
      
      for (i = 0; i<mrecp->nsubs; i++){ //Loop through the sub elements
	
	subElement = (*mrecp->subelem)[i]; 
	
	if (subElement->nattribs > 0){
	  
	  for (j = 0; j<subElement->nattribs; j++){ //Loop through the sub element's attributes to find element's with 'tag'
	    
	    int compareTag = atoi((*subElement->attrib)[j][1]); 
	    
	    if (tag == compareTag){ 

	      counter++;
	      
	      if (counter == tnum){ //Make copy of element at tnum, break out of loops for snum checking

		data = subElement;
		
		if (tag<10){
		  
		  return (data->text);
		  
		}
		
		break; 
		
	      }	      
	    }  
	  }        
	}	
      }  
    }
    
    if (snum == 0){
      
      return(data->text); 
      
    }
    
    if (snum > 0){
      
      for (i = 0; i<data->nsubs; i++){ //Loop through sub elements in search for 'sub' matches
	
	subElement = (*data->subelem)[i];    
	
	for (j = 0; j<subElement->nattribs; j++){
	  
	  /* copy the string from 'code' and compare the first character to 'sub' */
	  char *temp;
	  temp = (*subElement->attrib)[j][1];
	  char compareSub = temp[0];
	  
	  if (sub == compareSub){
	    
	    counter2++;
	    
	    if (counter2 == snum){ //once s-numth instance is found, return the data
	      
	      return(subElement->text); 
	      
	    }
	  }
	}
      }
    }
  }
return (NULL);  
}




void mxCleanElem( XmElem *top ){
  
  int i;
  
  if (top->nsubs > 0){
    
    for (i = 0; i<top->nsubs; i++){ //Recursively call mxCleanElem to free memory starting from the bottom of the tree
      
      mxCleanElem((*top->subelem)[i]);
      
    }
    
    if (top->subelem != NULL){ 
      
      free(top->subelem);
      
    } 
  }
  
  if (top->nattribs > 0){ //Check if there are attributes in the element
    
    for (i = 0; i<top->nattribs; i++){ //Loop through all the elements and free their attributes
      
      free((*top->attrib)[i][0]);
      free((*top->attrib)[i][1]);
      
    }
    
    if (top->attrib != NULL){ 
      
      free(top->attrib);
      
    }  
  }
  
  if (top->tag != NULL){ 
    
    free(top->tag); 
    
  }
  
  if(top->text !=NULL){ 
    
    free(top->text);
    
  }
 
  if(top!=NULL){ //Free the root of the element
    
    free(top);
    
  }
}



void mxTerm( xmlSchemaPtr sp ){
  
  xmlSchemaFree(sp); //Free Schema Pointer
  xmlSchemaCleanupTypes(); //Cleanup default XML schema library
  xmlCleanupParser(); //Clean memory allocated by the parser
  
}
