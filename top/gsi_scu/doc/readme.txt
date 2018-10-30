Generating DOXYGEN- documentation
=================================

Requirements:
-------------
I)
Documenttation-generator "doxygen" must be installed on your build-system.
http://www.doxygen.org

II)
Graphviz: Graph visualization software "dot". https://www.graphviz.org
must be installed on your build-system.


1)
Be sure that you are in the directory were this file (readme.txt) resides.

2)
Invoke the documentation-generator "doxygen"

$ doxygen

3)
Open in your preferred web-browser the following file:

./html/index.html


Appendix for Eclipse user:
---------------------------

Generating an Eclipse help plugin.
Set in the file "Doxyfile" the variable "GENERATE_ECLIPSEHELP" to "YES":

GENERATE_ECLIPSEHELP   = YES

Read additionally the instructions in "Doxyfile" surround this variable.
