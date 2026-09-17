
all: datamaster-tests fel0069 setup-tsl025-raspberryPi files_doc

datamaster-tests:
	$(MAKE) -C TestPattern/
	pdflatex $@.tex
	pdflatex $@.tex

fel0069:
	pdflatex $@.tex
	pdflatex $@.tex

files_doc:
	pdflatex $@.tex
	pdflatex $@.tex

setup-tsl025-raspberryPi:
	pdflatex $@.tex
	pdflatex $@.tex

clean:
	$(MAKE) -C TestPattern/ clean
	rm *.aux *.acn *.acr *.alg *.glg *.gls *.ist *.log *.sg[0-9] *.sl[0-9] *.sy[0-9] *.toc || true
