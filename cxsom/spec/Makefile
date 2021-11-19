DOC=cxsom

LATEX_JAM = $(DOC).toc $(DOC).log $(DOC).aux $(DOC).out

help:
	@echo "Compiling the documentation"
	@echo "---------------------------"
	@echo
	@echo "make clean     <-- cleans latex generated files"
	@echo "make doc       <-- compiles"
	@echo

clean:
	rm -f $(LATEX_JAM) *~

doc:
	pdflatex $(DOC)
	pdflatex $(DOC)



