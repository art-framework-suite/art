all: tutorial.html PolymorphismAndDataProducts.pdf

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
