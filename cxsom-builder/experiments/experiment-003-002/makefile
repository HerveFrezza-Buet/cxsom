help:
	@echo
	@echo
	@echo "make cxsom-help                                    <-- help for common cxsom manipulations."
	@echo
	@echo "make input-setup                                   <-- sets up the input generator (sets the image and sends the rules)."
	@echo "make send-input-rules                              <-- sets up the input rules only (in case of restart, for example)."
	@echo "make feed WALLTIME=500                             <-- feed inputs."
	@echo "make show-inputs                                   <-- shows current inputs."
	@echo
	@echo "make show-train-archi                              <-- displays the train architecture."
	@echo "make send-train-rules                              <-- sends training rules."

# Adapt this path if needed.
include /usr/share/cxsom/cxsom-makefile

.PHONY: send-input-rules
send-input-rules:
	@cxsom-builder-example-003-002-experiment send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- input

.PHONY: input-setup
input-setup:
	@make --quiet send-input-rules
	@python3 build-eye-input.py `cat .cxsom-rootdir-config`
	@cxsom-ping `cat .cxsom-hostname-config` `cat .cxsom-port-config`

.PHONY: feed
feed:
	@cxsom-builder-example-003-002-experiment send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- walltime ${WALLTIME}

.PHONY: show-inputs
show-inputs:
	@python3 show-inputs.py `cat .cxsom-rootdir-config`

.PHONY: show-train-archi
show-train-archi:
	@cxsom-builder-example-003-002-experiment graph-full train -- train
	@dot -Tpdf train.dot -o train.pdf
	@sfdp -Tpdf -Goverlap=prism train-updates.dot -o train-updates.pdf
	@sfdp -Tpdf -Goverlap=prism train-inits.dot -o train-inits.pdf
	@rm -f train.dot train-updates.dot train-inits.dot
	@evince train.pdf train-updates.pdf train-inits.pdf &

.PHONY: send-train-rules
send-train-rules:
	@cxsom-builder-example-003-002-experiment send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- train
	@cxsom-ping `cat .cxsom-hostname-config` `cat .cxsom-port-config`


