all: tutorial.html PolymorphismAndDataProducts.pdf tutorial.tar.gz

tutorial.tar.gz: tutorial.html
	tar zcvf $@ tutorial.html images/ slidy/

PolymorphismAndDataProducts.pdf: PolymorphismAndDataProducts.tex
	latexmk --pdf PolymorphismAndDataProducts

tutorial.html: tutorial.md makefile
	slideshow -t cet  $<

clean:
	latexmk -C  PolymorphismAndDataProducts
	rm -f *~

clobber: clean
	rm -f tutorial.html tutorial.pdf.html tutorial.css tutorial.pdf
	rm -rf ui/ slidy/ s6/ cet/ highlighter/ slippy/
