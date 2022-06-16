help:
	@echo
	@echo
	@echo "make cxsom-help                                    <-- help for common cxsom manipulations"
	@echo
	@echo "make main-figs                                     <-- generate figures for main rules"
	@echo "make relax-figs                                    <-- generate figures for expanded relaxation rules"
	@echo "make frozen-figs                                   <-- generate figures for frozen rules"
	@echo
	@echo "make send-main-rules                               <-- sends the main rules"
	@echo "make send-relax-rules TIMESTEP=<time>              <-- sends the relax rules"
	@echo "make declare-frozen-inputs                         <-- sends the rules for frozen inputs declaration"
	@echo "make send-frozen-rules TIMESTEP=<time>             <-- sends the frozen rules"
	@echo
	@echo "make feed-main-inputs                              <-- feeds X and Y"
	@echo "make feed-relax-inputs TIMESTEP=<time>...          <-- feeds the X,Y pair for relaxation at <time>"
	@echo "     ... THETA=<deg> [XBMU=<xbmu> YBMU=<ybmu>]"
	@echo "make feed-frozen-inputs                            <-- feeds the X,Y pairs for frozen tests"
	@echo
	@echo "make view-weights                                  <-- shows the weight evolution."
	@echo "make view-relaxation TIMESTEP=<time>               <-- shows the relaxation expansion."
	@echo "make view-frozen TIMESTEP=<time> [FRAME=<id>]      <-- shows the frozen analysis."
	@echo
	@echo "make movie                                         <-- compute a movie from frozen tests"
	@echo

# Adapt this path if needed.
include /usr/share/cxsom/cxsom-makefile

RULES         = cxsom-builder-example-003-001-experiment
RELAX_PREFIX  = zrlx
FROZEN_PREFIX = zfrz

.PHONY: main-figs
main-figs:
	@${RULES} graph main -- main
	@dot -Tpdf main-inits.dot -o main-inits.pdf
	@dot -Tpdf main-updates.dot -o main-updates.pdf
	@dot -Tpdf archi.dot -o archi.pdf
	@rm main-inits.dot main-updates.dot archi.dot
	@echo
	@echo
	@echo "main-inits.pdf, main-updates.pdf and archi.pdf have been generated."
	@echo

.PHONY: relax-figs
relax-figs:
	@${RULES} graph relax -- relax $(RELAX_PREFIX) 1234
	@dot -Tpdf relax-inits.dot -o relax-inits.pdf
	@dot -Tpdf relax-updates.dot -o relax-updates.pdf
	@rm relax-inits.dot relax-updates.dot 
	@echo
	@echo
	@echo "relax-inits.pdf and relax-updates.pdf have been generated."
	@echo

.PHONY: frozen-figs
frozen-figs:
	@${RULES} graph frozen -- frozen SHARED LOCAL 1234
	@dot -Tpdf frozen-inits.dot -o frozen-inits.pdf
	@dot -Tpdf frozen-updates.dot -o frozen-updates.pdf
	@rm frozen-inits.dot frozen-updates.dot 
	@echo
	@echo
	@echo "frozen-inits.pdf and frozen-updates.pdf have been generated."
	@echo

.PHONY: send-main-rules
send-main-rules:
	@${RULES} send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- main

.PHONY: feed-main-inputs
feed-main-inputs:
	@python3 feed-main.py `cat .cxsom-rootdir-config` `cat .cxsom-hostname-config` `cat .cxsom-port-config` 

.PHONY: view-weights
view-weights:
	@python3 view-weights.py `cat .cxsom-rootdir-config` 

.PHONY: send-relax-rules
send-relax-rules:
	@${RULES} send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- relax $(RELAX_PREFIX)-`printf %08d ${TIMESTEP}` ${TIMESTEP}

.PHONY: feed-relax-inputs
feed-relax-inputs:
	@python3 feed-relax.py `cat .cxsom-rootdir-config` `cat .cxsom-hostname-config` `cat .cxsom-port-config` $(RELAX_PREFIX)-`printf %08d ${TIMESTEP}` ${THETA} ${XBMU} ${YBMU}

.PHONY: view-relaxation
view-relaxation:
	@python3 view-relaxation.py `cat .cxsom-rootdir-config` ${TIMESTEP}

.PHONY: declare-frozen-inputs
declare-frozen-inputs:
	@${RULES} send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- test $(FROZEN_PREFIX) 

.PHONY: send-frozen-rules
send-frozen-rules:
	@${RULES} send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- frozen $(FROZEN_PREFIX) $(FROZEN_PREFIX)-`printf %08d ${TIMESTEP}` ${TIMESTEP}

.PHONY: feed-frozen-inputs
feed-frozen-inputs:
	@python3 feed-frozen.py `cat .cxsom-rootdir-config` `cat .cxsom-hostname-config` `cat .cxsom-port-config` $(FROZEN_PREFIX)

.PHONY: view-frozen
view-frozen:
	@python3 view-frozen.py `cat .cxsom-rootdir-config` ${TIMESTEP} ${FRAME}

./PHONY: movie
movie:
	@rm -f frame-*.png
	@python3 make-movie.py `cat .cxsom-rootdir-config`
