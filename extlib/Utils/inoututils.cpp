/***********************************************************************
   This file is part of SIXTE/SIRENA software.

   SIXTE is free software: you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   any later version.

   SIXTE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   For a copy of the GNU General Public License see
   <http://www.gnu.org/licenses/>.

   Copyright 2014:  Trigger has been developed by the INSTITUTO DE FISICA DE 
   CANTABRIA (CSIC-UC) with funding from the Spanish Ministry of Science and 
   Innovation (MICINN) under project  ESP2006-13608-C02-01, and Spanish 
   Ministry of Economy (MINECO) under projects AYA2012-39767-C02-01 and
   ESP2013-48637-C2-1-P.

/***********************************************************************
*                      INOUTUTILS
*                                                                     
*  File:      inoututils.cpp
*  Developer: Beatriz Cobo Martín
* 			  cobo@ifca.unican.es
*             IFCA
*             Irene Gonzélez Pérez
*             José Ramón Rodón Ortiz
*
*  Revision History:
*
*  version 1.0.0: 21/09/06	First version           
*  version 1.1.0: 16/04/07  Change readFitsSimple. Incluying new input parameter "obj" of type "IOData".
* 							Change readFitsComplex. Incluying new input parameter "obj" of type "IOData".
* 							Change writeFitsSimple. Incluying new input parameter "obj" of type "IOData".
* 							Change writeFitsComplex. Incluying new input parameter "obj" of type "IOData".
* 							IOData is a struct wich is has the next attributes:
* 									dal_element *inObject; --> object to open.
* 									char *nameTable; --> table to process.
* 									char *nameCol; --> column to process.
* 									char *unit; --> Information of unit of column.
* 									dal_dataType type; --> type of column.
* 									int iniCol; --> Only used in Complex Type. Number of inicial column.
* 									int endCol;	--> Only used in Complex Type. Number of finish column.
* 									int iniRow; Number of inicial row.
* 									int endRow; Number of finish row.
*  version 1.1.1 02/10/07	Change fromGslMatrix.
*  version 1.2.0 03/04/08 	Changing function toGslVector: 
* 							The input parameter "type "had been include. 
* 							Type includes the kind of data of the buffer.
*  version 1.4.0 16/09/08   Modify the function "writeFitsSimple2" and "int writeFitsComplex2"  
* 							Adding function called "writeLog" to process of the each level message of log file.
*  06/09/08	Adding new library "stdlib"
*  08/07/14 Free of PIL and RIL
*           New EPOK
*           'writeLog' modified
*  15/07/14 'interactivePars' modified in order to accept negative int or double (if parameters are asked for interactively)
***********************************************************************/

/******************************************************************************
 DESCRIPTION:

 The objective of this package is to get a easy read and write of FITS files.
 
 MAP OF SECTIONS IN THIS FILE:

 - 1. INCLUDE's
 - 2. readFitsSimple
 - 3. readFitsComplex
 - 4. writeFitsSimple
 - 5. writeFitsComplex
 - 6. toGslMatrix
 - 7. toGslVector
 - 8. fromGslVector
 - 9. fromGslMatrix
 -10. interactivePars

*******************************************************************************/


/***** SECTION 1 ************************************
*       INCLUDE's
****************************************************/
#include "inoututils.h"


/***** SECTION 2 ************************************
* readFitsSimple function: This function reads values of a simple column of a FITS file. After that, the
*                          function puts them into a GSL vector to an easy process.
****************************************************/
int readFitsSimple(IOData obj,gsl_vector **result)
{
	long nRows = 0L;
	int  type, status = EPOK;
	double *bufferD = NULL;
	int *bufferJ = NULL;
	short *bufferI = NULL;
	int extver = 0, anynulls,colnum=0;
	float nullval=-99;
	string message="";
	
	if (fits_movnam_hdu(obj.inObject, ANY_HDU,obj.nameTable, extver, &status))
	{
	    message = "Cannot move to HDU " + string(obj.nameTable);
	    EP_PRINT_ERROR(message,status);
	}
	if (fits_get_colnum(obj.inObject,0,obj.nameCol,&colnum,&status))
	{
	    message = "Cannot access column " + string(obj.nameCol) + "in table" + string(obj.nameTable);
	    EP_PRINT_ERROR(message,status);
	}
	
	nRows = obj.endRow - obj.iniRow + 1; // Number of rows to read
	switch (obj.type)
	{
		case TDOUBLE:
			bufferD = new double [nRows];
			if (fits_read_col(obj.inObject, TDOUBLE, colnum, obj.iniRow, 1, nRows, &nullval, bufferD, &anynulls, &status))
			{
				message = "Cannot read column " + string(obj.nameCol) + " in table" + string(obj.nameTable);
				EP_PRINT_ERROR(message,status);
			}
			status = toGslVector((void **)&bufferD,&(*result),nRows,0,type);
			delete bufferD;
			break;
		case TINT:
			bufferJ = new int [nRows];
			if (fits_read_col(obj.inObject, TINT, colnum, obj.iniRow, 1, nRows, &nullval, bufferJ, &anynulls, &status))
			{
				message = "Cannot access column " + string(obj.nameCol) + " in table" + string(obj.nameTable);
				EP_PRINT_ERROR(message,status);
			}
			status = toGslVector((void **)&bufferJ,&(*result),nRows,0,type);
			delete bufferJ;
			break;
		case TSHORT:
			bufferI = new short [nRows];
			if (fits_read_col(obj.inObject, TSHORT, colnum, obj.iniRow, 1, nRows, &nullval, bufferI, &anynulls, &status))
			{
				message = "Cannot access column " + string(obj.nameCol) + " in table" + string(obj.nameTable);
				EP_PRINT_ERROR(message,status);
			}
			status = toGslVector((void **)&bufferI,&(*result),nRows,0,type);
			delete bufferI;
			break;
	}
	
	if(status == EPFAIL)
	{
	  message = "Cannot convert " + string(obj.nameCol) + " to GSL vector";
	  EP_PRINT_ERROR(message,EPFAIL);
	}
    return EPOK;
}
/*xxxx end of SECTION 2 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx*/


/***** SECTION 3 ************************************
* readFitsComplex function: This function reads values of a complex column of a FITS file. After that, the
*                           function put it into a GSL matrix to an easy process.
****************************************************/
int readFitsComplex(IOData obj, gsl_matrix **result)
{   
	long nRows = 0L;
	int type,extver=0,colnum=0,anynulls=0;
	int maxdim=1;
	int naxis, status=EPOK;
	long naxes=0, matrixdim, nelemsInRow;
	double *bufferD = NULL;
	int *bufferJ = NULL;
	short *bufferI = NULL;
	double nullval=-99;
	string message = "";
	
	if (fits_movnam_hdu(obj.inObject, ANY_HDU,obj.nameTable, extver, &status))
	{
	    message = "Cannot move to HDU " + string(obj.nameTable);
	    EP_PRINT_ERROR(message,status);
	}
	if (fits_get_colnum(obj.inObject,0,obj.nameCol,&colnum,&status))
	{
	    message = "Cannot access column " + string(obj.nameCol) + " in table" + string(obj.nameTable);
	    EP_PRINT_ERROR(message,status);
	    
	}
	//MC read length of multidim array in each row (naxes[0] equal to the repeat count in the TFORM keyword: Ej. 703D)

	if (fits_read_tdim(obj.inObject, colnum, 1, &naxis, &naxes, &status))
	{
	    message = "Cannot read multidim column " + string(obj.nameCol) + " information in table " + string(obj.nameTable);
	    EP_PRINT_ERROR(message,status);
	}
	nelemsInRow = naxes;

	nRows = obj.endRow - obj.iniRow + 1; // Number of rows of the fits to read
	// binSize = obj.endCol - obj.iniCol + 1; //number of columns of the matrix to read
	matrixdim = nelemsInRow*nRows;

	//double *bufferD = new double [5];
	//double bufferD[matrixdim];
	
   	switch (obj.type)
   	{
		case TDOUBLE:
			bufferD = new double [matrixdim];
			if (fits_read_col(obj.inObject, TDOUBLE, colnum, obj.iniRow, 1, matrixdim, &nullval, bufferD, &anynulls, &status))
			{
				message = "Cannot read column " + string(obj.nameCol) + "in table" + string(obj.nameTable);
				EP_PRINT_ERROR(message,status);
			}
			status = toGslMatrix((void **)&bufferD,&(*result),nelemsInRow,nRows,obj.type,0);
			delete bufferD;
			break;
		case TINT:
			bufferJ = new int [matrixdim];
			if (fits_read_col(obj.inObject, TINT, colnum, obj.iniRow, 1, nRows, &nullval, bufferJ, &anynulls, &status))
			{
				message = "Cannot read column " + string(obj.nameCol) + "in table" + string(obj.nameTable);
				EP_PRINT_ERROR(message,status);
			}
			status = toGslMatrix((void **)&bufferJ,&(*result),nelemsInRow,nRows,obj.type,0);
			delete bufferJ;
			break;
		case TSHORT:
			bufferI = new short [matrixdim];
			if (fits_read_col(obj.inObject, TSHORT, colnum, obj.iniRow, 1, nRows, &nullval, bufferI, &anynulls, &status))
			{
				message = "Cannot read column " + string(obj.nameCol) + "in table" + string(obj.nameTable);
				EP_PRINT_ERROR(message,status);
			}
			status = toGslMatrix((void **)&bufferI,&(*result),nelemsInRow,nRows,obj.type,0);
			delete bufferI;
			break;
	}

   	if (status == EPFAIL)
   	{
   		message = "Cannot convert " + string(obj.nameCol) + " to GSL vector";
   		EP_PRINT_ERROR(message,EPFAIL);
	}

	return EPOK;
}
/*xxxx end of SECTION 3 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx*/


/***** SECTION 4 ************************************
* writeFitsSimple function: This function reads values of a GSL vector.	After that, the function put it into a
*                           column of output FITS file
****************************************************/
int writeFitsSimple(IOData obj, gsl_vector *vector)
{
	int *bufferJ = NULL;
	short *bufferI = NULL;
	double *bufferD = NULL;

	int blocksize = obj.endRow - obj.iniRow + 1;
	int extver = 0, trgNcols=0, status=EPOK;
	int  colnum=0, felem=0,hdunum=0,hdunums=0;
	char *comment=NULL;
	char extname[20],nameCol[20],charcol[20];
	char keyname[10];
	char keyvalstr[500];
	char *tform[1];
	char *ttype[1];
	char *tunit[1];
	char tform1[20];
	string strcol="", message="";

	switch ((int)obj.type)
	{
		case TDOUBLE:
			bufferD = new double [vector->size];
			for(int i=0; i<vector->size; i++)  bufferD[i] = gsl_vector_get(vector,i);
			strcpy(tform1,"1D");
			break;
		case TINT:
			bufferJ = new int [vector->size];
			for(int i=0; i<vector->size; i++)  bufferJ[i] = gsl_vector_get(vector,i);
			strcpy(tform1,"1J");
			break;
		case TSHORT:
			bufferI = new short [vector->size];
			for(int i=0; i<vector->size; i++)  bufferI[i] = gsl_vector_get(vector,i);
			strcpy(tform1,"1I");
			break;
	}
	
	if (fits_movnam_hdu(obj.inObject, ANY_HDU,obj.nameTable, extver, &status))
	{
		// Table is not found => create table
	    status = EPOK;
	    ttype[0]=obj.nameCol;
	    tform[0]=tform1;
	    tunit[0]=obj.unit;

	    if (fits_create_tbl(obj.inObject, BINARY_TBL, blocksize,1, ttype, tform,tunit, obj.nameTable, &status))
	    {
		  message = "Cannot create table " + string(obj.nameTable);
		  EP_PRINT_ERROR(message,status);
	    }
	}

	int firstelem=1;

	// Check for column existence

	if (fits_get_colnum(obj.inObject,0,obj.nameCol,&colnum,&status))
	{
		//Table found & column not found: insert column
		status=EPOK;
		if (fits_get_num_cols(obj.inObject, &trgNcols, &status))
		{
			message = "Cannot access column " + string(obj.nameCol) + "in table" + string(obj.nameTable);
			EP_PRINT_ERROR(message,status);
		}
		if (fits_insert_col(obj.inObject, ++trgNcols, obj.nameCol, tform1, &status))
		{
			message = "Cannot add column " + string(obj.nameCol) + "in table" + string(obj.nameTable);
			EP_PRINT_ERROR(message,status);
		}
		sprintf(charcol,"%d",trgNcols);
		strcol=string("TUNIT")+string(charcol);
		strcpy(keyname,strcol.c_str());
		if (fits_write_key(obj.inObject, TSTRING, keyname, obj.unit, comment,&status))
		{
			message = "Cannot set units for column " + string(obj.nameCol) + "in table" + string(obj.nameTable);
			EP_PRINT_ERROR(message,status);
		}
		if (fits_get_colnum(obj.inObject,0,obj.nameCol,&colnum,&status))
		{
			message = "Cannot get column number for " + string(obj.nameCol) + "in table" + string(obj.nameTable);
			EP_PRINT_ERROR(message,status);
		}
	}

	// Populate column
	switch (obj.type)
	{
		case TDOUBLE:
			status = fits_write_col(obj.inObject, obj.type, colnum, obj.iniRow, firstelem, vector->size, bufferD, &status);
			break;
		case TINT:
			status = fits_write_col(obj.inObject, obj.type, colnum, obj.iniRow, firstelem, vector->size, bufferJ, &status);
			break;
		case TSHORT:
			status = fits_write_col(obj.inObject, obj.type, colnum, obj.iniRow, firstelem, vector->size, bufferI, &status);
			break;
	}
	
	if (status != EPOK)
	{
		message = "Cannot populate column " + string(obj.nameCol) + " in table " + string(obj.nameTable);
	    EP_PRINT_ERROR(message,EPFAIL);
	}
	
	return EPOK;
}
/*xxxx end of SECTION 4 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx*/


/***** SECTION 5 ************************************
* writeFitsCOmplex function: This function reads values of a GSL matrix. After that, the function put it into a
*                            complex column of output FITS file
****************************************************/
int writeFitsComplex(IOData obj, gsl_matrix *matrix)
{
	int *bufferJ = NULL;
	short *bufferI = NULL;
	double *bufferD = NULL;
	int blocksize = obj.endRow - obj.iniRow + 1;
	int extver=0,trgNcols=0,dim=0,status=EPOK;
	int  colnum=0, felem=0;
	char *comment=NULL;
	char extname[20],charcol[20];
	char keyname[10];
	char keyvalstr[100];
	char chardim[100];
	char *tform[1];
	char *ttype[1];
	char *tunit[1];
	char tform1[20];
	string strtf1="",strcol="", message="";
	
	dim=matrix->size2;
	sprintf(chardim,"%d",dim);
	switch ((int)obj.type)
	{
		case TDOUBLE:
			bufferD = new double [matrix->size1*matrix->size2];
			strtf1 = string(chardim) + string("D");
			strcpy(tform1,strtf1.c_str());
			break;
		case TINT:
			bufferJ = new int [matrix->size1*matrix->size2];
			strtf1 = string(chardim) + string("J");
			strcpy(tform1,strtf1.c_str());
			break;
		case TSHORT:
			bufferI = new short [matrix->size1*matrix->size2];
			strtf1 = string(chardim) + string("I");
			strcpy(tform1,strtf1.c_str());
			break;
	}

	//??? borrar
	int n1 = (matrix)->size1;
	int n2 = (matrix)->size2;
	for (int i=0; i<n1; i++)
	{
		for (int j=0; j<n2; j++)
		{
			switch (obj.type)
			{
				case TDOUBLE:
				{
					bufferD[(n2*i)+j] = gsl_matrix_get(matrix, i, j);
					break;
				}
				case TSHORT:
				{
					bufferI[(n2*i)+j] = gsl_matrix_get(matrix, i, j);
					break;
				}
				case TINT:
				{
					bufferJ[(n2*i)+j] = gsl_matrix_get(matrix, i, j);
					break;
				}
			}
		}
	}
	fits_movnam_hdu(obj.inObject, ANY_HDU,obj.nameTable, extver, &status);

	if (status != EPOK)
	{
		// Table is not found => create table
	    status = EPOK;
	    ttype[0]=obj.nameCol;
	    tform[0]=tform1;
	    tunit[0]=obj.unit;
	    if (fits_create_tbl(obj.inObject, BINARY_TBL, blocksize,1, ttype, tform, tunit, obj.nameTable, &status))
	    {
	      message = "Cannot create table " + string(obj.nameTable);
	      EP_PRINT_ERROR(message,status);
	    }
	}
	int firstelem=1;	//t=obj.inObject;

	// Check for column existence
	if (fits_get_colnum(obj.inObject,0,obj.nameCol,&colnum,&status))
	{
		//Table found & column not found: insert column
	    status=EPOK;
	    if (fits_get_num_cols(obj.inObject, &trgNcols, &status))
	    {
	    	message = "Cannot access column " + string(obj.nameCol) + " in table " + string(obj.nameTable);
	    	EP_PRINT_ERROR(message,status);
	    }
	    if (fits_insert_col(obj.inObject, ++trgNcols, obj.nameCol, tform1,&status))
	    {
	    	message = "Cannot add column " + string(obj.nameCol) + " in table " + string(obj.nameTable);
	    	EP_PRINT_ERROR(message,status);
	    }
	    sprintf(charcol,"%d",trgNcols);
	    strcol=string("TUNIT")+string(charcol);
	    strcpy(keyname,strcol.c_str());
	    if (fits_write_key(obj.inObject, TSTRING, keyname, obj.unit, comment,&status))
	    {
	    	message = "Cannot set units for column " + string(obj.nameCol) + " in table " + string(obj.nameTable);
	    	EP_PRINT_ERROR(message,status);
	    }
	    if (fits_get_colnum(obj.inObject,0,obj.nameCol,&colnum,&status))
	    {
	    	message = "Cannot get column number for " + string(obj.nameCol) + " in table " + string(obj.nameTable);
	    	EP_PRINT_ERROR(message,status);
	    }

	}
	
	// Populate column
	switch (obj.type)
	{
		case TDOUBLE:
			status = fits_write_col(obj.inObject, obj.type, colnum, obj.iniRow, firstelem, matrix->size1*matrix->size2, bufferD, &status);
			break;
		case TINT:
			status = fits_write_col(obj.inObject, obj.type, colnum, obj.iniRow, firstelem, matrix->size1*matrix->size2, bufferJ, &status);
			break;
		case TSHORT:
			status = fits_write_col(obj.inObject, obj.type, colnum, obj.iniRow, firstelem, matrix->size1*matrix->size2, bufferI, &status);
			break;
	}
	if (status != EPOK)
	{
	      message = "Cannot populate column " + string(obj.nameCol) + " in table " + string(obj.nameTable);
	      EP_PRINT_ERROR(message,EPFAIL);
	}
	
	return status;
}
/*xxxx end of SECTION 5 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx*/


/***** SECTION 6 ************************************
* toGslMatrix function: The function puts the values of the input buffer into an output GSL matrix. Columns and
*                       rows are input parameters
****************************************************/
int toGslMatrix(void **buffer, gsl_matrix **matrix, long numCol, int numRow, int type, int eventini)
{
	int t=0,k=0,status=EPOK;
	string message="";
  
	while (t<numRow*numCol)
	{
		for (int j=0;j<numCol-eventini;j++)
		{
			switch (type)
			{
				case TDOUBLE:
					gsl_matrix_set(*matrix,k,j,((double *)*buffer)[t+eventini]);
					break;
				case TINT:
					gsl_matrix_set(*matrix,k,j,((int *)*buffer)[t+eventini]);
					break;
				case TSHORT:
					gsl_matrix_set(*matrix,k,j,((short *)*buffer)[t+eventini]);
					break;
			}
			t++;
		}
		t=t+eventini;
		k++;
	}

  return EPOK;
}
/*xxxx end of SECTION 6 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx*/


/***** SECTION 7 ************************************
* toGslVector function: The function puts the values of the input buffer into an output GSL vector.
****************************************************/
int toGslVector(void **buffer, gsl_vector **array, long nevent, int eventini, int type)
{ 
	int status = EPOK;
	
	for (int i=0; i<nevent-eventini;i++)
	{
		switch (type)
		{
			case TDOUBLE: gsl_vector_set(*array,i,((double*)*buffer)[i+eventini]); break;
			case TSHORT:  gsl_vector_set(*array,i,((short*)*buffer)[i+eventini]); break;
			case TINT:  gsl_vector_set(*array,i,((long*)*buffer)[i+eventini]); break;
		}
	}
	
	return EPOK;
}
/*xxxx end of SECTION 7 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx*/


/***** SECTION 8 ************************************
* fromGslVector function: The function puts the values of the input GSL vector into a output buffer
****************************************************/
int fromGslVector(void **buffer, gsl_vector **array, int type)
{
	int status = EPOK;
	
	for(int i=0; i<(*array)->size; i++)
	{
		switch (type)
		{
			case TDOUBLE: ((double *)*buffer)[i] = gsl_vector_get(*array,i); break;
			case TINT: ((int *)*buffer)[i] = gsl_vector_get(*array,i); break;			
			case TSHORT: ((short *)*buffer)[i] = gsl_vector_get(*array,i); break;
		}	
	}

	return EPOK;
	
}
/*xxxx end of SECTION 8 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx*/


/***** SECTION 9 ************************************
* fromGslMatrix function: The function puts the values of the input GSL matrix into a output buffer
****************************************************/
int fromGslMatrix(void **buffer, gsl_matrix **matrix, int type)
{
	int status = EPOK;
	
	int n1 = (*matrix)->size1;
	int n2 = (*matrix)->size2;
	for(int i=0; i<n1; i++)
	{
		for (int j=0; j<n2; j++)
		{
			switch (type)
			{
				case TDOUBLE: {((double *)(*buffer))[(n2*i)+j] = gsl_matrix_get(*matrix,i,j); break;}
				case TSHORT: {((short *)(*buffer))[(n2*i)+j] = gsl_matrix_get(*matrix,i,j); break;}
				case TINT: {((long *)(*buffer))[(n2*i)+j] = gsl_matrix_get(*matrix,i,j); break;}
			}		
		}
	}

	return EPOK;
}
/*xxxx end of SECTION 9 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx*/


/***** SECTION 10 ************************************
* interactivePars function: This function reads input parameters interactively, if not provided by the user
****************************************************/
int interactivePars(inparam *taskPars, int np, string task)
{
	char buf[1000];
	string message = "";
	
	// Print USAGE for no input parameters
	cout << "Usage:\n" + task + " ";
	for(int i=0;i<np; i++)
	{
		if (taskPars[i].type == "char")
		{
			cout << "--" << taskPars[i].name << "=" << taskPars[i].description << "(" << taskPars[i].defValStr << ")";
		}
		else if (taskPars[i].type == "int")
		{
			cout << "--" << taskPars[i].name << "=" << taskPars[i].description << "(" << taskPars[i].defValInt << ")";
		}
		else
		{
			cout << "--" << taskPars[i].name << "=" << taskPars[i].description << "(" << taskPars[i].defValReal << ")";
		}
	}
	printf("\n\n");

	// Ask for parameters interactively and save user input
	for (int i=0;i<np; i++)
	{
		if (taskPars[i].type == "char")
		{
			cout << taskPars[i].description << " [" << taskPars[i].defValStr << "]:";
			fgets(buf, sizeof(buf), stdin);
			*strchr(buf, '\n') = '\0';
			if (strlen(buf) != 0) taskPars[i].ValStr = buf;
		}
		else if (taskPars[i].type == "int")
		{
			cout << taskPars[i].description << " [" << taskPars[i].defValInt << "]:";
			fgets(buf, sizeof buf, stdin);
			*strchr(buf, '\n') = '\0';
			if (strlen(buf) != 0)
			{
				if ((!isdigit(buf[0]) && (buf[0] != '-')) ||
						(!isdigit(buf[0]) && (buf[0] == '-') && !isdigit(buf[1])) )
				{
					message = "Invalid value for param " + string(taskPars[i].name);
				    EP_PRINT_ERROR(message,EPFAIL);
				}
				taskPars[i].ValInt = atoi(buf);
			}
		}
		else
		{
			cout << taskPars[i].description << " [" << taskPars[i].defValReal << "]:";
			fgets(buf, sizeof buf, stdin);
			*strchr(buf, '\n') = '\0';
			if (strlen(buf) != 0)
			{
				if ((!isdigit(buf[0]) && (buf[0] != '-')) ||
						(!isdigit(buf[0]) && (buf[0] == '-') && !isdigit(buf[1])) )
				{
					message = "Invalid value for param " + string(taskPars[i].name);
				    EP_PRINT_ERROR(message,EPFAIL);
				}
				taskPars[i].ValReal = atof(buf);
			}
		}
	}

	return EPOK;
}
/*xxxx end of SECTION 10 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx*/
