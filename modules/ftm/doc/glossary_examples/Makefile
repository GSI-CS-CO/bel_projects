
all: customtable

customtable: 
	pdflatex $@.tex
	bibtex $@
	makeglossaries $@
	pdflatex $@.tex
	pdflatex $@.tex

clean:
	rm *.aux *.acn *.acr *.alg *.glg *.gls *.ist *.log *.sg[0-9] *.sl[0-9] *.sy[0-9] || true
