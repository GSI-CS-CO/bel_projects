all: scheduleCompare.pdf

scheduleCompare.pdf: scheduleCompare.tex attributeCompareTree.pdf
	pdflatex scheduleCompare.tex
	pdflatex scheduleCompare.tex

attributeCompareTree.pdf: attributeCompareTree.dot
	dot -Tpdf -o attributeCompareTree.pdf1 attributeCompareTree.dot
	gs -o attributeCompareTree.pdf -sDEVICE=pdfwrite -dColorConversionStrategy=/sRGB -dProcessColorModel=/DeviceRGB attributeCompareTree.pdf1
	rm attributeCompareTree.pdf1

