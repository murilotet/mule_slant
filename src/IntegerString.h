#ifndef INTEGERSTRING_H
#define INTEGERSTRING_H

class IntegerString {
public:
    int *mData;
    int mLength;
    int mMaxLength;
    int mSearchIndex;
    IntegerString(int initialSize = 512);
    ~IntegerString(void);
    void Append(int symbol);
    void Cat(IntegerString &intString);
    void Clear(void);
    void Rewind(void);
    int GetInt(void);
    void Copy(IntegerString &intString);
};

#endif /* INTEGERSTRING_H */
