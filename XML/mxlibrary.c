/*
 * Filename : mxlibrary.c -updated November 7
 * Name: Michael Thai - 0808957
 * contact: mthai@mail.uoguelph.ca
 * Functionality: Wrapper functions which allow python to 
 * communicate with the mxtool/mxutil C modules.
 */

#include <Python.h>
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

xmlSchemaPtr schema = NULL;

//Function Initialization
static PyObject *Mx_init( PyObject *self, PyObject *args );
static PyObject *Mx_readFile( PyObject *self, PyObject *args );
static PyObject *Mx_marc2bib( PyObject *self, PyObject *args );
static PyObject *Mx_insert( PyObject *self, PyObject *args );
static PyObject *Mx_append( PyObject *self, PyObject *args );
static PyObject *Mx_term( PyObject *self, PyObject *args );
static PyObject *Mx_writeFile( PyObject *self, PyObject *args );
static PyObject *Mx_keep(PyObject *self, PyObject *args);
//static PyObject *Mx_freeFile( PyObject *self, PyObject *args );

//MX Methods list
static PyMethodDef MxMethods[] = {
{"init", Mx_init, METH_VARARGS},
{"readFile", Mx_readFile, METH_VARARGS},
{"marc2bib", Mx_marc2bib, METH_VARARGS},
{"insert", Mx_insert, METH_VARARGS},
{"append", Mx_append, METH_VARARGS},
{"term", Mx_term, METH_VARARGS},
{"writeFile", Mx_writeFile, METH_VARARGS},
{"keep", Mx_keep, METH_VARARGS},
//{"freeFile", Mx_freeFile, METH_VARARGS},
{NULL, NULL} };

static struct PyModuleDef mxModuleDef = {
PyModuleDef_HEAD_INIT,
"Mx", //enable "import Mx"
NULL, //omit module documentation
-1, //module keeps state in global variables
MxMethods}; //link module name "Mx" to methods table 

PyMODINIT_FUNC PyInit_Mx(void) { return PyModule_Create( &mxModuleDef ); }
/* Wrapper function for mxInit() */
PyObject *Mx_init( PyObject *self, PyObject *args ){
  
  xmlSchemaPtr schema = NULL;
  schema = mxInit( getenv( "MXTOOL_XSD") );
  
  if (schema == NULL){
    return Py_BuildValue("i", 0); 
  }
  else{
    return Py_BuildValue("i",1); 
  }
  
  
}

/* Wrapper function for mxReadFile() */
static PyObject *Mx_readFile( PyObject *self, PyObject *args ){
  XmElem * top;
  schema = NULL;
  char *filename;
  int value;
  
  PyArg_ParseTuple( args, "s", &filename );
  
  FILE *fp = fopen(filename, "r");
  schema = mxInit( getenv( "MXTOOL_XSD" ));
  value = mxReadFile(fp, schema, &top);
  
  fclose(fp);
  
  if (value == 0){
   
    return Py_BuildValue("iki", 1, top, top->nsubs);
    
  }
  
  return Py_BuildValue("iki", 0, NULL, 0);
}

/* Wrapper Function for marc2bib */
static PyObject *Mx_marc2bib( PyObject *self, PyObject *args ){
  XmElem *top;
  int recNum;
  
  PyArg_ParseTuple( args, "ki", (unsigned long *)&top, &recNum );
  BibData bibInfo;
  marc2bib( (*top->subelem)[recNum], bibInfo);
  
  return Py_BuildValue("ssss", bibInfo[AUTHOR], bibInfo[TITLE], bibInfo[PUBINFO], bibInfo[CALLNUM]);
}

/* Wrapper function for concat, added file is inserted at the beginning */
static PyObject *Mx_insert( PyObject *self, PyObject *args ){
  XmElem *top, *top2, *insert;
  xmlSchemaPtr schema = NULL;
  char *filename;
  int value, nsubs;
  FILE *fp, *fp2;
  
  PyArg_ParseTuple(args, "sk", &filename, (unsigned long*)&top);
  
  fp = fopen(filename, "r");
  fp2 = fopen("concat.xml", "w"); //Temporary file
  schema = mxInit( getenv( "MXTOOL_XSD" ) );
  value = mxReadFile(fp, schema, &top2);
  fclose(fp);
  nsubs = top->nsubs + top2->nsubs; //File inserted at the beginning for display
  concat(top2, top, fp2);
  fclose(fp2);
  fp = fopen("concat.xml", "r");
  value = mxReadFile(fp, schema, &top2); //Get new top after concatenation
                                         //for easy display in Listbox
  fclose(fp);
  
  return Py_BuildValue("iki", value, top2, nsubs);
}
/* Wrapper Function for Mx_append, same as Mx_insert but adds records at the end*/
static PyObject *Mx_append( PyObject *self, PyObject *args ){
  XmElem *top, *top2, *insert;
  xmlSchemaPtr schema = NULL;
  char *filename;
  int value, nsubs;
  FILE *fp, *fp2;
  
  PyArg_ParseTuple(args, "sk", &filename, (unsigned long*)&top);
  
  fp = fopen(filename, "r");
  fp2 = fopen("concat.xml", "w");
  schema = mxInit( getenv( "MXTOOL_XSD" ) );
  value = mxReadFile(fp, schema, &top2);
  fclose(fp);
  nsubs = top->nsubs + top2->nsubs;
  concat(top, top2, fp2);
  fclose(fp2);
  fp = fopen("concat.xml", "r");
  value = mxReadFile(fp, schema, &top);
  
  return Py_BuildValue("iki", value, top, nsubs);
}
/* Wrapper function for mxTerm() */
static PyObject *Mx_term( PyObject *self, PyObject *args ){
  
  if (schema != NULL){
    
    mxTerm(schema);
  
  }
  return Py_BuildValue("i", 0);
}
/* Wrapper function for mxWriteFile() - not complete */
static PyObject *Mx_writeFile( PyObject *self, PyObject *args ){
  XmElem *top;
  FILE *fp;
  char *filename;
  int numRecs = 0;
  
  PyArg_ParseTuple(args, "sk", &filename, (unsigned long*)&top);
 
 
  fp = fopen(filename, "w");
  printf("will it print? %s", top->tag);
  //numRecs = mxWriteFile(collection, fp);
  fclose(fp);
  
  return Py_BuildValue("i", numRecs);
  
}

/*Wrapper function for keep/discard (selects() ) - not completed */ 
static PyObject *Mx_keep(PyObject *self, PyObject *args){
  XmElem *top;
  char *pattern, *regex;
  int keepOrDiscard;
  FILE *fp;
  fp = fopen("tempKeepDiscard.xml", "w");
  
  PyArg_ParseTuple(args, "sski", &regex, &pattern, (unsigned long*)&top, keepOrDiscard);
	
  return 0;	
	
	
}


/*
static PyObject *Mx_freeFile( PyObject *self, PyObject *args ){
  
  
  
}

*/













