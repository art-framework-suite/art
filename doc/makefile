all: tutorial.html

tutorial.html: tutorial.md makefile
	slideshow -t cet  $<

clean:
	rm -f tutorial.html tutorial.pdf.html tutorial.css tutorial.pdf
	rm -rf ui/ slidy/ s6/ cet/ highlighter/ slippy/
	rm -f *~

