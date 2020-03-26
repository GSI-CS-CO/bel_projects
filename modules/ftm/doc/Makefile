
all: FTN_dm_schedules

FTN_dm_schedules: 
	pdflatex $@.tex
#	makeindex $@
	makeglossaries $@
	pdflatex $@.tex

clean:
	rm *.aux *.acn *.acr *.alg *.glg *.gls *.ist *.log *.sg[0-9] *.sl[0-9] *.sy[0-9] || true
