
all: datamaster-tests

datamaster-tests:
	$(MAKE) -C TestPattern/
	pdflatex $@.tex
	pdflatex $@.tex

fel0069:
	pdflatex $@.tex
	pdflatex $@.tex

clean:
	rm *.aux *.acn *.acr *.alg *.glg *.gls *.ist *.log *.sg[0-9] *.sl[0-9] *.sy[0-9] || true
