help:
	@echo
	@echo "Read the README.txt file for instructions."
	@echo "make cxsom-help                                    <-- help for common cxsom manipulations."
	@echo
	@echo "# Calibration"
	@echo
	@echo "make clear-calibration                             <-- clears calibration stuff."
	@echo "make calibration-setup GRID_SIDE=100               <-- sends calibration rules."
	@echo "make calibrate                                     <-- sets calibration variable content."
	@echo "make show-calibration                              <-- plots the calibration."
	@echo
	@echo "# Inputs mode"
	@echo 
	@echo "make inputs-setup IMAGE_SIDE=100                   <-- sets up inputs."
	@echo "make show-samples                                  <-- shows the image check/test samples"
	@echo "make clear-samples                                 <-- deletes the samples."
	@echo
	@echo "# Train mode"
	@echo
	@echo "make show-train-rules                              <-- Shows how training is computed."
	@echo "make train-setup SAVE_PERIOD=100 IMAGE_SIDE=100    <-- sets up training rules, saving weights periodically."
	@echo "make feed-train-inputs WALLTIME=30000              <-- feed inputs for training."
	@echo "make show-weights-history       	                  <-- displays the saved weights."
	@echo "make show-rgb-mapping                              <-- displays the color map encoding by the RGB map."
	@echo "make clear-training                                <-- clears training variables (training cannot be continued further)."
	@echo "make clear-saved-weights                           <-- Danger zone ! You will loose the training result."
	@echo
	@echo "# Check mode"
	@echo
	@echo "make show-check-rules                              <-- Shows how checking is computed."
	@echo "make check WEIGHTS_AT=300 IMAGE_SIDE=100           <-- sends testing rules (for saved weights at 300)."
	@echo "make show-checks                                   <-- Shows the (w, h, rgb) checks."
	@echo "make clear-checks                                  <-- clears checkings."
	@echo
	@echo "# Predict mode"
	@echo
	@echo "make show-predict-rules                            <-- Shows how prediction is computed."
	@echo "make predict WEIGHTS_AT=300 IMAGE_SIDE=100         <-- sends testing rules (for saved weights at 300)."
	@echo "                                                       (clear or restart the processor first)."
	@echo "make show-predictions                              <-- Shows the rgb predictions."
	@echo "make reconstruct-image                             <-- generates the reconstructed image."
	@echo "make clear-predictions                             <-- clears prediction."
	@echo
	@echo "# Making movies"
	@echo
	@echo "make movie-help"
	@echo
	@echo

movie-help:
	@echo
	@echo "For making movies, a processor has to be started."
	@echo
	@echo "make checkpredict-frames IMAGE_SIDE=100                   <-- generates the frames."
	@echo "make movie-generation MOVIE_FILE=expe-003-002-predict.ogg <-- generates the movie from the frames."
	@echo
	@echo "make weights-frames                                       <-- generates the frames."
	@echo "make movie-generation MOVIE_FILE=expe-003-002-weights.ogg <-- generates the movie from the frames."
	@echo
	@echo "--------"
	@echo
	@echo "make clear-frames"
	@echo "make one-frame WEIGHTS_AT=300 IMAGE_SIDE=100"
	@echo

# Adapt this path if needed.
include /usr/share/cxsom/cxsom-makefile

# This is how we compile rules.
xsom: xsom.cpp
	g++ -o xsom `pkg-config --cflags cxsom-builder` xsom.cpp `pkg-config --libs cxsom-builder`


.PHONY: clear-calibration
clear-calibration :
	@rm -rf `cat .cxsom-rootdir-config`/calibration

.PHONY: calibration-setup
calibration-setup: xsom
	@./xsom send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- calibration ${GRID_SIDE}

.PHONY: calibrate
calibrate:
	@python3 set-calibration.py `cat .cxsom-rootdir-config`
	@make --quiet cxsom-ping-processor

.PHONY: show-calibration
show-calibration:
	@python3 show-calibration.py `cat .cxsom-rootdir-config`



.PHONY: inputs-setup
inputs-setup: xsom
	@./xsom send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- input ${IMAGE_SIDE}
	@python3 build-eye-input.py `cat .cxsom-rootdir-config`
	@python3 build-img-coords.py `cat .cxsom-rootdir-config` img ${IMAGE_SIDE}
	@make --quiet cxsom-ping-processor

.PHONY: show-samples
show-samples:
	@python3 show-samples.py `cat .cxsom-rootdir-config` img w img h img rgb ${FRAME_ID}

.PHONY: clear-samples
clear-samples:
	@rm -rf `cat .cxsom-rootdir-config`/img

.PHONY: train-setup
train-setup:
	@@./xsom send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- train ${SAVE_PERIOD} ${IMAGE_SIDE}


.PHONY: show-train-rules
show-train-rules:
	@./xsom graph-full train -- train 100 100
	@dot -Tpdf train.dot -o train.pdf
	@sfdp -Tpdf -Goverlap=prism train-updates.dot -o train-updates.pdf
	@sfdp -Tpdf -Goverlap=prism train-inits.dot -o train-inits.pdf
	@rm -f train.dot train-updates.dot train-inits.dot
	@evince train.pdf train-updates.pdf train-inits.pdf &

.PHONY: feed-train-inputs
feed-train-inputs:
	@./xsom send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- walltime ${WALLTIME}
	@make --quiet cxsom-ping-processor

.PHONY: show-weights-history
show-weights-history:
	@python3 show-weights-history.py `cat .cxsom-rootdir-config`

.PHONY: show-rgb-mapping
show-rgb-mapping:
	@python3 show-rgb-mapping.py `cat .cxsom-rootdir-config`

.PHONY: clear-training
clear-training:
	@rm -rf `cat .cxsom-rootdir-config`/train-*

.PHONY: clear-saved-weights
clear-saved-weights:
	@rm -rf `cat .cxsom-rootdir-config`/saved


.PHONY: show-check-rules
show-check-rules:
	@./xsom graph check -- check 100 100
	@dot -Tpdf check.dot -o check.pdf
	@dot -Tpdf check-updates.dot -o check-updates.pdf
	@dot -Tpdf check-inits.dot -o check-inits.pdf
	@rm -f check.dot check-updates.dot check-inits.dot
	@evince check.pdf check-updates.pdf check-inits.pdf &

.PHONY: clear-checks
clear-checks:
	@rm -rf `cat .cxsom-rootdir-config`/check-*

.PHONY: check
check:
	@./xsom send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- check ${WEIGHTS_AT} ${IMAGE_SIDE}
	@make --quiet cxsom-ping-processor

.PHONY: show-checks
show-checks:
	@python3 show-samples.py `cat .cxsom-rootdir-config` check-out 'W/We-0' check-out 'H/We-0' check-out 'RGB/We-0' ${FRAME_ID}

.PHONY: show-predict-rules
show-predict-rules:
	@./xsom graph predict -- predict 100 100
	@dot -Tpdf predict.dot -o predict.pdf
	@dot -Tpdf predict-updates.dot -o predict-updates.pdf
	@dot -Tpdf predict-inits.dot -o predict-inits.pdf
	@rm -f predict.dot predict-updates.dot predict-inits.dot
	@evince predict.pdf predict-updates.pdf predict-inits.pdf &

.PHONY: clear-predictions
clear-predictions:
	@rm -rf `cat .cxsom-rootdir-config`/predict-*

.PHONY: predict
predict:
	@./xsom send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- predict ${WEIGHTS_AT} ${IMAGE_SIDE}
	@make --quiet cxsom-ping-processor

.PHONY: show-predictions
show-predictions:
	@python3 show-samples.py `cat .cxsom-rootdir-config` img w img h predict-out rgb ${FRAME_ID}


.PHONY: reconstruct-image
reconstruct-image:
	@python3 reconstruct.py `cat .cxsom-rootdir-config`


.PHONY: clear-frames
clear-frames:
	@rm -f frame-*.png

.PHONY: one-frame
one-frame:
	@make --quiet cxsom-kill-processor
	@make --quiet clear-checks clear-predictions
	@sleep 1
	@make --quiet cxsom-launch-processor
	@sleep 1
	@make --quiet check predict
	@sleep 2
	@python3 wait_stable.py `cat .cxsom-rootdir-config` check-out 'H/We-0'
	@python3 wait_stable.py `cat .cxsom-rootdir-config` predict-out rgb
	@python3 make-frame.py  `cat .cxsom-rootdir-config` ${WEIGHTS_AT}


.PHONY: checkpredictframes
checkpredictframes:
	@make --quiet clear-frames
	@python3 frame-factory.py `cat .cxsom-rootdir-config` ${IMAGE_SIDE}

.PHONY: movie-generation
movie-generation:
	@rm -f ${MOVIE_FILE}
	@ffmpeg -i frame-%06d.png -b:v 5M ${MOVIE_FILE}

.PHONY: weights-frames
weights-frames:
	@make --quiet clear-frames
	@python3 weights-frames.py `cat .cxsom-rootdir-config` 
