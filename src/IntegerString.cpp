#include "IntegerString.h"
#include <stdio.h>
#include <stdlib.h>

/*******************************************************************************/
/*                         IntegerString class methods                               */
/*******************************************************************************/

IntegerString :: IntegerString(int initialSize) {

    mMaxLength = initialSize;
    mData = new int [mMaxLength];
    mLength = 0;
    mSearchIndex = -1;
}
IntegerString :: ~IntegerString(void) {

    delete [] mData;
}
void IntegerString :: Append(int symbol) {

    if(mLength >= mMaxLength) {

	    int *temp_buffer = new int [mMaxLength+512];
	    for(int n = 0; n < mMaxLength; n++)
		    temp_buffer[n] = mData[n];
			
		delete [] mData;
		mData = temp_buffer;
		mMaxLength += 512;
	}
	
	mData[mLength] = symbol;
	mLength++;

}

void IntegerString :: Cat(IntegerString &intString) {

    if(mMaxLength < mLength + intString.mLength) {
    
        int *temp_buffer = new int [mLength+intString.mLength+512];
        for(int n = 0; n < mLength; n++)
            temp_buffer[n] = mData[n];

        delete [] mData;
        mData = temp_buffer;
        mMaxLength = mLength+intString.mLength+512;
    }

    for(int n = 0; n < intString.mLength; n++)
        mData[mLength+n] = intString.mData[n];
        
    mLength += intString.mLength;
}

void IntegerString :: Clear(void) {
    mLength = 0;
    mSearchIndex = -1;
}

void IntegerString :: Rewind(void) {
    mSearchIndex = -1; 
}

int IntegerString :: GetInt(void) {
    if(mSearchIndex < mLength-1)
        mSearchIndex++;
    
    if(mSearchIndex >= 0)  
        return(mData[mSearchIndex]);
    else {
        printf("ERROR: attempting to read an empty integer string\n");
        exit(0);
    }
}

void IntegerString :: Copy(IntegerString &intString) {
  
    Clear();
    Cat(intString);
    
}
