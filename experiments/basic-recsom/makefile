help:
	@echo
	@echo "Read the README.md file for instructions."
	@echo
	@echo "make cxsom-help                                    <-- help for common cxsom manipulations"
	@echo
	@echo "make clean                                         <-- removes useless generated files"
	@echo "make recsom-figs                                   <-- generate figures for main rules"
	@echo "make send-rules                                    <-- send rules to the processor"
	@echo
	@echo "make feed SEQ=ABCDEF                               <-- feeds with a sequence."
	@echo "make show-weight-sequence                          <-- displays weights evolution (sequential view)."
	@echo "make show-weight-history                           <-- displays weights evolution (global view)."
	@echo



# Adapt this path if needed. This adds pre-defined cxsom targets to your makefile.
include /usr/share/cxsom/cxsom-makefile


.PHONY: clean
clean:
	@rm -f *~ *.dot *.pdf recsom

# This is how we compile rules.
recsom: recsom.cpp
	g++ -o recsom `pkg-config --cflags cxsom-builder` recsom.cpp `pkg-config --libs cxsom-builder`


.PHONY: recsom-figs
recsom-figs: recsom
	@./recsom graph recsom 
	@dot -Tpdf recsom-inits.dot -o recsom-inits.pdf
	@dot -Tpdf recsom-updates.dot -o recsom-updates.pdf
	@dot -Tpdf recsom-archi.dot -o recsom-archi.pdf
	@rm recsom-inits.dot recsom-updates.dot recsom-archi.dot
	@echo
	@echo
	@echo "recsom-inits.pdf, recsom-updates.pdf and recsom-archi.pdf have been generated."
	@echo

.PHONY: send-rules
send-rules: recsom
	@./recsom send `cat .cxsom-hostname-config` `cat .cxsom-port-config`

.PHONY: feed
feed:
	@python feed.py `cat .cxsom-rootdir-config` `cat .cxsom-hostname-config` `cat .cxsom-port-config` ${SEQ} 50

.PHONY: show-weights-sequence
show-weights-sequence:
	@python view.py `cat .cxsom-rootdir-config` &

.PHONY: show-weights-history
show-weights-history:
	@python history.py `cat .cxsom-rootdir-config` &
