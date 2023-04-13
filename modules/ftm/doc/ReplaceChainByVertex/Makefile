PDFs = tsl020.pdf tsl020-compact.pdf chain1.pdf chain2x3.pdf chain3.pdf \
	cycle2x3.pdf cycle3.pdf parallel1.pdf parallel2.pdf parallel1x2.pdf \
	parallel2x2.pdf star4.pdf \
	chain3-compact.pdf chain2x3-compact.pdf cycle3-compact.pdf \
	cycle2x3-compact.pdf parallel1x2-compact.pdf parallel2x2-compact.pdf \
	tsl020-sis100.pdf tsl020-sis100-compact.pdf

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

