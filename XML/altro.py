#!/usr/bin/python3
"""
File: altro.py - Updated November 7
Name: Michael Thai
Student ID: 0808957
Course: CIS2750
Functionality: To provide a MarcXML application by 
using python for the GUI (front end) and extending
to C for the back end functionality.
"""
#Import libraries 
from tkinter import *   
from time import sleep
from curses.ascii import ispunct
import tkinter.filedialog
import tkinter.messagebox
import os
import Mx

#Set the main window
root = Tk()
root.title('Altro')
root.minsize (750,520)
root.geometry( "750x520" )

#Some global variable declaration
statusText = StringVar()
statusText.set('')
openedFlag = 0 #used to check if a file was previously opened
recPtr = None

def status(string):
  statusText.set(string)
  root.update_idletasks()

def Open():
  #global variables needed for append/insert
  global recPtr
  global numRecs 
  global openedFlag
  status("Opening a file..")
  fileName = tkinter.filedialog.askopenfilename(title="Open XML File", filetypes=[("XML files", "*.xml"), ("All Files", "*")])
  
  if openedFlag == 1: 
    choice = tkinter.messagebox.askyesno(title="", message = 'Overwrite data currently open?')
    if choice == 1:
      display.delete(0,END) #clear listbox to insert more records on top
      if len(fileName) != 0:
        openedFlag = 1
        value, recPtr, numRecs = Mx.readFile(fileName)
        
        if numRecs > 0:
          status( str(numRecs) + ' records opened')
        elif value == 1:
          status( 'No records found in the xml file' )
        else:
          status( 'Failed to validate xml file' )
        #display the records in the listbox
        for i in range(0, numRecs):
          (author, title, pubinfo, callnum) = Mx.marc2bib(recPtr,i)
          bibData = str(i+1) + '. ' + author + '. ' + title + '. ' + pubinfo
          display.insert(END, bibData)
    
  else:
    if len(fileName) != 0:
      openedFlag = 1
      value, recPtr, numRecs = Mx.readFile(fileName)
      if numRecs > 0:
        status( str(numRecs) + ' records opened')
      elif value == 1:
        status( 'No records found in the xml file' )
      else:
        status( 'Failed to validate xml file' )

      for i in range(0, numRecs):
        (author, title, pubinfo, callnum) = Mx.marc2bib(recPtr,i)
        bibData = str(i+1) + '. ' + author + '. ' + title + '. ' + pubinfo
        display.insert(END, bibData)

  
def insert():
  global recPtr
  global numRecs
  status('Inserting more record files to the beginning')
  fileName = tkinter.filedialog.askopenfilename(title="Open XML File", filetypes=[("XML files", "*.xml")])
  
  if len(fileName) != 0: #check if a file was actually selected to prevent empty calls
    beforeInsert = numRecs
    value, recPtr, numRecs = Mx.insert(fileName, recPtr)
    
    if numRecs > 0:
      status( str(numRecs - beforeInsert) + ' records inserted. ' + str(numRecs) + ' total records')
    else:
      status( 'Failed to insert xml file' )
    
    display.delete(0, END)
    
    for i in range(0, numRecs):
      (author, title, pubinfo, callnum) = Mx.marc2bib(recPtr,i)
      bibData = str(i+1) + '. ' + author + '. ' + title + '. ' + pubinfo
      display.insert(END, bibData)   
    
def append(): #same as insert, but adds records to the end of the display
  status('Inserting more record files to the end')
  global recPtr
  global numRecs
  fileName = tkinter.filedialog.askopenfilename(title="Open XML File", filetypes=[("XML files", "*.xml")])
  
  if len(fileName) != 0:
    beforeAppend = numRecs
    value, recPtr, numRecs = Mx.append(fileName, recPtr)
    
    if numRecs > 0:
      status( str(numRecs - beforeAppend) + ' records appended. ' + str(numRecs) + ' total records')
    else:
      status( 'Failed to append xml file' )  
      
    display.delete(0, END)
    for i in range(0, numRecs):
      (author, title, pubinfo, callnum) = Mx.marc2bib(recPtr,i)
      bibData = str(i+1) + '. ' + author + '. ' + title + '. ' + pubinfo
      display.insert(END, bibData)  
      
      
def saveAs():
  global recPtr
  global numRecs
  status('Saving current records into a file')
  fileName = tkinter.filedialog.asksaveasfilename(title = "Save As..", filetypes=[("All Files", "*")])
  """
  value = Mx.writeFile(recPtr,fileName)
  status( str(value) + ' records saved')
  """
  
def exit():
  choice = tkinter.messagebox.askyesno(title="", message = 'Exit? All unsaved data will be lost.')
  if choice == 1:	
    value = Mx.term()
    quit()
  
def library():
  status('Printing the records in library format')
  fileName = tkinter.filedialog.asksaveasfilename(title = "Save As..", filetypes=[("All Files", "*")])
  
def bibliography():
  status('Printing the records in bibliography format')
  fileName = tkinter.filedialog.asksaveasfilename(title = "Save As..", filetypes=[("All Files", "*")])

def review():
  status('Starting review of records') 
  
def concat():
  status('Starting concat of 2 records')
  
def keep():
  status('Keeping specific records')
  
def discard():  
  status('Discarding specific records')
 
  
  
def libFormat():
  status('Sorting records by library')
  
def bibFormat():
  status('Sorting records by Call number')
  
def about():
  tkinter.messagebox.showinfo('About Altro..', \
'Altro is a GUI program designed to utilize the mxutil and mxtool functions' +
 ' following MARC21Slim  schema written by Michael Thai.')
  
def regex():
  tkinter.messagebox.showinfo('About Regex', 'Regex is an in depth way ' +
 'to search for a substring in a line or a file. For example the regex' +
 ' *programming* would return *programming bible lv.8* *programming for ' +
 'dummies* *How to survive CIS2750 programming*')

def undo():
  status('Undid the last action')  

def delete():
  global numRecs
  status('Deleting Selected Records')
  items = display.curselection() #get indexes of selected items
  offset = 0
  deleted = 0
  #loop through selected items and delete
  for i in items:  
    index = int(i) - offset
    display.delete(index,index)
    offset = offset + 1
    deleted = deleted + 1
    numRecs = numRecs - 1
  status( str(deleted) + ' records deleted.' + str(numRecs) + ' left in total')	
	  
  
value = Mx.init()
if value == 0:
  status('Schema Validated')
else:
  status('Failed to Validate Schema')

menu = Menu(root)
root.config(menu = menu)

# Adding 'File' tab
menuBar = Menu(menu)
menu.add_cascade(label= 'File', menu = menuBar)

menuBar.add_command(label = 'Open', command = Open)
menuBar.add_command(label = 'Insert', command = insert)
menuBar.add_command(label = 'Append', command = append)
menuBar.add_command(label = 'Save as..', command = saveAs)
menuBar.add_separator()
menuBar.add_command(label = 'Exit', command = exit)

# Adding 'Print' tab
printBar = Menu(menu)
menu.add_cascade(label = 'Print', menu = printBar)

printBar.add_command(label = 'Bibliography', command = bibliography)
printBar.add_command(label = 'Library', command = library)

#Adding 'Help' tab
helpBar = Menu(menu)
menu.add_cascade(label = 'Help', menu = helpBar)

helpBar.add_command(label = 'Keep/discard regex', command = regex)
helpBar.add_command(label = 'About altro...', command = about)

#Adding 'MxTool' Function tab
mxtoolBar = Menu(menu)
menu.add_cascade(label = 'MxTool', menu = mxtoolBar)

mxtoolBar.add_command(label = 'Review', command = review)
mxtoolBar.add_command(label = 'Concat', command = concat)
mxtoolBar.add_command(label = 'LibFormat', command = libFormat)
mxtoolBar.add_command(label = 'BibFormat', command = bibFormat)

#Adding Listbox and vertical scroll to display MarcXML records
displayFrame = Frame( root )

vScroll = Scrollbar( displayFrame, orient = VERTICAL )
display = Listbox( displayFrame, selectmode = EXTENDED, bg = 'white', \
		  yscrollcommand=vScroll.set )
vScroll.config( command = display.yview )

vScroll.pack( side=RIGHT, fill=Y )
display.pack( anchor=N, fill=BOTH, expand=1 )
displayFrame.pack( anchor=N, fill=BOTH, expand=1 )

#Adding Edit Control Panel functionality
editFrame = Frame(root)

regexLabel = Label(editFrame, text = '  Regex Input Field')
regex = StringVar()
regexBox = Entry(editFrame, bg = 'white', width = 50, textvariable = regex)

key = IntVar()
authorRadio = Radiobutton( editFrame, text = 'Author', value = 1, variable = key)
titleRadio = Radiobutton( editFrame, text = 'Title', value = 2, variable = key)
pubInfoRadio = Radiobutton( editFrame, text = 'Pub. Info', value = 3, variable = key)

authorRadio.pack(side = LEFT)
titleRadio.pack(side = LEFT)
pubInfoRadio.pack(side = LEFT)
regexLabel.pack(side = LEFT)
regexBox.pack(side = LEFT)

editFrame.pack(anchor = S)

#Adding Edit Control Panel Functionality - row 2
editFrame2 = Frame(root)

deleteButton = Button(editFrame2, text = 'Delete', command = delete)
keepButton = Button(editFrame2, text = 'Keep records with the keyword', command = keep)  
discardButton = Button(editFrame2, text = 'Discard records with the keyword', command = discard)  
undoButton = Button(editFrame2, text = 'Undo', command = undo)

deleteButton.pack(side = LEFT)
keepButton.pack(side = LEFT)
discardButton.pack(side = LEFT)
undoButton.pack(side = RIGHT)
editFrame2.pack(anchor = S)

# Adding Status Bar 
statusBar = Label(root ,textvariable = statusText, bd = 1, relief = SUNKEN, anchor = S)
statusBar.pack(side = BOTTOM, fill = X)

#Check to see if MXTOOL_XSD was set automatically when the program is executed
xsd = os.environ.get('MXTOOL_XSD')
if xsd is None:
	fileName = tkinter.filedialog.askopenfilename(title="Open XSD File", filetypes=[("XML files", "*.xsd"), ("All Files", "*")])


root.mainloop() #execute main frame
