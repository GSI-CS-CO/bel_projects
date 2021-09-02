.PHONY: clean

all: FTN_dm_schedules datamaster-tests

FTN_dm_schedules:
	pdflatex $@.tex
#	makeindex $@
	makeglossaries $@
	pdflatex $@.tex

datamaster-tests:
	$(MAKE) -C testdoc/

clean:
	rm *.aux *.acn *.acr *.alg *.glg *.gls *.ist *.log *.sg[0-9] *.sl[0-9] *.sy[0-9] || true
