PDFs = tsl020.pdf tsl020-compact.pdf

all: ReplaceChainByVertex

ReplaceChainByVertex: $(PDFs)
	pdflatex $@.tex
	pdflatex $@.tex

clean:
	rm -f $(PDFs) *.aux *.acn *.acr *.alg *.glg *.gls *.ist *.log *.sg[0-9] *.sl[0-9] *.sy[0-9] || true


tsl020.pdf: tsl020.dot

tsl020-compact.pdf: tsl020-compact.dot
	./create-pdf2.sh $< dot

%.pdf: %.dot
	./create-pdf2.sh $<

