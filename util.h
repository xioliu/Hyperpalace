
#define NUMBER(VAL, TO)         (((VAL) + (TO)-1) / (TO))
#define ALIGN(VAL, TO)          ((((VAL) + (TO)-1) / (TO)) * TO)

#define STR(s)    #s
#define XSTR(s)   STR(s)
