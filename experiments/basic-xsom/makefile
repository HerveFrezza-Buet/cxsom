help:
	@echo
	@echo "Read the README.md file for instructions."
	@echo
	@echo "make cxsom-help                                    <-- help for common cxsom manipulations"
	@echo "make clean                                         <-- removes useless generated files"
	@echo
	@echo "Main mode"
	@echo "---------"
	@echo
	@echo "make main-figs                                     <-- generate figures for main rules"
	@echo "make send-main-rules TRACE=2500                    <-- send the rules (defining the trace size)."
	@echo "make feed-main SHAPE=banana                        <-- feed inputs."
	@echo
	@echo



# Adapt this path if needed. This adds pre-defined cxsom targets to your makefile.
include /usr/share/cxsom/cxsom-makefile


.PHONY: clean
clean:
	@rm -f *~ *.dot *.pdf

# This is how we compile rules.
xsom: xsom.cpp
	g++ -o xsom `pkg-config --cflags cxsom-builder` xsom.cpp `pkg-config --libs cxsom-builder`


.PHONY: main-figs
main-figs: xsom
	@./xsom graph main -- main 0
	@dot -Tpdf main-inits.dot -o main-inits.pdf
	@dot -Tpdf main-updates.dot -o main-updates.pdf
	@dot -Tpdf archi.dot -o archi.pdf
	@rm main-inits.dot main-updates.dot archi.dot
	@echo
	@echo
	@echo "main-inits.pdf, main-updates.pdf and archi.pdf have been generated."
	@echo

.PHONY: send-main-rules
send-main-rules:
	@./xsom send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- main $(TRACE)

.PHONY: feed-main
feed-main:
	@python3 feed-main.py `cat .cxsom-rootdir-config` `cat .cxsom-hostname-config` `cat .cxsom-port-config` ${SHAPE}
