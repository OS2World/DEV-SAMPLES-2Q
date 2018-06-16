
Important:  This source code assumes that the definition of EXPENTRY
            has a code attribute of _loadds.  Some early toolkits
            (IBM's, anyway) did not define it this way.  Check your
            OS2DEF.H,  it should look like this:

                 #define EXPENTRY pascal far _loadds

Since this program uses an MLE (Multi Line Entry) conrol,  it must
be compiled with an IBM/MS 1.2+ toolkit,  and executed under OS/2
1.2+.

If you have MS C 6.0,  you don't need a toolkit to compile this
program,  provided you don't change an *.RC file.
