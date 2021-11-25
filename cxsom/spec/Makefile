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
	rm -f $(LATEX_JAM) *~ *cheatsheet.*

doc: commands.tex
	pdflatex $(DOC)
	pdflatex $(DOC)

commands.tex : notations.py
	python3 notations.py
	pdflatex commands-cheatsheet.tex



